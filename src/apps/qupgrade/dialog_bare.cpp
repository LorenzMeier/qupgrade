#include "qextserialport.h"
#include "qextserialenumerator.h"
#include "dialog_bare.h"
#include "ui_dialog_bare.h"
#include <QtCore>
#include <QDebug>
#include <QFileDialog>
#include <QThread>
#include <QNetworkRequest>
#include <QNetworkReply>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QStandardPaths>
#else
#include <QDesktopServices>
#endif
#include <QFileInfo>
#include <QApplication>
#include <QDesktopWidget>
#include <QTimer>
#include <QWebHistory>
#include <QWebSettings>
#include <QMessageBox>
#include <QSettings>

#include <QGC.h>
#include "qgcfirmwareupgradeworker.h"

DialogBare::DialogBare(QWidget *parent) :
    QWidget(parent),
    loading(false),
    ui(new Ui::DialogBare)
{
    ui->setupUi(this);

    foreach (QextPortInfo info, QextSerialEnumerator::getPorts())
        if (!info.portName.isEmpty())
            ui->portBox->addItem(info.portName);
    //make sure user can input their own port name!
    ui->portBox->setEditable(true);

//    ui->led->turnOff();

    PortSettings settings = {BAUD9600, DATA_8, PAR_NONE, STOP_1, FLOW_OFF, 10};
    port = new QextSerialPort(ui->portBox->currentText(), settings, QextSerialPort::Polling);

    enumerator = new QextSerialEnumerator(this);
    enumerator->setUpNotifications();

    ui->boardComboBox->addItem("PX4FMU v1.6+", 5);
    ui->boardComboBox->addItem("PX4FLOW v1.1+", 6);
    ui->boardComboBox->addItem("PX4IO v1.3+", 7);
    ui->boardComboBox->addItem("PX4 board #8", 8);
    ui->boardComboBox->addItem("PX4 board #9", 9);
    ui->boardComboBox->addItem("PX4 board #10", 10);
    ui->boardComboBox->addItem("PX4 board #11", 11);

    connect(ui->portBox, SIGNAL(editTextChanged(QString)), SLOT(onPortNameChanged(QString)));
    connect(ui->flashButton, SIGNAL(clicked()), SLOT(onUploadButtonClicked()));
    connect(ui->scanButton, SIGNAL(clicked()), SLOT(onDetect()));
    connect(ui->selectFileButton, SIGNAL(clicked()), SLOT(onFileSelectRequested()));
    connect(ui->cancelButton, SIGNAL(clicked()), SLOT(onCancelButtonClicked()));

    connect(ui->advancedCheckBox, SIGNAL(clicked(bool)), this, SLOT(onToggleAdvancedMode(bool)));

    ui->versionComboBox->hide();

    connect(enumerator, SIGNAL(deviceDiscovered(QextPortInfo)), SLOT(onPortAddedOrRemoved()));
    connect(enumerator, SIGNAL(deviceRemoved(QextPortInfo)), SLOT(onPortAddedOrRemoved()));

    setWindowTitle(tr("QUpgrade Firmware Upload / Configuration Tool"));

    // Adjust the size
    const int screenHeight = qMin(1000, QApplication::desktop()->height() - 100);

    resize(700, qMax(screenHeight, 550));

    // load settings
    loadSettings();

    // Set up initial state
    if (!lastFilename.isEmpty()) {
        ui->flashButton->setEnabled(true);
    } else {
        ui->flashButton->setEnabled(false);
    }

    ui->advancedCheckBox->setCheckState(Qt::Checked);
}

DialogBare::~DialogBare()
{
    storeSettings();
    delete ui;
    delete port;
}

void DialogBare::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void DialogBare::onToggleAdvancedMode(bool enabled)
{
    ui->selectFileButton->setVisible(enabled);
    ui->portBox->setVisible(enabled);
    ui->portLabel->setVisible(enabled);
    ui->boardIdLabel->setVisible(enabled);
    ui->boardComboBox->setVisible(enabled);
    ui->flashButton->setVisible(enabled);
    ui->versionComboBox->setVisible(!enabled);
    ui->scanButton->setVisible(!enabled);
}

