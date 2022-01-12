#ifndef FEPSENDUNIT_H
#define FEPSENDUNIT_H

#include <QObject>
#include <QString>
#include <QTcpSocket>

enum class SendState;
class FepSendUnitImpl;
class FepSendUnit : public QObject
{
    Q_OBJECT
public:
    explicit FepSendUnit(QObject* parent = nullptr);
    ~FepSendUnit() override;
    /* 设置服务器的地址信息,在服务启动后此函数调用无效 */
    void setServerAddr(const QString& dstIP, quint16 dstPort);
    /* 设置工作目录信息,在服务启动后此函数调用无效 */
    void setWorkDir(const QString& openDir);
    /* 发送指定文件 */
    bool startSendFile(const QString& filePath, QString& errMsg);
    bool start(QString& errMsg);
    void stop();

signals:
    void sg_connectToHost(const QString& hostName, quint16 port);
    void sg_error(const QString&);
    void sg_msg(bool, const QString&);

private:
    void stateSwitch(SendState state, const QString& errMsg = QString());
    void readyRead();
    void errorMsg(QAbstractSocket::SocketError);
    void sendData(const QByteArray& sendData);
    /* 参数是否异常断开 */
    void disconnectLink(bool abort);
    /* 定时器超时 */
    void timerEvent(QTimerEvent* event) override;

private:
    FepSendUnitImpl* m_impl;
};

#endif  // FEPSENDUNIT_H
