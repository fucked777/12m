#include "DeviceParamWidget.h"

#include <QDebug>
#include <QString>
#include <QVBoxLayout>

#include "CheckBox.h"
#include "ComplexCheckBox.h"
#include "CustomPacketMessageDefine.h"
#include "DeviceParamWidgetGenerator.h"
#include "MainScreenControlFactory.h"
#include "PlatformObjectTools.h"
#include "ProtocolXmlTypeDefine.h"
#include <QThread>
#include <QtConcurrent>

static const int CK_BBE_A = 0x4001;
static const int CK_BBE_B = 0x4002;
static const int CK_BBE_C = 0x4003;
static const int CK_BBE_D = 0x4004;
static const int longTimeInterval = 5;
static const int CK_ModeID_SKuo2 = SystemWorkModeHelper::systemWorkModeConverToModeId(Skuo2);
static const int CK_ModeID_KaKuo2 = SystemWorkModeHelper::systemWorkModeConverToModeId(KaKuo2);

uint DeviceParamWidget::staticCount = 0;
uint DeviceParamWidget::dynamicCount = 0;

DeviceParamWidget::DeviceParamWidget(QWidget* parent)
    : QWidget(parent)
{
    auto vLayout = new QVBoxLayout;
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);
    setLayout(vLayout);

    vLayout->addWidget(DeviceParamWidgetGenerator::getStatusWidget(this));

    bool res = false;
    QList<QWidget*> widgets = DeviceParamWidgetGenerator::getControlWidget();
    for (QWidget* widget : widgets)
    {
        ComplexCheckBox* checkBox = dynamic_cast<ComplexCheckBox*>(widget);
        if (checkBox)
        {
            res = connect(checkBox, &ComplexCheckBox::signalsCmdDeviceJson, this, &DeviceParamWidget::signalsCmdDeviceJson);
            res = connect(checkBox, &ComplexCheckBox::signalsUnitDeviceJson, this, &DeviceParamWidget::signalsUnitDeviceJson);
        }
    }

    startTimer(1000);
    connect(this, &DeviceParamWidget::signalRefreshStaticUI, this, &DeviceParamWidget::slotsRefreshStatic);
    connect(this, &DeviceParamWidget::signalRefreshDynamicUI, this, &DeviceParamWidget::slotsRefreshDynamic);
}

DeviceParamWidget::~DeviceParamWidget() {}

QString DeviceParamWidget::getSBandDownFreqId_24(int modeId) const
{
    switch (modeId)
    {
    case 0x1: return "SBandDownFreq";
    case 0x4: return "K2_SBandDownFreq";
    case 0x10: return "SMJ_SBandDownFreq";
    case 0x11: return "WX_SBandDownFreq";
    case 0x12: return "GML_SBandDownFreq";
    case 0x13: return "K2_SBandDownFreq";
    case 0x15: return "K2_SBandDownFreq";
    case 0x17: return "K2_SBandDownFreq";
    case 0x19: return "K2_SBandDownFreq";
    case 0x21: return "";
    default: return "";
    }
}

QVariant DeviceParamWidget::getSBandDownFreqValue_24(int deviceId, int modeId, int unitId, int onlineSta)
{
    QVariant value = -1234;
    if (onlineSta == 1)
    {
        QString paramId = getSBandDownFreqId_24(modeId);
        int targetId = isMultiTarget_24(modeId);
        switch (targetId)
        {
        case 0:
        {
            value = currentDataMap[deviceId].unitReportMsgMap[unitId].paramMap[paramId];
            break;
        }
        case 1:
        case 2:
        {
            value = currentDataMap[deviceId].unitReportMsgMap[unitId].multiTargetParamMap[targetId][paramId];
            break;
        }
        case 4:
        {
            int CurrTrackTarget = currentDataMap[deviceId].unitReportMsgMap[unitId].paramMap["CurrTrackTarget"].toInt();
            value = currentDataMap[deviceId].unitReportMsgMap[unitId].multiTargetParamMap[CurrTrackTarget][paramId];
            break;
        }
        case -1:
        {
            value = -1;
            break;
        }
        default: break;
        }
    }
    return value;
}

