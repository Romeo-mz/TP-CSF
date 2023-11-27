#include "TP_Odalid.h"
#include "ui_TP_Odalid.h"
#include <QMessageBox>
#include "MfErrNo.h"
#include "Sw_Device.h"
#include "Sw_Mf_Classic.h"
#include "Tools.h"
#include "TypeDefs.h"
#include "Librairie.h"
#include <QDebug>
#include "TypeDefs.h"
#include "Hardware.h"
#include "Sw_ISO14443A-3.h"

TP_Odalid::TP_Odalid(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::TP_Odalid)
{
    ui->setupUi(this);
    connect(ui->connectButton, SIGNAL(clicked(bool)), this, SLOT(onConnectClicked()));
    connect(ui->disconnectButton, SIGNAL(clicked(bool)), this, SLOT(onDisconnectClicked()));
    connect(ui->leaveButton, SIGNAL(clicked(bool)), this, SLOT(onQuitClicked()));
    connect(ui->readButton, SIGNAL(clicked(bool)), this, SLOT(onReadClicked()));
    connect(ui->updateButton, SIGNAL(clicked(bool)), this, SLOT(onUpdateClicked()));
    connect(ui->loadPMButton, SIGNAL(clicked(bool)), this, SLOT(onLoadClicked()));
    connect(ui->payButton, SIGNAL(clicked(bool)), this, SLOT(onPayButtonClicked()));
}
ReaderName Reader;

TP_Odalid::~TP_Odalid()
{
    delete ui;
}

void TP_Odalid::onConnectClicked()
{
    Reader.Type = ReaderCDC;
    Reader.device = 6; // Update the correct port

    int16_t status = OpenCOM(&Reader);

    if (status == MI_OK) {
        status = LEDBuzzer(&Reader, LED_RED_ON);

        status = Version(&Reader);
        QString versionStr = QString("Version : ") + QString::fromStdString(Reader.version);
        ui->version->setText(versionStr);
        ui->version->update();

        status = RF_Power_Control(&Reader, TRUE, 0);
    } else {
        QMessageBox::critical(this, "Error", "Reader undefined");
    }
}
void TP_Odalid::onQuitClicked()
{
    RF_Power_Control(&Reader, FALSE, 0);
    LEDBuzzer(&Reader, LED_OFF);
    CloseCOM(&Reader);
    qApp->quit();
}

void TP_Odalid::onReadClicked()
{
    uint8_t data[240] = {0};
    uint32_t creditValue = 0;

    uint8_t atq[2];
    uint8_t sak[1];
    uint8_t uid[12];
    uint16_t uid_len = 12;

    int16_t status = ISO14443_3_A_PollCard(&Reader, atq, sak, uid, &uid_len);

    uid_len = 0;

    if (status != MI_OK){
        qDebug()<<"Load Key [FAILED]\n";
        return;
    }
    else{

        status = Mf_Classic_Read_Block(&Reader, TRUE, 10, data, AuthKeyA, 2);
        ui->nameTextEdit->setText((char*)data);
        ui->nameTextEdit->update();

        status = Mf_Classic_Read_Block(&Reader, TRUE, 9, data, AuthKeyA, 2);
        ui->lastnameTextEdit->setText((char*)data);
        ui->lastnameTextEdit->update();

        status = Mf_Classic_Read_Value(&Reader, TRUE, 14, &creditValue, AuthKeyA, 3);
        ui->unitTextEdit->setText(QString::number(creditValue));
        ui->unitTextEdit->update();

        if(status != MI_OK) {
            QMessageBox messageBox;
            messageBox.critical(0, "Error", "Can't Read.");
        }
    }

    LEDBuzzer(&Reader, BUZZER_ON + LED_YELLOW_ON + LED_RED_ON + LED_GREEN_ON);
    DELAYS_MS(10);
    LEDBuzzer(&Reader, BUZZER_OFF + LED_YELLOW_OFF + LED_GREEN_OFF + LED_RED_ON);
}

