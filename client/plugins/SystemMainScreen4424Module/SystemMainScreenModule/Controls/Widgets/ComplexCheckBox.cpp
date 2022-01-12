#include "ComplexCheckBox.h"
#include "DeviceProcessMessageSerialize.h"
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

    if (this->d_id > 100)
    {
        {
            if (value.toInt() == 1) {}
        }
        mCheckBox->setChecked(true);
        int a = 1;
    }
    if (value.toInt() == 1)
        mCheckBox->setCheckState(Qt::Checked);
    else
        mCheckBox->setCheckState(Qt::Unchecked);
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

            CmdRequest cmdRequest = d_command.cmdRequest;
            cmdRequest.m_paramdataMap.insert("ReportSendCtrl", controlValue2CommandValue(getValue().toInt()));

            {
                QString json = "";
                cmdRequest.m_systemNumb = acuAID.sysID;
                cmdRequest.m_deviceNumb = acuAID.devID;
                cmdRequest.m_extenNumb = acuAID.extenID;
                cmdRequest.m_modeID = 0xFFFF;
                cmdRequest.m_cmdID = 18;
                json << cmdRequest;
                emit signalsCmdDeviceJson(json);
            }

            {
                cmdRequest.m_systemNumb = acuBID.sysID;
                cmdRequest.m_deviceNumb = acuBID.devID;
                cmdRequest.m_extenNumb = acuBID.extenID;
                cmdRequest.m_modeID = 0xFFFF;
                cmdRequest.m_cmdID = 18;

                QString json = "";

                json << cmdRequest;
                emit signalsCmdDeviceJson(json);
            }
            return;
        }
        // DTE特殊处理
        if ((int)DTE_ID == d_id)
        {
            DeviceID dteID(8, 0, 1);

            bool isRemoteCtrl = GlobalData::getDeviceSelfControl(dteID);
            if (!isRemoteCtrl)
            {
                QMessageBox::warning(this, QString("提示"), QString("当前DTE为分控状态,命令下发失败"), QString("确定"));
                return;
            }

            auto idListVariant = this->property("idList");
            auto idList = idListVariant.value<QStringList>();

            CmdRequest cmdRequest;
            cmdRequest.m_systemNumb = dteID.sysID;
            cmdRequest.m_deviceNumb = dteID.devID;
            cmdRequest.m_extenNumb = dteID.extenID;
            cmdRequest.m_modeID = 0xFFFF;
            cmdRequest.m_cmdID = 0x18;

            for (auto id : idList)
            {
                cmdRequest.m_paramdataMap.insert("CenterID", id);
                cmdRequest.m_paramdataMap.insert("Router", 0);
                cmdRequest.m_paramdataMap.insert("IsSendTtc", controlValue2CommandValue(getValue().toInt()));
                cmdRequest.m_paramdataMap.insert("IsSendLink", controlValue2CommandValue(getValue().toInt()));
                QString json = "";

                json << cmdRequest;
                emit signalsCmdDeviceJson(json);
            }

            return;
        }

        //增加本分控判断逻辑
        bool isRemoteCtrl = GlobalData::getDeviceSelfControl(d_deviceId);

        if (!isRemoteCtrl)
        {
            QMessageBox::warning(this, QString("提示"), QString("当前设备为分控状态,命令下发失败"), QString("确定"));
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
                    unitParamRequest.m_paramdataMap.insert(info.paramId, controlValue2CommandValue(getValue().toInt()));
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
