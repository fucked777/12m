#include "ComplexCheckBox.h"
#include "DeviceProcessMessageSerialize.h"
#include "MessagePublish.h"
#include "ServiceCheck.h"
#include <QHBoxLayout>
#include <QMessageBox>

ComplexCheckBox::ComplexCheckBox(const QDomElement& domEle, QWidget* parent, const int deviceId, const int modeId)
    : BaseWidget(domEle, parent)
    , d_deviceId(deviceId)
    , d_modeId(modeId)
{
    parseNode(domEle);

    initLayout();

    setText(d_text);
}

void ComplexCheckBox::setText(const QString& text) { mCheckBox->setText(text); }

QString ComplexCheckBox::getText() const { return mCheckBox->text(); }

void ComplexCheckBox::setValue(const QVariant& value)
{
    //命令控制阶段不做刷新
    if (mCheckBox->isEnabled())
        return;

    if (d_id == (int)CKK2_TZJT)
    {
        if (value.toInt() == 2)
            mCheckBox->setChecked(true);
        else
            mCheckBox->setChecked(false);
    }
    else
    {
        if (value.toInt() == 1)
            mCheckBox->setChecked(true);
        else
            mCheckBox->setChecked(false);
    }
}

QVariant ComplexCheckBox::getValue() const { return mCheckBox->isChecked() ? 1 : 0; }

void ComplexCheckBox::setIsControl(const bool isControl) { d_isControl = isControl; }

bool ComplexCheckBox::getIsControl() const { return d_isControl; }

void ComplexCheckBox::setEnable(const bool isEnable) { mCheckBox->setEnabled(isEnable); }

bool ComplexCheckBox::getEnable() const { return mCheckBox->isEnabled(); }