int DeviceParamWidget::isMultiTarget_24(int modeId) const
{
    switch (modeId)
    {
    case 0x1: return 0;
    case 0x4: return 4;
    case 0x10: return 1;
    case 0x11: return 1;
    case 0x12: return 1;
    case 0x13: return 2;
    case 0x15: return 2;
    case 0x17: return 2;
    case 0x19: return 2;
    case 0x21: return -1;
    default: return 0;
    }
}

void DeviceParamWidget::slotsRefreshStatic()
{
    QWidget* acuSendStatusWidget = nullptr;

    QWidget* dteSendStatusWidget = nullptr;
    auto widgetMap = MainScreenControlFactory::getWidgetMap();
    for (auto deviceID : widgetMap.keys())
    {
        if (!currentDataMap.contains(deviceID))
        {
            continue;
        }
        auto statusReportMsg = currentDataMap[deviceID];
        auto modeWidgetMap = widgetMap[deviceID];
        auto modeWidgetIter = modeWidgetMap.find(statusReportMsg.modeId);
        if (modeWidgetIter == modeWidgetMap.end())
        {
            continue;
        }

        auto unitWidgetMap = modeWidgetIter.value();
        for (auto unitId : unitWidgetMap.keys())
        {
            auto targetWidgetMap = unitWidgetMap[unitId];
            for (auto targetId : targetWidgetMap.keys())
            {
                auto paramWidgetMap = targetWidgetMap[targetId];
                for (auto paramId : paramWidgetMap.keys())
                {
                    auto widget = paramWidgetMap[paramId];
                    if (widget == nullptr)
                    {
                        continue;
                    }
                    if (paramId == "ACU_SendDataStat")
                    {
                        acuSendStatusWidget = widget;
                        continue;
                    }
                    if (paramId == "DET_SendDataStat")
                    {
                        dteSendStatusWidget = widget;
                        continue;
                    }
                    QVariant paramValue;
                    if (targetId == 0)
                    {
                        paramValue = statusReportMsg.unitReportMsgMap[unitId].paramMap[paramId];
                    }
                    else
                    {
                        paramValue = statusReportMsg.unitReportMsgMap[unitId].multiTargetParamMap[targetId][paramId];
                    }

                    if (paramValue.isValid())
                    {
                        widget->setProperty("value", paramValue);
                    }
                }
            }
        }
    }

    if (acuSendStatusWidget)
    {
        DeviceID acuDeviceIDA(1, 0, 17);
        auto statusReportMsgA = currentDataMap[acuDeviceIDA];
        //????????????????????????
        if (statusReportMsgA.unitReportMsgMap[2].paramMap.contains("AngleTranStatus"))
        {
            auto paramValue = statusReportMsgA.unitReportMsgMap[2].paramMap.value("AngleTranStatus");
            if (paramValue.isValid())
            {
                acuSendStatusWidget->setProperty("value", paramValue);
            }
        }

        DeviceID acuDeviceIDB(1, 0, 18);
        auto statusReportMsgB = currentDataMap[acuDeviceIDB];
        //????????????????????????
        if (statusReportMsgB.unitReportMsgMap[2].paramMap.contains("AngleTranStatus"))
        {
            auto paramValue = statusReportMsgB.unitReportMsgMap[2].paramMap.value("AngleTranStatus");
            if (paramValue.isValid())
            {
                acuSendStatusWidget->setProperty("value", paramValue);
            }
        }
    }
    //        if (dteSendStatusWidget)
    //        {
    //            DeviceID dteDeviceID(8, 0, 1);

    //            auto recycleReportMap = currentDataMap[dteDeviceID].unitReportMsgMap[1].multiTargetParamMap2;  // ??????????????????

    //            //???????????????????????????
    //            int currentKey = 7;
    //            if (recycleReportMap.contains(7))
    //            {
    //                auto currentRecycleReportMap = recycleReportMap.value(currentKey);

    //                QStringList idList;

    //                bool haveSend = false;

    //                for (int i = 0; i < currentRecycleReportMap.size(); ++i)
    //                {
    //                    auto valueMap = currentRecycleReportMap.at(i);

    //                    auto id = valueMap.value("CenterDataControlItemId").toString().toUpper();
    //                    auto isUing = valueMap.value("CenterDataControlItemIsUsing").toInt();
    //                    if (isUing)
    //                    {
    //                        int isSendTtcFirstRouter = valueMap.value("CenterDataControlItemIsSendTtcFirstRouter").toInt();
    //                        int isSendLinkFirstRouter = valueMap.value("CenterDataControlItemIsSendLinkFirstRouter").toInt();
    //                        int isSendTtcSecondRouter = valueMap.value("CenterDataControlItemIsSendTtcSecondRouter").toInt();
    //                        int isSendLinkSecondRouter = valueMap.value("CenterDataControlItemIsSendLinkSecondRouter").toInt();
    //                        if (isSendTtcFirstRouter == 2 || isSendLinkFirstRouter == 2 || isSendTtcSecondRouter == 2 || isSendLinkSecondRouter ==
    //                        2)
    //                        {
    //                            haveSend = true;
    //                        }

    //                        idList.append(id);
    //                    }
    //                }
    //                if (haveSend)
    //                {
    //                    dteSendStatusWidget->setProperty("value", 1);
    //                }
    //                else
    //                {
    //                    dteSendStatusWidget->setProperty("value", 0);
    //                }

    //                QVariant variantList1 = QVariant::fromValue<QStringList>(idList);
    //                dteSendStatusWidget->setProperty("idList", variantList1);
    //            }
    //        }

    auto multiStatusLabels = MainScreenControlFactory::getMultiStatusLabels();
    for (auto labelId : multiStatusLabels.keys())
    {
        QStringList list = labelId.split("_");
        if (list.size() == 4)
        {
            int deviceId = list.at(0).toInt();
            int modeId = list.at(1).toInt();
            int unitId = list.at(2).toInt();
            int targetId = list.at(3).toInt();

            DeviceID id;
            id << deviceId;
            int curModeId = GlobalData::getOnlineDeviceModeIdByObject(id);
            if (curModeId != modeId)
            {
                continue;
            }

            if (!currentDataMap.contains(deviceId))
            {
                continue;
            }
            auto statusReportMsg = currentDataMap[deviceId];
            if (statusReportMsg.modeId == -1)
            {
                continue;
            }

            QVariant unitValue;
            if (targetId == 0)
            {
                unitValue = QVariant(statusReportMsg.unitReportMsgMap[unitId].paramMap);
            }
            else
            {
                unitValue = QVariant(statusReportMsg.unitReportMsgMap[unitId].multiTargetParamMap[targetId]);
            }
            //??????????????????????????????????????????????????????????????????????????????????????????
            multiStatusLabels.value(labelId)->setProperty("unitValue", unitValue);
        }
    }
}

