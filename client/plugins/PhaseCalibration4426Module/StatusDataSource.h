#ifndef STATUSDATASOURCE_H
#define STATUSDATASOURCE_H
#include <QObject>

/* 框图的数据源 */
struct ExtensionStatusReportMessage;
class StatusDataSourceImpl;
class StatusDataSource : public QObject
{
    Q_OBJECT
public:
    StatusDataSource();
    ~StatusDataSource();
    /* 直接调用就是在本地线程,发信号是使用movethread后多线程准备的 */
    void start();
    void stop();

private:
    void timerEvent(QTimerEvent* event) override;  //定时检测参数更新的时间戳

signals:
    void sig_sendTFS(bool, const ExtensionStatusReportMessage& message); /* 发送天线的数据 */
    void sg_start();
    void sg_stop();

private:
    StatusDataSourceImpl* m_impl;
};

#endif  // BLOCKDIAGRAMDATASOURCE_H
