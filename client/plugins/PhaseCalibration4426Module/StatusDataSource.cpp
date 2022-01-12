#include "StatusDataSource.h"
#include "CustomPacketMessageDefine.h"
#include "GlobalData.h"

class StatusDataSourceImpl
{
public:
    int timerID{ -1 };

    static bool getACUMainPreparation(ExtensionStatusReportMessage& message);
};
bool StatusDataSourceImpl::getACUMainPreparation(ExtensionStatusReportMessage& message)
{
    /* 设备状态 */
    message = GlobalData::getExtenStatusReportData(0x01, 0x00, 0x11);
    if (message.modeId <= 0)
    {
        return false;
    }
    /*
     * 主备信息在第一个单元
     * key是OnLineMachine
     */
    auto& extenReportMap = message.unitReportMsgMap;
    auto find = extenReportMap.find(0X01);
    if (find == extenReportMap.end())
    {
        return false;
    }

    auto& reportMap = find->paramMap;
    auto valueFind = reportMap.find("OnLineMachine");
    if (valueFind == reportMap.end())
    {
        return false;
    }

    //<entry svalue="0" uvalue="0" desc="无此参数" />
    //<entry svalue="1" uvalue="1" desc="在线" />
    //<entry svalue="2" uvalue="2" desc="备份" />
    /* 0x11是主机 */
    if (valueFind->toInt() == 1)
    {
        return true;
    }

    /* 否则0x12是主机 */
    message = GlobalData::getExtenStatusReportData(0x01, 0x00, 0x12);
    return message.modeId > 0;
}

/*********************************************************************************************************/
/*********************************************************************************************************/
/*********************************************************************************************************/
/*********************************************************************************************************/
/*********************************************************************************************************/

StatusDataSource::StatusDataSource()
    : m_impl(new StatusDataSourceImpl)
{
    connect(this, &StatusDataSource::sg_start, this, &StatusDataSource::start);
    connect(this, &StatusDataSource::sg_stop, this, &StatusDataSource::stop);

    qRegisterMetaType<ExtensionStatusReportMessage>("ExtensionStatusReportMessage");
    qRegisterMetaType<ExtensionStatusReportMessage>("ExtensionStatusReportMessage&");
    qRegisterMetaType<ExtensionStatusReportMessage>("const ExtensionStatusReportMessage&");
}
StatusDataSource::~StatusDataSource() { delete m_impl; }
void StatusDataSource::start()
{
    if (m_impl->timerID == -1)
    {
        m_impl->timerID = startTimer(1500);
    }
}
void StatusDataSource::stop()
{
    if (m_impl->timerID != -1)
    {
        killTimer(m_impl->timerID);
        m_impl->timerID = -1;
    }
}

void StatusDataSource::timerEvent(QTimerEvent* /*event*/)
{
    /* 这里只获取ACU的数据 */
    ExtensionStatusReportMessage reportMsg;
    if (!StatusDataSourceImpl::getACUMainPreparation(reportMsg))
    {
        emit sig_sendTFS(false, ExtensionStatusReportMessage());
        return;
    }
    emit sig_sendTFS(false, reportMsg);
}
