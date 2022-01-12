#ifndef BLOCKDIAGRAMDATASOURCE_H
#define BLOCKDIAGRAMDATASOURCE_H

#include "ProtocolXmlTypeDefine.h"
#include <QMutex>
#include <QObject>

/* 框图的数据源 */
struct SystemBlockInfo;
class BlockDiagramDataSourceImpl;
class BlockDiagramDataSource : public QObject
{
    Q_OBJECT
public:
    BlockDiagramDataSource();
    ~BlockDiagramDataSource();
    /* 直接调用就是在本地线程,发信号是使用movethread后多线程准备的 */
    void start(const SystemBlockInfo&);
    void stop();

    enum ItemType
    {
        TwoValueSwitchType = 0,
        DeviceStatusType,
        BorderColorType
    };

private:
    void timerEvent(QTimerEvent* event) override;  //定时检测参数更新的时间戳

signals:
    /*
     * 发送监视的参数的数据
     * id 对应界面item的ID
     * status对应Item的状态参数
     */
    void sg_sendParamData(const QString& id, const QVariant& status, const int type);
    void sg_start(const SystemBlockInfo&);
    void sg_stop();
    void sg_refresh();

private slots:
    void slotRefresh();

private:
    BlockDiagramDataSourceImpl* m_impl;
    QMutex m_refreshMutex;
    QMap<QString, ExtensionStatusReportMessage> currentDataMap;
};

#endif  // BLOCKDIAGRAMDATASOURCE_H
