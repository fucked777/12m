#ifndef STATUSTCPSERVER_H
#define STATUSTCPSERVER_H

#include <QObject>
#include <QTcpServer>

class QTcpSocket;
class QTimer;

class StatusTcpServer : public QTcpServer

{
    Q_OBJECT
public:
    explicit StatusTcpServer(QObject *parent = nullptr);

signals:
    void asynStart();
    void asynStop();
    void statusResultReceived(int device, const QString &json);

protected:
    virtual void incomingConnection(qintptr socketDescriptor) override;

private:
    void start();
    void stop();
};

#endif  // STATUSTCPSERVER_H
