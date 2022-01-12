#include "SystemCaptureProcessCmd.h"

#include "CustomPacketMessageDefine.h"
#include "DeviceProcessMessageSerialize.h"
#include "DopplerSet.h"
#include "GlobalData.h"
#include "MessagePublish.h"
#include "PlanRunMessageSerialize.h"
#include "PlatformConfigTools.h"
#include "PlatformInterface.h"
#include "ProtocolXmlTypeDefine.h"
#include "RedisHelper.h"
#include "RemoteWorkPeriod.h"
#include "ServiceCheck.h"
#include "SystemCaptureProcessControl.h"
#include "SystemWorkMode.h"

#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSpacerItem>
#include <QSpinBox>
#include <QVariantMap>

class SystemCaptureProcessXmlImpl
{
public:
    QList<CommandTemplate> d_cmd;
    int d_deviceId;
    int d_modeId;
    QComboBox* cmb_deviceId{ nullptr };
};

SystemCaptureProcessXml::SystemCaptureProcessXml()
    : mImpl(new SystemCaptureProcessXmlImpl())
{
    SystemCaptureProcessXmlReader reader;
    QString errorMsg;
    if (!reader.getCmdList(mImpl->d_cmd, &errorMsg))
    {
        QMessageBox::warning(nullptr, "提示", errorMsg, QString("确定"));
    }
}

void SystemCaptureProcessXml::setModeID(int modeID) { mImpl->d_modeId = modeID; }

