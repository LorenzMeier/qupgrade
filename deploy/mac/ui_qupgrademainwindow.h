/********************************************************************************
** Form generated from reading UI file 'qupgrademainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.0.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QUPGRADEMAINWINDOW_H
#define UI_QUPGRADEMAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_QUpgradeMainWindow
{
public:
    QAction *actionAbout_QUpgrade;
    QWidget *centralwidget;
    QMenuBar *menubar;
    QMenu *menuFile;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *QUpgradeMainWindow)
    {
        if (QUpgradeMainWindow->objectName().isEmpty())
            QUpgradeMainWindow->setObjectName(QStringLiteral("QUpgradeMainWindow"));
        QUpgradeMainWindow->resize(489, 600);
        actionAbout_QUpgrade = new QAction(QUpgradeMainWindow);
        actionAbout_QUpgrade->setObjectName(QStringLiteral("actionAbout_QUpgrade"));
        centralwidget = new QWidget(QUpgradeMainWindow);
        centralwidget->setObjectName(QStringLiteral("centralwidget"));
        QUpgradeMainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(QUpgradeMainWindow);
        menubar->setObjectName(QStringLiteral("menubar"));
        menubar->setGeometry(QRect(0, 0, 489, 22));
        menuFile = new QMenu(menubar);
        menuFile->setObjectName(QStringLiteral("menuFile"));
        QUpgradeMainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(QUpgradeMainWindow);
        statusbar->setObjectName(QStringLiteral("statusbar"));
        QUpgradeMainWindow->setStatusBar(statusbar);

        menubar->addAction(menuFile->menuAction());
        menuFile->addAction(actionAbout_QUpgrade);

        retranslateUi(QUpgradeMainWindow);

        QMetaObject::connectSlotsByName(QUpgradeMainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *QUpgradeMainWindow)
    {
        QUpgradeMainWindow->setWindowTitle(QApplication::translate("QUpgradeMainWindow", "MainWindow", 0));
        actionAbout_QUpgrade->setText(QApplication::translate("QUpgradeMainWindow", "About QUpgrade", 0));
        menuFile->setTitle(QApplication::translate("QUpgradeMainWindow", "File", 0));
    } // retranslateUi

};

namespace Ui {
    class QUpgradeMainWindow: public Ui_QUpgradeMainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QUPGRADEMAINWINDOW_H
