#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>

class TcpServer : public QTcpServer
{
    Q_OBJECT
public:
    TcpServer(QObject* parent = nullptr);
    ~TcpServer() override;

signals:
    /* 新链接 */
    void sg_newConnect(qintptr handle);

private:
    void incomingConnection(qintptr handle) override;
};

#endif  // TCPSERVER_H
