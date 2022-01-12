#include "WMatrixDiagramWidget.h"

#include <QLabel>
#include <QPainter>

class ArrowWidget : public QWidget
{
public:
    ArrowWidget(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        mArrowDire = Right;

        mTextWidth = 0;
        mTextHeight = 0;

        mArrowBodyHeight = 1;
        mArrowTipWidth = 10;
        mArrowTipHeight = 10;

        setMinimumWidth(100);
        setMinimumHeight(mArrowTipHeight);

        updatePoint();
    }

    void setArrowDirection(ArrowDirection dire)
    {
        mArrowDire = dire;
        updatePoint();
    }

    void setText(const QString& text)
    {
        mText = text;

        QFontMetrics metrics(font());
        mTextWidth = metrics.width(mText);
        mTextHeight = metrics.height();

        auto minWidth = mTextWidth > width() ? mTextWidth : width();
        setMinimumWidth(minWidth + mArrowTipWidth);
        setMinimumHeight(2 * mTextHeight + mArrowBodyHeight + 4);
        updatePoint();
    }

protected:
    void paintEvent(QPaintEvent* event)
    {
        QPen pen;
        pen.setColor(Qt::black);

        QPainter painter(this);
        painter.setPen(pen);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setBrush(QBrush(Qt::black));

        painter.drawText(mTextPosPoint, mText);
        painter.drawPolygon(mArrowTipPoints, 3);

        pen.setWidth(mArrowBodyHeight);
        painter.setPen(pen);
        painter.drawLine(mArrowBodyStartPoint, mArrowBodyEndPoint);
    }

    void resizeEvent(QResizeEvent* event) { updatePoint(); }

private:
    void updatePoint()
    {
        if (mArrowDire == Right)
        {
            mArrowBodyStartPoint = QPointF(0, (height() - mArrowBodyHeight) / 2);
            mArrowBodyEndPoint = QPointF(width() - mArrowTipWidth, (height() - mArrowBodyHeight) / 2);

            mArrowTipPoints[0] = QPointF(width() - mArrowTipWidth, (height() - mArrowTipHeight) / 2);
            mArrowTipPoints[1] = QPointF(width() - mArrowTipWidth, (height() + mArrowTipHeight) / 2);
            mArrowTipPoints[2] = QPointF(width(), height() / 2);
        }
        else if (mArrowDire == Left)
        {
            mArrowBodyStartPoint = QPointF(width(), (height() - mArrowBodyHeight) / 2);
            mArrowBodyEndPoint = QPointF(mArrowTipWidth, (height() - mArrowBodyHeight) / 2);

            mArrowTipPoints[0] = QPointF(mArrowTipWidth, (height() - mArrowTipHeight) / 2);
            mArrowTipPoints[1] = QPointF(mArrowTipWidth, (height() + mArrowTipHeight) / 2);
            mArrowTipPoints[2] = QPointF(0, height() / 2);
        }

        mTextPosPoint = QPointF((width() - mTextWidth) / 2, mArrowBodyStartPoint.y() - 4);
    }

private:
    ArrowDirection mArrowDire;

    QString mText;
    int mTextWidth;
    int mTextHeight;

    int mArrowBodyHeight;
    int mArrowTipWidth;
    int mArrowTipHeight;

    QPointF mTextPosPoint;
    QPointF mArrowBodyStartPoint, mArrowBodyEndPoint;
    QPointF mArrowTipPoints[3];
};

WMatrixDiagramWidget::WMatrixDiagramWidget(QWidget* parent)
    : WWidget(parent)
{
    mGridLayout = new QGridLayout(this);
    mGridLayout->setHorizontalSpacing(0);
    mGridLayout->setVerticalSpacing(10);
    setLayout(mGridLayout);
}

