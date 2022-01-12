#ifndef FEPSENDUNIT_H
#define FEPSENDUNIT_H

#include <QString>
#include <QTcpSocket>
#include <QThread>

enum class FepSendState;
struct FepSendFileInfo;
class FepSendUnitImpl;
class FepSendUnit : public QThread
{
    Q_OBJECT
public:
    explicit FepSendUnit(QObject* parent = nullptr);
    ~FepSendUnit() override;
    /* 设置服务器的地址信息,在服务启动后此函数调用无效 */
    void setServerAddr(const QString& dstIP, quint16 dstPort);
    /* 发送指定文件 */
    bool startSendFile(const QString& filePath, QString& errMsg);
    bool startSendDir(const QString& dirPath, QString& errMsg);
    void stopFep();

private:
    /* 日志输出 */
    void pushInfoLog(const QString& logStr);
    void pushWarningLog(const QString& logStr);
    void pushErrorLog(const QString& logStr);
    void pushSpecificTipLog(const QString& logStr);

    void run() override;
    bool sendFlow(QTcpSocket& tcpLink, FepSendFileInfo& currFile);

    /* 发送请求包 */
    FepSendState sendREQPack(QTcpSocket& tcpLink, const FepSendFileInfo& fileInfo);
    /* 接收请求应答 */
    FepSendState recvAnserREQ(QTcpSocket& tcpLink, FepSendFileInfo& fileInfo);
    /* 发送数据 */
    FepSendState sendDataPack(QTcpSocket& tcpLink, FepSendFileInfo& fileInfo);
    /* 接收结束应答 */
    FepSendState recvFepFinish(QTcpSocket& tcpLink);

    /* 发送数据 */
    FepSendState sendData(QTcpSocket& tcpLink, const QByteArray& sendBuffer);
signals:
    void sg_close();

private:
    FepSendUnitImpl* m_impl;
};

#endif  // FEPSENDUNIT_H
