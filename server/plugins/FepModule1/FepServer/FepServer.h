#ifndef FEPSERVER_H
#define FEPSERVER_H

#include <QList>
#include <QObject>
#include <QString>
#include <QVariantMap>

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
    /*
     * 设置ip检查
     * 一切不在此数据内的IP都会被拒绝
     * 如果不设置或者为空则跳过检查
     * key为ip
     * value为附加数据
     * 在ip检查通过后,正常接收文件完成后此参数会直接通过信号槽sg_oneFileRecvSuccess发送
     * 此类不检查内容
     */
    void setIPCheck(const QMap<QString, QVariantMap>&);
    /* 默认端口5555 目录为当前目录下的FepRecv */
    bool start(QString& errMsg);
    void stop();

signals:
    void sg_newConnectStart(qintptr handle, const QMap<QString, QVariantMap>& ipCheckData = {});
    /* 文件接收完成后触发的信号
     * 参数内容为配置文件中的内容
     * 你写的啥这里就是啥
     */
    void sg_oneFileRecvSuccess(const QVariantMap&);

private:
    void doNewClientNew(qintptr handle);
    void timerEvent(QTimerEvent* /*event*/) override;
    //    void sendData(QString SrcIp, QByteArray sendData);
    //    void readNetworkData();

private:
    FepServerImpl* m_impl;
};

#endif  // FEPSERVER_H
