/* Copyright 2019 Tronlong Elec. Tech. Co. Ltd. All Rights Reserved. */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QPixmap>
#include <QLabel>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    bool LoadImage(QString img_path);

private:
    QPushButton m_btn_exit;
    QLabel      m_label;
    QPixmap     m_pixmap;

private slots:
    void exit();

protected:
    // Rewrite resizeEvent() of QMainWindow.
    void resizeEvent(QResizeEvent*);
};

#endif // MAINWINDOW_H
