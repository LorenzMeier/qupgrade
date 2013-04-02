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
#include <QMessageBox>
#include <QSettings>

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
    connect(ui->flashButton, SIGNAL(clicked()), SLOT(onUploadButtonClicked()));
    connect(ui->selectFileButton, SIGNAL(clicked()), SLOT(onFileSelectRequested()));
    connect(ui->cancelButton, SIGNAL(clicked()), SLOT(onUploadButtonClicked()));

    connect(ui->webView->page(), SIGNAL(downloadRequested(const QNetworkRequest&)), this, SLOT(onDownloadRequested(const QNetworkRequest&)));
    ui->webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    connect(ui->webView->page(), SIGNAL(linkClicked(const QUrl&)), this, SLOT(onLinkClicked(const QUrl&)));

    connect(ui->prevButton, SIGNAL(clicked()), ui->webView, SLOT(back()));
    connect(ui->homeButton, SIGNAL(clicked()), this, SLOT(onHomeRequested()));

    connect(ui->advancedCheckBox, SIGNAL(clicked(bool)), this, SLOT(onToggleAdvancedMode(bool)));

    connect(enumerator, SIGNAL(deviceDiscovered(QextPortInfo)), SLOT(onPortAddedOrRemoved()));
    connect(enumerator, SIGNAL(deviceRemoved(QextPortInfo)), SLOT(onPortAddedOrRemoved()));

    setWindowTitle(tr("QUpgrade Firmware Upload / Configuration Tool"));

    // Adjust the size
    const int screenHeight = qMin(1000, QApplication::desktop()->height() - 100);

    resize(700, qMax(screenHeight, 550));

    // about:blank shouldn't be part of the history
    ui->webView->history()->clear();
    onHomeRequested();

    // load settings
    loadSettings();
}

