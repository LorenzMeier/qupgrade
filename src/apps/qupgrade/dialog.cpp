#include "qextserialport.h"
#include "qextserialenumerator.h"
#include "dialog.h"
#include "ui_dialog.h"
#include <QtCore>
#include <QDebug>
#include <QFileDialog>
#include <QThread>

#include "qgc.h"
#include "uploader.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
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

    // Pick file
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Firmware File"), "/home/", tr("Firmware Files (*.px4 *.bin)"));

    if (fileName.length() > 0) {

        port->close();

        PX4_Uploader uploader(port);


        // XXX revisit
        port->setTimeout(0);
        port->setQueryMode(QextSerialPort::Polling);

        int i = 0;

        while (i < 100) {
            i++;


        QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
        //! [1]
        qDebug() << "List of ports:";
        //! [2]
        foreach (QextPortInfo info, ports) {

            if (!(info.physName.contains("PX4") || info.vendorID == 9900 /* 3DR */))
                continue;

            qDebug() << "port name:"       << info.portName;
            qDebug() << "friendly name:"   << info.friendName;
            qDebug() << "physical name:"   << info.physName;
            qDebug() << "enumerator name:" << info.enumName;
            qDebug() << "vendor ID:"       << info.vendorID;
            qDebug() << "product ID:"      << info.productID;

            qDebug() << "===================================";

            QString openString = info.portName;

            qDebug() << "UPLOAD ATTEMPT";

            // Spawn upload thread

            port->setPortName(openString);

            // Die-hard flash the binary
            qDebug() << "ATTEMPTING TO FLASH" << fileName << "ON PORT" << port->portName();

//            quint32 board_id;
//            quint32 board_rev;
//            quint32 flash_size;
//            bool insync = false;
//            QString humanReadable;
            //uploader.get_bl_info(board_id, board_rev, flash_size, humanReadable, insync);

            int ret = uploader.upload(fileName);
            return;

            port->close();

            // bail out on success
            if (ret == 0)
                return;
        }

        usleep(100000);

        }

    }
}

void Dialog::onReadyRead()
{
    if (port->bytesAvailable()) {
        ui->recvEdit->moveCursor(QTextCursor::End);
        ui->recvEdit->insertPlainText(QString::fromLatin1(port->readAll()));
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

//! [4]
