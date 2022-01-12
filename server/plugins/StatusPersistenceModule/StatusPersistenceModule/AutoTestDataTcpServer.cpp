#include "AutoTestDataTcpServer.h"
#include <QFile>
#include <QTcpSocket>
#include <QThread>

#include "AutomateTestDefine.h"
#include "ProtocolXmlTypeDefine.h"

AutoTestDataTcpServer::AutoTestDataTcpServer(QObject* parent)
    : QTcpServer(parent)
{
    connect(this, &AutoTestDataTcpServer::asynStart, this, &AutoTestDataTcpServer::start, Qt::QueuedConnection);
    connect(this, &AutoTestDataTcpServer::asynStop, this, &AutoTestDataTcpServer::stop, Qt::QueuedConnection);
}

void AutoTestDataTcpServer::incomingConnection(qintptr socketDescriptor)
{
    auto tcp_socket = new QTcpSocket();

    // 设置发送缓存为5M,防止出现粘包现象
    tcp_socket->setSocketOption(QAbstractSocket::SendBufferSizeSocketOption, 1024 * 1024 * 5);
    tcp_socket->setSocketDescriptor(socketDescriptor);

    QThread* thread = new QThread();
    tcp_socket->moveToThread(thread);
    thread->start();

    connect(thread, &QThread::finished, tcp_socket, [=]() {
        if (tcp_socket)
        {
            tcp_socket->close();
            tcp_socket->deleteLater();
        }
        thread->deleteLater();
    });

    tcp_socket->setSocketOption(QAbstractSocket::KeepAliveOption, true);
    connect(tcp_socket, &QTcpSocket::readyRead, tcp_socket, [=]() {
        auto data = tcp_socket->readAll();
        qDebug() << "收到健康管理数据";
    });
    connect(tcp_socket, static_cast<void (QTcpSocket::*)(QAbstractSocket::SocketError)>(&QTcpSocket::error), tcp_socket,
            [=](QAbstractSocket::SocketError error) { qDebug() << "健康管理tcp链接错误" << error; });

    connect(tcp_socket, &QAbstractSocket::disconnected, thread, &QThread::quit);
    connect(this, &AutoTestDataTcpServer::asynStop, thread, &QThread::quit);

    // 发送自动化测试数据
    connect(
        this, &AutoTestDataTcpServer::autoTestDataReceived, tcp_socket,
        [=](const QString& json) {
            if (!tcp_socket)
            {
                return;
            }
            if (tcp_socket->state() != QAbstractSocket::ConnectedState)
            {
                return;
            }
            QByteArray data;

            unsigned short head = 0xfbfb;
            unsigned short tail = 0xfefe;
            QByteArray json_data = json.toUtf8();
            unsigned int json_length = static_cast<unsigned int>(json_data.size());

            unsigned int total_length = sizeof(head) + sizeof(total_length) + sizeof(json_length) + json_length + sizeof(tail);

            data.append(QByteArray(reinterpret_cast<char*>(&head), sizeof(head)));
            data.append(QByteArray(reinterpret_cast<char*>(&total_length), sizeof(total_length)));
            data.append(QByteArray(reinterpret_cast<char*>(&json_length), sizeof(json_length)));
            data.append(json_data);
            data.append(QByteArray(reinterpret_cast<char*>(&tail), sizeof(tail)));

            //            AutomateTestResult testData;
            //            QFile file(QString("E:\\auototest\\%1_%2.txt").arg(testData.id).arg(testData.testTypeID));
            //            if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            //                return;

            //            QTextStream out(&file);
            //            out << data;

            tcp_socket->write(data);
        },
        Qt::QueuedConnection);
}

void AutoTestDataTcpServer::start() { listen(QHostAddress::Any, 1235); }

void AutoTestDataTcpServer::stop() { close(); }
