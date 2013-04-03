#include "qupgrademainwindow.h"
#include "ui_qupgrademainwindow.h"

#include <QSettings>

#include "dialog.h"

QUpgradeMainWindow::QUpgradeMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::QUpgradeMainWindow)
{
    ui->setupUi(this);
    dialog = new Dialog(this);
    setCentralWidget(dialog);

    // Restore the widget positions and size
    QSettings set;
    if (set.contains("WINDOW_GEOMETRY"))
    {
        restoreState(set.value("WINDOW_GEOMETRY").toByteArray());
    }
    statusBar()->hide();
    readPositionSettings();
}

void QUpgradeMainWindow::writePositionSettings()
{
    QSettings qsettings( "PX4", "QUpgrade" );

    qsettings.beginGroup( "mainwindow" );

    qsettings.setValue( "geometry", saveGeometry() );
    qsettings.setValue( "savestate", saveState() );
    qsettings.setValue( "maximized", isMaximized() );
    if ( !isMaximized() ) {
        qsettings.setValue( "pos", pos() );
        qsettings.setValue( "size", size() );
    }

    qsettings.endGroup();
}

void QUpgradeMainWindow::readPositionSettings()
{
    QSettings qsettings( "PX4", "QUpgrade" );

    qsettings.beginGroup( "mainwindow" );

    restoreGeometry(qsettings.value( "geometry", saveGeometry() ).toByteArray());
    restoreState(qsettings.value( "savestate", saveState() ).toByteArray());
    move(qsettings.value( "pos", pos() ).toPoint());
    resize(qsettings.value( "size", size() ).toSize());
    if ( qsettings.value( "maximized", isMaximized() ).toBool() )
        showMaximized();

    qsettings.endGroup();
}

QUpgradeMainWindow::~QUpgradeMainWindow()
{
    delete ui;
    delete dialog;
}

void QUpgradeMainWindow::closeEvent(QCloseEvent* event)
{
    Q_UNUSED(event);
    writePositionSettings();
}
