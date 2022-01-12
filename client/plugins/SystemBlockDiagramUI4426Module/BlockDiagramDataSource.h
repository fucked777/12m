#ifndef BLOCKDIAGRAMDATASOURCE_H
#define BLOCKDIAGRAMDATASOURCE_H

#include "BlockDiagramDataSourceThread.h"
#include "CustomPacketMessageDefine.h"
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
    void setSystemBlockInfo(SystemBlockInfo* blockInfo, DeviceReportMessageMap* currentDataMap, QMutex* refreshMutex);
    enum ItemType
    {
        TwoValueSwitchType = 0,
        DeviceStatusType,
        BorderColorType
    };

signals:
    /*
     * 发送监视的参数的数据
     * id 对应界面item的ID
     * status对应Item的状态参数
     */
    void sg_sendParamData(const QString& id, const QVariant& status, const int type);
    void sg_start(const SystemBlockInfo&);

public slots:
    void slotRefreshData();

private slots:
    void slotRefresh();

private:
    BlockDiagramDataSourceImpl* m_impl;
};

#endif  // BLOCKDIAGRAMDATASOURCE_H