void SystemCaptureProcessXml::start(QWidget* comBox_MacId, QWidget* widget, const QString& widgetName)
{
    mImpl->cmb_deviceId = dynamic_cast<QComboBox*>(comBox_MacId);

    if (widgetName == "SystemCaptureProcessControl")
    {
        if (widget == nullptr)
        {
            return;
        }
        SystemCaptureProcessControl* curWindow = dynamic_cast<SystemCaptureProcessControl*>(widget);

        for (CommandTemplate& temp : mImpl->d_cmd)
        {
            if (temp.cmdType == DevMsgType::ProcessControlCmd)
            {
                QObject* object = curWindow->findChild<QObject*>(temp.name);

                if (object == nullptr)
                {
                    continue;
                }

                if (object->inherits("QRadioButton")) {}
                else if (object->inherits("QPushButton"))
                {
                    QPushButton* pushButton = dynamic_cast<QPushButton*>(object);
                    connect(pushButton, &QPushButton::clicked, this, [=]() {
                        CmdRequest cmdRequest;
                        mImpl->d_deviceId = getCurrentDeviceId(mImpl->cmb_deviceId);
                        DeviceID id;
                        id << mImpl->d_deviceId;

                        if (temp.cmdIdOrUnitId.toInt() == 103)  //主备机切换
                        {
                            dealMasterSlaveSwitch(temp);
                            return;
                        }

                        cmdRequest.m_systemNumb = id.sysID;
                        cmdRequest.m_deviceNumb = id.devID;
                        cmdRequest.m_extenNumb = id.extenID;
                        cmdRequest.m_modeID = mImpl->d_modeId;
                        cmdRequest.m_cmdID = temp.cmdIdOrUnitId.toInt();
                        cmdRequest.m_paramdataMap = temp.staticParams;

                        if (temp.dynamicParams.size())
                        {
                            QVariantMap params = temp.dynamicParams;
                            for (QDomElement ctrlElem : temp.domELems)
                            {
                                QString tagName = ctrlElem.tagName();
                                QString paramId = ctrlElem.attribute("paramId");
                                if (tagName == "RadioButton")
                                {
                                    QDomElement childCtrlElem = ctrlElem.firstChildElement();
                                    QString childCtrlTagName = childCtrlElem.tagName();
                                    while (!childCtrlElem.isNull())
                                    {
                                        QRadioButton* radioButton = curWindow->findChild<QRadioButton*>(childCtrlElem.attribute("name"));
                                        if (radioButton->isChecked())
                                        {
                                            params[paramId] = childCtrlElem.attribute("value").toInt();
                                            break;
                                        }
                                        childCtrlElem = childCtrlElem.nextSiblingElement();
                                    }
                                }
                                if (tagName == "PopWindow")
                                {
                                    QString name = ctrlElem.attribute("name");
                                    if (name == "DopplerPreset")
                                    {
                                        DopplerSet dialog(nullptr, mImpl->d_deviceId, mImpl->d_modeId);
                                        connect(&dialog, &DopplerSet::signalsCmdDeviceJson, this, &SystemCaptureProcessXml::signalsCmdDeviceJson);
                                        dialog.exec();
                                        return;
                                    }
                                }
                                if (tagName == "Unit")
                                {
                                    auto statusReportMsg = GlobalData::getExtenStatusReportData(mImpl->d_deviceId);
                                    int targetId = 0, unitId = 0;
                                    if (ctrlElem.hasAttribute("targetId"))
                                        targetId = ctrlElem.attribute("targetId").toInt();
                                    if (ctrlElem.hasAttribute("unitId"))
                                        unitId = ctrlElem.attribute("unitId").toInt();
                                    QString unitParamId = ctrlElem.attribute("unitParamId");
                                    QVariant paramValue;
                                    if (targetId == 0)
                                    {
                                        paramValue = statusReportMsg.unitReportMsgMap[unitId].paramMap[unitParamId];
                                    }
                                    else
                                    {
                                        bool tNumCheck = targetNumCheck(mImpl->d_deviceId, targetId);
                                        if (!tNumCheck)
                                            return;
                                        paramValue = statusReportMsg.unitReportMsgMap[unitId].multiTargetParamMap[targetId][unitParamId];
                                    }
                                    params[paramId] = paramValue;
                                }
                            }

                            cmdRequest.m_paramdataMap.unite(params);
                        }
                        emit signalsCmdRequest(cmdRequest);
                    });
                }
            }
            else if (temp.cmdType == DevMsgType::UnitParameterSetCmd)
            {
                QObject* object = curWindow->findChild<QObject*>(temp.name);

                if (object->inherits("QRadioButton"))
                {
                    QRadioButton* radioButton = dynamic_cast<QRadioButton*>(object);
                    connect(radioButton, &QRadioButton::clicked, this, [=]() {
                        UnitParamRequest unitRequest;
                        mImpl->d_deviceId = getCurrentDeviceId(mImpl->cmb_deviceId);
                        DeviceID id;
                        id << mImpl->d_deviceId;

                        unitRequest.m_systemNumb = id.sysID;
                        unitRequest.m_deviceNumb = id.devID;
                        unitRequest.m_extenNumb = id.extenID;
                        unitRequest.m_modeID = mImpl->d_modeId;
                        unitRequest.m_unitID = temp.cmdIdOrUnitId.toInt();
                        unitRequest.m_paramdataMap = temp.staticParams;
                        auto statusReportMsg = GlobalData::getExtenStatusReportData(id);
                        int targetNum = statusReportMsg.unitReportMsgMap[1].paramMap["TargetNum"].toInt();
                        unitRequest.m_validIdent = targetNum + 1;  //扩频有效标识为当前目标数+1

                        if (temp.dynamicParams.size())
                        {
                            int targetId = 0;
                            QVariantMap params;
                            for (QDomElement ctrlElem : temp.domELems)
                            {
                                QString tagName = ctrlElem.tagName();
                                QString paramId = ctrlElem.attribute("paramId");
                                if (ctrlElem.hasAttribute("targetId"))
                                    targetId = ctrlElem.attribute("targetId").toInt();

                                if (tagName == "RadioButton")
                                {
                                    QDomElement childCtrlElem = ctrlElem.firstChildElement();
                                    QString childCtrlTagName = childCtrlElem.tagName();
                                    while (!childCtrlElem.isNull())
                                    {
                                        QRadioButton* radioButton = curWindow->findChild<QRadioButton*>(childCtrlElem.attribute("name"));
                                        if (radioButton->isChecked())
                                        {
                                            params[paramId] = childCtrlElem.attribute("value").toInt();
                                            break;
                                        }
                                        childCtrlElem = childCtrlElem.nextSiblingElement();
                                    }
                                }
                                if (tagName == "PopWindow") {}
                            }
                            if (targetId == 0)
                                unitRequest.m_paramdataMap.unite(params);
                            else
                            {
                                bool tNumCheck = targetNumCheck(mImpl->d_deviceId, targetId);
                                if (!tNumCheck)
                                    return;
                                unitRequest.m_multiTargetParamMap[targetId].m_paramdataMap.unite(params);
                            }
                        }

                        emit signalsUnitParamRequest(unitRequest);
                    });
                }
                else if (object->inherits("QPushButton"))
                {
                    QPushButton* pushButton = dynamic_cast<QPushButton*>(object);
                    connect(pushButton, &QRadioButton::clicked, this, [=]() {
                        UnitParamRequest unitRequest;
                        mImpl->d_deviceId = getCurrentDeviceId(mImpl->cmb_deviceId);
                        DeviceID id;
                        id << mImpl->d_deviceId;

                        unitRequest.m_systemNumb = id.sysID;
                        unitRequest.m_deviceNumb = id.devID;
                        unitRequest.m_extenNumb = id.extenID;
                        unitRequest.m_modeID = mImpl->d_modeId;
                        unitRequest.m_unitID = temp.cmdIdOrUnitId.toInt();
                        unitRequest.m_paramdataMap = temp.staticParams;
                        auto statusReportMsg = GlobalData::getExtenStatusReportData(id);
                        int targetNum = statusReportMsg.unitReportMsgMap[1].paramMap["TargetNum"].toInt();
                        unitRequest.m_validIdent = targetNum + 1;  //扩频有效标识为当前目标数+1

                        if (temp.dynamicParams.size())
                        {
                            int targetId = temp.defaultTargetId.toInt();
                            QVariantMap params;
                            for (QDomElement ctrlElem : temp.domELems)
                            {
                                QString tagName = ctrlElem.tagName();
                                QString paramId = ctrlElem.attribute("paramId");

                                if (ctrlElem.hasAttribute("targetId"))
                                    targetId = ctrlElem.attribute("targetId").toInt();

                                if (tagName == "RadioButton")
                                {
                                    QDomElement childCtrlElem = ctrlElem.firstChildElement();
                                    QString childCtrlTagName = childCtrlElem.tagName();
                                    while (!childCtrlElem.isNull())
                                    {
                                        QRadioButton* radioButton = curWindow->findChild<QRadioButton*>(childCtrlElem.attribute("name"));
                                        if (radioButton->isChecked())
                                        {
                                            params[paramId] = childCtrlElem.attribute("value").toInt();
                                            break;
                                        }
                                        childCtrlElem = childCtrlElem.nextSiblingElement();
                                    }
                                }
                                if (tagName == "PopWindow")
                                {
                                    QString name = ctrlElem.attribute("name");
                                    if (name == "RemoteWorkPeriod")
                                    {
                                        RemoteWorkPeriod dialog(nullptr, mImpl->d_deviceId, mImpl->d_modeId);
                                        connect(&dialog, &RemoteWorkPeriod::signalsUnitDeviceJson, this,
                                                &SystemCaptureProcessXml::signalsUnitDeviceJson);
                                        dialog.exec();
                                        return;
                                    }
                                }
                            }
                            if (targetId == 0)
                                unitRequest.m_paramdataMap.unite(params);
                            else
                            {
                                bool tNumCheck = targetNumCheck(mImpl->d_deviceId, targetId);
                                if (!tNumCheck)
                                    return;
                                unitRequest.m_multiTargetParamMap[targetId].m_paramdataMap.unite(params);
                            }
                        }

                        emit signalsUnitParamRequest(unitRequest);
                    });
                }
            }
        }
    }
}

