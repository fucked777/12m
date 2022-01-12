#ifndef FEPSERVER_H
#define FEPSERVER_H

#include <QObject>

class FepServerImpl;
class FepServer : public QObject
{
    Q_OBJECT
public:
    explicit FepServer(QObject* parent = nullptr);
    ~FepServer() override;
    /* 设置服务器的地址信息,在服务启动后此函数调用无效 */
    void setPort(quint16 dstPort);
    /* 设置工作目录信息,在服务启动后此函数调用无效 */
    void setWorkDir(const QString& openDir);
    /* 默认端口5555 目录为当前目录下的FepRecv */
    bool start(QString& errMsg);
    void stop();

signals:
    void sg_newConnectStart(qintptr handle);

private:
    void doNewClientNew(qintptr handle);
    void timerEvent(QTimerEvent* /*event*/) override;
    //    void sendData(QString SrcIp, QByteArray sendData);
    //    void readNetworkData();

private:
    FepServerImpl* m_impl;
};

#endif  // FEPSERVER_H
