#ifndef FEPTCPSERVER_H
#define FEPTCPSERVER_H

#include <QTcpServer>

class FepTcpServer : public QTcpServer
{
    Q_OBJECT
public:
    FepTcpServer(QObject* parent = nullptr);
    ~FepTcpServer() override;

signals:
    /* 新链接 */
    void sg_newConnect(qintptr handle);

private:
    void incomingConnection(qintptr handle) override;
};

#endif  // FEPTCPSERVER_H
