#include "BlockDiagramDataSourceThread.h"
#include <QMutexLocker>

BlockDiagramDataSourceThread::BlockDiagramDataSourceThread() {}

void BlockDiagramDataSourceThread::stopRunning() { m_isRunning = false; }

void BlockDiagramDataSourceThread::setSystemBlockInfo(SystemBlockInfo* blockInfo, DeviceReportMessageMap* currentDataMap, QMutex* refreshMutex)
{
    m_refreshMutex = refreshMutex;
    m_currentDataMap = currentDataMap;
    m_blockInfo = blockInfo;
}

void BlockDiagramDataSourceThread::run()
{
    while (m_isRunning)
    {
        m_refreshMutex->lock();
        m_currentDataMap->clear();
        for (auto& item : m_blockInfo->deviceStatusTypeMap)
        {
            if (!m_currentDataMap->contains(item.redisKey))
            {
                (*m_currentDataMap)[item.redisKey] = GlobalData::getExtenStatusReportDataByObject(item.redisKey);
            }
        }
        m_refreshMutex->unlock();
        emit sg_refresh();

        QThread::sleep(1);
    }
}
