#include "WPlainTextEdit.h"
#include "ControlAttribute.h"
#include <QPlainTextEdit>

class WStrPlainEditImpl
{
public:
    QPlainTextEdit* valueWidget{ nullptr };

    QList<ParameterAttribute> setAttr;
};

WPlainTextEdit::WPlainTextEdit(QWidget* parent)
    : WWidget(parent)
    , m_impl(new WStrPlainEditImpl)
{
    m_impl->valueWidget = new QPlainTextEdit(this);
    m_impl->valueWidget->setReadOnly(true);
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_impl->valueWidget);
}

WPlainTextEdit::~WPlainTextEdit() { delete m_impl; }

void WPlainTextEdit::setValue(const QVariant& value) {}

QVariant WPlainTextEdit::value() { return m_impl->valueWidget->toPlainText().trimmed(); }

void WPlainTextEdit::init(const QVector<QString>& attr, CmdAttribute cmdAttribute)
{
    setObjectName("WPlainTextEdit");

    m_initValue = attr;
    m_currentCmdAttribute = cmdAttribute;

    if (cmdAttribute.resultRecycle.isEmpty())
    {
        m_type = 0;
    }
    else
    {
        m_type = 1;
    }
}

void WPlainTextEdit::initUI() { m_impl->valueWidget->clear(); }

void WPlainTextEdit::dealCmdValue(int cmdID, int modeID, QMap<QString, QVariant> setParamMap, DeviceID deviceID)
{
    QVector<QString> value;
    QString time = GlobalData::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    value.push_back(time);
    //这里是将参数进行排序，且把值替换为上报上来的值
    QList<ParameterAttribute> resultList = m_currentCmdAttribute.resultList;
    for (auto& parm : resultList)
    {
        for (auto cmd : setParamMap.keys())
        {
            if (parm.id == cmd)
            {
                parm.initValue = setParamMap.value(cmd);
            }
        }
    }
    /////////////////////中间的判断是把不需要的参数去掉//////////////////////
    if ((deviceID.sysID == 7 && deviceID.devID == 0 && cmdID == 9) || (deviceID.sysID == 7 && deviceID.devID == 1 && cmdID == 9) ||
        (deviceID.sysID == 7 && deviceID.devID == 0 && cmdID == 10) || (deviceID.sysID == 7 && deviceID.devID == 1 && cmdID == 10))
    {
        for (auto& parm : resultList)
        {
            if (!parm.isShow || parm.id == "TestState")
            {
                continue;
            }
            else if (!parm.unit.isEmpty())
            {
                value.push_back(parm.initValue.toString() + parm.unit);
            }
            else
            {
                value.push_back(parm.initValue.toString());
            }
        }
    }
    else
    {
        for (auto& parm : resultList)
        {
            if (!parm.isShow)
            {
                continue;
            }
            else if (!parm.unit.isEmpty())
            {
                value.push_back(parm.initValue.toString() + parm.unit);
            }
            else
            {
                value.push_back(parm.initValue.toString());
            }
        }
    }
    /////////////////////////////////////////////////////////////////
    //下面是将数据添加进显示框里
    if (value.size() > 0 && value.size() == m_initValue.size())
    {
        int size = value.size();
        QString text;
        for (int i = 0; i < size; ++i)
        {
            text += m_initValue[i] + value[i] + "     ";
        }
        m_impl->valueWidget->appendPlainText(text);
    }
}

void WPlainTextEdit::dealCmdMultiValue(int cmdID, int modeID, QMap<int, QList<QPair<QString, QVariant>>> multiParamMap, DeviceID deviceID)
{
    for (auto targetNum : multiParamMap.keys())
    {
        auto dataList = multiParamMap.value(targetNum);

        auto recycleNum = dataList.size();
        for (int i = 0; i < recycleNum; ++i)
        {
            auto pair = dataList.at(i);
            m_impl->valueWidget->appendPlainText(pair.second.toString());
        }
    }
}

int WPlainTextEdit::getParamType() { return m_type; }