int SystemCaptureProcessXml::getCurrentDeviceId(QComboBox* combox)
{
    int deviceId = combox->currentData().toInt();
    return deviceId;
}

int SystemCaptureProcessXml::getCurrentModeId(QComboBox* combox)
{
    SystemWorkMode workMode = (SystemWorkMode)(combox->currentData().toInt());
    return SystemWorkModeHelper::systemWorkModeConverToModeId(workMode);
}

void SystemCaptureProcessXml::dealMasterSlaveSwitch(const CommandTemplate& temp)
{
    auto cmdID = temp.cmdIdOrUnitId.toInt();
    QMap<QString, QVariant> paramMap;
    paramMap = temp.staticParams;

    //下第一台测控基带的主机/备机
    DeviceID devID(mImpl->d_deviceId);

    //    qDebug() << "第一台测控基带设备ID：" << QString::number(mImpl->d_deviceId, 16) << " "
    //             << "主备机值：" << paramMap["OnlineSelect"];
    cmdSetting(cmdID, mImpl->d_modeId, devID, paramMap);

    //下第二台测控基带的主机/备机
    auto onlineValue = paramMap["OnlineSelect"];
    DeviceID secondDevID;
    getSlaveCKDeviceIDAndValue(onlineValue, secondDevID);
    auto curModeId = GlobalData::getOnlineDeviceModeId(secondDevID);
    paramMap.clear();
    paramMap["OnlineSelect"] = onlineValue;
    //    int deviceIDSecond;
    //    secondDevID >> deviceIDSecond;
    //    qDebug() << "第二台测控基带设备ID：" << QString::number(deviceIDSecond, 16) << " "
    //             << "主备机值：" << paramMap["OnlineSelect"];
    cmdSetting(cmdID, curModeId, secondDevID, paramMap);

    //下发射开关矩阵的参数
    auto manualMsg = GlobalData::getTaskRunningInfoByObject();
    if (manualMsg.linkLineMap.isEmpty())
    {
        return;
    }

    auto dealWithSXData = [&](const DeviceID& ckqdDeviceID) {
        auto unitID = 1;
        int modeID = 65535;
        DeviceID sendMatrixDevID(0x2501);  //发射开关矩阵
        QMap<QString, QVariant> paramMap;

        if (ckqdDeviceID.extenID == 1)  //主用测控前端是1
        {
            paramMap["ChooseConnect_5"] = 1;
            paramMap["ChooseConnect_6"] = 1;
        }
        else
        {
            paramMap["ChooseConnect_5"] = 3;
            paramMap["ChooseConnect_6"] = 3;
        }

        unitSetting(unitID, modeID, sendMatrixDevID, paramMap);
    };

    auto configParam = manualMsg.configMacroData;

    if (mImpl->d_deviceId == 0x4001 || mImpl->d_deviceId == 0x4003)  // S频段
    {
        if (manualMsg.linkLineMap.contains(STTC))
        {
            DeviceID taskPlanCKDevID;
            configParam.getCKJDDeviceID(STTC, taskPlanCKDevID);

            if (taskPlanCKDevID.extenID == devID.extenID)  //所选测控基带和当前任务主用测控基带ID相等
            {
                DeviceID ckqdMasterDevID;
                configParam.getCKQDDeviceID(STTC, ckqdMasterDevID);  //注意：测控前端约定好只有1 2 数传是3 4  这里因为基带ID相等，取主用

                dealWithSXData(ckqdMasterDevID);
            }
            else
            {
                DeviceID ckqdSlaveDevID;
                configParam.getCKQDDeviceID(STTC, ckqdSlaveDevID,
                                            2);  //注意：测控前端约定好只有1 2 数传是3 4 这里因为基带ID不等（说明当前基带坏了），取备用

                dealWithSXData(ckqdSlaveDevID);
            }
        }
        else if (manualMsg.linkLineMap.contains(Skuo2))
        {
            DeviceID taskPlanCKDevID;
            configParam.getCKJDDeviceID(Skuo2, taskPlanCKDevID);

            if (taskPlanCKDevID.extenID == devID.extenID)  //所选测控基带和当前任务主用测控基带ID相等
            {
                DeviceID ckqdMasterDevID;
                configParam.getCKQDDeviceID(Skuo2, ckqdMasterDevID);  //注意：测控前端约定好只有1 2 数传是3 4  这里因为基带ID相等，取主用

                dealWithSXData(ckqdMasterDevID);
            }
            else
            {
                DeviceID ckqdSlaveDevID;
                configParam.getCKQDDeviceID(STTC, ckqdSlaveDevID,
                                            2);  //注意：测控前端约定好只有1 2 数传是3 4 这里因为基带ID不等（说明当前基带坏了），取备用

                dealWithSXData(ckqdSlaveDevID);
            }
        }
    }
    else if (mImpl->d_deviceId == 0x4002 || mImpl->d_deviceId == 0x4004)  // Ka频段
    {
        if (manualMsg.linkLineMap.contains(KaKuo2))
        {
            DeviceID taskPlanCKDevID;
            configParam.getCKJDDeviceID(KaKuo2, taskPlanCKDevID);

            if (taskPlanCKDevID.extenID == devID.extenID)  //所选测控基带和当前任务主用测控基带ID相等
            {
                DeviceID ckqdMasterDevID;
                configParam.getCKQDDeviceID(KaKuo2, ckqdMasterDevID);  //注意：测控前端约定好只有1 2 数传是3 4  这里因为基带ID相等，取主用

                dealWithSXData(ckqdMasterDevID);
            }
            else
            {
                DeviceID ckqdSlaveDevID;
                configParam.getCKQDDeviceID(STTC, ckqdSlaveDevID,
                                            2);  //注意：测控前端约定好只有1 2 数传是3 4 这里因为基带ID不等（说明当前基带坏了），取备用

                dealWithSXData(ckqdSlaveDevID);
            }
        }
    }
}