void DeviceParamWidget::slotsRefreshDynamic()
{
    auto dynamicParamWidget = MainScreenControlFactory::getDynamicParamWidget();
    for (QWidget* widget : dynamicParamWidget)
    {
        QString dealType = widget->property("dealType").toString();
        if (dealType == "24_Main_MACB_DnFreq")
        {
            int bbeA = dynamicDataMap[0x4001].unitReportMsgMap[1].paramMap["OnLineMachine"].toInt();
            int bbeB = dynamicDataMap[0x4002].unitReportMsgMap[1].paramMap["OnLineMachine"].toInt();
            int bbeC = dynamicDataMap[0x4003].unitReportMsgMap[1].paramMap["OnLineMachine"].toInt();
            int modeIdA = dynamicDataMap[0x4001].modeId;
            int modeIdB = dynamicDataMap[0x4002].modeId;
            int modeIdC = dynamicDataMap[0x4003].modeId;
            QVariant paramValue = -1;

            if (bbeA == 1)
            {
                paramValue = getSBandDownFreqValue_24(0x4001, modeIdA, 1, bbeA);
                widget->setProperty("value", paramValue);
                continue;
            }
            if (bbeB == 1)
            {
                paramValue = getSBandDownFreqValue_24(0x4002, modeIdB, 1, bbeB);
                widget->setProperty("value", paramValue);
                continue;
            }
            if (bbeC == 1)
            {
                paramValue = getSBandDownFreqValue_24(0x4003, modeIdC, 1, bbeC);
                widget->setProperty("value", paramValue);
                continue;
            }
        }
        if (dealType == "All_Decode_Led_Status")
        {
            // 1.????????????????????????,???????????????,????????????
            // 2.?????????????????????????????????????????????,???????????????????????????tooltip?????????????????????????????????
            // 3.??????????????????????????????tooltip??????????????????
            QString uniqueKey = widget->property("uniqueKey").toString();

            if (d_decodeInfos.contains(uniqueKey))
            {
                if (d_currentComplextDataMap.contains(uniqueKey))
                {
                    widget->setProperty("value", d_currentComplextDataMap.value(uniqueKey));
                }
            }
            else
            {
                LEDStatusDecodeLabel* ledLabel = dynamic_cast<LEDStatusDecodeLabel*>(widget);
                d_decodeInfos.insert(uniqueKey, ledLabel->getParamInfos());
            }
        }

        if (dealType == "All_Power_Number_Status")
        {
            QString uniqueKey = widget->property("uniqueKey").toString();

            if (d_powerInfos.contains(uniqueKey))
            {
                if (d_currentComplextDataMap.contains(uniqueKey))
                {
                    widget->setProperty("value", d_currentComplextDataMap.value(uniqueKey));
                }
            }
            else
            {
                NumberStatusPowerLabel* ledLabel = dynamic_cast<NumberStatusPowerLabel*>(widget);
                d_powerInfos.insert(uniqueKey, ledLabel->getParamInfos());
            }
        }
        if (dealType == "All_Stimu_Number_Status")
        {
            QString uniqueKey = widget->property("uniqueKey").toString();

            if (d_stimuInfos.contains(uniqueKey))
            {
                if (d_currentComplextDataMap.contains(uniqueKey))
                {
                    widget->setProperty("value", d_currentComplextDataMap.value(uniqueKey));
                }
            }
            else
            {
                LEDStatusStimuLabel* ledLabel = dynamic_cast<LEDStatusStimuLabel*>(widget);
                d_stimuInfos.insert(uniqueKey, ledLabel->getParamInfos());
            }
        }
        if (dealType == "All_Freq_Number_Status")
        {
            QString uniqueKey = widget->property("uniqueKey").toString();
            bool isMultiTarget = widget->property("isMultiTarget").toBool();
            QVariantList devids = widget->property("devids").toList();
            if (isMultiTarget)
            {
                if (d_multiTargetFreqInfos.contains(uniqueKey))
                {
                    if (d_currentComplextDataMap.contains(uniqueKey))
                    {
                        widget->setProperty("value", d_currentComplextDataMap.value(uniqueKey));
                    }
                }
                else
                {
                    NumberStatusFreqLabel* label = dynamic_cast<NumberStatusFreqLabel*>(widget);
                    d_multiTargetFreqInfos.insert(uniqueKey, label->getMultiTargetParamInfos());
                    d_currentComplexDevids.insert(uniqueKey, devids);
                }
            }
            else
            {
                if (d_freqInfos.contains(uniqueKey))
                {
                    if (d_currentComplextDataMap.contains(uniqueKey))
                    {
                        widget->setProperty("value", d_currentComplextDataMap.value(uniqueKey));
                    }
                }
                else
                {
                    NumberStatusFreqLabel* label = dynamic_cast<NumberStatusFreqLabel*>(widget);
                    d_freqInfos.insert(uniqueKey, label->getParamInfos());
                }
            }
        }
        if (dealType == "All_Rotation_Number_Status")
        {
            QString uniqueKey = widget->property("uniqueKey").toString();

            if (d_rotationInfos.contains(uniqueKey))
            {
                if (d_currentComplextDataMap.contains(uniqueKey))
                {
                    widget->setProperty("value", d_currentComplextDataMap.value(uniqueKey));
                }
            }
            else
            {
                NumberStatusRotationLabel* label = dynamic_cast<NumberStatusRotationLabel*>(widget);
                d_rotationInfos.insert(uniqueKey, label->getParamInfos());
            }
        }
        if (dealType == "All_Complex_Number_Status")
        {
            QString uniqueKey = widget->property("uniqueKey").toString();

            if (d_26TrkBBENumberInfos.contains(uniqueKey))
            {
                if (d_currentComplextDataMap.contains(uniqueKey))
                {
                    widget->setProperty("value", d_currentComplextDataMap.value(uniqueKey));
                }
            }
            else
            {
                ComplexNumberStatusLabel* label = dynamic_cast<ComplexNumberStatusLabel*>(widget);
                d_26TrkBBENumberInfos.insert(uniqueKey, label->getParamInfos());
            }
        }
        if (dealType == "All_Complex_LED_Status")
        {
            QString uniqueKey = widget->property("uniqueKey").toString();

            if (d_26TrkBBELedInfos.contains(uniqueKey))
            {
                if (d_currentComplextDataMap.contains(uniqueKey))
                {
                    widget->setProperty("value", d_currentComplextDataMap.value(uniqueKey));
                }
            }
            else
            {
                ComplexLEDStatusLabel* label = dynamic_cast<ComplexLEDStatusLabel*>(widget);
                d_26TrkBBELedInfos.insert(uniqueKey, label->getSumInfo());
            }
        }
    }
}
#include "MessagePublish.h"

