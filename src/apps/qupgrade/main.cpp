#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include "qupgrademainwindow.h"

int main(int argc, char *argv[])
{
#ifdef Q_OS_WINDOWS
    // Fixup plugin search path
    QStringList paths;
    paths << QCoreApplication::applicationDirPath();
    QCoreApplication::setLibraryPaths(paths);
#endif

    QApplication a(argc, argv);
    QObject::connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));
    QUpgradeMainWindow w;
    w.show();

    return a.exec();
}
