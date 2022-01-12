#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "FepSendUnit.h"
#include <QMainWindow>
namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_3_clicked();

    void on_pushButton_6_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();
    void test(const QString&);
    void test2(bool, const QString&);

    void on_timerSend_clicked();

    void timerEvent(QTimerEvent* event);

private:
    Ui::MainWindow* ui;
    FepSendUnit* m_sendContext;
    int m_timerID{ -1 };
};

#endif  // MAINWINDOW_H
