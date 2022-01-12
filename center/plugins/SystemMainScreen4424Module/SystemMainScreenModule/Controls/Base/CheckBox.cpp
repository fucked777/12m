#include "CheckBox.h"

#include "CustomPacketMessageDefine.h"
#include "DeviceProcessMessageDefine.h"
#include "DeviceProcessMessageSerialize.h"
#include "GlobalData.h"
#include "ProtocolXmlTypeDefine.h"
#include "RedisHelper.h"
#include "ServiceCheck.h"
#include "SystemMainScreenServer.h"
#include <QHBoxLayout>
#include <QMessageBox>

CheckBox::CheckBox(const QDomElement& domEle, QWidget* parent, const int deviceId, const int modeId)
    : BaseWidget(domEle, parent)
    , d_targetNumOverSizeCount(0)
    , d_deviceId(deviceId)
    , d_modeId(modeId)
    , d_controlMode("ControlMode")
    , d_relateBox(nullptr)
{
    parseNode(domEle);

    initLayout();

    setText(d_text);
}

void CheckBox::setText(const QString& text) { mCheckBox->setText(text); }

QString CheckBox::getText() const { return mCheckBox->text(); }

void CheckBox::setValue(const QVariant& value)
{
    //命令控制阶段不做刷新
    if (mCheckBox->isEnabled())
        return;

    if (value.toInt() == 1)
        mCheckBox->setCheckState(Qt::Checked);
    else
        mCheckBox->setCheckState(Qt::Unchecked);
}

QVariant CheckBox::getValue() const { return mCheckBox->isChecked() ? 1 : 0; }

void CheckBox::setCheckable(const bool enabled)
{
    d_checkable = enabled;
    mCheckBox->setEnabled(enabled);
}

bool CheckBox::getCheckable() const { return d_checkable; }

int CheckBox::getId() const { return d_id; }

bool CheckBox::isControlType() { return d_isControlType; }