void TP_Odalid::onUpdateClicked()
{
    int16_t status = 0;
    char name[16];
    char lastname[16];

    uint8_t atq[2];
    uint8_t sak[1];
    uint8_t uid[12];
    uint16_t uid_len = 12;

    status = ISO14443_3_A_PollCard(&Reader, atq, sak, uid, &uid_len);

    if(status != MI_OK){
        qDebug()<<"CAN'T WRITE\n";
        return;
    }
    else {
        //UPDATE NAME
        sprintf(name, ui->nameTextEdit->toPlainText().toUtf8().data(), 16);
        status = Mf_Classic_Write_Block(&Reader, TRUE, 10, (uint8_t*)name, AuthKeyB, 2);

        //UPDATE LASTNAME
        sprintf(lastname, ui->lastnameTextEdit->toPlainText().toUtf8().data(), 16);
        status = Mf_Classic_Write_Block(&Reader, TRUE, 9, (uint8_t*)lastname, AuthKeyB, 2);
    }

    LEDBuzzer(&Reader, LED_YELLOW_ON + LED_RED_ON + LED_GREEN_ON);
    DELAYS_MS(10);
    LEDBuzzer(&Reader, LED_YELLOW_OFF + LED_GREEN_OFF + LED_RED_ON);
}

void TP_Odalid::onLoadClicked()
{
    int16_t status = 0;
    uint8_t atq[2];
    uint8_t sak[1];
    uint8_t uid[12];
    uint16_t uid_len = 12;
    uint32_t creditValue = 0;


    status = ISO14443_3_A_PollCard(&Reader, atq, sak, uid, &uid_len);
    status = Mf_Classic_Read_Value(&Reader, TRUE, 14, &creditValue, AuthKeyA, 3);

    if(status != MI_OK){
        qDebug()<<"CAN'T UPDATE WALLET\n";
        return;
    }

    else {
        // GET THE AMOUNT TO INCREASE
        uint32_t newAmount = creditValue + (uint32_t)ui->increaseSpinBox->value();
        status = Mf_Classic_Write_Value(&Reader, TRUE, 14, newAmount, AuthKeyB, 3);
        ui->unitTextEdit->setText(QString::number(newAmount));
        ui->unitTextEdit->update();
    }

    LEDBuzzer(&Reader, LED_YELLOW_ON + LED_RED_ON + LED_GREEN_ON);
    DELAYS_MS(10);
    LEDBuzzer(&Reader, LED_YELLOW_OFF + LED_GREEN_OFF + LED_RED_ON);
}

void TP_Odalid::onDisconnectClicked()
{
    int16_t status = MI_OK;

    // Turn off RF field
    status = RF_Power_Control(&Reader, FALSE, 0);
    if (status != MI_OK) {
        qDebug() << "Error turning off RF field";
    }

    // Turn off LED
    status = LEDBuzzer(&Reader, LED_OFF);
    if (status != MI_OK) {
        qDebug() << "Error turning off LED";
    }

    // Close communication port
    status = CloseCOM(&Reader);
    if (status != MI_OK) {
        qDebug() << "Error closing communication port";
    } else {
        qDebug() << "Disconnected";
        ui->nameTextEdit->clear();
        ui->lastnameTextEdit->clear();
        ui->unitTextEdit->clear();
        ui->version->clear();
    }

}


void TP_Odalid::onPayButtonClicked()
{
    int16_t status = 0;
    uint8_t atq[2];
    uint8_t sak[1];
    uint8_t uid[12];
    uint16_t uid_len = 12;
    uint32_t creditValue = 0;

    status = ISO14443_3_A_PollCard(&Reader, atq, sak, uid, &uid_len);
    status = Mf_Classic_Read_Value(&Reader, TRUE, 14, &creditValue, AuthKeyA, 3);

    if (status != MI_OK) {
        qDebug() << "CAN'T UPDATE WALLET\n";
        return;
    } else {
        // GET THE AMOUNT TO DECREASE
        uint32_t decreaseAmount = (uint32_t)ui->decreaseSpinBox->value();
        if (creditValue < decreaseAmount) {
            qDebug() << "INSUFFICIENT FUNDS\n";
            return;
        }
        uint32_t newAmount = creditValue - decreaseAmount;
        status = Mf_Classic_Write_Value(&Reader, TRUE, 14, newAmount, AuthKeyB, 3);
        ui->unitTextEdit->setText(QString::number(newAmount));
        ui->unitTextEdit->update();
    }

    LEDBuzzer(&Reader, LED_YELLOW_ON + LED_RED_ON + LED_GREEN_ON);
    DELAYS_MS(10);
    LEDBuzzer(&Reader, LED_YELLOW_OFF + LED_GREEN_OFF + LED_RED_ON);
}
