#include "StarMapThread.h"
#include "CustomPacketMessageSerialize.h"
#include "DeviceProcessMessageSerialize.h"
#include "ProtocolXmlTypeDefine.h"
#include "RedisHelper.h"

StarMapThread::StarMapThread() {qRegisterMetaType<QVector<QPointF>>("const QVector<QPointF>&");}

void StarMapThread::stopRunning() { m_isRunning = false; }

void StarMapThread::setStarMapData(int deviceID, int channel)
{
    mDeviceId = deviceID;
    mChannel = channel;
}

void StarMapThread::run()
{
    while (m_isRunning)
    {
        CmdRequest result;
        QString pointInfo;

        QString key = QString("StarMap_%1_%2").arg(mDeviceId).arg(mChannel);

        RedisHelper::getInstance().getData(key, pointInfo);
        pointInfo >> result;

        mAllPoints.clear();
        int count = result.m_paramdataMap.value("count").toInt();
        for (int i = 0; i < count; i++)
        {
            float x = result.m_paramdataMap.value(QString("x_%1").arg(QString::number(i))).toDouble();
            float y = result.m_paramdataMap.value(QString("y_%1").arg(QString::number(i))).toDouble();
            mAllPoints.append(QPointF(x, y));
        }

        emit signalRefreshUI(mAllPoints);
        QThread::sleep(1);
    }
}
