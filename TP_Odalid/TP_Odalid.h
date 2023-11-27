#ifndef TP_ODALID_H
#define TP_ODALID_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class TP_Odalid;
}
QT_END_NAMESPACE

class TP_Odalid : public QMainWindow
{
    Q_OBJECT

public:
    explicit TP_Odalid(QWidget *parent = nullptr);
    ~TP_Odalid();

private slots:
    void onConnectClicked();
    void onDisconnectClicked();
    void onQuitClicked();
    void onReadClicked();
    void onUpdateClicked();
    void onLoadClicked();
    void onPayButtonClicked();

private:
    Ui::TP_Odalid *ui;

    uint8_t Sector2ReadKEY[6] = {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5}; // Key A
    uint8_t Sector2WriteKEY[6] = {0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5}; // Key B

    uint8_t Sector3ReadKEY[6] = {0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5}; // Increments & decrements key
    uint8_t Sector3WriteKEY[6] = {0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5}; // Writes & increments key

    int16_t readCard(int block, char *data, int keyIndex);
    int16_t readCardInt(int block, uint8_t *data, int keyIndex);
};

#endif // TP_ODALID_H
