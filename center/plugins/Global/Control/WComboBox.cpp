#include "WComboBox.h"
#include "ControlAttribute.h"

#include <QComboBox>
#include <QLabel>

class WComboBoxImpl
{
public:
    ParameterAttribute paraAttr;
    QMap<QVariant, QString> enumValueMap;                   //按协议排序的map
    QMap<int, QPair<QString, QVariant>> enumValueMapOrder;  //按svalue排序的map
    QComboBox* valueWidget{ nullptr };
    QLabel* nameLabel{ nullptr };
};

WComboBox::WComboBox(QWidget* parent)
    : WWidget(parent)
    , m_impl(new WComboBoxImpl)
{
    m_impl->nameLabel = new QLabel(this);
    m_impl->valueWidget = new QComboBox(this);

    m_impl->valueWidget->setFixedWidth(ControlValueWidth);
    m_impl->nameLabel->setFixedWidth(ControlLabelWidth);
    //  m_impl->nameLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    GlobalData::setDeviceControlAlignment(m_impl->nameLabel);
    m_impl->nameLabel->setProperty("nameLabel", "true");

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(InputControlSpace);
    layout->setDirection(QBoxLayout::Direction::LeftToRight);
    layout->addWidget(m_impl->nameLabel);
    layout->addWidget(m_impl->valueWidget);

    // this->setMinimumWidth(ControlWidget);
    this->setFixedWidth(ControlWidgetI);
    this->setMaximumHeight(ControlHeight);

    connect(m_impl->valueWidget, &QComboBox::currentTextChanged, this, &WComboBox::currentTextChanged);
}

WComboBox::~WComboBox() { delete m_impl; }

void WComboBox::setNameWidth(int w)
{
    if (w <= 0)
    {
        return;
    }
    auto tempWidth = m_impl->nameLabel->width();
    m_impl->nameLabel->setFixedWidth(w);
    tempWidth = width() + w - tempWidth;
    setFixedWidth(tempWidth);
}
void WComboBox::setValueWidth(int w)
{
    if (w <= 0)
    {
        return;
    }
    auto tempWidth = m_impl->valueWidget->width();
    m_impl->valueWidget->setFixedWidth(w);
    tempWidth = width() + w - tempWidth;
    setFixedWidth(tempWidth);
}
void WComboBox::setControlHeight(int h)
{
    if (h <= 0)
    {
        return;
    }
    m_impl->valueWidget->setFixedHeight(h);
    setMinimumHeight(h);
}

void WComboBox::setAttr(const ParameterAttribute& attr, const QMap<QString, QVariant>& map)
{
    paramAttr = attr;
    setObjectName(attr.id);
    setItem(map);
    setName(attr.desc);
    setUnit(attr.unit);
    setValue(attr.initValue);
}

void WComboBox::setAttrOrder(const ParameterAttribute& attr, const QMap<int, QPair<QString, QVariant>>& map)
{
    paramAttr = attr;
    setObjectName(attr.id);
    setItemOrder(map);
    setName(attr.desc);
    setUnit(attr.unit);
    setValue(attr.initValue);
}
void WComboBox::addItem(const QVariant& text, const QVariant& data)
{
    auto tempText = text.toString().trimmed();
    (m_impl->enumValueMap)[data] = tempText;
    m_impl->valueWidget->addItem(tempText, data);
}
void WComboBox::setItem(const QMap<QString, QVariant>& map)
{
    /* 这里两个循环
     * 第一个循环是将map里面的值与key反过来存储
     * 第二个循环是combobox里面的数据其实就是数字,返回来再插入借用map的自动排序
     */
    m_impl->valueWidget->blockSignals(true);
    QVariant curData = m_impl->valueWidget->currentData();
    m_impl->valueWidget->clear();
    m_impl->enumValueMap.clear();

    for (auto iter = map.begin(); iter != map.end(); ++iter)
    {
        if (iter.key() != QString("无此参数"))
        {
            bool ok = false;
            iter.value().toInt(&ok);
            if (ok)
            {
                (m_impl->enumValueMap)[iter.value().toInt()] = iter.key().trimmed();
            }
            else
            {
                (m_impl->enumValueMap)[iter.value()] = iter.key().trimmed();
            }
        }
    }

    for (auto iter = m_impl->enumValueMap.begin(); iter != m_impl->enumValueMap.end(); ++iter)
    {
        auto text = iter.value();
        text = text.replace(":R", "").replace(":G", "").replace(":Y", "");  // 如果有LED颜色的值，在ComboBox中不需要显示
        m_impl->valueWidget->addItem(text, iter.key());
    }
    m_impl->valueWidget->blockSignals(false);
    setValue(curData);
}

