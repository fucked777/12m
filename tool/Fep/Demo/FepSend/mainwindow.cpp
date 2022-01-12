#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QThread>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_sendContext = NULL;
}

MainWindow::~MainWindow()
{
    if (m_sendContext != NULL)
    {
        delete m_sendContext;
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
    if (m_sendContext == nullptr)
    {
        m_sendContext = new FepSendUnit;
        connect(m_sendContext, &FepSendUnit::sg_error, this, &MainWindow::test);
        connect(m_sendContext, &FepSendUnit::sg_msg, this, &MainWindow::test2);
    }
    QString dstIp = ui->lineEdit->text();
    quint16 port = ui->lineEdit_2->text().toInt();
    QString path = ui->lineEdit_3->text();
    // m_sendContext = new FepSendUnit();
    m_sendContext->setServerAddr(dstIp, port);
    m_sendContext->setWorkDir(path);
    QString errMsg;
    if (!m_sendContext->start(errMsg))
    {
        QMessageBox::critical(this, "启动错误", errMsg);
    }
}
void MainWindow::test(const QString& msg) { qDebug() << "-----------------------------" << msg; }
void MainWindow::test2(bool msg, const QString& errMsg) { qDebug() << "-----------------------------" << msg << errMsg; }
void MainWindow::on_pushButton_4_clicked()
{
    if (m_sendContext == nullptr)
    {
        m_sendContext = new FepSendUnit;
        connect(m_sendContext, &FepSendUnit::sg_error, this, &MainWindow::test);
        connect(m_sendContext, &FepSendUnit::sg_msg, this, &MainWindow::test2);
    }
    QString dstIp = ui->lineEdit->text();
    quint16 port = ui->lineEdit_2->text().toInt();
    // QString path = ui->lineEdit_3->text();
    // m_sendContext = new FepSendUnit();
    m_sendContext->setServerAddr(dstIp, port);
    // m_sendContext->setWorkDir(path);
    QString errMsg;
    if (!m_sendContext->startSendFile(ui->lineEdit_4->text(), errMsg))
    {
        QMessageBox::critical(this, "启动错误", errMsg);
    }
}

void MainWindow::on_pushButton_5_clicked()
{
    auto fileName = QFileDialog::getOpenFileName(this, "caption", ".");
    ui->lineEdit_4->setText(fileName);
}
