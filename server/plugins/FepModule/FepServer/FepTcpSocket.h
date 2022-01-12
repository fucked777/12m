#ifndef FEPTCPSOCKET_H
#define FEPTCPSOCKET_H

#include <QTcpSocket>
#include <QThread>

enum class FepRecvState;
class QFileInfo;
struct FepSendREQ;
struct FepData;
class FepTcpSocketImpl;
class FepTcpSocket : public QThread
{
    Q_OBJECT
public:
    FepTcpSocket(const QString& workDir, const QString& desDir, QObject* parent = nullptr);
    ~FepTcpSocket() override;
    bool startFep(qintptr handle, const QMap<QString, QVariantMap>& ipCheckData = {});
    void stopFep();
signals:
    /* 文件接收完成后触发的信号 */
    void sg_oneFileRecvSuccess(const QVariantMap&);

private:
    /* 日志输出 */
    void pushInfoLog(const QString& logStr);
    void pushWarningLog(const QString& logStr);
    void pushErrorLog(const QString& logStr);
    void pushSpecificTipLog(const QString& logStr);

    void run() override;
    /* 接收数据处理 */
    bool recvDataHandle(QTcpSocket& tcpLink);
    /* 处理发送请求包 */
    bool recvSendREQHandle(QTcpSocket& tcpLink, const QByteArray& rawData);
    /* 文件存在检查 */
    bool fileExistsOpt(QTcpSocket& tcpLink, QFileInfo& info, const FepSendREQ& pack);
    /* 处理数据包 */
    bool recvFepDataHandle(QTcpSocket& tcpLink, const QByteArray& rawData);
    /* 发送完成包 */
    bool sendFinishPack(QTcpSocket& tcpLink);

    /* 状态切换 */
    void stateSwitch(FepRecvState state, const QString& msgError = QString());
    /* 数据发送  */
    bool sendData(QTcpSocket& tcpLink, const QByteArray& sendData);
    bool init(QTcpSocket& tcpLink);

private:
    FepTcpSocketImpl* m_impl;
};

#endif  // TcpSocket