void DeviceParamWidget::timerEvent(QTimerEvent* event)
{
    Q_UNUSED(event)
    if (!mIsShow)
    {
        return;
    }

    staticCount++;
    dynamicCount++;

    currentDataMap.clear();
    auto widgetMap = MainScreenControlFactory::getWidgetMap();
    for (auto deviceID : widgetMap.keys())
    {
        if (!GlobalData::getDeviceOnlineByObject(deviceID))
        {
            continue;
        }
        DeviceID tempDeviceID;
        tempDeviceID << deviceID;
        if (tempDeviceID.sysID != 4)
        {
            if (staticCount % tempDeviceID.sysID != 0 && staticCount % 3 == 0)
            {
                continue;
            }
        }
        else
        {
            if (staticCount % tempDeviceID.extenID)
            {
                continue;
            }
        }
        auto statusReportMsg = GlobalData::getExtenStatusReportDataByObject(deviceID);
        if (statusReportMsg.modeId == -1)
        {
            continue;
        }

        auto modeWidgetMap = widgetMap[deviceID];
        auto modeWidgetIter = modeWidgetMap.find(statusReportMsg.modeId);
        if (modeWidgetIter == modeWidgetMap.end())
        {
            continue;
        }
        currentDataMap[deviceID] = statusReportMsg;
    }

    auto multiStatusLabels = MainScreenControlFactory::getMultiStatusLabels();
    for (auto labelId : multiStatusLabels.keys())
    {
        QStringList list = labelId.split("_");
        if (list.size() == 4)
        {
            int deviceId = list.at(0).toInt();
            int modeId = list.at(1).toInt();
            int unitId = list.at(2).toInt();
            int targetId = list.at(3).toInt();

            DeviceID id;
            id << deviceId;
            int curModeId = GlobalData::getOnlineDeviceModeIdByObject(id);
            if (curModeId != modeId)
            {
                continue;
            }

            auto statusReportMsg = GlobalData::getExtenStatusReportDataByObject(deviceId);
            if (statusReportMsg.modeId == -1)
            {
                continue;
            }
            currentDataMap[deviceId] = statusReportMsg;
        }
    }

    // 26???????????????????????????,??????????????????????????????????????????
    for (auto deviceID : widgetMap.keys())
    {
        if (deviceID == CK_BBE_A || deviceID == CK_BBE_B || deviceID == CK_BBE_C || deviceID == CK_BBE_D)
        {
            ExtensionStatusReportMessage statusReportMessage = currentDataMap[deviceID];
            int modeId = statusReportMessage.modeId;
            if (modeId == CK_ModeID_SKuo2 || modeId == CK_ModeID_KaKuo2)
            {
                if (currentDataMap[deviceID].unitReportMsgMap.contains(1))
                {
                    UnitReportMessage unitReportMessage = currentDataMap[deviceID].unitReportMsgMap[1];
                    if (unitReportMessage.paramMap.contains("TargetNum"))
                    {
                        int targetNum = unitReportMessage.paramMap["TargetNum"].toInt();
                        for (auto unitId : currentDataMap[deviceID].unitReportMsgMap.keys())
                        {
                            for (int i = targetNum + 1; i <= 4; i++)
                            {
                                currentDataMap[deviceID].unitReportMsgMap[unitId].multiTargetParamMap[i].clear();
                            }
                        }
                    }
                }
            }
        }
    }

    slotsRefreshStatic();

    dynamicDataMap.clear();

    auto devidList = MainScreenControlFactory::getDynamicDevids();
    for (auto deviceID : devidList)
    {
        if (!GlobalData::getDeviceOnlineByObject(deviceID))
        {
            continue;
        }
        DeviceID tempDeviceID;
        tempDeviceID << deviceID;
        if (tempDeviceID.sysID != 4)
        {
            if (dynamicCount % tempDeviceID.sysID != 0 && dynamicCount % 3 == 0)
            {
                continue;
            }
        }
        else
        {
            if (dynamicCount % tempDeviceID.extenID)
            {
                continue;
            }
        }

        auto statusReportMsg = GlobalData::getExtenStatusReportDataByObject(deviceID);
        if (statusReportMsg.modeId == -1)
        {
            continue;
        }

        dynamicDataMap[deviceID] = statusReportMsg;
    }

    d_currentComplextDataMap.clear();

    for (QString uniqueKey : d_powerInfos.keys())
    {
        for (const NumberStatusPowerLabel::SumInfo info : d_powerInfos.value(uniqueKey))
        {
            auto& mainInfo = info.mainInfo;
            QString currentOnlineSta = dynamicDataMap[mainInfo.deviceId].unitReportMsgMap[mainInfo.unitId].paramMap[mainInfo.paramId].toString();
            QString mainDevSta = mainInfo.value;
            if (mainDevSta == currentOnlineSta)
            {
                auto& paramInfo = info.paramInfo;
                QVariant paramValue = dynamicDataMap[paramInfo.deviceId].unitReportMsgMap[paramInfo.unitId].paramMap[paramInfo.paramId];

                d_currentComplextDataMap[uniqueKey] = paramValue;
                break;
            }
        }
    }
    for (QString uniqueKey : d_decodeInfos.keys())
    {
        bool hasGreenLight = false;
        int count = 0;
        DeviceID id;
        for (const LEDStatusDecodeLabel::LEDParam& param : d_decodeInfos.value(uniqueKey))
        {
            QString cacheVal = dynamicDataMap[param.deviceId].unitReportMsgMap[param.unitId].paramMap[param.paramId].toString();
            QString normalVal = param.normalValue;
            id = param.deviceId;
            if (cacheVal == normalVal)
            {
                d_currentComplextDataMap[uniqueKey] = count;
                hasGreenLight = true;
                break;
            }
            count++;
        }

        if (GlobalData::getDeviceOnlineByObject(id))
        {
            if (!hasGreenLight)
            {
                d_currentComplextDataMap[uniqueKey] = -1;
            }
        }
        else
        {
            d_currentComplextDataMap[uniqueKey] = -2;
        }
    }

    for (QString uniqueKey : d_stimuInfos.keys())
    {
        for (const LEDStatusStimuLabel::SumInfo info : d_stimuInfos.value(uniqueKey))
        {
            auto& mainInfo = info.mainInfo;
            QString currentOnlineSta = dynamicDataMap[mainInfo.deviceId].unitReportMsgMap[mainInfo.unitId].paramMap[mainInfo.paramId].toString();
            QString mainDevSta = mainInfo.value;
            if (mainDevSta == currentOnlineSta)
            {
                auto& paramInfo = info.paramInfo;
                QVariant uValue = dynamicDataMap[paramInfo.deviceId].unitReportMsgMap[paramInfo.unitId].paramMap[paramInfo.paramId];
                if (uValue.isValid())
                {
                    QString sValue = info.ledInfo[uValue.toString()];
                    d_currentComplextDataMap[uniqueKey] = sValue;
                }
                break;
            }
        }
    }

    for (QString uniqueKey : d_multiTargetFreqInfos.keys())
    {
        QMap<int, QList<NumberStatusFreqLabel::MultiTargetSumInfo>> multiTargetInfos = d_multiTargetFreqInfos.value(uniqueKey);
        QVariantList devids = d_currentComplexDevids.value(uniqueKey);  //?????????ID
        bool fetch = false;                                             //?????????A????????????????????????????????????????????????A?????????B

        for (QVariant devid : devids)
        {
            if (fetch)
                continue;

            int modeId = dynamicDataMap[devid.toInt()].modeId;  //??????????????????
            if (multiTargetInfos.contains(modeId))
            {
                for (const NumberStatusFreqLabel::MultiTargetSumInfo& info : multiTargetInfos.value(modeId))
                {
                    auto& mainInfo = info.mainInfo;  //????????????
                    int configDeviceId = 0;
                    mainInfo.deviceId >> configDeviceId;
                    if (configDeviceId != devid)
                    {
                        continue;  //???????????????????????????????????????id
                    }

                    QString currentOnlineSta =
                        dynamicDataMap[mainInfo.deviceId].unitReportMsgMap[mainInfo.unitId].paramMap[mainInfo.paramId].toString();
                    QString mainDevSta = mainInfo.value;
                    if (mainDevSta == currentOnlineSta)  //??????value??????????????????1
                    {
                        auto& targetInfo = info.targetInfo;  //????????????

                        if (!(targetInfo.type == NumberStatusFreqLabel::Single_Type))  //???????????????multiTargetParamMap
                        {
                            auto& paramInfo = info.paramInfo;
                            QVariant paramValue;

                            if (targetInfo.type == NumberStatusFreqLabel::K2_Type)  //????????????targetInfo??????????????????
                            {
                                int targetId =
                                    dynamicDataMap[targetInfo.deviceId].unitReportMsgMap[targetInfo.unitId].paramMap[targetInfo.paramId].toInt();
                                paramValue = dynamicDataMap[paramInfo.deviceId]
                                                 .unitReportMsgMap[paramInfo.unitId]
                                                 .multiTargetParamMap[targetId][paramInfo.paramId];
                            }
                            if (targetInfo.type == NumberStatusFreqLabel::Multi_Type)  //???????????????????????????????????????multiTargetParamMap??????????????????
                            {
                                paramValue = dynamicDataMap[paramInfo.deviceId]
                                                 .unitReportMsgMap[paramInfo.unitId]
                                                 .multiTargetParamMap[paramInfo.targetId][paramInfo.paramId];
                            }

                            if (paramValue.isValid())  //??????????????????
                            {
                                d_currentComplextDataMap[uniqueKey] = paramValue;
                            }
                        }
                        else
                        {
                            auto& paramInfo = info.paramInfo;  //???????????????paramMap
                            QVariant paramValue = dynamicDataMap[paramInfo.deviceId].unitReportMsgMap[paramInfo.unitId].paramMap[paramInfo.paramId];

                            if (paramValue.isValid())
                            {
                                d_currentComplextDataMap[uniqueKey] = paramValue;
                            }
                        }
                        fetch = true;
                        break;
                    }
                }
            }
        }
    }

    for (QString uniqueKey : d_freqInfos.keys())
    {
        for (const NumberStatusFreqLabel::SumInfo info : d_freqInfos.value(uniqueKey))
        {
            auto& mainInfo = info.mainInfo;
            QString currentOnlineSta = dynamicDataMap[mainInfo.deviceId].unitReportMsgMap[mainInfo.unitId].paramMap[mainInfo.paramId].toString();
            QString mainDevSta = mainInfo.value;
            if (mainDevSta == currentOnlineSta)
            {
                auto& paramInfo = info.paramInfo;
                QVariant paramValue = dynamicDataMap[paramInfo.deviceId].unitReportMsgMap[paramInfo.unitId].paramMap[paramInfo.paramId];
                if (paramValue.isValid())
                {
                    d_currentComplextDataMap[uniqueKey] = paramValue;
                }
                break;
            }
        }
    }

    for (QString uniqueKey : d_rotationInfos.keys())
    {
        QList<NumberStatusRotationLabel::ParamInfo> paramInfos = d_rotationInfos.value(uniqueKey);

        QStringList valueList;
        for (const NumberStatusRotationLabel::ParamInfo info : paramInfos)
        {
            QString paramValue = dynamicDataMap[info.deviceId].unitReportMsgMap[info.unitId].paramMap[info.paramId].toString();

            if (info.mapping.contains(paramValue))
            {
                valueList.append(info.mapping.value(paramValue));
            }
        }
        if (valueList.size())
        {
            d_currentComplextDataMap[uniqueKey] = valueList.join(" / ");
        }
    }

    for (QString uniqueKey : d_26TrkBBENumberInfos.keys())
    {
        ComplexNumberStatusLabel::SumInfo sumInfo = d_26TrkBBENumberInfos.value(uniqueKey);
        ComplexNumberStatusLabel::MainInfo mainInfo = sumInfo.mainInfo;
        //????????????????????????????????????????????????????????????
        if (mainInfo.mainInfo.isValid) {}
        else
        {
            ComplexNumberStatusLabel::Info targetInfo = sumInfo.targetInfo;
            ComplexNumberStatusLabel::ParamInfo paramInfo = sumInfo.paramInfo;
            ComplexNumberStatusLabel::Info pInfo = paramInfo.paramInfo;
            int targetId = paramInfo.paramInfo.targetId;
            if (targetInfo.isValid)
            {
                QVariant var = dynamicDataMap[targetInfo.deviceId].unitReportMsgMap[targetInfo.unitId].paramMap[targetInfo.paramId];
                targetId = dynamicDataMap[targetInfo.deviceId].unitReportMsgMap[targetInfo.unitId].paramMap[targetInfo.paramId].toInt();
            }
            QVariant paramValue;
            if (paramInfo.isMulti)
            {
                paramValue = dynamicDataMap[pInfo.deviceId].unitReportMsgMap[pInfo.unitId].multiTargetParamMap[targetId][pInfo.paramId];
            }
            else
            {
                paramValue = dynamicDataMap[pInfo.deviceId].unitReportMsgMap[pInfo.unitId].paramMap[pInfo.paramId];
            }

            if (paramValue.isValid())
            {
                d_currentComplextDataMap[uniqueKey] = paramValue;
            }
        }
    }

    for (QString uniqueKey : d_26TrkBBELedInfos.keys())
    {
        ComplexLEDStatusLabel::SumInfo sumInfo = d_26TrkBBELedInfos.value(uniqueKey);
        ComplexLEDStatusLabel::MainInfo mainInfo = sumInfo.mainInfo;
        //????????????????????????????????????????????????????????????
        if (mainInfo.mainInfo.isValid) {}
        else
        {
            ComplexLEDStatusLabel::TargetInfo targetInfo = sumInfo.targetInfo;
            ComplexLEDStatusLabel::ParamInfo paramInfo = sumInfo.paramInfo;
            ComplexLEDStatusLabel::Info pInfo = paramInfo.paramInfo;
            ComplexLEDStatusLabel::Info tInfo = targetInfo.targetInfo;
            int targetId = paramInfo.paramInfo.targetId;
            if (tInfo.isValid)
            {
                targetId = dynamicDataMap[tInfo.deviceId].unitReportMsgMap[tInfo.unitId].paramMap[tInfo.paramId].toInt();
            }
            QVariant paramValue;
            if (paramInfo.isMulti)
            {
                paramValue = dynamicDataMap[pInfo.deviceId].unitReportMsgMap[pInfo.unitId].multiTargetParamMap[targetId][pInfo.paramId];
            }
            else
            {
                paramValue = dynamicDataMap[pInfo.deviceId].unitReportMsgMap[pInfo.unitId].paramMap[pInfo.paramId];
            }

            if (paramValue.isValid())
            {
                d_currentComplextDataMap[uniqueKey] = paramValue;
            }
        }
    }
    slotsRefreshDynamic();
}

void DeviceParamWidget::showEvent(QShowEvent* event)
{
    mIsShow = true;
    QWidget::showEvent(event);
}

void DeviceParamWidget::hideEvent(QHideEvent* event)
{
    mIsShow = false;
    QWidget::hideEvent(event);
}
