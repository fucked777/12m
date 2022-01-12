#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QThread>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_server = NULL;
}

MainWindow::~MainWindow()
{
    if (m_server != nullptr)
    {
        m_server->stop();
        m_server->setParent(nullptr);
        delete m_server;
    }
    delete ui;
}

void MainWindow::on_pushButton_3_clicked()
{
    QString folderName = QFileDialog::getExistingDirectory(NULL, "caption", ".");
    ui->lineEdit_3->setText(folderName);
}
#include <QMessageBox>
void MainWindow::on_pushButton_2_clicked()
{
    if (m_server != nullptr)
    {
        m_server->stop();
        m_server->setParent(nullptr);
        delete m_server;
    }
    m_server = nullptr;
}

void MainWindow::on_pushButton_4_clicked()
{
    if (m_server == nullptr)
    {
        m_server = new FepServer;
    }

    quint16 port = ui->lineEdit_2->text().toInt();
    QString path = ui->lineEdit_3->text();

    m_server->setWorkDir(path);
    m_server->setPort(port);
    QString errMsg;
    if (!m_server->start(errMsg))
    {
        QMessageBox::critical(this, "启动错误", errMsg);
    }
}