void ComplexCheckBox::onStateChanged(int state)
{
    if (d_isControl)
    {
        if (parent() == nullptr)
            return;

        if (d_childBoxs.isEmpty())
        {
            QList<ComplexCheckBox*> checkBoxs = parent()->findChildren<ComplexCheckBox*>();
            for (ComplexCheckBox* checkBox : checkBoxs)
            {
                for (int childId : d_childId)
                {
                    if (checkBox->getId() == childId)
                    {
                        d_childBoxs.append(checkBox);
                        break;
                    }
                }
            }
        }

        for (ComplexCheckBox* checkBox : d_childBoxs)
        {
            checkBox->setEnable(state == Qt::Checked);
        }
    }
    else
    {
        if (!mCheckBox->isEnabled())
            return;

        SERVICEONLINECHECK();

        // ACU特殊处理
        if ((int)ACU_ID == d_id)
        {
            DeviceID acuAID(1, 0, 17);
            DeviceID acuBID(1, 0, 18);

            auto isOnlneA = GlobalData::getDeviceOnlineByObject(acuAID);
            auto isOnlneB = GlobalData::getDeviceOnlineByObject(acuBID);

            DeviceID useACUID;
            if (!isOnlneA && !isOnlneB)
            {
                QString errorMsg = QString("ACU_A和ACU_B都处于离线状态");
                SystemLogPublish::warningMsg(errorMsg);
                QMessageBox::warning(nullptr, QString("提示"), errorMsg, QString("确定"));
                return;
            }
            //判断ACU是否在线
            if (isOnlneA)
            {
                auto acuExMapA = GlobalData::getExtenStatusReportDataByObject(acuAID);
                if (acuExMapA.unitReportMsgMap[1].paramMap["OnLineMachine"] == 1)  //判断是否是主机
                {
                    useACUID = acuAID;
                }
            }

            if (isOnlneB)
            {
                auto acuExMapB = GlobalData::getExtenStatusReportDataByObject(acuBID);
                if (acuExMapB.unitReportMsgMap[1].paramMap["OnLineMachine"] == 1)  //判断是否是主机
                {
                    useACUID = acuBID;
                }
            }

            if (!useACUID.isValid())
            {
                QString errorMsg = QString("可使用的ACU都处于备机状态，下发命令失败！");
                SystemLogPublish::warningMsg(errorMsg);
                QMessageBox::warning(nullptr, QString("提示"), errorMsg, QString("确定"));
                return;
            }
            //判断ACU是否是本控
            if (!deviceIsSelfControl(useACUID))
            {
                return;
            }

            CmdRequest cmdRequest;
            if (getValue().toInt() == 1)  //选中
            {
                cmdRequest.m_paramdataMap["SendDataMode"] = 2;    //人工
                cmdRequest.m_paramdataMap["ReportSendCtrl"] = 1;  //开始送数
            }
            else
            {
                cmdRequest.m_paramdataMap["SendDataMode"] = 2;    //人工
                cmdRequest.m_paramdataMap["ReportSendCtrl"] = 2;  //停止送数
            }

            cmdRequest.m_systemNumb = useACUID.sysID;
            cmdRequest.m_deviceNumb = useACUID.devID;
            cmdRequest.m_extenNumb = useACUID.extenID;
            cmdRequest.m_modeID = 0xFFFF;
            cmdRequest.m_cmdID = 18;

            QString json;

            json << cmdRequest;
            emit signalsCmdDeviceJson(json);
            return;
        }
        // DTE特殊处理
        if ((int)DTE_ID == d_id)
        {
            DeviceID dteID(8, 0, 1);

            // 设备在线检测
            if (!deviceIsOnline(dteID))
            {
                return;
            }
            // 设备本分控检测
            if (!deviceIsSelfControl(dteID))
            {
                return;
            }

            auto dteRecycleReport = GlobalData::getExtenStatusReportDataByObject(dteID);
            auto recycleReportMap = dteRecycleReport.unitReportMsgMap[1].multiTargetParamMap2;
            auto dteTaskBZAndCenterBZMap = dteRecycleReport.unitReportMsgMap[1].dteTaskBZAndCenterBZMap;  //动态数据里面再嵌套的动态数据
            if (recycleReportMap.contains(1))
            {
                auto taskDataList = recycleReportMap[1];

                for (int i = 0; i < taskDataList.size(); ++i)
                {
                    auto valueMap = taskDataList.at(i);
                    QString taskBZ;       //任务标识
                    QString tianxianUAC;  //天线UAC
                    taskBZ = valueMap.value("MissionID").toString().toUpper();
                    tianxianUAC = valueMap.value("MissionUAC").toString().toUpper();

                    for (auto taskBZDTE : dteTaskBZAndCenterBZMap.keys())
                    {
                        if (taskBZ != taskBZDTE.toString().toUpper())
                            continue;
                        auto centerBZList = dteTaskBZAndCenterBZMap[taskBZDTE];

                        for (auto centerBZ : centerBZList)  //每个任务有多少个中心就下多少个送数命令
                        {
                            CmdRequest cmdRequest;
                            cmdRequest.m_systemNumb = dteID.sysID;
                            cmdRequest.m_deviceNumb = dteID.devID;
                            cmdRequest.m_extenNumb = dteID.extenID;
                            cmdRequest.m_modeID = 0xFFFF;
                            cmdRequest.m_cmdID = 13;

                            cmdRequest.m_paramdataMap.insert("MID", taskBZ);                              //任务标识
                            cmdRequest.m_paramdataMap.insert("UAC", tianxianUAC);                         //天线标识
                            cmdRequest.m_paramdataMap.insert("CenterID", centerBZ.toString().toUpper());  //中心标识
                            cmdRequest.m_paramdataMap.insert("Type", 1);                                  //默认测控数据
                            if (getValue().toInt() == 1)                                                  //被选中
                            {
                                cmdRequest.m_paramdataMap.insert("IsSend", 2);  //发送
                            }
                            else
                            {
                                cmdRequest.m_paramdataMap.insert("IsSend", 1);  //不发送
                            }

                            QString json;

                            json << cmdRequest;
                            emit signalsCmdDeviceJson(json);
                        }
                    }
                }
            }
            return;
        }

        auto statusReportMsg = GlobalData::getExtenStatusReportData(d_deviceId);
        int modeId = statusReportMsg.modeId;
        QString json = "";

        switch (d_command.msgType)
        {
        case DevMsgType::ProcessControlCmd:
        {
            if (d_command.cmdRequest.m_modeID != modeId)
            {
                return;
            }
            CmdRequest cmdRequest = d_command.cmdRequest;

            if (d_isKP)
            {
                //扩频模式需要判断所控目标数是否小于当前目标数
                if (d_command.targetInfos.keys().size() == 1)
                {
                    QString key = d_command.targetInfos.keys().at(0);
                    TargetInfo info = d_command.targetInfos.value(key);
                    int maxTargetId = statusReportMsg.unitReportMsgMap[info.unitId].paramMap[info.paramId].toInt();
                    if (d_targetId > maxTargetId)
                    {
                        QMessageBox::warning(this, QString("提示"), QString("所控参数目标数小于当前目标数,命令下发失败"), QString("确定"));
                        return;
                    }
                }
            }

            if (d_command.controlInfos.keys().size() > 0)
            {
                QString key = d_command.controlInfos.keys().at(0);
                ControlInfo info = d_command.controlInfos.value(key);
                cmdRequest.m_paramdataMap.insert(info.paramId, controlValue2CommandValue(getValue().toInt()));
            }

            json << cmdRequest;
            emit signalsCmdDeviceJson(json);
            break;
        }
        case DevMsgType::UnitParameterSetCmd:
        {
            if (d_command.unitParamRequest.m_modeID != modeId)
            {
                return;
            }
            UnitParamRequest unitParamRequest = d_command.unitParamRequest;

            if (d_isKP)
            {
                //扩频模式需要判断所控目标数是否小于当前目标数
                if (d_command.targetInfos.keys().size() == 1)
                {
                    QString key = d_command.targetInfos.keys().at(0);
                    TargetInfo info = d_command.targetInfos.value(key);
                    int maxTargetId = statusReportMsg.unitReportMsgMap[info.unitId].paramMap[info.paramId].toInt();
                    if (d_targetId > maxTargetId)
                    {
                        QMessageBox::warning(this, QString("提示"), QString("所控参数目标数小于当前目标数,命令下发失败"), QString("确定"));
                        return;
                    }
                }
            }

            switch (d_command.position)
            {
            case UnitParam_DataMap:
            {
                if (d_command.controlInfos.size())
                {
                    QString key = d_command.controlInfos.keys().at(0);
                    ControlInfo info = d_command.controlInfos.value(key);
                    if (d_id == (int)CKK2_TZJT)
                    {
                        unitParamRequest.m_paramdataMap.insert(info.paramId, controlValueCommandValue(getValue().toInt()));
                    }
                    else
                    {
                        unitParamRequest.m_paramdataMap.insert(info.paramId, controlValue2CommandValue(getValue().toInt()));
                    }
                }
                break;
            }
            case UnitParam_MultiDataMap:
            {
                if (d_command.controlInfos.size())
                {
                    QString key = d_command.controlInfos.keys().at(0);
                    ControlInfo info = d_command.controlInfos.value(key);
                    unitParamRequest.m_multiTargetParamMap[info.targetId].m_paramdataMap.insert(info.paramId,
                                                                                                controlValue2CommandValue(getValue().toInt()));
                }
                break;
            }
            default: break;
            }

            json << unitParamRequest;
            emit signalsUnitDeviceJson(json);

            break;
        }
        default: break;
        }
    }
}

