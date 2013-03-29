#include "qextserialport.h"
#include "qextserialenumerator.h"
#include "dialog.h"
#include "ui_dialog.h"
#include <QtCore>
#include <QDebug>
#include <QFileDialog>
#include <QThread>

#include "qgc.h"
#include "qgcfirmwareupgradeworker.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    loading(false),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    //! [0]
    foreach (QextPortInfo info, QextSerialEnumerator::getPorts())
        ui->portBox->addItem(info.portName);
    //make sure user can input their own port name!
    ui->portBox->setEditable(true);

//    ui->led->turnOff();

    //! [1]
    PortSettings settings = {BAUD9600, DATA_8, PAR_NONE, STOP_1, FLOW_OFF, 10};
    port = new QextSerialPort(ui->portBox->currentText(), settings, QextSerialPort::Polling);
    //! [1]

    enumerator = new QextSerialEnumerator(this);
    enumerator->setUpNotifications();

    connect(ui->portBox, SIGNAL(editTextChanged(QString)), SLOT(onPortNameChanged(QString)));
    connect(ui->uploadButton, SIGNAL(clicked()), SLOT(onUploadButtonClicked()));

    connect(enumerator, SIGNAL(deviceDiscovered(QextPortInfo)), SLOT(onPortAddedOrRemoved()));
    connect(enumerator, SIGNAL(deviceRemoved(QextPortInfo)), SLOT(onPortAddedOrRemoved()));

    setWindowTitle(tr("QUpgrade Firmware Upload / Configuration Tool"));
}

Dialog::~Dialog()
{
    delete ui;
    delete port;
}

void Dialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void Dialog::onPortNameChanged(const QString & /*name*/)
{
    if (port->isOpen()) {
        port->close();
//        ui->led->turnOff();
    }
}

void Dialog::onUploadButtonClicked()
{
    if (loading) {
        worker->abortUpload();
        loading = false;
        ui->uploadButton->setText(tr("Select File and Upload"));
    } else {

        // Pick file
        QString fileName = QFileDialog::getOpenFileName(this,
                tr("Open Firmware File"), lastFilename, tr("Firmware Files (*.px4 *.bin)"));

        if (fileName.length() > 0) {
            // Got a filename, upload
            loading = true;
            ui->uploadButton->setText(tr("Cancel upload"));
            lastFilename = filename;

            worker = QGCFirmwareUpgradeWorker::putWorkerInThread(fileName);
            // Hook up status from worker to progress bar
            connect(worker, SIGNAL(upgradeProgressChanged(int)), ui->upgradeProgressBar, SLOT(setValue(int)));
            // Hook up text from worker to label
            connect(worker, SIGNAL(upgradeStatusChanged(QString)), ui->upgradeLog, SLOT(appendPlainText(QString)));
            // Hook up error handling
            connect(worker, SIGNAL(error(QString)), ui->upgradeLog, SLOT(appendPlainText(QString)));
            // Hook up status from worker to this class
            connect(worker, SIGNAL(loadFinished(bool)), this, SLOT(onLoadFinished(bool)));
        }
    }
}

void Dialog::onPortAddedOrRemoved()
{
    QString current = ui->portBox->currentText();

    ui->portBox->blockSignals(true);
    ui->portBox->clear();
    foreach (QextPortInfo info, QextSerialEnumerator::getPorts())
        ui->portBox->addItem(info.portName);

    ui->portBox->setCurrentIndex(ui->portBox->findText(current));

    ui->portBox->blockSignals(false);
}

void Dialog::onLoadFinished(bool success)
{
    loading = false;
    ui->uploadButton->setText(tr("Select File and Upload"));
}
