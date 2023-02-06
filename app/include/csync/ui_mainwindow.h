/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "clickable_tree_view.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_accessToken;
    QLineEdit *lineEdit_accessToken;
    QLabel *label_authenticationStatus;
    QPushButton *pushButton_changeToken;
    QLabel *label_accessTokenHelp;
    QHBoxLayout *horizontalLayout_actionButtons;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButton_fetch;
    QPushButton *pushButton_pull;
    QProgressBar *progressBar;
    QLabel *guideText;
    ClickableTreeView *treeView;
    QMenuBar *menubar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(600, 500);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(centralwidget->sizePolicy().hasHeightForWidth());
        centralwidget->setSizePolicy(sizePolicy);
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setSpacing(2);
        verticalLayout->setObjectName("verticalLayout");
        horizontalLayout_accessToken = new QHBoxLayout();
        horizontalLayout_accessToken->setSpacing(10);
        horizontalLayout_accessToken->setObjectName("horizontalLayout_accessToken");
        horizontalLayout_accessToken->setContentsMargins(-1, 0, -1, -1);
        lineEdit_accessToken = new QLineEdit(centralwidget);
        lineEdit_accessToken->setObjectName("lineEdit_accessToken");
        lineEdit_accessToken->setEchoMode(QLineEdit::Password);

        horizontalLayout_accessToken->addWidget(lineEdit_accessToken);

        label_authenticationStatus = new QLabel(centralwidget);
        label_authenticationStatus->setObjectName("label_authenticationStatus");

        horizontalLayout_accessToken->addWidget(label_authenticationStatus);

        pushButton_changeToken = new QPushButton(centralwidget);
        pushButton_changeToken->setObjectName("pushButton_changeToken");
        pushButton_changeToken->setEnabled(true);

        horizontalLayout_accessToken->addWidget(pushButton_changeToken);


        verticalLayout->addLayout(horizontalLayout_accessToken);

        label_accessTokenHelp = new QLabel(centralwidget);
        label_accessTokenHelp->setObjectName("label_accessTokenHelp");
        label_accessTokenHelp->setWordWrap(true);
        label_accessTokenHelp->setOpenExternalLinks(true);

        verticalLayout->addWidget(label_accessTokenHelp);

        horizontalLayout_actionButtons = new QHBoxLayout();
        horizontalLayout_actionButtons->setSpacing(10);
        horizontalLayout_actionButtons->setObjectName("horizontalLayout_actionButtons");
        horizontalLayout_actionButtons->setContentsMargins(-1, 0, -1, -1);
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

        horizontalLayout_actionButtons->addItem(horizontalSpacer);

        pushButton_fetch = new QPushButton(centralwidget);
        pushButton_fetch->setObjectName("pushButton_fetch");
        pushButton_fetch->setMinimumSize(QSize(80, 0));

        horizontalLayout_actionButtons->addWidget(pushButton_fetch);

        pushButton_pull = new QPushButton(centralwidget);
        pushButton_pull->setObjectName("pushButton_pull");
        QSizePolicy sizePolicy1(QSizePolicy::Maximum, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(pushButton_pull->sizePolicy().hasHeightForWidth());
        pushButton_pull->setSizePolicy(sizePolicy1);
        pushButton_pull->setMinimumSize(QSize(80, 0));

        horizontalLayout_actionButtons->addWidget(pushButton_pull);


        verticalLayout->addLayout(horizontalLayout_actionButtons);

        progressBar = new QProgressBar(centralwidget);
        progressBar->setObjectName("progressBar");
        progressBar->setValue(24);
        progressBar->setInvertedAppearance(false);

        verticalLayout->addWidget(progressBar);

        guideText = new QLabel(centralwidget);
        guideText->setObjectName("guideText");
        guideText->setWordWrap(true);
        guideText->setMargin(0);
        guideText->setIndent(0);

        verticalLayout->addWidget(guideText);

        treeView = new ClickableTreeView(centralwidget);
        treeView->setObjectName("treeView");
        treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        treeView->setSelectionBehavior(QAbstractItemView::SelectItems);
        treeView->setUniformRowHeights(true);
        treeView->setAnimated(false);
        treeView->header()->setVisible(true);

        verticalLayout->addWidget(treeView);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 600, 37));
        MainWindow->setMenuBar(menubar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Canvas Sync", nullptr));
        lineEdit_accessToken->setPlaceholderText(QCoreApplication::translate("MainWindow", "Canvas Access Token", nullptr));
        label_authenticationStatus->setText(QCoreApplication::translate("MainWindow", "unauthenticated", nullptr));
        pushButton_changeToken->setText(QCoreApplication::translate("MainWindow", "Change Token", nullptr));
        label_accessTokenHelp->setText(QCoreApplication::translate("MainWindow", "<span style=\"color: #555555\"\n"
"  >To obtain your Canvas Access Token, head over to your\n"
"  <a href=\"https://canvas.nus.edu.sg/profile/settings\"\n"
"    >Canvas profile settings</a\n"
"  >. Generate a new token by clicking on \"New Access Token\", and leave the\n"
"  expiry date blank to have it last forever.</span\n"
">\n"
"", nullptr));
        pushButton_fetch->setText(QCoreApplication::translate("MainWindow", "Fetch", nullptr));
        pushButton_pull->setText(QCoreApplication::translate("MainWindow", "Pull", nullptr));
        guideText->setText(QCoreApplication::translate("MainWindow", "<html><head/><body><p><span style=\" color:#555555;\">Right-click on any folder to start configuring.</span></p></body></html>", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