void DialogBare::loadSettings()
{
    QSettings set;
    lastFilename = set.value("LAST_FILENAME", lastFilename).toString();
    ui->advancedCheckBox->setChecked(set.value("ADVANCED_MODE", false).toBool());

    int boardIndex = ui->boardComboBox->findData(set.value("BOARD_ID", 5));
    if (boardIndex >= 0)
        ui->boardComboBox->setCurrentIndex(boardIndex);

    if (set.value("PORT_NAME", "").toString().trimmed().length() > 0) {
        int portIndex = ui->portBox->findText(set.value("PORT_NAME", "").toString());
        if (portIndex >= 0) {
            ui->portBox->setCurrentIndex(portIndex);
        } else {
            qDebug() << "could not find port" << set.value("PORT_NAME", "");
        }
    }

    onToggleAdvancedMode(ui->advancedCheckBox->isChecked());

    // Check if in advanced mode
    if (!lastFilename.isEmpty() && ui->advancedCheckBox->isChecked()) {
        ui->upgradeLog->appendPlainText(tr("Pre-selected file %1\nfor flashing (click 'Flash' to upgrade)").arg(lastFilename));

        updateBoardId(lastFilename);
    }
}

void DialogBare::storeSettings()
{
    QSettings set;
    if (lastFilename != "")
        set.setValue("LAST_FILENAME", lastFilename);
    set.setValue("ADVANCED_MODE", ui->advancedCheckBox->isChecked());
    set.setValue("BOARD_ID", ui->boardComboBox->itemData(ui->boardComboBox->currentIndex()));
    set.setValue("PORT_NAME", ui->portBox->currentText());
}

void DialogBare::updateBoardId(const QString &fileName) {
    // XXX this should be moved in separe classes

    // Attempt to decode JSON
    QFile json(lastFilename);
    json.open(QIODevice::ReadOnly | QIODevice::Text);
    QByteArray jbytes = json.readAll();

    if (fileName.endsWith(".px4")) {

        int checkBoardId;

#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
        QJsonDocument doc = QJsonDocument::fromJson(jbytes);

        if (doc.isNull()) {
            // Error, bail out
            ui->upgradeLog->appendPlainText(tr("supplied file is not a valid JSON document"));
            return;
        }

        QJsonObject px4 = doc.object();

        checkBoardId = (int) (px4.value(QString("board_id")).toDouble());
#else
        QString j8(jbytes);

        // BOARD ID
        QStringList decode_list = j8.split("\"board_id\":");
        decode_list = decode_list.last().split(",");
        if (decode_list.count() < 1)
            ui->upgradeLog->appendPlainText(tr("supplied file is not a valid JSON document"));
            return;
        QString board_id = QString(decode_list.first().toUtf8()).trimmed();
        checkBoardId = board_id.toInt();

#endif
        ui->upgradeLog->appendPlainText(tr("loaded file for board ID %1").arg(checkBoardId));

        // Set correct board ID
        int index = ui->boardComboBox->findData(checkBoardId);

        if (index >= 0) {
            ui->boardComboBox->setCurrentIndex(index);
        } else {
            qDebug() << "Combo box: board not found:" << index;
        }
    }
}

