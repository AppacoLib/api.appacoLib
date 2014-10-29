/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created: Thu 19. Dec 15:26:23 2013
**      by: Qt User Interface Compiler version 4.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QRadioButton>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionOpen_Dict;
    QAction *actionExit;
    QWidget *centralWidget;
    QLineEdit *lineEdit;
    QComboBox *comboBox;
    QLabel *label;
    QLabel *label_2;
    QComboBox *comboBox_2;
    QCheckBox *checkBox;
    QGroupBox *groupBox;
    QRadioButton *radioButton;
    QRadioButton *radioButton_2;
    QGroupBox *groupBox_2;
    QRadioButton *radioButton_searchExact;
    QRadioButton *radioButton_search_1error;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(751, 335);
        actionOpen_Dict = new QAction(MainWindow);
        actionOpen_Dict->setObjectName(QString::fromUtf8("actionOpen_Dict"));
        actionExit = new QAction(MainWindow);
        actionExit->setObjectName(QString::fromUtf8("actionExit"));
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        lineEdit = new QLineEdit(centralWidget);
        lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
        lineEdit->setGeometry(QRect(200, 130, 331, 31));
        comboBox = new QComboBox(centralWidget);
        comboBox->setObjectName(QString::fromUtf8("comboBox"));
        comboBox->setGeometry(QRect(140, 40, 81, 22));
        label = new QLabel(centralWidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(140, 13, 101, 20));
        label_2 = new QLabel(centralWidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(270, 20, 121, 16));
        comboBox_2 = new QComboBox(centralWidget);
        comboBox_2->setObjectName(QString::fromUtf8("comboBox_2"));
        comboBox_2->setGeometry(QRect(260, 40, 221, 22));
        checkBox = new QCheckBox(centralWidget);
        checkBox->setObjectName(QString::fromUtf8("checkBox"));
        checkBox->setGeometry(QRect(500, 40, 70, 17));
        groupBox = new QGroupBox(centralWidget);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setGeometry(QRect(590, 10, 120, 71));
        radioButton = new QRadioButton(groupBox);
        radioButton->setObjectName(QString::fromUtf8("radioButton"));
        radioButton->setGeometry(QRect(20, 20, 82, 17));
        radioButton_2 = new QRadioButton(groupBox);
        radioButton_2->setObjectName(QString::fromUtf8("radioButton_2"));
        radioButton_2->setGeometry(QRect(20, 40, 82, 17));
        groupBox_2 = new QGroupBox(centralWidget);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        groupBox_2->setGeometry(QRect(10, 10, 101, 71));
        radioButton_searchExact = new QRadioButton(groupBox_2);
        radioButton_searchExact->setObjectName(QString::fromUtf8("radioButton_searchExact"));
        radioButton_searchExact->setGeometry(QRect(20, 20, 82, 17));
        radioButton_search_1error = new QRadioButton(groupBox_2);
        radioButton_search_1error->setObjectName(QString::fromUtf8("radioButton_search_1error"));
        radioButton_search_1error->setGeometry(QRect(20, 40, 82, 17));
        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 751, 25));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);

        menuBar->addAction(menuFile->menuAction());
        menuFile->addAction(actionOpen_Dict);
        menuFile->addSeparator();
        menuFile->addAction(actionExit);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "Ibra-Math-Djam", 0, QApplication::UnicodeUTF8));
        actionOpen_Dict->setText(QApplication::translate("MainWindow", "Open Dict", 0, QApplication::UnicodeUTF8));
        actionExit->setText(QApplication::translate("MainWindow", "Exit", 0, QApplication::UnicodeUTF8));
        comboBox->clear();
        comboBox->insertItems(0, QStringList()
         << QApplication::translate("MainWindow", "50 Ms", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("MainWindow", "100 Ms", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("MainWindow", "200 Ms", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("MainWindow", "300 Ms", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("MainWindow", "400 Ms", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("MainWindow", "500 Ms", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("MainWindow", "600 Ms", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("MainWindow", "700 Ms", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("MainWindow", "800 Ms", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("MainWindow", "900 Ms", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("MainWindow", "1000 Ms", 0, QApplication::UnicodeUTF8)
        );
        label->setText(QApplication::translate("MainWindow", "Separate Time", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("MainWindow", "search Methode", 0, QApplication::UnicodeUTF8));
        comboBox_2->clear();
        comboBox_2->insertItems(0, QStringList()
         << QApplication::translate("MainWindow", "WHOLE_WORD_EACH_TIME", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("MainWindow", "PREFIXE_total", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("MainWindow", "PREFIXE_C_saveWay", 0, QApplication::UnicodeUTF8)
        );
        checkBox->setText(QApplication::translate("MainWindow", "TOP K", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("MainWindow", "prefixe lenght :", 0, QApplication::UnicodeUTF8));
        radioButton->setText(QApplication::translate("MainWindow", "by function", 0, QApplication::UnicodeUTF8));
        radioButton_2->setText(QApplication::translate("MainWindow", "by event", 0, QApplication::UnicodeUTF8));
        groupBox_2->setTitle(QApplication::translate("MainWindow", "search :", 0, QApplication::UnicodeUTF8));
        radioButton_searchExact->setText(QApplication::translate("MainWindow", "exact", 0, QApplication::UnicodeUTF8));
        radioButton_search_1error->setText(QApplication::translate("MainWindow", "1 error", 0, QApplication::UnicodeUTF8));
        menuFile->setTitle(QApplication::translate("MainWindow", "File", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
