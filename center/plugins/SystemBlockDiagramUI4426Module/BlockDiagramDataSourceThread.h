#ifndef BLOCKDIAGRAMDATASOURCETHREAD_H
#define BLOCKDIAGRAMDATASOURCETHREAD_H

#include "BlockDiagramDefine.h"
#include "CustomPacketMessageDefine.h"
#include "GlobalData.h"
#include <QObject>
#include <QThread>
#include <atomic>

struct DeviceStatusTypeInfo;
class BlockDiagramDataSourceThread : public QThread
{
    Q_OBJECT
public:
    BlockDiagramDataSourceThread();

    void stopRunning();
    void setSystemBlockInfo(SystemBlockInfo* blockInfo, DeviceReportMessageMap* currentDataMap, QMutex* refreshMutex);

signals:
    void sg_refresh();

protected:
    void run() override;

private:
    std::atomic<bool> m_isRunning{ true };
    QMutex* m_refreshMutex = nullptr;
    DeviceReportMessageMap* m_currentDataMap = nullptr;
    SystemBlockInfo* m_blockInfo = nullptr;
};
#endif  // BLOCKDIAGRAMDATASOURCETHREAD_H
