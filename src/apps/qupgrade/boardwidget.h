#ifndef BOARDWIDGET_H
#define BOARDWIDGET_H

#include <QWidget>

namespace Ui {
class boardWidget;
}

class BoardWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit BoardWidget(QWidget *parent = 0);
    ~BoardWidget();
    
private:
    Ui::boardWidget *ui;
};

#endif // BOARDWIDGET_H
