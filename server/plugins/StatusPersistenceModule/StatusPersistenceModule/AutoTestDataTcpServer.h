#ifndef AUTOTESTDATATCPSERVER_H
#define AUTOTESTDATATCPSERVER_H

#include <QTcpServer>

class AutoTestDataTcpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit AutoTestDataTcpServer(QObject* parent = nullptr);

signals:
    void asynStart();
    void asynStop();
    void autoTestDataReceived(const QString& json);

protected:
    virtual void incomingConnection(qintptr socketDescriptor) override;

private:
    void start();
    void stop();
};

#endif  // AUTOTESTDATATCPSERVER_H