Dialog::~Dialog()
{
    storeSettings();
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

void Dialog::onToggleAdvancedMode(bool enabled)
{
    ui->selectFileButton->setVisible(enabled);
    ui->flashButton->setVisible(enabled);
    ui->portBox->setVisible(enabled);
    ui->portLabel->setVisible(enabled);
    ui->boardIdLabel->setVisible(enabled);
    ui->boardIdSpinBox->setVisible(enabled);
    // Hide web view if advanced
    ui->webView->setVisible(!enabled);
    ui->homeButton->setVisible(!enabled);
    ui->prevButton->setVisible(!enabled);
}

void Dialog::loadSettings()
{
    QSettings set;
    lastFilename = set.value("LAST_FILENAME", lastFilename).toString();
    ui->advancedCheckBox->setChecked(set.value("ADVANCED_MODE", false).toBool());
    ui->boardIdSpinBox->setValue(set.value("BOARD_ID", 5).toInt());
    onToggleAdvancedMode(ui->advancedCheckBox->isChecked());
}

void Dialog::storeSettings()
{
    QSettings set;
    if (lastFilename != "")
        set.setValue("LAST_FILENAME", lastFilename);
    set.setValue("ADVANCED_MODE", ui->advancedCheckBox->isChecked());
    set.setValue("BOARD_ID", ui->boardIdSpinBox->value());
}

void Dialog::onHomeRequested()
{
    // Load start file into web view
    // for some reason QWebView has substantiall issues with local files.
    // Trick it by providing HTML directly.
//    QFile html(QCoreApplication::applicationDirPath()+"/files/html/index.html");
//    html.open(QIODevice::ReadOnly | QIODevice::Text);
//    QString str = html.readAll();
//    ui->webView->setHtml(str);
//    ui->webView->history()->clear();
    ui->webView->setUrl(QUrl::fromUserInput(QCoreApplication::applicationDirPath()+"/files/html/index.html"));
    ui->homeButton->setEnabled(false);
    ui->prevButton->setEnabled(false);
}

void Dialog::onLinkClicked(const QUrl &url)
{
    qDebug() << "LINK" << url.toString();

    QString firmwareFile = QFileInfo(url.toString()).fileName();

    // If not a firmware file, ignore
    if (!(firmwareFile.endsWith(".px4") || firmwareFile.endsWith(".bin"))) {
        ui->webView->load(url);
        ui->homeButton->setEnabled(true);
        ui->prevButton->setEnabled(true);
        return;
    }

    qDebug() << "FW FILE:" << firmwareFile;

    QString filename;

    // If a IO firmware file, open save as Dialog
    if (firmwareFile.endsWith(".bin") && firmwareFile.contains("px4io")) {
        QString path = QString(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
        qDebug() << path;
        filename = QFileDialog::getExistingDirectory(this, tr("Select folder (microSD Card)"),
                                                path);
        filename.append("/" + firmwareFile);
    } else {

        // Make sure the user doesn't screw up flashing
        QMessageBox msgBox;
        msgBox.setText("Please unplug your PX4 board now");
        msgBox.exec();

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

    if (filename == "") {
        return;
    }

    // Else, flash the firmware
    lastFilename = filename;

    // Pattern matched, abort current QWebView load
    ui->webView->stop();

    qDebug() << "LASTFILENAME" << lastFilename;

    ui->upgradeLog->appendHtml(tr("Downloading firmware file <a href=\"%1\">%1</a>").arg(url.toString()));

    QNetworkRequest newRequest(url);
    newRequest.setUrl(url);
    newRequest.setAttribute(QNetworkRequest::User, filename);

    QNetworkAccessManager *networkManager = ui->webView->page()->networkAccessManager();
    QNetworkReply *reply = networkManager->get(newRequest);
    connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(onDownloadProgress(qint64, qint64)));
    connect(reply, SIGNAL(finished()), this, SLOT(onDownloadFinished()));
//    connect(networkManager, SIGNAL(finished(QNetworkReply*)),
//                SLOT(onDownloadFinished(QNetworkReply*)));
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

    QNetworkReply *reply = qobject_cast<QNetworkReply*>(QObject::sender());

    if (!reply) {
        // bail out, nothing to do
        return;
    }

    if (loading) {
        worker->abortUpload();
        loading = false;
        ui->flashButton->setText(tr("Flash"));
        ui->cancelButton->setEnabled(false);
    } else {

        // Reset progress
        ui->upgradeProgressBar->setValue(0);

        // Pick file
        QString fileName = lastFilename;

        qDebug() << "Handling filename:" << fileName;

        // Store file in download location
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly)) {
            fprintf(stderr, "Could not open %s for writing: %s\n",
                    qPrintable(fileName),
                    qPrintable(file.errorString()));
            return;
        }

        file.write(reply->readAll());


        if (lastFilename.contains("px4io")) {
            // Done, bail out
            return;
        }

        if (fileName.length() > 0) {
            // Got a filename, upload
            loading = true;
            ui->flashButton->setText(tr("Cancel"));
            ui->cancelButton->setEnabled(true);
            lastFilename = fileName;

            worker = QGCFirmwareUpgradeWorker::putWorkerInThread(fileName);
            // Hook up status from worker to progress bar
            connect(worker, SIGNAL(upgradeProgressChanged(int)), ui->upgradeProgressBar, SLOT(setValue(int)));
            // Hook up text from worker to label
            connect(worker, SIGNAL(upgradeStatusChanged(QString)), ui->upgradeLog, SLOT(appendPlainText(QString)));
            // Hook up error handling
            //connect(worker, SIGNAL(error(QString)), ui->upgradeLog, SLOT(appendPlainText(QString)));
            // Hook up status from worker to this class
            connect(worker, SIGNAL(loadFinished(bool)), this, SLOT(onLoadFinished(bool)));

            // Make sure user gets the board going now
            QMessageBox msgBox;
            msgBox.setText("Please connect your PX4 board now");
            msgBox.exec();
        }
    }
}

void Dialog::onFileSelectRequested()
{
    if (loading) {
        worker->abortUpload();
        loading = false;
        ui->flashButton->setText(tr("Flash"));
    }

    // Pick file
    QString fileName = QFileDialog::getOpenFileName(this,
                            tr("Open Firmware File"), lastFilename, tr("Firmware Files (*.px4 *.bin)"));

    if (fileName != "")
        lastFilename = fileName;
}

void Dialog::onUploadButtonClicked()
{
    if (loading) {
        worker->abortUpload();
        loading = false;
        ui->flashButton->setText(tr("Flash"));
        ui->cancelButton->setEnabled(false);
    } else {

        if (lastFilename.length() > 0) {
            // Got a filename, upload
            loading = true;
            ui->flashButton->setText(tr("Cancel"));
            ui->cancelButton->setEnabled(true);

            worker = QGCFirmwareUpgradeWorker::putWorkerInThread(lastFilename);

            // Set board ID for worker
            if (ui->boardIdSpinBox->isVisible()) {
                worker->setBoardId(ui->boardIdSpinBox->value());
            }

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
    ui->flashButton->setText(tr("Flash"));

    if (success) {
        ui->upgradeLog->appendPlainText(tr("Upload succeeded."));
    } else {
        ui->upgradeLog->appendPlainText(tr("Upload aborted and failed."));
    }
}

void Dialog::onDownloadProgress(qint64 curr, qint64 total)
{
    // Take care of cases where 0 / 0 is emitted as error return value
    if (total > 0)
        ui->upgradeProgressBar->setValue((curr*100) / total);
}