void SystemCaptureProcessXml::getSlaveCKDeviceIDAndValue(QVariant& value, DeviceID& secondDevID)
{
    //如果是主机，值就替换为备机 备机就替换值为主机
    switch (value.toInt())
    {
    case 1: value = 2; break;
    case 2: value = 1; break;
    default: break;
    }

    //测控基带A-1和B-1对应   A-2和B-2对应
    switch (mImpl->d_deviceId)
    {
    case 0x4001:
    {
        DeviceID devID(0x4003);
        secondDevID = devID;
        break;
    }
    case 0x4002:
    {
        DeviceID devID(0x4004);
        secondDevID = devID;
        break;
    }
    case 0x4003:
    {
        DeviceID devID(0x4001);
        secondDevID = devID;
        break;
    }
    case 0x4004:
    {
        DeviceID devID(0x4002);
        secondDevID = devID;
        break;
    }
    default: break;
    }
}

void SystemCaptureProcessXml::unitSetting(int unitID, int modeID, const DeviceID& devID, const QMap<QString, QVariant>& paramMap)
{
    UnitParamRequest unitRequest;

    unitRequest.m_systemNumb = devID.sysID;
    unitRequest.m_deviceNumb = devID.devID;
    unitRequest.m_extenNumb = devID.extenID;
    unitRequest.m_modeID = modeID;
    unitRequest.m_unitID = unitID;
    unitRequest.m_paramdataMap = paramMap;

    emit signalsUnitParamRequest(unitRequest);
}