void ComplexCheckBox::initLayout()
{
    mCheckBox = new QCheckBox(this);
    connect(mCheckBox, &QCheckBox::stateChanged, this, &ComplexCheckBox::onStateChanged);
    if (!d_isControl)
        mCheckBox->setEnabled(false);
    else
        mCheckBox->setEnabled(true);

    auto hLayout = new QHBoxLayout;
    hLayout->setSpacing(0);
    hLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(hLayout);

    hLayout->addWidget(mCheckBox);
}

void ComplexCheckBox::parseNode(const QDomElement& domEle)
{
    QString isControl = domEle.attribute("isControl");
    (isControl == "true") ? (d_isControl = true) : (d_isControl = false);
    d_isKP = domEle.hasAttribute("isKP");
    d_id = domEle.attribute("id").toInt();
    d_targetId = domEle.attribute("targetId").toInt();
    //    d_deviceId = domEle.attribute("deviceId").toInt(nullptr, 16);
    d_text = domEle.attribute("text");

    if (d_isControl)
    {
        QStringList strChildren = domEle.attribute("childId").split("|");
        for (QString& strChild : strChildren)
        {
            d_childId.append(strChild.toInt());
        }
    }
    else
    {
        QString msgType = domEle.attribute("msgType");
        if (msgType == "ProcessControl")
        {
            d_command.msgType = DevMsgType::ProcessControlCmd;
        }
        else if (msgType == "UnitParam")
        {
            d_command.msgType = DevMsgType::UnitParameterSetCmd;
        }

        auto domNode = domEle.firstChild();
        while (!domNode.isNull())
        {
            auto domEle = domNode.toElement();
            if (domEle.isNull())
            {
                domNode = domNode.nextSibling();
                continue;
            }

            if (domEle.tagName() == "ProcessControl")
            {
                parseProcessControlNode(domEle);
            }
            if (domEle.tagName() == "UnitParam")
            {
                parseUnitParamNode(domEle);
            }

            domNode = domNode.nextSibling();
        }
    }
}

