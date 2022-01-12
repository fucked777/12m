#include "WMatrixLEDWidget.h"

#include <QGridLayout>
#include <QPushButton>

#define LEDWidth  16
#define LEDHeight 16

class WMatrixLED : public QPushButton
{
public:
    QIcon greenIcon;
    QIcon greyIcon;

    WMatrixLED(QWidget* parent = nullptr)
        : QPushButton(parent)
    {
        setStyleSheet(".QPushButton {border-style: none;background-color: rgb(240,240,240);border-radius: 0px;min-width: 16px;min-height: 16px;}");

        QPixmap pixmap;

        auto len = static_cast<uint>(sizeof(gGreyIcon) / sizeof(gGreyIcon[0]));
        pixmap.loadFromData(gGreyIcon, len);
        greyIcon = QIcon(pixmap);

        len = static_cast<uint>(sizeof(gGreenIcon) / sizeof(gGreenIcon[0]));
        pixmap.loadFromData(gGreenIcon, len);
        greenIcon = QIcon(pixmap);
    }

    void setStatus(int status)
    {
        if (status == 1)
        {
            setIcon(greenIcon);
            m_flagStatus = 1;
        }
        else
        {
            setIcon(greyIcon);
            m_flagStatus = 0;
        }
    }

    void setFlagStartPosition(int flagStartPosition) { m_flagStartPosition = flagStartPosition; }

    int getFlag() { return m_flagStatus; }

    int getFlagStartPosition() { return m_flagStartPosition; }

private:
    int m_flagStatus = -1;
    int m_flagStartPosition = -1;
};

WMatrixLEDWidget::WMatrixLEDWidget(QWidget* parent)
    : WWidget(parent)
{
    ledGridLayout = new QGridLayout(this);
    ledGridLayout->setSpacing(0);
    setLayout(ledGridLayout);
}

void WMatrixLEDWidget::init(const QList<QPair<QString, QString>>& idDescList, const QMap<int, QString>& uValueDescMap)
{
    if (idDescList.isEmpty() || uValueDescMap.isEmpty())
        return;

    ledGridLayout->addWidget(new QLabel("输入", this), 0, 1, 1, uValueDescMap.size() - 1, Qt::AlignCenter);
    ledGridLayout->addWidget(new QLabel("输\n出", this), 1, 0, idDescList.size() - 1, 1, Qt::AlignCenter);

    int row = 2;
    for (auto& pair : idDescList)
    {
        auto descLab = new QLabel(this);
        descLab->setText(QString("%1").arg(row - 1));

        ledGridLayout->addWidget(descLab, row++, 1, 1, 1, Qt::AlignCenter);
    }

    int col = 2;
    if (uValueDescMap.contains(0))  //如果第一个输入口是0 从0开始
    {
        for (auto& desc : uValueDescMap)
        {
            auto descLab = new QLabel(this);
            descLab->setText(QString("%1").arg(col - 2));

            ledGridLayout->addWidget(descLab, 1, col++, 1, 1, Qt::AlignCenter);
        }
    }
    else  //第一个输入口不为0  从1开始
    {
        for (auto& desc : uValueDescMap)
        {
            auto descLab = new QLabel(this);
            descLab->setText(QString("%1").arg(col - 1));

            ledGridLayout->addWidget(descLab, 1, col++, 1, 1, Qt::AlignCenter);
        }
    }

    row = 2;
    for (auto& pair : idDescList)
    {
        QList<WMatrixLED*> ledList;
        col = 2;
        for (auto& uValue : uValueDescMap.keys())
        {
            auto ledBtn = new WMatrixLED(this);
            if (uValueDescMap.contains(0))  //如果第一个输入口是0 从0开始
            {
                ledBtn->setFlagStartPosition(0);
            }
            else  //第一个输入口不为0  从1开始
            {
                ledBtn->setFlagStartPosition(1);
            }
            ledBtn->setFixedSize(LEDWidth, LEDHeight);
            ledBtn->setStatus(0);
            ledGridLayout->addWidget(ledBtn, row, col, 1, 1, Qt::AlignCenter);

            ledList.append(ledBtn);
            ++col;
        }
        mIdRowLedMap.insert(pair.first, ledList);
        ++row;
    }
}

void WMatrixLEDWidget::resetDefaultStatus()
{
    for (auto& ledList : mIdRowLedMap)
    {
        for (auto ledWidget : ledList)
            ledWidget->setStatus(0);
    }
}

void WMatrixLEDWidget::setValue(const QVariant& value)
{
    if (!value.canConvert<QMap<QString, QVariant>>())
        return;

    auto idValueMap = value.toMap();
    for (auto& id : idValueMap.keys())
    {
        if (mIdRowLedMap.contains(id))
        {
            bool isOk;
            auto value = idValueMap.value(id).toInt(&isOk);
            if (!isOk)
                continue;

            auto& ledList = mIdRowLedMap.value(id);
            if (ledList.at(0))
            {
                auto led = ledList.at(0);  //只取第一个灯看是否是从0开始计数
                if (led->getFlagStartPosition() == 0)
                {
                    if (value < 0 || value >= ledList.size())
                        continue;

                    if (ledList[value]->getFlag() == 1)  //避免重复刷新灯的颜色导致界面卡顿  只刷新一次
                        continue;
                    // 该行LED状态设置为0
                    for (auto ledBtn : ledList)
                        ledBtn->setStatus(0);

                    // 设置上报的LED状态
                    ledList[value]->setStatus(1);
                }
                else if (led->getFlagStartPosition() == 1)  //从1开始的话，就需要减1避免越界
                {
                    if (value - 1 < 0 || value - 1 >= ledList.size())
                        continue;

                    if (ledList[value - 1]->getFlag() == 1)  //避免重复刷新灯的颜色导致界面卡顿  只刷新一次
                        continue;
                    // 该行LED状态设置为0
                    for (auto ledBtn : ledList)
                        ledBtn->setStatus(0);

                    // 设置上报的LED状态
                    ledList[value - 1]->setStatus(1);
                }
                else
                {
                    continue;
                }
            }
        }
    }
}

QVariant WMatrixLEDWidget::value() { return QVariant(); }
