/* Copyright 2019 Tronlong Elec. Tech. Co. Ltd. All Rights Reserved. */

/* Copyright 2019 Tronlong Elec. Tech. Co. Ltd. All Rights Reserved. */

#include "mainwindow.h"
#include <stdio.h>
#include <QApplication>
#include <QString>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsProxyWidget>
#include <QScreen>
#include <QApplication>
#include <QRect>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        // e.g. tl_image_display ./cat.jpg
        printf("usage : %s <image path> [rotation]\n", argv[0]);
        printf("\te.g. : %s ./test.jpg\n", argv[0]);
        printf("\te.g. : %s ./test.jpg 1\n", argv[0]);
        return -1;
    }

    QApplication a(argc, argv);
    MainWindow w;
    QString rotation = argv[2];

    w.LoadImage(argv[1]);

    if(rotation.toInt() == 1)
    {
        QGraphicsScene *scene   = new QGraphicsScene;
        QGraphicsView *gview    = new QGraphicsView(scene);
        QGraphicsProxyWidget *g_proxy_widget;

        QScreen *desktop        = QApplication::primaryScreen();
        QRect screen_info       = desktop->geometry();

        // Rotation 90 degree
        g_proxy_widget = scene->addWidget((QWidget*)&w);
        g_proxy_widget->setRotation(90);

        // Set window size
        w.resize(screen_info.height(), screen_info.width());

        //Scroll bars are not displayed
        gview->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        gview->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        gview->setStyleSheet("border:none");

        gview->showFullScreen();
    }
    else
    {
        w.setWindowState(Qt::WindowFullScreen);
        w.show();
    }

    // This will not return until main window close.
    return a.exec();
}