void WComboBox::setItemOrder(const QMap<int, QPair<QString, QVariant>>& map)
{
    m_impl->valueWidget->blockSignals(true);
    QVariant curData = m_impl->valueWidget->currentData();
    m_impl->valueWidget->clear();
    m_impl->enumValueMapOrder.clear();

    m_impl->enumValueMapOrder = map;

    for (auto iter = m_impl->enumValueMapOrder.begin(); iter != m_impl->enumValueMapOrder.end(); ++iter)
    {
        auto text = iter.value().first;
        if (text.contains("无此参数"))
            continue;
        text = text.replace(":R", "").replace(":G", "").replace(":Y", "");  // 如果有LED颜色的值，在ComboBox中不需要显示
        m_impl->valueWidget->addItem(text, iter.value().second);
    }
    m_impl->valueWidget->blockSignals(false);
    setValue(curData);
}

void WComboBox::resetValue() { m_impl->valueWidget->setCurrentIndex(0); }

void WComboBox::setValue(const QVariant& value)
{
    if (!m_impl->enumValueMapOrder.isEmpty())
    {
        int flag = 0;
        for (auto order : m_impl->enumValueMapOrder.keys())
        {
            auto pair = m_impl->enumValueMapOrder[order];
            //之所以先去enumValueMap找值是因为有些下拉框因为关联关系时设置的值在enumValueMap里面而不是在enumValueMapOrder里
            //所以这里先去enumValueMap里找值是为了应对关联关系的下拉框，没找到的话再去enumValueMapOrder里面去找，都没有找到
            //就默认第一位数
            auto find = m_impl->enumValueMap.find(value);
            if (find != m_impl->enumValueMap.end())
            {
                m_impl->valueWidget->setCurrentText(find.value());
            }
            else if (pair.second == value)
            {
                m_impl->valueWidget->setCurrentText(pair.first);
            }
            else
            {
                flag++;
            }
        }
        if (flag == m_impl->enumValueMapOrder.size())
            m_impl->valueWidget->setCurrentIndex(0);  //遍历了整个map都没有这个值的话默认设置为第一个值
    }
    else
    {
        auto find = m_impl->enumValueMap.find(value);
        if (find != m_impl->enumValueMap.end())
        {
            m_impl->valueWidget->setCurrentText(find.value());
        }
        else
        {
            m_impl->valueWidget->setCurrentIndex(0);
        }
    }
}

void WComboBox::setName(const QString& name)
{
    QString setName = name.trimmed();
    if (!setName.endsWith(":"))
    {
        setName += ":";
    }
    m_impl->nameLabel->setText(setName);
}

QVariant WComboBox::value() { return m_impl->valueWidget->currentData(); }
QVariant WComboBox::value(bool& isValid)
{
    auto curData = m_impl->valueWidget->currentData();
    isValid = curData.isValid() && m_impl->valueWidget->currentIndex() >= 0;
    return curData;
}
QString WComboBox::text() { return m_impl->valueWidget->currentText().trimmed(); }

void WComboBox::setParamStatus(DataBuilder::FormsStatus status)
{
    QColor color = (status == DataBuilder::FormsStatus::Normal) ? QColor(Qt::white) : QColor(Qt::red);

    QPalette palette;
    palette.setColor(QPalette::Background, color);

    m_impl->nameLabel->setAutoFillBackground(true);
    m_impl->nameLabel->setPalette(palette);
}
void WComboBox::setValueStyleSheet(const QString& qss) { m_impl->valueWidget->setStyleSheet(qss); }
void WComboBox::setEnabled(bool enable)
{
    m_impl->valueWidget->setEnabled(enable);
    m_impl->nameLabel->setEnabled(enable);
}

void WComboBox::currentTextChanged(const QString& arg1)
{
    QVariant data = value();
    emit sig_valueChange(arg1);
    emit sig_valueChange(data);
}