void DialogBare::onLinkClicked(const QUrl &url)
{
    QString firmwareFile = QFileInfo(url.toString()).fileName();

    QString filename;

    // If a IO firmware file, open save as DialogBare
    if (firmwareFile.endsWith(".bin") && firmwareFile.contains("px4io")) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        QString path = QString(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
#else
        QString path = QString(QDesktopServices::storageLocation(QDesktopServices::DesktopLocation));
#endif
        qDebug() << path;
        filename = QFileDialog::getExistingDirectory(this, tr("Select folder (microSD Card)"),
                                                path);
        filename.append("/" + firmwareFile);
    } else {

        // Make sure the user doesn't screw up flashing
        //QMessageBox msgBox;
        //msgBox.setText("Please unplug your PX4 board now");
        //msgBox.exec();

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        filename = QString(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
#else
        filename = QString(QDesktopServices::storageLocation(QDesktopServices::DesktopLocation));
#endif

        if (filename.isEmpty()) {
            qDebug() << "Falling back to temp dir";
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        QString filename = QString(QStandardPaths::writableLocation(QStandardPaths::TempLocation));
#else
        QString filename = QString(QDesktopServices::storageLocation(QDesktopServices::TempLocation));
#endif
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

    ui->upgradeLog->appendHtml(tr("Downloading firmware file <a href=\"%1\">%1</a>").arg(url.toString()));

    QNetworkRequest newRequest(url);
    newRequest.setUrl(url);
    newRequest.setAttribute(QNetworkRequest::User, filename);

    // XXX rework
    QNetworkAccessManager* networkManager = new QNetworkAccessManager(this); //ui->webView->page()->networkAccessManager();
    QNetworkReply *reply = networkManager->get(newRequest);
    connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(onDownloadProgress(qint64, qint64)));
    connect(reply, SIGNAL(finished()), this, SLOT(onDownloadFinished()));
}

void DialogBare::onDownloadRequested(const QNetworkRequest &request)
{
    onLinkClicked(request.url());
}

void DialogBare::onPortNameChanged(const QString & /*name*/)
{
    if (port->isOpen()) {
        port->close();
//        ui->led->turnOff();
    }
}

void DialogBare::onDownloadFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(QObject::sender());

    if (!reply) {
        // bail out, nothing to do
        ui->upgradeLog->appendPlainText("Download failed, invalid context");
        return;
    }

    if (loading) {
        onCancelButtonClicked();
        ui->upgradeLog->appendPlainText(tr("Still flashing. Waiting for firmware flashing to complete.."));
    } else {

        // Reset progress
        ui->upgradeProgressBar->setValue(0);

        // Pick file
        QString fileName = lastFilename;

        qDebug() << "Handling filename:" << fileName;

        // Store file in download location
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly)) {
            ui->upgradeLog->appendPlainText(tr("Could not open %s for writing: %s\n").arg(fileName).arg(file.errorString()));
            return;
        }

        file.write(reply->readAll());
        file.close();

        if (lastFilename.contains("px4io")) {
            // Done, bail out
            return;
        }

        if (fileName.length() > 0) {
            // Got a filename, upload
            onLoadStart();
            lastFilename = fileName;

            if (worker) {
                worker->abortUpload();
                worker->deleteLater();
            }

            // Try to reserve links for us
            emit disconnectLinks();

            worker = QGCFirmwareUpgradeWorker::putWorkerInThread(fileName);
            // Hook up status from worker to progress bar
            connect(worker, SIGNAL(upgradeProgressChanged(int)), ui->upgradeProgressBar, SLOT(setValue(int)));
            // Hook up text from worker to label
            connect(worker, SIGNAL(upgradeStatusChanged(QString)), ui->upgradeLog, SLOT(appendPlainText(QString)));
            // Hook up error handling
            //connect(worker, SIGNAL(error(QString)), ui->upgradeLog, SLOT(appendPlainText(QString)));
            // Hook up status from worker to this class
            connect(worker, SIGNAL(loadFinished(bool)), this, SLOT(onLoadFinished(bool)));

//            // Make sure user gets the board going now
//            QMessageBox msgBox;
//            msgBox.setText("Please unplug your PX4 board now and plug it back in");
//            msgBox.exec();
        }
    }
}

void DialogBare::onFileSelectRequested()
{
    if (loading) {
        worker->abortUpload();
        loading = false;
    }

    // Pick file
    QString fileName = QFileDialog::getOpenFileName(this,
                            tr("Open Firmware File"), lastFilename, tr("Firmware Files (*.px4 *.bin)"));

    if (fileName != "") {
        lastFilename = fileName;
        ui->flashButton->setEnabled(true);

        updateBoardId(lastFilename);
    }
}

void DialogBare::onCancelButtonClicked()
{
    if (loading) {
        worker->abortUpload();
        ui->cancelButton->setEnabled(false);
        ui->upgradeLog->appendPlainText(tr("Waiting for last upgrade operaton to abort.."));
    }
}

void DialogBare::onUploadButtonClicked()
{
    if (loading) {
        onCancelButtonClicked();
    } else {

        if (lastFilename.length() > 0) {
            // Got a filename, upload
            loading = true;
            ui->flashButton->setEnabled(false);
            ui->cancelButton->setEnabled(true);

            int id = -1;

            // Try to reserve links for us
            emit disconnectLinks();

            // Set board ID for worker
            if (ui->boardComboBox->isVisible()) {
                bool ok;
                int tmp = ui->boardComboBox->itemData(ui->boardComboBox->currentIndex()).toInt(&ok);
                if (ok)
                    id = tmp;
            }

            worker = QGCFirmwareUpgradeWorker::putWorkerInThread(lastFilename, ui->portBox->currentText(), id);

            connect(ui->portBox, SIGNAL(editTextChanged(QString)), worker, SLOT(setPort(QString)));

            // Hook up status from worker to progress bar
            connect(worker, SIGNAL(upgradeProgressChanged(int)), ui->upgradeProgressBar, SLOT(setValue(int)));
            // Hook up text from worker to label
            connect(worker, SIGNAL(upgradeStatusChanged(QString)), ui->upgradeLog, SLOT(appendPlainText(QString)));
            // Hook up status from worker to this class
            connect(worker, SIGNAL(loadFinished(bool)), this, SLOT(onLoadFinished(bool)));
        }
    }
}

