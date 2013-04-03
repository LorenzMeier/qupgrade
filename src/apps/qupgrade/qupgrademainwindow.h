#ifndef QUPGRADEMAINWINDOW_H
#define QUPGRADEMAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class QUpgradeMainWindow;
}

class Dialog;

class QUpgradeMainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit QUpgradeMainWindow(QWidget *parent = 0);
    ~QUpgradeMainWindow();

protected:
    void closeEvent(QCloseEvent* event);
    void writePositionSettings();
    void readPositionSettings();
    
private:
    Ui::QUpgradeMainWindow *ui;
    Dialog* dialog;
};

#endif // QUPGRADEMAINWINDOW_H
