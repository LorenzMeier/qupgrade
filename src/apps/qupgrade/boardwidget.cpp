#include "boardwidget.h"
#include "ui_boardwidget.h"

BoardWidget::BoardWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::boardWidget)
{
    ui->setupUi(this);
}

BoardWidget::~BoardWidget()
{
    delete ui;
}