void DialogBare::onDetect()
{
    if (loading) {
        onCancelButtonClicked();
    } else {

            ui->flashButton->setEnabled(false);
            ui->cancelButton->setEnabled(true);

            // Try to reserve links for us
            emit disconnectLinks();

            worker = QGCFirmwareUpgradeWorker::putDetectorInThread();

            connect(ui->portBox, SIGNAL(editTextChanged(QString)), worker, SLOT(setPort(QString)));

            // Hook up status from worker to progress bar
            connect(worker, SIGNAL(upgradeProgressChanged(int)), ui->upgradeProgressBar, SLOT(setValue(int)));
            // Hook up text from worker to label
            connect(worker, SIGNAL(upgradeStatusChanged(QString)), ui->upgradeLog, SLOT(appendPlainText(QString)));
            // Hook up status from worker to this class
            connect(worker, SIGNAL(detectFinished(bool, int)), this, SLOT(onDetectFinished(bool, int)));
    }
}

void DialogBare::onPortAddedOrRemoved()
{
    ui->portBox->blockSignals(true);

    // Delete old ports
    for (int i = 0; i < ui->portBox->count(); i++)
    {
        bool found = false;
        foreach (QextPortInfo info, QextSerialEnumerator::getPorts())
            if (info.portName == ui->portBox->itemText(i))
                found = true;

        if (!found && !ui->portBox->itemText(i).contains("Automatic"))
            ui->portBox->removeItem(i);
    }

    // Add new ports
    foreach (QextPortInfo info, QextSerialEnumerator::getPorts())
        if (ui->portBox->findText(info.portName) < 0) {
            if (!info.portName.isEmpty())
                ui->portBox->addItem(info.portName);
        }

    ui->portBox->blockSignals(false);
}

void DialogBare::onLoadStart()
{
    loading = true;
    ui->flashButton->setEnabled(false);
    ui->cancelButton->setEnabled(true);
}

void DialogBare::onLoadFinished(bool success)
{
    loading = false;
    ui->flashButton->setEnabled(true);
    ui->cancelButton->setEnabled(false);

    if (success) {
        ui->upgradeLog->appendPlainText(tr("Upload succeeded."));
    } else {
        ui->upgradeLog->appendPlainText(tr("Upload aborted and failed."));
        ui->upgradeProgressBar->setValue(0);
    }

    // Reconnect links after upload
    QTimer::singleShot(2000, this, SIGNAL(connectLinks()));
}

void DialogBare::onDetectFinished(bool success, int board_id)
{
    loading = false;
    ui->flashButton->setEnabled(true);
    ui->cancelButton->setEnabled(false);

    if (success) {
        ui->upgradeLog->appendPlainText(tr("Board found."));
        // XXX this should not be hardcoded

        if (ui->versionComboBox->currentIndex() == 0) {
            if (board_id == 5) {
                // Download and flash
                onLinkClicked(QUrl("http://www.inf.ethz.ch/personal/lomeier/downloads/firmware/px4fmu-v1_default.px4"));
            } else if (board_id == 6) {
                // Download and flash
                onLinkClicked(QUrl("http://www.inf.ethz.ch/personal/lomeier/downloads/firmware/px4flow.px4"));
            } else if (board_id == 9) {
                // Download and flash
                onLinkClicked(QUrl("http://www.inf.ethz.ch/personal/lomeier/downloads/firmware/board9.px4"));
            }
         } else if (ui->versionComboBox->currentIndex() == 1) {
            if (board_id == 5) {
                // Download and flash
                onLinkClicked(QUrl("http://www.inf.ethz.ch/personal/lomeier/downloads/beta/px4fmu-v1_default.px4"));
            } else if (board_id == 6) {
                // Download and flash
                onLinkClicked(QUrl("http://www.inf.ethz.ch/personal/lomeier/downloads/beta/px4flow.px4"));
            } else if (board_id == 9) {
                // Download and flash
                onLinkClicked(QUrl("http://www.inf.ethz.ch/personal/lomeier/downloads/beta/board9.px4"));
            }
         } else if (ui->versionComboBox->currentIndex() == 2) {
            if (board_id == 5) {
                // Download and flash
                onLinkClicked(QUrl("http://www.inf.ethz.ch/personal/lomeier/downloads/nightly/px4fmu-v1_default.px4"));
            } else if (board_id == 6) {
                // Download and flash
                onLinkClicked(QUrl("http://www.inf.ethz.ch/personal/lomeier/downloads/nightly/px4flow.px4"));
            } else if (board_id == 9) {
                // Download and flash
                onLinkClicked(QUrl("http://www.inf.ethz.ch/personal/lomeier/downloads/nightly/board9.px4"));
            }
         }
    } else {
        ui->upgradeLog->appendPlainText(tr("No suitable device to upgrade."));
        ui->upgradeProgressBar->setValue(0);
    }
}

void DialogBare::onDownloadProgress(qint64 curr, qint64 total)
{
    // Take care of cases where 0 / 0 is emitted as error return value
    if (total > 0)
        ui->upgradeProgressBar->setValue((curr*100) / total);
}
