
#include "WRadioButton.h"
#include "CConverter.h"
#include "ControlAttribute.h"
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>

class WRadioButtonImpl
{
public:
    QMap<int, QRadioButton*> enumValueMap;
    QHBoxLayout* mainLayout{ nullptr };
    QButtonGroup* buttonGroup{ nullptr };
    QLabel* nameLabel{ nullptr };
    QWidget* buttonWidget{ nullptr };
};

WRadioButton::WRadioButton(QWidget* parent)
    : WWidget(parent)
    , m_impl(new WRadioButtonImpl)
{
    m_impl->nameLabel = new QLabel(this);
    m_impl->nameLabel->setFixedWidth(ControlLabelWidth);
    m_impl->nameLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    m_impl->buttonGroup = new QButtonGroup(this);
    m_impl->buttonWidget = new QWidget(this);

    m_impl->mainLayout = new QHBoxLayout(this);
    m_impl->mainLayout->setContentsMargins(0, 0, 0, 0);
    m_impl->mainLayout->setSpacing(InputControlSpace);
    m_impl->mainLayout->setDirection(QBoxLayout::Direction::LeftToRight);
    m_impl->mainLayout->addWidget(m_impl->nameLabel);
    m_impl->mainLayout->addWidget(m_impl->buttonWidget);

    m_impl->buttonWidget->setFixedWidth(ControlValueWidth);

    this->setFixedWidth(ControlWidgetI);
    this->setMaximumHeight(ControlHeight);

    //    connect(m_impl->valueWidget, &QLineEdit::textChanged, this, &WRadioButton::textChange);
    //    connect(m_impl->valueWidget, &QLineEdit::editingFinished, this,
    //    &WRadioButton::editingFinished);
}

WRadioButton::~WRadioButton() { delete m_impl; }
void WRadioButton::setItem(const QMap<QString, QVariant>& itemMap)
{
    // 记录当前的数据
    auto valueWidget = m_impl->buttonWidget->width();
    auto valueHeight = m_impl->buttonWidget->height();
    auto isEnable = m_impl->buttonWidget->isEnabled();

    if (m_impl->buttonGroup != nullptr)
    {
        m_impl->buttonGroup->setParent(nullptr);
        delete m_impl->buttonGroup;
    }
    if (m_impl->buttonWidget != nullptr)
    {
        m_impl->mainLayout->removeWidget(m_impl->buttonWidget);
        m_impl->buttonWidget->setParent(nullptr);
        delete m_impl->buttonWidget;
    }
    // isEnable
    m_impl->buttonGroup = new QButtonGroup(this);
    m_impl->buttonWidget = new QWidget(this);
    m_impl->buttonWidget->setEnabled(isEnable);
    m_impl->buttonWidget->setFixedSize(valueWidget, valueHeight);
    m_impl->mainLayout->addWidget(m_impl->buttonWidget);

    auto layout = new QHBoxLayout(m_impl->buttonWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(20);
    layout->setDirection(QBoxLayout::Direction::LeftToRight);

    /* 这里两个循环
     * 第一个循环是将map里面的值与key反过来存储
     * 第二个循环是combobox里面的数据其实就是数字,返回来再插入借用map的自动排序
     */
    m_impl->enumValueMap.clear();

    for (auto iter = itemMap.begin(); iter != itemMap.end(); ++iter)
    {
        auto radio = new QRadioButton(m_impl->buttonWidget);
        radio->setText(iter.key().trimmed());
        layout->addWidget(radio);

        auto id = iter->toInt();
        m_impl->buttonGroup->addButton(radio, id);
        (m_impl->enumValueMap)[id] = radio;
    }
    layout->addStretch();

    connect(m_impl->buttonGroup, static_cast<void (QButtonGroup::*)(QAbstractButton*)>(&QButtonGroup::buttonClicked), this,
            &WRadioButton::valueChange);
    // setValue(curData);
}

void WRadioButton::setNameWidth(int w)
{
    if (w <= 0)
    {
        return;
    }
    m_impl->nameLabel->setFixedWidth(w);
}
void WRadioButton::setValueWidth(int w)
{
    if (w <= 0)
    {
        return;
    }
    // 为空是bug
    auto tempWidth = m_impl->buttonWidget->width();
    m_impl->buttonWidget->setFixedWidth(w);
    tempWidth = width() + w - tempWidth;
    setFixedWidth(tempWidth);
}
void WRadioButton::setControlHeight(int h)
{
    if (h <= 0)
    {
        return;
    }
    m_impl->buttonWidget->setFixedHeight(h);
    setMinimumHeight(h);
}

void WRadioButton::setAttr(const ParameterAttribute& attr, const QMap<QString, QVariant>& map)
{
    paramAttr = attr;
    setObjectName(attr.id);
    setName(attr.desc);
    setValue(attr.initValue);
    setItem(map);
}

void WRadioButton::resetValue()
{
    auto buttons = m_impl->buttonGroup->buttons();
    if (!buttons.isEmpty())
    {
        reinterpret_cast<QRadioButton*>(buttons.first())->setChecked(true);
    }
}
void WRadioButton::setValue(const QVariant& value)
{
    bool isOK = false;
    auto valueID = value.toInt(&isOK);
    if (!isOK)
    {
        return;
    }
    auto find = m_impl->enumValueMap.find(valueID);
    if (find != m_impl->enumValueMap.end())
    {
        find.value()->setChecked(true);
    }
}

QVariant WRadioButton::value() { return m_impl->buttonGroup->checkedId(); }
QVariant WRadioButton::value(bool& isValid)
{
    isValid = true;
    return value();
}
QString WRadioButton::text()
{
    auto button = m_impl->buttonGroup->checkedButton();
    return button != nullptr ? button->text() : "";
}

void WRadioButton::setName(const QString& name)
{
    QString setName = name.trimmed();
    if (!setName.endsWith(":"))
    {
        setName += ":";
    }
    m_impl->nameLabel->setText(setName);
}

void WRadioButton::setParamStatus(DataBuilder::FormsStatus status)
{
    QColor color = (status == DataBuilder::FormsStatus::Normal) ? QColor(Qt::white) : QColor(Qt::red);

    QPalette palette;
    palette.setColor(QPalette::Background, color);

    m_impl->nameLabel->setAutoFillBackground(true);
    m_impl->nameLabel->setPalette(palette);
}

void WRadioButton::setEnabled(bool enable) { m_impl->buttonWidget->setEnabled(enable); }
void WRadioButton::setValueStyleSheet(const QString& qss)
{
    auto buttons = m_impl->buttonGroup->buttons();
    for (auto item : buttons)
    {
        item->setStyleSheet(qss);
    }
}

void WRadioButton::valueChange(QAbstractButton* button)
{
    emit sig_valueChange(button->text());
    emit sig_valueChange(m_impl->buttonGroup->id(button));
}