void SystemCaptureProcessXml::cmdSetting(int cmdID, int modeID, const DeviceID& devID, const QMap<QString, QVariant>& paramMap)
{
    CmdRequest cmdRequest;

    cmdRequest.m_systemNumb = devID.sysID;
    cmdRequest.m_deviceNumb = devID.devID;
    cmdRequest.m_extenNumb = devID.extenID;
    cmdRequest.m_modeID = modeID;
    cmdRequest.m_cmdID = cmdID;
    cmdRequest.m_paramdataMap = paramMap;

    emit signalsCmdRequest(cmdRequest, false);  //主备机命令切换不需要判断基带是否是主备，直接下就行
}

bool SystemCaptureProcessXml::targetNumCheck(const int deviceId, const int curTargetNum)
{
    //  只有当前目标数小于公共单元里面的目标数才可以下发命令
    auto statusReportMsg = GlobalData::getExtenStatusReportData(deviceId);
    int targetNum = statusReportMsg.unitReportMsgMap[1].paramMap["TargetNum"].toInt();
    if (curTargetNum > targetNum)
    {
        QString errorMsg;
        errorMsg = QString("所控参数目标数小于当前目标数,命令下发失败");
        SystemLogPublish::errorMsg(errorMsg);
        QMessageBox::warning(nullptr, QString("提示"), errorMsg, QString("确定"));
        return false;
    }
    else
    {
        return true;
    }
}
