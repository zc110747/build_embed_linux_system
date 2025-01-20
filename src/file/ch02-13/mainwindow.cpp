/* Copyright 2019 Tronlong Elec. Tech. Co. Ltd. All Rights Reserved. */

#include <QApplication>
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    this->setStyleSheet("background-color:rgba(66, 66, 66, 150)");

    // Use label to show image pixmap.
    m_label.setParent(this);
    m_label.setAlignment(Qt::AlignCenter);

    // Click this button to close main window.
    m_btn_exit.setParent(this);
    m_btn_exit.setText("Exit");
    m_btn_exit.setGeometry(20, 20, 80, 25);
    m_btn_exit.setStyleSheet(
                "QPushButton{"
                "   background-color:rgba(0, 0, 0, 88);"
                "   border-radius:12px;"
                "   color: white;}"
                "QPushButton:hover{"
                "   background-color:rgba(80, 160, 255, 88);"
                "   color:black;}"
                );
    m_btn_exit.raise(); /* On top of the UI */

    // Connect signal and slot.
    connect(&m_btn_exit, SIGNAL(clicked(bool)), this, SLOT(exit()));
}

void MainWindow::resizeEvent(QResizeEvent*) {
    // Set label's size to the same as main window.
    m_label.resize(this->size());

    if (m_pixmap.isNull()) {
        return;
    }

    // Scale image's size to fix main window(full screen).
    QPixmap pixmap = m_pixmap.scaled(m_label.size(),
            Qt::KeepAspectRatio, Qt::SmoothTransformation);
    // Show image with label.
    m_label.setPixmap(pixmap);

    return;
}

bool MainWindow::LoadImage(QString img_path) {
    // Load image file to pixmap.
    if (m_pixmap.load(img_path) == false) {
        m_label.setText("Load " + img_path + " failed");
        return false;
    }
    resizeEvent(NULL);  // This is needed in Qt4.8
    return true;
}

void MainWindow::exit()
{
    QApplication::exit(0);
}

MainWindow::~MainWindow() {
    /* Do nothing */
}
