#include "WMatrixWidget.h"

#include <QGroupBox>
#include <QHBoxLayout>

#include "WMatrixDiagramWidget.h"
#include "WMatrixLEDWidget.h"

WMatrixWidget::WMatrixWidget(QWidget* parent)
    : WWidget(parent)
{
    setObjectName("WMatrixWidget");

    mLedWidget = new WMatrixLEDWidget(this);

    auto ledHLayout = new QHBoxLayout(this);
    ledHLayout->addWidget(mLedWidget);

    auto ledGroupBox = new QGroupBox("矩阵", this);
    ledGroupBox->setLayout(ledHLayout);

    mDiagramWidget = new WMatrixDiagramWidget(this);
    connect(mDiagramWidget, &WMatrixDiagramWidget::triggered,
            [=](const QString& attrId, const QVariant& uValue) { emit actionTriggered(attrId, uValue.toInt()); });

    auto diaHLayout = new QHBoxLayout(this);
    diaHLayout->addWidget(mDiagramWidget);

    auto diaGroupBox = new QGroupBox("框图", this);
    diaGroupBox->setLayout(diaHLayout);

    auto hLayout = new QHBoxLayout(this);
    hLayout->addWidget(ledGroupBox);
    hLayout->addWidget(diaGroupBox);
    setLayout(hLayout);
}

WMatrixWidget::~WMatrixWidget() {}

void WMatrixWidget::setMatrixInfo(const QList<QPair<ParameterAttribute, QMap<int, QString>>>& attrEnumList)
{
    QList<QPair<QString, QString>> idDescList;
    QMap<int, QString> allEnumValueDescMap;  // 全部参数的枚举值和描述
    for (int i = 0; i < attrEnumList.size(); ++i)
    {
        auto pair = attrEnumList[i];
        auto attr = pair.first;
        idDescList.append(QPair<QString, QString>(attr.id, attr.desc));

        auto attrEnum = pair.second;
        for (auto key : attrEnum.keys())
        {
            allEnumValueDescMap.insert(key, attrEnum.value(key));
        }
    }

    mLedWidget->init(idDescList, allEnumValueDescMap);
    mDiagramWidget->init(attrEnumList);
}

void WMatrixWidget::setDiagramArrowDirection(ArrowDirection dire) { mDiagramWidget->setArrowDirection(dire); }

void WMatrixWidget::resetDefaultStatus()
{
    mLedWidget->resetDefaultStatus();
    mDiagramWidget->resetDefaultStatus();
}

void WMatrixWidget::setValue(const QVariant& value)
{
    mLedWidget->setValue(value);
    mDiagramWidget->setValue(value);
}

QVariant WMatrixWidget::value() { return QVariant(); }
