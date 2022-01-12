#include "MutexCheckBox.h"

#include <QHBoxLayout>
#include "DeviceProcessMessageDefine.h"
#include "RedisHelper.h"
#include "ProtocolXmlTypeDefine.h"
#include "GlobalData.h"
#include "SystemMainScreenServer.h"

#include "MainScreenControlFactory.h"

MutexCheckBox::MutexCheckBox(const QDomElement& domEle, QWidget* parent, const int deviceId, const int modeId)
    : BaseWidget(domEle, parent), d_deviceId(deviceId), d_modeId(modeId)
{
    parseNode(domEle);

    initLayout();

    setText(d_text);

    setCheckable(true);
}

void MutexCheckBox::setText(const QString& text) { mCheckBox->setText(text); }

QString MutexCheckBox::getText() const { return mCheckBox->text(); }

void MutexCheckBox::setValue(const QVariant &value)
{
    if(value.toInt() == 1)
        mCheckBox->setCheckState(Qt::Checked);
    else
        mCheckBox->setCheckState(Qt::Unchecked);
}

QVariant MutexCheckBox::getValue() const
{
    return mCheckBox->isChecked() ? 1 : 0;
}

void MutexCheckBox::setCheckable(const bool enabled)
{
    d_checkable = enabled;
    mCheckBox->setEnabled(enabled);
}

bool MutexCheckBox::getCheckable() const
{
    return d_checkable;
}

int MutexCheckBox::getId() const
{
    return d_id;
}

bool MutexCheckBox::isControlType()
{
    return d_isControlType;
}

void MutexCheckBox::onStateChanged(int state)
{
    if(d_isControlType)
    {
        if(parent() == nullptr) return;

        if(d_childBoxs.isEmpty())
        {
            QList<MutexCheckBox*> checkBoxs = parent()->findChildren<MutexCheckBox*>();
            for(MutexCheckBox* checkBox : checkBoxs)
            {
                for(int childId : d_childId)
                {
                    if(checkBox->getId() == childId)
                    {
                        d_childBoxs.append(checkBox);
                        break;
                    }
                }
            }
        }

        for(MutexCheckBox* checkBox : d_childBoxs)
        {
            checkBox->setCheckable(state == Qt::Checked);
        }
    }
    if(getValue().toBool())
    {
        //互斥处理
        // QMap<设备id, QMap<模式id, QMap<单元id, QMap<目标id, QMap<QString, QWidget*>>>>>
        QMap<int, QMap<int, QMap<int, QMap<int, QMap<QString, QWidget*>>>>> deviceMap = MainScreenControlFactory::getWidgetMap();

        foreach(Mutex mutex,d_mutexs)
        {
           if(deviceMap.contains(mutex.deviceId))
           {
               QMap<int, QMap<int, QMap<int, QMap<QString, QWidget*>>>> modeMap = deviceMap.find(mutex.deviceId).value();
               if(modeMap.contains(mutex.modeId))
               {
                   QMap<int, QMap<int, QMap<QString, QWidget*>>> unitMap = modeMap.find(mutex.modeId).value();
                   if(unitMap.contains(mutex.unitId))
                   {
                       QMap<int, QMap<QString, QWidget*>> targetMap = unitMap.find(mutex.unitId).value();
                       if(targetMap.contains(mutex.targetId))
                       {
                           QMap<QString, QWidget*> paramMap = targetMap.find(mutex.targetId).value();
                           if(paramMap.contains(mutex.paramId))
                           {
                               MutexCheckBox* widget = (MutexCheckBox*)paramMap.find(mutex.paramId).value();
                               if(widget->getValue().toBool())
                               {
                                   widget->setValue(0);
                               }
                           }
                       }
                   }
               }
           }
        }
    }
}

void MutexCheckBox::initLayout()
{
    mCheckBox = new QCheckBox(this);
    connect(mCheckBox, &QCheckBox::stateChanged, this, &MutexCheckBox::onStateChanged);
    if(!isControlType())  mCheckBox->setEnabled(false);
    else mCheckBox->setEnabled(true);

    auto hLayout = new QHBoxLayout;
    hLayout->setSpacing(0);
    hLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(hLayout);

    hLayout->addWidget(mCheckBox);
}

void MutexCheckBox::parseNode(const QDomElement& domEle)
{
    if (domEle.isNull())
    {
        return;
    }
    auto domNode = domEle.toElement();

    (domEle.hasAttribute("childId")) ? (d_isControlType = true) : (d_isControlType = false);
    d_text = domEle.attribute("text");
    d_id = domEle.attribute("id").toInt();

    if(d_isControlType)
    {
        QStringList strChildren = domEle.attribute("childId").split("|");
        for(QString& strChild : strChildren)
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
    }

    auto domChildNode = domEle.firstChild();
    Mutex temp;
    QString tagName;
    QDomElement subDomEle;
    while (!domChildNode.isNull())
    {
        subDomEle = domChildNode.toElement();
        if (subDomEle.isNull())
        {
            domChildNode = domChildNode.nextSibling();
            continue;
        }

        tagName = subDomEle.tagName();
        if (tagName == "Mutex")
        {
            temp.deviceId = subDomEle.attribute("deviceId").toUInt(nullptr,16);
            temp.modeId = subDomEle.attribute("modeId").toUInt(nullptr,16);
            temp.unitId = subDomEle.attribute("unitId").toUInt();
            temp.targetId = subDomEle.attribute("targetId").toUInt();
            temp.paramId = subDomEle.attribute("paramId","");

            d_mutexs.append(temp);
        }

        domChildNode = domChildNode.nextSibling();
    }
}
