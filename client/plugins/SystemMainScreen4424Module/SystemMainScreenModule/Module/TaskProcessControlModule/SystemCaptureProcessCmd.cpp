#include "SystemCaptureProcessCmd.h"

#include "CustomPacketMessageDefine.h"
#include "DeviceProcessMessageSerialize.h"
#include "DopplerSet.h"
#include "GlobalData.h"
#include "PlatformConfigTools.h"
#include "PlatformInterface.h"
#include "ProtocolXmlTypeDefine.h"
#include "RedisHelper.h"
#include "RemoteWorkPeriod.h"
#include "ServiceCheck.h"
#include "SystemCaptureProcessControl.h"
#include "SystemCaptureProcessXmlReader.h"
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
    QComboBox* cmb_modeId{ nullptr };
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

void SystemCaptureProcessXml::start(QWidget* comBox_MacId, QWidget* comBox_MacMode, QWidget* widget, const QString& widgetName)
{
    mImpl->cmb_deviceId = dynamic_cast<QComboBox*>(comBox_MacId);
    mImpl->cmb_modeId = dynamic_cast<QComboBox*>(comBox_MacMode);

    if (widgetName == "SystemCaptureProcessControl")
    {
        SystemCaptureProcessControl* curWindow = dynamic_cast<SystemCaptureProcessControl*>(widget);

        for (CommandTemplate& temp : mImpl->d_cmd)
        {
            if (temp.cmdType == DevMsgType::ProcessControlCmd)
            {
                QObject* object = curWindow->findChild<QObject*>(temp.name);

                if (object->inherits("QRadioButton")) {}
                else if (object->inherits("QPushButton"))
                {
                    QPushButton* pushButton = dynamic_cast<QPushButton*>(object);
                    connect(pushButton, &QPushButton::clicked, this, [=]() {
                        CmdRequest cmdRequest;
                        mImpl->d_deviceId = getCurrentDeviceId(mImpl->cmb_deviceId);
                        mImpl->d_modeId = getCurrentModeId(mImpl->cmb_modeId);
                        DeviceID id;
                        id << mImpl->d_deviceId;

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
                        mImpl->d_modeId = getCurrentModeId(mImpl->cmb_modeId);
                        DeviceID id;
                        id << mImpl->d_deviceId;

                        unitRequest.m_systemNumb = id.sysID;
                        unitRequest.m_deviceNumb = id.devID;
                        unitRequest.m_extenNumb = id.extenID;
                        unitRequest.m_modeID = mImpl->d_modeId;
                        unitRequest.m_unitID = temp.cmdIdOrUnitId.toInt();
                        unitRequest.m_paramdataMap = temp.staticParams;
                        unitRequest.m_validIdent = temp.validIdent.toInt();

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
                        mImpl->d_modeId = getCurrentModeId(mImpl->cmb_modeId);
                        DeviceID id;
                        id << mImpl->d_deviceId;

                        unitRequest.m_systemNumb = id.sysID;
                        unitRequest.m_deviceNumb = id.devID;
                        unitRequest.m_extenNumb = id.extenID;
                        unitRequest.m_modeID = mImpl->d_modeId;
                        unitRequest.m_unitID = temp.cmdIdOrUnitId.toInt();
                        unitRequest.m_paramdataMap = temp.staticParams;
                        unitRequest.m_validIdent = temp.validIdent.toInt();

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

bool SystemCaptureProcessXml::targetNumCheck(const int deviceId, const int curTargetNum)
{
    //  只有当前目标数小于公共单元里面的目标数才可以下发命令
    auto statusReportMsg = GlobalData::getExtenStatusReportData(deviceId);
    int targetNum = statusReportMsg.unitReportMsgMap[1].paramMap["TargetNum"].toInt();
    if (curTargetNum > targetNum)
    {
        QMessageBox::warning(nullptr, QString("提示"), QString("所控参数目标数小于当前目标数,命令下发失败"), QString("确定"));
        return false;
    }
    else
    {
        return true;
    }
}
