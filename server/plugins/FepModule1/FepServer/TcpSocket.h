#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include <QTcpSocket>
#include <QThread>

enum class FepRecvState;
class TcpSocketImpl;
class TcpSocket : public QThread
{
    Q_OBJECT
public:
    TcpSocket(const QString& workDir, QObject* parent = nullptr);
    ~TcpSocket() override;
    bool startFep(qintptr handle, const QMap<QString, QVariantMap>& ipCheckData = {});
    void stopFep();
signals:
    void sg_destroyed();
    void sg_error(const QString&);
    /* 文件接收完成后触发的信号 */
    void sg_oneFileRecvSuccess(const QVariantMap&);

private:
    void run() override;

private:
    void disconnectLink(bool abort);
    void readyRead();
    void errorMsg(QAbstractSocket::SocketError /*sockerError*/);
    void stateSwitch(FepRecvState state, const QString& msgError);
    void sendData(const QByteArray& sendData);
    void timerEvent(QTimerEvent* event) override;

private:
    TcpSocketImpl* m_impl;
};

#endif  // TcpSocket
