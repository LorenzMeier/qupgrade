//#include <QJsonDocument>
#include <QFile>

#include "QGCFirmwareUpgradeWorker.h"

#include <SerialLink.h>
#include <QGC.h>
#include "uploader.h"

#include <QDebug>

#define PROTO_GET_SYNC		0x21
#define PROTO_EOC            0x20
#define PROTO_NOP		0x00
#define PROTO_OK		0x10
#define PROTO_FAILED		0x11
#define PROTO_INSYNC		0x12

QGCFirmwareUpgradeWorker::QGCFirmwareUpgradeWorker(QObject *parent) :
    QObject(parent),
    link(NULL)
{
}

QGCFirmwareUpgradeWorker* QGCFirmwareUpgradeWorker::putWorkerInThread(QObject *parent)
{
    QGCFirmwareUpgradeWorker *worker = new QGCFirmwareUpgradeWorker;
    QThread *workerThread = new QThread(parent);

    connect(workerThread, SIGNAL(started()), worker, SLOT(startContinousScan()));
    connect(workerThread, SIGNAL(finished()), worker, SLOT(deleteLater()));
    worker->moveToThread(workerThread);

    // Starts an event loop, and emits workerThread->started()
    workerThread->start();
    return worker;
}


void QGCFirmwareUpgradeWorker::startContinousScan()
{
    exitThread = false;
    while (!exitThread) {
//        if (detect()) {
//            break;
//        }
        QGC::SLEEP::msleep(20);
    }

    if (exitThread) {
        link->disconnect();
        delete link;
        exit(0);
    }
}

void QGCFirmwareUpgradeWorker::detect()
{
    if (!link)
    {
        link = new SerialLink("", 921600);
        connect(link, SIGNAL(bytesReceived(LinkInterface*,QByteArray)), this, SLOT(receiveBytes(LinkInterface*,QByteArray)));
    }

    // Get a list of ports
    QVector<QString>* ports = link->getCurrentPorts();

    // Scan
    for (int i = 0; i < ports->size(); i++)
    {
        // Ignore known wrong link names

        if (ports->at(i).contains("Bluetooth")) {
            //continue;
        }

        link->setPortName(ports->at(i));
        // Open port and talk to it
        link->connect();
        char buf[2] = { PROTO_GET_SYNC, PROTO_EOC };
        if (!link->isConnected()) {
            continue;
        }
        // Send sync request
        insync = false;
        link->writeBytes(buf, 2);
        // Wait for response
        QGC::SLEEP::msleep(20);

        if (insync)
            emit validPortFound(ports->at(i));
            break;
    }

    //ui.portName->setCurrentIndex(ui.baudRate->findText(QString("%1").arg(this->link->getPortName())));

    // Set port

    // Load current link config

}

void QGCFirmwareUpgradeWorker::receiveBytes(LinkInterface* link, QByteArray b)
{
    for (int position = 0; position < b.size(); position++) {
        qDebug() << "BYTES";
        qDebug() << (char)(b[position]);
        if (((const char)b[position]) == PROTO_INSYNC)
        {
            qDebug() << "SYNC";
            insync = true;
        }

        if (insync && ((const char)b[position]) == PROTO_OK)
        {
            emit detectionStatusChanged("Found PX4 board");
        }
    }

    printf("\n");
}

void QGCFirmwareUpgradeWorker::loadFirmware(const QString &filename)
{
    qDebug() << __FILE__ << __LINE__ << "LOADING FW" << filename;

    PX4_Uploader uploader;
    const char* filenames[2];
    filenames[0] = filename.toStdString().c_str();
    filenames[1] = NULL;
    uploader.upload(filenames, "/dev/tty.usbmodem1");

//    QFile f(filename);
//    if (f.open(QIODevice::ReadOnly))
//    {
//        QByteArray buf = f.readAll();
//        f.close();
//        firmware = QJsonDocument::fromBinaryData(buf);
//        if (firmware.isNull()) {
//            emit upgradeStatusChanged(tr("Failed decoding file %1").arg(filename));
//        } else {
//            emit upgradeStatusChanged(tr("Ready to flash %1").arg(filename));
//        }
//    } else {
//        emit upgradeStatusChanged(tr("Failed opening file %1").arg(filename));
//    }
}

void QGCFirmwareUpgradeWorker::upgrade()
{
    emit upgradeStatusChanged(tr("Starting firmware upgrade.."));
}

void QGCFirmwareUpgradeWorker::abort()
{
    exitThread = true;
}
