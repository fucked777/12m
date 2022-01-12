#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include <QObject>
#include <QTcpSocket>

struct RecvContent;
enum class RecvState;
class TcpSocketImpl;
class TcpSocket : public QObject
{
    Q_OBJECT
public:
    TcpSocket(const QString& workDir, QObject* parent = nullptr);
    ~TcpSocket() override;
    void start(qintptr handle);
    void stop();
signals:
    void sg_destroyed();
    void sg_error(const QString&);

private:
    void disconnectLink(bool abort);
    void readyRead();
    void errorMsg(QAbstractSocket::SocketError /*sockerError*/);
    void stateSwitch(RecvState state, const QString& msgError);
    void sendData(const QByteArray& sendData);
    void timerEvent(QTimerEvent* event) override;

private:
    TcpSocketImpl* m_impl;
};

#endif  // TcpSocket
