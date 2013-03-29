//#include <QJsonDocument>
#include <QFile>

#include "qgcfirmwareupgradeworker.h"

#include <qgc.h>
#include "uploader.h"
#include "qextserialenumerator.h"

#include <QDebug>

QGCFirmwareUpgradeWorker::QGCFirmwareUpgradeWorker(QObject *parent) :
    QObject(parent),
    _abortUpload(false),
    port(NULL)
{
}

QGCFirmwareUpgradeWorker* QGCFirmwareUpgradeWorker::putWorkerInThread(const QString &filename)
{
    QGCFirmwareUpgradeWorker *worker = NULL;
    QThread *thread = NULL;

    worker = new QGCFirmwareUpgradeWorker;
    worker->setFilename(filename);
    thread = new QThread;

    worker->moveToThread(thread);
    //connect(worker, SIGNAL(error(QString)), this, SLOT(errorString(QString)));
    connect(thread, SIGNAL(started()), worker, SLOT(loadFirmware()));
    connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
    connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    thread->start();

    // Starts an event loop, and emits workerThread->started()
    thread->start();
    return worker;




//    static QGCFirmwareUpgradeWorker *worker = NULL;
//    static QThread *workerThread = NULL;
    
//    if (!worker) {
//        worker = new QGCFirmwareUpgradeWorker;
//        workerThread = new QThread(parent);
//    } else {
//        // Already instantiated and running, return running thread
//        return worker;
//    }

//    worker->moveToThread(workerThread);
//    connect(worker, SIGNAL(error(QString)), this, SLOT(errorString(QString)));
//    connect(thread, SIGNAL(started()), worker, SLOT(process()));
//    connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
//    connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
//    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
//    thread->start();

//    // Starts an event loop, and emits workerThread->started()
//    thread->start();
//    return worker;
}


void QGCFirmwareUpgradeWorker::startContinousScan()
{
    exitThread = false;
    while (!exitThread) {
        //        if (detect()) {
        //            break;
        //        }
        SLEEP::msleep(100);
    }

    if (exitThread) {
        port->close();
        delete port;
        exit(0);
    }
}

void QGCFirmwareUpgradeWorker::detect()
{
    

}

void QGCFirmwareUpgradeWorker::setFilename(const QString &filename)
{
    this->filename = filename;
}

void QGCFirmwareUpgradeWorker::loadFirmware()
{
    qDebug() << __FILE__ << __LINE__ << "LOADING FW" << filename;

    emit upgradeStatusChanged(tr("Attempting to upload file:\n%1").arg(filename));

    while (!_abortUpload) {

        SLEEP::usleep(100000);

        QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
        //! [1]
        qDebug() << "List of ports:";
        //! [2]
        foreach (QextPortInfo info, ports) {

            qDebug() << "port name:"       << info.portName;
            qDebug() << "friendly name:"   << info.friendName;
            qDebug() << "physical name:"   << info.physName;
            qDebug() << "enumerator name:" << info.enumName;
            qDebug() << "vendor ID:"       << info.vendorID;
            qDebug() << "product ID:"      << info.productID;

            qDebug() << "===================================";

            if (!(info.physName.contains("PX4") || info.vendorID == 9900 /* 3DR */))
                continue;

            QString openString = info.portName;

            qDebug() << "UPLOAD ATTEMPT";

            // Spawn upload thread

            if (port == NULL) {
                PortSettings settings = {BAUD115200, DATA_8, PAR_NONE, STOP_1, FLOW_OFF, 10};

                port = new QextSerialPort(openString, settings, QextSerialPort::Polling);
                port->setTimeout(0);
                port->setQueryMode(QextSerialPort::Polling);
            } else {
                port->close();
                port->setPortName(openString);
            }

            PX4_Uploader uploader(port);
            connect(&uploader, SIGNAL(upgradeProgressChanged(int percent)), this, SIGNAL(upgradeProgressChanged(int)));

            // Die-hard flash the binary
            emit upgradeStatusChanged(tr("Found PX4 board on port %1").arg(info.portName));

            //            quint32 board_id;
            //            quint32 board_rev;
            //            quint32 flash_size;
            //            bool insync = false;
            //            QString humanReadable;
            //uploader.get_bl_info(board_id, board_rev, flash_size, humanReadable, insync);

            int ret = uploader.upload(filename);
            return;

            port->close();

            // bail out on success
            if (ret == 0) {
                emit loadFinished(true);
                return;
            }
        }
    }

    _abortUpload = false;
    emit loadFinished(false);

}

void QGCFirmwareUpgradeWorker::abortUpload()
{
    _abortUpload = true;
}

void QGCFirmwareUpgradeWorker::abort()
{
    exitThread = true;
}
