#include "NumberStatusLabel.h"
#include "GlobalData.h"
#include "TaskPlanMessageSerialize.h"

NumberStatusLabel::NumberStatusLabel(const QDomElement& domEle, QWidget* parent)
    : StatusLabel(domEle, parent)
{
    parseNode(domEle);
}

void NumberStatusLabel::setValue(const QVariant& value)
{
    // 去掉小数后多于的0
    QRegExp rx(R"((\.){0,1}0+$)");
    auto valueString = QString("%1").arg(value.toDouble(), 0, 'f', 3).replace(rx, "");
    setText(valueString);
    setStatus(Normal);
}

QVariant NumberStatusLabel::getValue() const { return QVariant(); }

void NumberStatusLabel::setSpacialData(const QString& value) { dealSpcialData(); }

QString NumberStatusLabel::getSpacialData() const { return QString(); }

QString NumberStatusLabel::getSatePolarString(const SystemOrientation& orientation)
{
    if (orientation == SystemOrientation::LCircumflex)
    {
        return QString("左旋");
    }
    else if (orientation == SystemOrientation::RCircumflex)
    {
        return QString("右旋");
    }
    else if (orientation == SystemOrientation::LRCircumflex)
    {
        return QString("左右旋同时");
    }
    else
    {
        return QString("-");
    }
}

void NumberStatusLabel::dealPolarGetWay(const LinkLine& link, QString& sendPolar, QString& receivePolar)
{
    auto taskCode = link.targetMap[link.masterTargetNo].taskCode;
    auto point = link.targetMap[link.masterTargetNo].pointFreqNo;
    auto satelite = m_taskCodeSateliteMap[taskCode];
    //发射极化
    SystemOrientation orientation;
    satelite.getLaunchPolar(link.workMode, point, orientation);
    sendPolar = getSatePolarString(orientation);
    //接收极化
    satelite.getSReceivPolar(link.workMode, point, orientation);
    receivePolar = getSatePolarString(orientation);
}

void NumberStatusLabel::dealSpcialData()
{
    m_taskCodeSateliteMap.clear();

    auto dataList = GlobalData::getSatelliteManagementDataByObject();

    if (!dataList.isEmpty())
    {
        for (auto item : dataList)
        {
            m_taskCodeSateliteMap[item.m_satelliteCode] = item;
        }
    }

    //从redis取任务计划的信息，看这个任务是否完结了，如果完结了就重置界面，没有完结就按照当前任务的运行状态来处理
    auto manualMsg = GlobalData::getTaskRunningInfoByObject();
    if (!manualMsg.linkLineMap.isEmpty())
    {
        for (const auto& link : manualMsg.linkLineMap)
        {
            //找到S测控的信息并获得极化数据
            if (SystemWorkModeHelper::isMeasureContrlWorkMode(link.workMode) && link.workMode != KaKuo2)
            {
                dealPolarGetWay(link, m_sSendPolar, m_sReceivePolar);
            }
            // Ka测控
            if (link.workMode == KaKuo2)
            {
                dealPolarGetWay(link, m_kaCKSendPolar, m_kaCKReceivePolar);
            }
        }
        if (manualMsg.linkLineMap.contains(KaGS))
        {
            auto link = manualMsg.linkLineMap[KaGS];
            dealPolarGetWay(link, m_kaSCSendPolar, m_kaSCReceivePolar);
        }
        if (manualMsg.linkLineMap.contains(KaDS))
        {
            auto link = manualMsg.linkLineMap[KaDS];
            dealPolarGetWay(link, m_kaSCSendPolar, m_kaSCReceivePolar);
        }
        if (manualMsg.linkLineMap.contains(KaGS) && manualMsg.linkLineMap.contains(KaDS))
        {
            auto linkData = manualMsg.linkLineMap[KaGS];
            auto taskCode = linkData.targetMap[linkData.masterTargetNo].taskCode;  //高低速都是同一个任务代号，取一个就行
            auto satelite = m_taskCodeSateliteMap[taskCode];
            //高速和低速存在时需要判断跟踪方式来选择旋向
            auto trackWay = satelite.m_trackingMode;
            if (trackWay == _4426_KaGSDT || trackWay == _4426_STTC_KaGSDT || trackWay == _4426_STTC_KaCK_KaGSDT || trackWay == _4426_SK2_KaGSDT ||
                trackWay == _4426_SK2_KaCK_KaGSDT)
            {
                dealPolarGetWay(linkData, m_kaSCSendPolar, m_kaSCReceivePolar);
            }
            else
            {
                auto linkDSData = manualMsg.linkLineMap[KaDS];
                dealPolarGetWay(linkDSData, m_kaSCSendPolar, m_kaSCReceivePolar);
            }
        }
    }
    else
    {
        m_sSendPolar.clear();
        m_kaCKSendPolar.clear();
        m_kaSCSendPolar.clear();
        m_sReceivePolar.clear();
        m_kaCKReceivePolar.clear();
        m_kaSCReceivePolar.clear();
    }

    auto controalName = this->objectName();
    if (controalName.contains("SpacialData_SEmissPolar"))  // S发射极化
    {
        setText(m_sSendPolar.isEmpty() ? "一" : m_sSendPolar);
    }
    else if (controalName.contains("SpacialData_KaCKEmissPolar"))  // Ka测控发射极化
    {
        setText(m_kaCKSendPolar.isEmpty() ? "一" : m_kaCKSendPolar);
    }
    else if (controalName.contains("SpacialData_KaSCEmissPolar"))  // Ka数传发射极化
    {
        setText(m_kaSCSendPolar.isEmpty() ? "一" : m_kaSCSendPolar);
    }
    else if (controalName.contains("SpacialData_SReceivPolar"))  // S接收极化
    {
        setText(m_sReceivePolar.isEmpty() ? "一" : m_sReceivePolar);
    }
    else if (controalName.contains("SpacialData_KaCKReceivPolar"))  // Ka测控接收极化
    {
        setText(m_kaCKReceivePolar.isEmpty() ? "一" : m_kaCKReceivePolar);
    }
    else if (controalName.contains("SpacialData_KaSCReceivPolar"))  // Ka数传接收极化
    {
        setText(m_kaSCReceivePolar.isEmpty() ? "一" : m_kaSCReceivePolar);
    }
}

void NumberStatusLabel::parseNode(const QDomElement& domEle)
{
    if (domEle.isNull())
    {
        return;
    }
}