void WMatrixDiagramWidget::init(const QList<QPair<ParameterAttribute, QMap<int, QString>>>& attrEnumList)
{
    QMap<int, QString> allEnumValueDescMap;  // 全部参数的枚举值和描述
    for (int i = 0; i < attrEnumList.size(); ++i)
    {
        auto pair = attrEnumList[i];

        auto attrEnum = pair.second;
        for (auto key : attrEnum.keys())
        {
            allEnumValueDescMap.insert(key, attrEnum.value(key));
        }
    }
    // 最大行数
    auto maxRow = attrEnumList.size() > allEnumValueDescMap.size() ? attrEnumList.size() : allEnumValueDescMap.size();

    auto backgroundLabel = new QLabel(this);
    backgroundLabel->setStyleSheet("QLabel {background-color: rgb(192, 192, 192); border: 1px solid black; border-radius: 4px;}");
    mGridLayout->addWidget(backgroundLabel, 0, 2, maxRow, 2);

    int row = 0;
    for (auto& uVlaue : allEnumValueDescMap.keys())
    {
        auto descArrow = new ArrowWidget(this);
        descArrow->setText(QString("%2").arg(allEnumValueDescMap.value(uVlaue)));
        descArrow->setMinimumWidth(200);
        mArrowWidgetList.append(descArrow);

        auto valLab = new QLabel(this);
        valLab->setText(QString::number(uVlaue));
        valLab->setMinimumWidth(50);
        valLab->setAlignment(Qt::AlignCenter);
        valLab->setStyleSheet("QLabel {background-color: rgb(240, 240, 240); border: 1px solid black; border-radius: 2px;}");

        mGridLayout->addWidget(descArrow, row, 0, 1, 2, Qt::AlignVCenter | Qt::AlignRight);
        mGridLayout->addWidget(valLab, row, 2, 1, 1, Qt::AlignVCenter | Qt::AlignLeft);
        row++;
    }

    row = 0;
    for (auto& pair : attrEnumList)
    {
        auto attr = pair.first;
        auto menuLab = new MenuLabel(this);
        menuLab->setId(attr.id);
        menuLab->setText("0");
        menuLab->setMinimumWidth(50);
        menuLab->setStyleSheet("QWidget {background-color: rgb(255, 255, 0); border: 1px solid black; border-radius: 2px;}");
        menuLab->setActions(pair.second);
        connect(menuLab, &MenuLabel::triggered, this, [=](const QVariant& value) {
            auto menuLabel = qobject_cast<MenuLabel*>(sender());
            if (!menuLabel)
                return;
            emit triggered(menuLabel->getId(), value);
        });

        mMenuLabMap.insert(attr.id, menuLab);

        auto descArrow = new ArrowWidget(this);
        descArrow->setText(QString("%1").arg(attr.desc));
        descArrow->setMinimumWidth(200);
        mArrowWidgetList.append(descArrow);

        mGridLayout->addWidget(menuLab, row, 3, 1, 1, Qt::AlignVCenter | Qt::AlignRight);
        mGridLayout->addWidget(descArrow, row, 4, 1, 2, Qt::AlignVCenter | Qt::AlignLeft);
        row++;
    }
}

void WMatrixDiagramWidget::setArrowDirection(ArrowDirection dire)
{
    for (auto arrowWidget : mArrowWidgetList)
        arrowWidget->setArrowDirection(dire);
}

void WMatrixDiagramWidget::resetDefaultStatus()
{
    for (auto menuLab : mMenuLabMap)
        menuLab->setText("0");
}

void WMatrixDiagramWidget::setValue(const QVariant& value)
{
    if (!value.canConvert<QMap<QString, QVariant>>())
        return;

    auto idValueMap = value.toMap();
    for (auto& id : idValueMap.keys())
    {
        auto labIter = mMenuLabMap.find(id);
        if (labIter == mMenuLabMap.end())
            continue;

        auto numberText = QString::number(idValueMap.value(id).toInt());
        labIter.value()->setText(numberText);
    }
}

QVariant WMatrixDiagramWidget::value() { return QVariant(); }
