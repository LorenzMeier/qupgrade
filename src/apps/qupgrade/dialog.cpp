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
#include <QStandardPaths>
#include <QFileInfo>
#include <QApplication>
#include <QDesktopWidget>
#include <QTimer>
#include <QWebHistory>

#include "qgc.h"
#include "qgcfirmwareupgradeworker.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    loading(false),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    foreach (QextPortInfo info, QextSerialEnumerator::getPorts())
        ui->portBox->addItem(info.portName);
    //make sure user can input their own port name!
    ui->portBox->setEditable(true);

//    ui->led->turnOff();

    PortSettings settings = {BAUD9600, DATA_8, PAR_NONE, STOP_1, FLOW_OFF, 10};
    port = new QextSerialPort(ui->portBox->currentText(), settings, QextSerialPort::Polling);

    enumerator = new QextSerialEnumerator(this);
    enumerator->setUpNotifications();

    connect(ui->portBox, SIGNAL(editTextChanged(QString)), SLOT(onPortNameChanged(QString)));
    connect(ui->uploadButton, SIGNAL(clicked()), SLOT(onUploadButtonClicked()));

    connect(ui->webView->page(), SIGNAL(downloadRequested(const QNetworkRequest&)), this, SLOT(onDownloadRequested(const QNetworkRequest&)));
    ui->webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    connect(ui->webView->page(), SIGNAL(linkClicked(const QUrl&)), this, SLOT(onLinkClicked(const QUrl&)));

    connect(ui->prevButton, SIGNAL(clicked()), ui->webView, SLOT(back()));
    connect(ui->forwardButton, SIGNAL(clicked()), ui->webView, SLOT(forward()));
    connect(ui->homeButton, SIGNAL(clicked()), this, SLOT(onHomeRequested()));

    connect(enumerator, SIGNAL(deviceDiscovered(QextPortInfo)), SLOT(onPortAddedOrRemoved()));
    connect(enumerator, SIGNAL(deviceRemoved(QextPortInfo)), SLOT(onPortAddedOrRemoved()));

    setWindowTitle(tr("QUpgrade Firmware Upload / Configuration Tool"));

    // Adjust the size
    const int screenWidth = QApplication::desktop()->width();
    const int screenHeight = QApplication::desktop()->height();

    if (screenWidth < 1200)
    {
        showFullScreen();
    }
    else
    {
        resize(700, qMin(screenHeight, 750));
    }

    QTimer::singleShot(500, this, SLOT(onHomeRequested()));
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

void Dialog::onHomeRequested()
{
    // Load start file into web view
    // for some reason QWebView has substantiall issues with local files.
    // Trick it by providing HTML directly.
    QFile html(QCoreApplication::applicationDirPath()+"/files/html/index.html");
    html.open(QIODevice::ReadOnly | QIODevice::Text);
    QString str = html.readAll();
    ui->webView->setHtml(str);
    ui->webView->history()->clear();
    ui->webView->setUrl(QUrl::fromUserInput(QCoreApplication::applicationDirPath()+"/files/html/index.html"));
}

void Dialog::onLinkClicked(const QUrl &url)
{
    qDebug() << "LINK" << url.toString();

    QString firmwareFile = QFileInfo(url.toString()).fileName();

    // If not a firmware file, ignore
    if (!(firmwareFile.endsWith(".px4") || firmwareFile.endsWith(".bin"))) {
        ui->webView->load(url);
        return;
    }

    qDebug() << "FW FILE:" << firmwareFile;

    QString filename;

    // If a IO firmware file, open save as Dialog
    if (firmwareFile.endsWith(".bin") && firmwareFile.contains("px4io")) {
        filename = QFileDialog::getSaveFileName(this, tr("Save PX4IO Firmware File to microSD Card"),
                                                QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/" + firmwareFile,
                                    tr("PX4IO Firmware (*.bin)"));
    } else {
        filename = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);

        if (filename.isEmpty()) {
            qDebug() << "Falling back to temp dir";
            QString filename = QStandardPaths::writableLocation(QStandardPaths::TempLocation);

            // If still empty, bail out
            if (filename.isEmpty())
                ui->upgradeLog->appendHtml(tr("FAILED storing firmware to downloads or temp directory. Harddisk not writable."));
                return;
        }

        filename += "/" + firmwareFile;
    }

    // Else, flash the firmware
    lastFilename = filename;

    // Pattern matched, abort current QWebView load
    ui->webView->stop();

    qDebug() << "LASTFILENAME" << lastFilename;

    ui->upgradeLog->appendHtml(tr("Downloading firmware file <a href=\"%1\">%1</a>").arg(url.toString()));

    QNetworkRequest newRequest;
    newRequest.setUrl(url);
    newRequest.setAttribute(QNetworkRequest::User, filename);

    QNetworkAccessManager *networkManager = ui->webView->page()->networkAccessManager();
    QNetworkReply *reply = networkManager->get(newRequest);
    connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(onDownloadProgress(qint64, qint64)));
    connect(reply, SIGNAL(finished()), this, SLOT(onDownloadFinished()));
}

void Dialog::onDownloadRequested(const QNetworkRequest &request)
{
    onLinkClicked(request.url());
}

void Dialog::onPortNameChanged(const QString & /*name*/)
{
    if (port->isOpen()) {
        port->close();
//        ui->led->turnOff();
    }
}

void Dialog::onDownloadFinished()
{
    qDebug() << "DOWNLOAD FINISHED";
    if (loading) {
        worker->abortUpload();
        loading = false;
        ui->uploadButton->setText(tr("Select File and Upload"));
    } else {

        // Reset progress
        ui->upgradeProgressBar->setValue(0);

        // Pick file
        QString fileName = lastFilename;

        qDebug() << "Handling filename:" << fileName;

        if (lastFilename.contains("px4io")) {
            // Done, bail out
            return;
        }

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
    // Take care of cases where 0 / 0 is emitted as error return value
    if (total > 0)
        ui->upgradeProgressBar->setValue((curr*100) / total);
}