void ComplexCheckBox::parseProcessControlNode(const QDomElement& domEle)
{
    DeviceID deviceID = /*domEle.attribute("deviceId").toInt(nullptr, 16);*/ d_deviceId;
    QString strDataMap = domEle.attribute("param");
    QMap<QString, QVariant> paramDataMap;
    string2Map(strDataMap, paramDataMap);

    d_command.msgType = DevMsgType::ProcessControlCmd;
    d_command.cmdRequest.m_systemNumb = deviceID.sysID;
    d_command.cmdRequest.m_deviceNumb = deviceID.devID;
    d_command.cmdRequest.m_extenNumb = deviceID.extenID;
    d_command.cmdRequest.m_modeID = /*domEle.attribute("modeId").toInt(nullptr, 16);*/ d_modeId;
    d_command.cmdRequest.m_cmdID = domEle.attribute("cmdId").toInt();
    d_command.cmdRequest.m_paramdataMap = paramDataMap;

    auto domNode = domEle.firstChild();
    while (!domNode.isNull())
    {
        auto domEle = domNode.toElement();
        if (domEle.isNull())
        {
            domNode = domNode.nextSibling();
            continue;
        }

        if (domEle.tagName() == "UnitInfo")
        {
            parseUnitInfoNode(domEle, d_command.unitInfos);
        }
        if (domEle.tagName() == "ControlInfo")
        {
            parseControlInfoNode(domEle, d_command.controlInfos);
        }
        if (domEle.tagName() == "TargetInfo")
        {
            parseTargetInfoNode(domEle, d_command.targetInfos);
        }
        domNode = domNode.nextSibling();
    }
}

void ComplexCheckBox::parseUnitParamNode(const QDomElement& domEle)
{
    DeviceID deviceID = /*domEle.attribute("deviceId").toInt(nullptr, 16);*/ d_deviceId;
    QString strDataMap = domEle.attribute("param");
    QString strPosition = domEle.attribute("position");
    Position position = ProcessControl_DataMap;
    QMap<QString, QVariant> paramDataMap;
    int validIdent = -1;
    if (domEle.hasAttribute("validIdent"))
    {
        validIdent = domEle.attribute("validIdent").toInt();
    }
    string2Position(strPosition, position);

    d_command.msgType = DevMsgType::UnitParameterSetCmd;
    d_command.unitParamRequest.m_systemNumb = deviceID.sysID;
    d_command.unitParamRequest.m_deviceNumb = deviceID.devID;
    d_command.unitParamRequest.m_extenNumb = deviceID.extenID;
    d_command.unitParamRequest.m_modeID = d_modeId;
    d_command.unitParamRequest.m_unitID = domEle.attribute("unitId").toInt(nullptr, 16);
    d_command.unitParamRequest.m_validIdent = validIdent;
    d_command.position = position;
    switch (position)
    {
    case UnitParam_DataMap:
    {
        string2Map(strDataMap, paramDataMap);
        d_command.unitParamRequest.m_paramdataMap = paramDataMap;
        break;
    }
    case UnitParam_MultiDataMap:
    {
        int targetId = domEle.attribute("targetId").toInt(nullptr, 16);
        string2Map(strDataMap, paramDataMap);
        d_command.unitParamRequest.m_multiTargetParamMap[targetId].m_paramdataMap = paramDataMap;
        break;
    }
    default: break;
    }

    auto domNode = domEle.firstChild();
    while (!domNode.isNull())
    {
        auto domEle = domNode.toElement();
        if (domEle.isNull())
        {
            domNode = domNode.nextSibling();
            continue;
        }

        if (domEle.tagName() == "UnitInfo")
        {
            parseUnitInfoNode(domEle, d_command.unitInfos);
        }
        if (domEle.tagName() == "ControlInfo")
        {
            parseControlInfoNode(domEle, d_command.controlInfos);
        }
        if (domEle.tagName() == "TargetInfo")
        {
            parseTargetInfoNode(domEle, d_command.targetInfos);
        }
        domNode = domNode.nextSibling();
    }
}

