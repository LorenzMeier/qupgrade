#include "boardwidget.h"
#include "ui_boardwidget.h"

BoardWidget::BoardWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::boardWidget)
{
    ui->setupUi(this);

    connect(ui->flashButton, SIGNAL(clicked()), this, SLOT(flashFirmware()));

    setBoardImage("./files/images/px4/calibration/accel_z-.png");
}

BoardWidget::~BoardWidget()
{
    delete ui;
}

void BoardWidget::setBoardImage(const QString &path)
{
    boardIcon.load(path);

    int w = ui->iconLabel->width();
    int h = ui->iconLabel->height();

    ui->iconLabel->setPixmap(boardIcon.scaled(w, h, Qt::KeepAspectRatio));
}

void BoardWidget::resizeEvent(QResizeEvent* event)
{

    int w = ui->iconLabel->width();
    int h = ui->iconLabel->height();

    ui->iconLabel->setPixmap(boardIcon.scaled(w, h, Qt::KeepAspectRatio));

    QWidget::resizeEvent(event);
}

void BoardWidget::flashFirmware()
{
    QString url = ui->firmwareComboBox->itemData(ui->firmwareComboBox->currentIndex()).toString();
    emit flashFirmwareURL(url);
}

void BoardWidget::updateStatus(const QString &status)
{
    ui->statusLabel->setText(status);
}

void BoardWidget::setBoardInfo(int board_id, const QString &boardName, const QString &bootLoader)
{
    // XXX this should not be hardcoded
    ui->firmwareComboBox->clear();

    ui->boardNameLabel->setText(boardName);
    ui->bootloaderLabel->setText(bootLoader);

    switch (board_id) {
    case 5:
    {
        setBoardImage(":/files/boards/px4fmu_1.x.png");
        ui->firmwareComboBox->addItem("Stable Version", "http://www.inf.ethz.ch/personal/lomeier/downloads/firmware/px4fmu-v1_default.px4");
        ui->firmwareComboBox->addItem("Beta Testing", "http://www.inf.ethz.ch/personal/lomeier/downloads/beta/px4fmu-v1_default.px4");
        ui->firmwareComboBox->addItem("Continous Build", "http://www.inf.ethz.ch/personal/lomeier/downloads/nightly/px4fmu-v1_default.px4");
    }
        break;
    case 6:
    {
        setBoardImage(":/files/boards/px4flow_1.x.png");
        ui->firmwareComboBox->addItem("Stable Version", "http://www.inf.ethz.ch/personal/lomeier/downloads/firmware/px4flow.px4");
        ui->firmwareComboBox->addItem("Beta Testing", "http://www.inf.ethz.ch/personal/lomeier/downloads/beta/px4flow.px4");
        ui->firmwareComboBox->addItem("Continous Build", "http://www.inf.ethz.ch/personal/lomeier/downloads/nightly/px4flow.px4");
    }
        break;
    case 9:
        ui->firmwareComboBox->addItem("Stable Version", "http://www.inf.ethz.ch/personal/lomeier/downloads/firmware/board9.px4");
        ui->firmwareComboBox->addItem("Beta Testing", "http://www.inf.ethz.ch/personal/lomeier/downloads/beta/px4fmu-v1_default.px4");
        ui->firmwareComboBox->addItem("Continous Build", "http://www.inf.ethz.ch/personal/lomeier/downloads/nightly/px4fmu-v1_default.px4");
        break;

    }

}
