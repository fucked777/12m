//#include "ParseStatusPack.h"
//#include "GlobalConfigMacroData.h"
//#include "GlobalSystemData.h"
//#include "LxHelper.h"
//#include "PCDefine.h"
//#include <QMutex>
//#include <QMutexLocker>
//#include <QSet>
//#include <functional>

// class ParseStatusPackImpl
//{
// public:
//    int id{ -1 };
//    int curTFSPCMode{ 3 };
//    static bool getACUMainPreparation(const UnpackMessage& unpackMessage);
//    static bool getACUMainPack(UnpackMessage& unpackMessage, quint64& deviceID, bool& online);
//    static int getACUPCMode(const UnpackMessage& unpackMessage);
//};
// bool ParseStatusPackImpl::getACUMainPreparation(const UnpackMessage& unpackMessage)
//{
//    /*
//     * 主备信息在第一个单元
//     * key是OnLineMachine
//     */
//    auto& extenReportMap = unpackMessage.extenReportMap;
//    auto find = extenReportMap.find(0X01);
//    if (find == extenReportMap.end())
//    {
//        return false;
//    }

//    auto& reportMap = find->reportMap;
//    auto valueFind = reportMap.find("OnLineMachine");
//    if (valueFind == reportMap.end())
//    {
//        return false;
//    }
//    //<entry svalue="0" uvalue="0" desc="无此参数" />
//    //<entry svalue="1" uvalue="1" desc="在线" />
//    //<entry svalue="2" uvalue="2" desc="备份" />

//    return valueFind->toInt() == 1;
//}
// bool ParseStatusPackImpl::getACUMainPack(UnpackMessage& unpackMessage, quint64& deviceID, bool& isOnline)
//{
//    auto globalSystemData = getGlobalSystemInterface();
//    if (globalSystemData == nullptr)
//    {
//        return false;
//    }

//    /* 0x1011 0x1012 获取当前主用ACU的信息 */
//    int modeID = -1;
//    deviceID = 0x1011;
//    isOnline = globalSystemData->getUnpackMessageData(deviceID, modeID, unpackMessage);
//    bool main1 = ParseStatusPackImpl::getACUMainPreparation(unpackMessage);
//    if (main1)
//    {
//        return true;
//    }

//    deviceID = 0x1012;
//    isOnline = globalSystemData->getUnpackMessageData(deviceID, modeID, unpackMessage);
//    return true;
//}
// int ParseStatusPackImpl::getACUPCMode(const UnpackMessage& unpackMessage)
//{
//    /*
//     * 模式信息在ACU单元 2
//     * key是PhaseCorrMeth
//     * <entry svalue="1" uvalue="1" desc="对塔" />
//     * <entry svalue="2" uvalue="2" desc="对当前位置" />
//     * <entry svalue="3" uvalue="3" desc="对星" />
//     * 失败默认返回3
//     */
//    auto& extenReportMap = unpackMessage.extenReportMap;
//    auto find = extenReportMap.find(0X02);
//    if (find == extenReportMap.end())
//    {
//        return 3;
//    }

//    auto& reportMap = find->reportMap;
//    auto valueFind = reportMap.find("PhaseCorrMeth");
//    if (valueFind == reportMap.end())
//    {
//        return 3;
//    }

//    return valueFind->toInt();
//}

// ParseStatusPack::ParseStatusPack(QObject* parent)
//    : QObject(parent)
//    , m_impl(new ParseStatusPackImpl)
//{
//}

// ParseStatusPack::~ParseStatusPack()
//{
//    stop();
//    delete m_impl;
//}
// void ParseStatusPack::start()
//{
//    if (m_impl->id != -1)
//    {
//        return;
//    }
//    m_impl->id = startTimer(2000);
//}
// void ParseStatusPack::stop()
//{
//    if (m_impl->id != -1)
//    {
//        killTimer(m_impl->id);
//        m_impl->id = -1;
//    }
//}
// int ParseStatusPack::getTFSPCModle()
//{
//    UnpackMessage unpackMessage;
//    quint64 deviceID = 0;
//    bool isOnline = false;

//    ParseStatusPackImpl::getACUMainPack(unpackMessage, deviceID, isOnline);
//    return ParseStatusPackImpl::getACUPCMode(unpackMessage);
//}

///* 定时刷新数据 */
// void ParseStatusPack::timerEvent(QTimerEvent* /*event*/)
//{
//    UnpackMessage unpackMessage;
//    quint64 deviceID = 0;
//    bool isOnline = false;
//    if (!ParseStatusPackImpl::getACUMainPack(unpackMessage, deviceID, isOnline))
//    {
//        return;
//    }

//    auto globalSystemData = getGlobalSystemInterface();
//    if (globalSystemData == nullptr)
//    {
//        stop();
//        return;
//    }
//    emit sig_sendTFS(deviceID, 0xFFFF, isOnline, unpackMessage);
//}