void ComplexCheckBox::parseUnitInfoNode(const QDomElement& domEle, QMap<QString, UnitInfo>& map)
{
    UnitInfo info;
    info.deviceID = /*domEle.attribute("deviceId").toInt(nullptr, 16);*/ d_deviceId;
    info.modeId = /*domEle.attribute("modeId").toInt(nullptr, 16)*/ d_modeId;
    info.unitId = domEle.attribute("unitId").toInt(nullptr, 16);
    info.targetId = domEle.attribute("targetId").toInt(nullptr, 16);
    info.paramId = domEle.attribute("paramId");
    QString strPosition = domEle.attribute("position");
    Position position = ProcessControl_DataMap;
    string2Position(strPosition, position);
    info.position = position;

    map.insert(info.paramId, info);
}

void ComplexCheckBox::parseControlInfoNode(const QDomElement& domEle, QMap<QString, ControlInfo>& map)
{
    ControlInfo info;
    info.paramId = domEle.attribute("paramId");
    info.checkBoxId = domEle.attribute("checkBoxId").toInt();
    info.targetId = domEle.attribute("targetId").toInt(nullptr, 16);
    map.insert(info.paramId, info);
}

void ComplexCheckBox::parseTargetInfoNode(const QDomElement& domEle, QMap<QString, ComplexCheckBox::TargetInfo>& map)
{
    TargetInfo info;
    info.deviceID = /*domEle.attribute("deviceId").toInt(nullptr, 16);*/ d_deviceId;
    info.modeId = /*domEle.attribute("modeId").toInt(nullptr, 16)*/ d_modeId;
    info.unitId = domEle.attribute("unitId").toInt(nullptr, 16);
    info.targetId = domEle.attribute("targetId").toInt(nullptr, 16);
    info.paramId = domEle.attribute("paramId");

    map.insert(info.paramId, info);
}

void ComplexCheckBox::string2Map(QString string, QMap<QString, QVariant>& params)
{
    for (QString& str : string.split(" "))
    {
        QStringList list = str.split("|");
        if (list.size() == 2)
        {
            params.insert(list.at(0), list.at(1));
        }
    }
}

void ComplexCheckBox::string2Position(const QString& string, ComplexCheckBox::Position& pos)
{
    if (string == "DataMap")
    {
        pos = UnitParam_DataMap;
    }
    if (string == "MultiDataMap")
    {
        pos = UnitParam_MultiDataMap;
    }
}

int ComplexCheckBox::controlValue2CommandValue(int controlValue)
{
    switch (controlValue)
    {
    case 0: return 2;
    case 1: return 1;
    default: return -1;
    }
}

int ComplexCheckBox::controlValueCommandValue(int controlValue)
{
    switch (controlValue)
    {
    case 0: return 1;
    case 1: return 2;
    default: return -1;
    }
}

bool ComplexCheckBox::deviceIsOnline(const DeviceID& deviceID)
{
    QString errorMsg;
    if (!GlobalData::getDeviceOnline(deviceID))
    {
        auto deviceName = GlobalData::getExtensionName(deviceID);
        errorMsg = QString("%1设备离线，命令下发失败").arg(deviceName);
        SystemLogPublish::errorMsg(errorMsg);
        QMessageBox::warning(nullptr, QString("提示"), errorMsg, QString("确定"));
        return false;
    }
    return true;
}

bool ComplexCheckBox::deviceIsSelfControl(const DeviceID& deviceID)
{
    QString errorMsg;
    if (!GlobalData::getDeviceSelfControl(deviceID))
    {
        auto deviceName = GlobalData::getExtensionName(deviceID);
        errorMsg = QString("%1设备为分控状态，命令下发失败").arg(deviceName);
        SystemLogPublish::warningMsg(errorMsg);
        QMessageBox::warning(nullptr, QString("提示"), errorMsg, QString("确定"));
        return false;
    }

    return true;
}
