#include <QApplication>
#include "qupgrademainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QObject::connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));
    QUpgradeMainWindow w;
    w.show();

    return a.exec();
}
