#include "TP_Odalid.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    TP_Odalid odalid;
    odalid.show();
    return app.exec();
}
