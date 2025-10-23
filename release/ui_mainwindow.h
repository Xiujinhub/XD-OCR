/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout_2;
    QLabel *lb_show;
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QTextBrowser *tb_text;
    QLabel *lb_captured;
    QHBoxLayout *horizontalLayout;
    QPushButton *pb_openCamera;
    QPushButton *pb_captureImage;
    QPushButton *pb_selectImage;
    QPushButton *pb_saveText;
    QPushButton *pb_toggleSaveMode;
    QRadioButton *rb_singleDoc;
    QRadioButton *rb_multiDoc;
    QCheckBox *cb_showRect;
    QSpacerItem *horizontalSpacer;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->setEnabled(true);
        MainWindow->resize(960, 640);
        QSizePolicy sizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(MainWindow->sizePolicy().hasHeightForWidth());
        MainWindow->setSizePolicy(sizePolicy);
        MainWindow->setMinimumSize(QSize(960, 640));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/resource/XD-OCR.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        MainWindow->setWindowIcon(icon);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName("centralWidget");
        verticalLayout_2 = new QVBoxLayout(centralWidget);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName("verticalLayout_2");
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        lb_show = new QLabel(centralWidget);
        lb_show->setObjectName("lb_show");
        lb_show->setMinimumSize(QSize(640, 480));
        lb_show->setStyleSheet(QString::fromUtf8(""));

        horizontalLayout_2->addWidget(lb_show);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(6);
        verticalLayout->setObjectName("verticalLayout");
        label = new QLabel(centralWidget);
        label->setObjectName("label");

        verticalLayout->addWidget(label);

        tb_text = new QTextBrowser(centralWidget);
        tb_text->setObjectName("tb_text");
        tb_text->setMinimumSize(QSize(300, 0));
        tb_text->setMaximumSize(QSize(300, 16777215));
        tb_text->setBaseSize(QSize(0, 0));
        tb_text->viewport()->setProperty("cursor", QVariant(QCursor(Qt::CursorShape::IBeamCursor)));
        tb_text->setStyleSheet(QString::fromUtf8("border-color: rgb(33, 150, 243);"));

        verticalLayout->addWidget(tb_text);

        lb_captured = new QLabel(centralWidget);
        lb_captured->setObjectName("lb_captured");
        lb_captured->setMinimumSize(QSize(300, 200));
        lb_captured->setMaximumSize(QSize(300, 200));

        verticalLayout->addWidget(lb_captured);


        horizontalLayout_2->addLayout(verticalLayout);


        verticalLayout_2->addLayout(horizontalLayout_2);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName("horizontalLayout");
        pb_openCamera = new QPushButton(centralWidget);
        pb_openCamera->setObjectName("pb_openCamera");
        pb_openCamera->setStyleSheet(QString::fromUtf8("QPushButton { padding: 8px; font-weight: bold; background-color: #ffff8f; color: black; }"));

        horizontalLayout->addWidget(pb_openCamera);

        pb_captureImage = new QPushButton(centralWidget);
        pb_captureImage->setObjectName("pb_captureImage");
        pb_captureImage->setStyleSheet(QString::fromUtf8("QPushButton { padding: 8px; font-weight: bold; background-color: #7bffb6; color: black; }"));

        horizontalLayout->addWidget(pb_captureImage);

        pb_selectImage = new QPushButton(centralWidget);
        pb_selectImage->setObjectName("pb_selectImage");
        pb_selectImage->setAutoFillBackground(false);
        pb_selectImage->setStyleSheet(QString::fromUtf8("QPushButton { padding: 8px; font-weight: bold; background-color: #61c2ff; color: black; }\n"
""));

        horizontalLayout->addWidget(pb_selectImage);

        pb_saveText = new QPushButton(centralWidget);
        pb_saveText->setObjectName("pb_saveText");
        pb_saveText->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        pb_saveText->setAutoFillBackground(false);
        pb_saveText->setStyleSheet(QString::fromUtf8("QPushButton { padding: 8px; font-weight: bold; background-color: #a3ff5d; color: black; }\n"
""));

        horizontalLayout->addWidget(pb_saveText);

        pb_toggleSaveMode = new QPushButton(centralWidget);
        pb_toggleSaveMode->setObjectName("pb_toggleSaveMode");
        pb_toggleSaveMode->setCursor(QCursor(Qt::CursorShape::ArrowCursor));
        pb_toggleSaveMode->setAutoFillBackground(false);
        pb_toggleSaveMode->setStyleSheet(QString::fromUtf8("QPushButton { padding: 8px; font-weight: bold; background-color: #ff7dd8; color: black; }\n"
""));

        horizontalLayout->addWidget(pb_toggleSaveMode);

        rb_singleDoc = new QRadioButton(centralWidget);
        rb_singleDoc->setObjectName("rb_singleDoc");

        horizontalLayout->addWidget(rb_singleDoc);

        rb_multiDoc = new QRadioButton(centralWidget);
        rb_multiDoc->setObjectName("rb_multiDoc");

        horizontalLayout->addWidget(rb_multiDoc);

        cb_showRect = new QCheckBox(centralWidget);
        cb_showRect->setObjectName("cb_showRect");

        horizontalLayout->addWidget(cb_showRect);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);


        verticalLayout_2->addLayout(horizontalLayout);

        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName("menuBar");
        menuBar->setGeometry(QRect(0, 0, 960, 21));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName("mainToolBar");
        MainWindow->addToolBar(Qt::ToolBarArea::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName("statusBar");
        MainWindow->setStatusBar(statusBar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        lb_show->setText(QCoreApplication::translate("MainWindow", "<html><head/><body><p align=\"center\">\346\230\276\347\244\272\345\233\276\345\203\217</p></body></html>", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "\350\257\206\345\210\253\347\273\223\346\236\234\357\274\232", nullptr));
        lb_captured->setText(QCoreApplication::translate("MainWindow", "<html><head/><body><p align=\"center\">\346\213\215\346\221\204\347\232\204\347\205\247\347\211\207\345\260\206\346\230\276\347\244\272\345\234\250\350\277\231\351\207\214</p></body></html>", nullptr));
        pb_openCamera->setText(QCoreApplication::translate("MainWindow", "\346\211\223\345\274\200\346\221\204\345\203\217\345\244\264", nullptr));
        pb_captureImage->setText(QCoreApplication::translate("MainWindow", "\346\213\215\347\205\247\350\257\206\345\210\253", nullptr));
        pb_selectImage->setText(QCoreApplication::translate("MainWindow", "\351\200\211\346\213\251\345\233\276\345\203\217", nullptr));
        pb_saveText->setText(QCoreApplication::translate("MainWindow", "\344\277\235\345\255\230\346\226\207\346\234\254", nullptr));
        pb_toggleSaveMode->setText(QCoreApplication::translate("MainWindow", "\344\277\235\345\255\230\346\250\241\345\274\217", nullptr));
        rb_singleDoc->setText(QCoreApplication::translate("MainWindow", "\345\215\225\346\226\207\346\241\243\346\250\241\345\274\217", nullptr));
        rb_multiDoc->setText(QCoreApplication::translate("MainWindow", "\345\244\232\346\226\207\346\241\243\346\250\241\345\274\217", nullptr));
        cb_showRect->setText(QCoreApplication::translate("MainWindow", "\346\230\276\347\244\272\346\226\207\346\234\254\346\243\200\346\265\213\346\241\206", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
