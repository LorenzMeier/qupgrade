#include "qextserialport.h"
#include "qextserialenumerator.h"
#include "dialog.h"
#include "ui_dialog.h"
#include <QtCore>
#include <QDebug>
#include <QFileDialog>
#include <QThread>
#include <QNetworkRequest>
#include <QNetworkReply>

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

    connect(ui->webView->page(), SIGNAL(downloadRequested(const QNetworkRequest&)), this, SLOT(onDownloadRequested(const QNetworkRequest&)));

    connect(enumerator, SIGNAL(deviceDiscovered(QextPortInfo)), SLOT(onPortAddedOrRemoved()));
    connect(enumerator, SIGNAL(deviceRemoved(QextPortInfo)), SLOT(onPortAddedOrRemoved()));

    setWindowTitle(tr("QUpgrade Firmware Upload / Configuration Tool"));

    // Load start file into web view
    // for some reason QWebView has substantiall issues with local files.
    // Trick it by providing HTML directly.
    QFile html(QCoreApplication::applicationDirPath()+"/files/index.html");
    html.open(QIODevice::ReadOnly | QIODevice::Text);
    QString str = html.readAll();
    ui->webView->setHtml(str);
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

void Dialog::onDownloadRequested(const QNetworkRequest &request)
{
    qDebug() << "Download Request";
    QString defaultFileName = QFileInfo(request.url().toString()).fileName();
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), defaultFileName);
    if (fileName.isEmpty()) return;

    QNetworkRequest newRequest = request;
    newRequest.setAttribute(QNetworkRequest::User, fileName);

    ui->upgradeLog->appendHtml(tr("Downloading firmware file <a href=\"%1\">%1</a>").arg(request.url().toString()));

    QNetworkAccessManager *networkManager = ui->webView->page()->networkAccessManager();
    QNetworkReply *reply = networkManager->get(newRequest);
    connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(onDownloadProgress(qint64, qint64)));
    //connect( reply, SIGNAL(finished()), this, SLOT(downloadIssueFinished()));
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
            lastFilename = fileName;

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

void Dialog::onDownloadProgress(qint64 curr, qint64 total)
{
    ui->upgradeProgressBar->setValue((curr*100) / total);
}