void CheckBox::onStateChanged(int state)
{
    if (d_isControlType)
    {
        if (parent() == nullptr)
            return;

        if (d_childBoxs.isEmpty())
        {
            QList<CheckBox*> checkBoxs = parent()->findChildren<CheckBox*>();
            for (CheckBox* checkBox : checkBoxs)
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

        for (CheckBox* checkBox : d_childBoxs)
        {
            checkBox->setCheckable(state == Qt::Checked);
        }
    }
    else
    {
        if (!mCheckBox->isEnabled())
            return;

        SERVICEONLINECHECK();

        //增加本分控判断逻辑
        bool isRemoteCtrl = GlobalData::getDeviceSelfControl(d_deviceId);
        //        if (!isRemoteCtrl)
        //        {
        //            QMessageBox::warning(this, QString("提示"), QString("当前设备为分控状态,命令下发失败"), QString("确定"));
        //            return;
        //        }

        switch (d_control.cmdType)
        {
        case STTC_SC: processSTTC_SC(); break;
        case STTC_JT: processSTTC_JT(); break;
        case Skuo2_SC: processSkuo2_SC(); break;
        case Skuo2_JT:
        {
            if (d_relateBox == nullptr)
            {
                QList<CheckBox*> checkBoxs = parent()->findChildren<CheckBox*>();
                for (CheckBox* checkBox : checkBoxs)
                {
                    if (checkBox->getId() == d_control.relateId)
                    {
                        d_relateBox = checkBox;
                        break;
                    }
                }
            }
            processSkuo2_JT();
        }

        case SYTHSMJ_SC: processSYTHSMJ_SC(); break;
        case SYTHSMJ_JT: processSYTHSMJ_JT(); break;
        case SYTHWX_SC: processSYTHWX_SC(); break;
        case SYTHWX_JT: processSYTHWX_JT(); break;
        case SYTHGML_SC: processSYTHGML_SC(); break;
        case SYTHGML_JT: processSYTHGML_JT(); break;
        case SYTHSMJK2GZB_SC: processSYTHSMJK2GZB_SC(); break;
        case SYTHSMJK2GZB_JT: processSYTHSMJK2GZB_JT(); break;
        case SYTHSMJK2BGZB_SC: processSYTHSMJK2BGZB_SC(); break;
        case SYTHSMJK2BGZB_JT: processSYTHSMJK2BGZB_JT(); break;
        case SYTHWXSK2_SC: processSYTHWXSK2_SC(); break;
        case SYTHWXSK2_JT: processSYTHWXSK2_JT(); break;
        case SYTHGMLSK2_SC: processSYTHGMLSK2_SC(); break;
        case SYTHGMLSK2_JT: processSYTHGMLSK2_JT(); break;
        case SKT_SC: processSKT_SC(); break;
        case SKT_JT: processSKT_JT(); break;
        default: break;
        }
    }
}

void CheckBox::initLayout()
{
    mCheckBox = new QCheckBox(this);
    connect(mCheckBox, &QCheckBox::stateChanged, this, &CheckBox::onStateChanged);
    if (!isControlType())
        mCheckBox->setEnabled(false);
    else
        mCheckBox->setEnabled(true);

    auto hLayout = new QHBoxLayout;
    hLayout->setSpacing(0);
    hLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(hLayout);

    hLayout->addWidget(mCheckBox);
}

void CheckBox::parseNode(const QDomElement& domEle)
{
    if (domEle.isNull())
    {
        return;
    }
    auto domNode = domEle.toElement();

    (domEle.hasAttribute("childId")) ? (d_isControlType = true) : (d_isControlType = false);
    d_text = domEle.attribute("text");
    d_id = domEle.attribute("id").toInt();

    if (d_isControlType)
    {
        QStringList strChildren = domEle.attribute("childId").split("|");
        for (QString& strChild : strChildren)
        {
            d_childId.append(strChild.toInt());
        }
    }
    else
    {
        d_update.deviceId = d_deviceId;
        d_update.modeId = d_modeId;
        d_update.unitId = domEle.attribute("unitId").toInt();
        d_update.targetId = domEle.attribute("targetId").toInt();
        d_update.paramId = domEle.attribute("paramId");

        d_control.deviceId = d_deviceId;
        d_control.modeId = d_modeId;
        d_relateBox = nullptr;

        domNode = domNode.firstChild().toElement();
        if (domNode.hasAttribute("controlMode"))
        {
            d_controlMode = domNode.attribute("controlMode");
        }
        QString tagName = domNode.tagName();

        while (!domNode.isNull())
        {
            auto subDomEle = domNode.toElement();
            if (subDomEle.isNull())
            {
                continue;
            }

            d_control.cmdid = domNode.attribute("cmdid").toInt();
            QString cmdType = domNode.attribute("cmdType");

            if (cmdType.toUpper() == "STTC_SC")
            {
                d_control.cmdType = static_cast<int>(STTC_SC);
            }
            if (cmdType.toUpper() == "STTC_JT")
            {
                d_control.cmdType = static_cast<int>(STTC_JT);
            }
            if (cmdType.toUpper() == "Skuo2_SC")
            {
                d_control.cmdType = static_cast<int>(Skuo2_SC);
            }
            if (cmdType.toUpper() == "Skuo2_JT")
            {
                d_control.cmdType = static_cast<int>(Skuo2_JT);
            }
            if (cmdType.toUpper() == "SYTHSMJ_SC")
            {
                d_control.cmdType = static_cast<int>(SYTHSMJ_SC);
            }
            if (cmdType.toUpper() == "SYTHSMJ_JT")
            {
                d_control.cmdType = static_cast<int>(SYTHSMJ_JT);
            }
            if (cmdType.toUpper() == "SYTHWX_SC")
            {
                d_control.cmdType = static_cast<int>(SYTHWX_SC);
            }
            if (cmdType.toUpper() == "SYTHWX_JT")
            {
                d_control.cmdType = static_cast<int>(SYTHWX_JT);
            }
            if (cmdType.toUpper() == "SYTHGML_SC")
            {
                d_control.cmdType = static_cast<int>(SYTHGML_SC);
            }
            if (cmdType.toUpper() == "SYTHGML_JT")
            {
                d_control.cmdType = static_cast<int>(SYTHGML_JT);
            }
            if (cmdType.toUpper() == "SYTHSMJK2GZB_SC")
            {
                d_control.cmdType = static_cast<int>(SYTHSMJK2GZB_SC);
            }
            if (cmdType.toUpper() == "SYTHSMJK2GZB_JT")
            {
                d_control.cmdType = static_cast<int>(SYTHSMJK2GZB_JT);
            }
            if (cmdType.toUpper() == "SYTHSMJK2BGZB_SC")
            {
                d_control.cmdType = static_cast<int>(SYTHSMJK2BGZB_SC);
            }
            if (cmdType.toUpper() == "SYTHSMJK2BGZB_JT")
            {
                d_control.cmdType = static_cast<int>(SYTHSMJK2BGZB_JT);
            }
            if (cmdType.toUpper() == "SYTHWXSK2_SC")
            {
                d_control.cmdType = static_cast<int>(SYTHWXSK2_SC);
            }
            if (cmdType.toUpper() == "SYTHWXSK2_JT")
            {
                d_control.cmdType = static_cast<int>(SYTHWXSK2_JT);
            }
            if (cmdType.toUpper() == "SYTHGMLSK2_SC")
            {
                d_control.cmdType = static_cast<int>(SYTHGMLSK2_SC);
            }
            if (cmdType.toUpper() == "SYTHGMLSK2_JT")
            {
                d_control.cmdType = static_cast<int>(SYTHGMLSK2_JT);
            }
            if (cmdType.toUpper() == "SKT_SC")
            {
                d_control.cmdType = static_cast<int>(SKT_SC);
            }
            if (cmdType.toUpper() == "SKT_JT")
            {
                d_control.cmdType = static_cast<int>(SKT_JT);
            }

            QString controlInfo = domNode.attribute("controlInfo");
            for (QString info : controlInfo.split(" "))
            {
                if (info.split("|").size() == 2)
                {
                    d_control.controlInfo.insert(info.split("|").at(0), info.split("|").at(1));
                }
            }

            //扩二的加调控制特殊处理
            if (domNode.hasAttribute("relateId"))
            {
                d_control.relateId = domNode.attribute("relateId").toInt();

                if (domNode.hasAttribute("desc"))
                {
                    d_control.desc = domNode.attribute("desc");
                }
            }
            else
            {
                d_control.relateId = -1;
                d_control.desc = "";
            }

            domNode = domNode.nextSiblingElement();
        }
    }
}

void CheckBox::processSTTC_SC()
{
    DeviceID id;
    id << d_deviceId;

    CmdRequest cmdRequest;
    cmdRequest.m_systemNumb = id.sysID;
    cmdRequest.m_deviceNumb = id.devID;
    cmdRequest.m_extenNumb = id.extenID;

    cmdRequest.m_modeID = d_modeId;
    cmdRequest.m_cmdID = d_control.cmdid;

    QMap<QString, QVariant> params;
    params = d_control.controlInfo;
    params.insert("DelivCtrl", (mCheckBox->isChecked()) ? 1 : 2);
    cmdRequest.m_paramdataMap = params;

    JsonWriter writer;
    writer& cmdRequest;
    RedisHelper::getInstance().setData("过程控制命令发送数据", writer.GetQString());

    emit signalsCmdDeviceJson(writer.GetString());
}

void CheckBox::processSTTC_JT()
{
    DeviceID id;
    id << d_deviceId;

    CmdRequest cmdRequest;
    cmdRequest.m_systemNumb = id.sysID;
    cmdRequest.m_deviceNumb = id.devID;
    cmdRequest.m_extenNumb = id.extenID;

    cmdRequest.m_modeID = d_modeId;
    cmdRequest.m_cmdID = d_control.cmdid;

    QMap<QString, QVariant> params;
    if (d_control.cmdid == 10)
        params.insert("RangAndAdjus", (mCheckBox->isChecked()) ? 1 : 2);
    if (d_control.cmdid == 11)
        params.insert("RemCtrlPlus", (mCheckBox->isChecked()) ? 1 : 2);
    if (d_control.cmdid == 42)
        params.insert("ModulCtrl", (mCheckBox->isChecked()) ? 1 : 2);

    cmdRequest.m_paramdataMap = params;

    JsonWriter writer;
    writer& cmdRequest;
    RedisHelper::getInstance().setData("过程控制命令发送数据", writer.GetQString());

    emit signalsCmdDeviceJson(writer.GetString());
}

void CheckBox::processSkuo2_SC()
{
    //  只有当前目标数小于公共单元里面的目标数才可以下发命令
    auto statusReportMsg = GlobalData::getExtenStatusReportData(d_deviceId);
    //为了赶进度，写死了固定从单元1取目标数
    int targetNum = statusReportMsg.unitReportMsgMap[1].paramMap["TargetNum"].toInt();
    int curTargetNum = d_update.targetId;
    if (curTargetNum > targetNum)
    {
        if (d_targetNumOverSizeCount > 0)
        {
            d_targetNumOverSizeCount = 0;
            return;
        }
        else
        {
            d_targetNumOverSizeCount++;
            QMessageBox::warning(this, QString("提示"), QString("所控参数目标数小于当前目标数,命令下发失败"), QString("确定"));
            mCheckBox->setCheckState(Qt::Unchecked);
            return;
        }
    }

    DeviceID id;
    id << d_deviceId;

    CmdRequest cmdRequest;
    cmdRequest.m_systemNumb = id.sysID;
    cmdRequest.m_deviceNumb = id.devID;
    cmdRequest.m_extenNumb = id.extenID;

    cmdRequest.m_modeID = d_modeId;
    cmdRequest.m_cmdID = d_control.cmdid;

    QMap<QString, QVariant> params;
    params = d_control.controlInfo;
    params.insert("DelivCtrl", (mCheckBox->isChecked()) ? 1 : 2);
    params.insert("DelivMode", 2);
    cmdRequest.m_paramdataMap = params;

    JsonWriter writer;
    writer& cmdRequest;
    RedisHelper::getInstance().setData("过程控制命令发送数据", writer.GetQString());

    emit signalsCmdDeviceJson(writer.GetString());
}

void CheckBox::processSkuo2_JT()
{
    //  只有当前目标数小于公共单元里面的目标数才可以下发命令
    auto statusReportMsg = GlobalData::getExtenStatusReportData(d_deviceId);
    //为了赶进度，写死了固定从单元1取目标数
    int targetNum = statusReportMsg.unitReportMsgMap[1].paramMap["TargetNum"].toInt();
    int curTargetNum = d_update.targetId;
    if (curTargetNum > targetNum)
    {
        if (d_targetNumOverSizeCount > 0)
        {
            d_targetNumOverSizeCount = 0;
            return;
        }
        else
        {
            d_targetNumOverSizeCount++;
            QMessageBox::warning(this, QString("提示"), QString("所控参数目标数小于当前目标数,命令下发失败"), QString("确定"));
            mCheckBox->setCheckState(Qt::Unchecked);
            return;
        }
    }

    DeviceID id;
    id << d_deviceId;

    CmdRequest cmdRequest;
    cmdRequest.m_systemNumb = id.sysID;
    cmdRequest.m_deviceNumb = id.devID;
    cmdRequest.m_extenNumb = id.extenID;

    cmdRequest.m_modeID = d_modeId;
    cmdRequest.m_cmdID = d_control.cmdid;

    QMap<QString, QVariant> params = d_control.controlInfo;

    if (d_relateBox == nullptr)
        return;
    int relateValue = d_relateBox->getValue().toInt();

    if (d_control.desc == "遥伪" || d_control.desc == "测伪")
    {
        params.insert("PNCodeAdd", (mCheckBox->isChecked()) ? 1 : 2);
        params.insert("DataCodeAdd", (relateValue == 1) ? 1 : 2);
    }
    if (d_control.desc == "遥数" || d_control.desc == "测数")
    {
        params.insert("PNCodeAdd", (relateValue == 1) ? 1 : 2);
        params.insert("DataCodeAdd", (mCheckBox->isChecked()) ? 1 : 2);
    }

    cmdRequest.m_paramdataMap = params;

    JsonWriter writer;
    writer& cmdRequest;
    RedisHelper::getInstance().setData("过程控制命令发送数据", writer.GetQString());

    emit signalsCmdDeviceJson(writer.GetString());
}

void CheckBox::processSYTHSMJ_SC()
{
    DeviceID id;
    id << d_deviceId;

    CmdRequest cmdRequest;
    cmdRequest.m_systemNumb = id.sysID;
    cmdRequest.m_deviceNumb = id.devID;
    cmdRequest.m_extenNumb = id.extenID;

    cmdRequest.m_modeID = d_modeId;
    cmdRequest.m_cmdID = d_control.cmdid;

    QMap<QString, QVariant> params;
    params = d_control.controlInfo;
    params.insert("DelivCtrl", (mCheckBox->isChecked()) ? 1 : 2);
    cmdRequest.m_paramdataMap = params;

    JsonWriter writer;
    writer& cmdRequest;
    RedisHelper::getInstance().setData("过程控制命令发送数据", writer.GetQString());

    emit signalsCmdDeviceJson(writer.GetString());
}

void CheckBox::processSYTHSMJ_JT()
{
    DeviceID id;
    id << d_deviceId;

    UnitParamRequest unitParamRequest;

    unitParamRequest.m_systemNumb = id.sysID;
    unitParamRequest.m_deviceNumb = id.devID;
    unitParamRequest.m_extenNumb = id.extenID;

    JsonWriter writer;
    writer& unitParamRequest;
    emit signalsCmdDeviceJson(writer.GetString());
}
