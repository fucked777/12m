#include "WComboBoxOnly.h"

#include "ControlAttribute.h"

#include <QComboBox>
#include <QLabel>

class WComboBoxOnlyImpl
{
public:
    ParameterAttribute paraAttr;
    QMap<QVariant, QString> enumValueMap;
    QComboBox* valueWidget{ nullptr };
};

WComboBoxOnly::WComboBoxOnly(QWidget* parent)
    : WWidget(parent)
    , m_impl(new WComboBoxOnlyImpl)
{
    m_impl->valueWidget = new QComboBox(this);

    m_impl->valueWidget->setFixedWidth(ControlValueWidth);

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(InputControlSpace);
    layout->setDirection(QBoxLayout::Direction::LeftToRight);
    layout->addWidget(m_impl->valueWidget);

    // this->setMinimumWidth(ControlWidget);
    this->setMaximumHeight(ControlHeight);

    connect(m_impl->valueWidget, &QComboBox::currentTextChanged, this, &WComboBoxOnly::currentTextChanged);
}

WComboBoxOnly::~WComboBoxOnly() { delete m_impl; }

void WComboBoxOnly::setNameWidth(int w)
{
    if (w <= 0)
    {
        return;
    }
}
void WComboBoxOnly::setValueWidth(int w)
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
void WComboBoxOnly::setControlHeight(int h)
{
    if (h <= 0)
    {
        return;
    }
    m_impl->valueWidget->setFixedHeight(h);
    setMinimumHeight(h);
}

void WComboBoxOnly::setAttr(const ParameterAttribute& attr, const QMap<QString, QVariant>& map)
{
    paramAttr = attr;
    setObjectName(attr.id);
    setItem(map);
    setName(attr.desc);
    setUnit(attr.unit);
    setValue(attr.initValue);
}
void WComboBoxOnly::addItem(const QVariant& text, const QVariant& data)
{
    auto tempText = text.toString().trimmed();
    (m_impl->enumValueMap)[data] = tempText;
    m_impl->valueWidget->addItem(tempText, data);
}
void WComboBoxOnly::setItem(const QMap<QString, QVariant>& map)
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
            (m_impl->enumValueMap)[iter.value()] = iter.key().trimmed();
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

void WComboBoxOnly::resetValue() { m_impl->valueWidget->setCurrentIndex(0); }

void WComboBoxOnly::setValue(const QVariant& value)
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

void WComboBoxOnly::setName(const QString& name)
{
    QString setName = name.trimmed();
    if (!setName.endsWith(":"))
    {
        setName += ":";
    }
}

QVariant WComboBoxOnly::value() { return m_impl->valueWidget->currentData(); }

QVariant WComboBoxOnly::value(bool& isValid)
{
    auto curData = m_impl->valueWidget->currentData();
    isValid = curData.isValid() && m_impl->valueWidget->currentIndex() >= 0;
    return curData;
}
QString WComboBoxOnly::text() { return m_impl->valueWidget->currentText().trimmed(); }

void WComboBoxOnly::setParamStatus(DataBuilder::FormsStatus status)
{
    QColor color = (status == DataBuilder::FormsStatus::Normal) ? QColor(Qt::white) : QColor(Qt::red);

    QPalette palette;
    palette.setColor(QPalette::Background, color);
}

void WComboBoxOnly::setValueStyleSheet(const QString& qss) { m_impl->valueWidget->setStyleSheet(qss); }

void WComboBoxOnly::setEnabled(bool enable) { m_impl->valueWidget->setEnabled(enable); }

void WComboBoxOnly::currentTextChanged(const QString& arg1)
{
    QVariant data = value();
    emit sig_valueChange(arg1);
    emit sig_valueChange(data);
}
