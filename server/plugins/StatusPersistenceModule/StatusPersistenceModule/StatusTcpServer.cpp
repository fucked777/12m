#include "StatusTcpServer.h"
#include <QFile>
#include <QTcpSocket>
#include <QThread>

#include "ProtocolXmlTypeDefine.h"

StatusTcpServer::StatusTcpServer(QObject* parent)
    : QTcpServer(parent)
{
    connect(this, &StatusTcpServer::asynStart, this, &StatusTcpServer::start, Qt::QueuedConnection);
    connect(this, &StatusTcpServer::asynStop, this, &StatusTcpServer::stop, Qt::QueuedConnection);
}

void StatusTcpServer::incomingConnection(qintptr socketDescriptor)
{
    auto tcp_socket = new QTcpSocket();

    //设置发送缓存为5M,防止出现粘包现象
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
    connect(this, &StatusTcpServer::asynStop, thread, &QThread::quit);

    connect(
        this, &StatusTcpServer::statusResultReceived, tcp_socket,
        [=](int device, const QString& json) {
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
            unsigned short device_id = static_cast<unsigned short>(device);

            unsigned int total_length = sizeof(head) + sizeof(total_length) + sizeof(device_id) + sizeof(json_length) + json_length + sizeof(tail);

            data.append(QByteArray(reinterpret_cast<char*>(&head), sizeof(head)));
            data.append(QByteArray(reinterpret_cast<char*>(&total_length), sizeof(total_length)));
            data.append(QByteArray(reinterpret_cast<char*>(&device_id), sizeof(device_id)));
            data.append(QByteArray(reinterpret_cast<char*>(&json_length), sizeof(json_length)));
            data.append(json_data);
            data.append(QByteArray(reinterpret_cast<char*>(&tail), sizeof(tail)));

            //            if (device_id == 0x8101)
            //            {
            //                data.append(QByteArray(reinterpret_cast<char*>(&head), sizeof(head)));
            //                qDebug() << "head : " << sizeof(head);
            //                data.append(QByteArray(reinterpret_cast<char*>(&total_length), sizeof(total_length)));
            //                qDebug() << "total_length : " << sizeof(total_length);
            //                data.append(QByteArray(reinterpret_cast<char*>(&device_id), sizeof(device_id)));
            //                qDebug() << "device_id : " << sizeof(device_id);
            //                data.append(QByteArray(reinterpret_cast<char*>(&json_length), sizeof(json_length)));
            //                qDebug() << "json_length : " << sizeof(json_length);
            //                data.append(json_data);
            //                qDebug() << "json_data : " << sizeof(json_data);
            //                data.append(QByteArray(reinterpret_cast<char*>(&tail), sizeof(tail)));
            //                qDebug() << "tail : " << sizeof(tail);

            //                DeviceID deviceID(device);
            //                QFile
            //                file(QString("C:\\Users\\19375\\Desktop\\%1_%2_%3.txt").arg(deviceID.sysID).arg(deviceID.devID).arg(deviceID.extenID));
            //                if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            //                    return;

            //                QTextStream out(&file);
            //                out << data.mid(12, json_length);
            //            }
            //            if (device_id == 0x8101)
            //            {
            //                qDebug() << "*******************************************";
            //                qDebug() << data.toHex();
            //                qDebug() << "*******************************************";
            //            }

            tcp_socket->write(data);
        },
        Qt::QueuedConnection);
}

void StatusTcpServer::start() { listen(QHostAddress::Any, 1234); }

void StatusTcpServer::stop() { close(); }
