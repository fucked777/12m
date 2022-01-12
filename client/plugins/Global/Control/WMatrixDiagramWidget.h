#ifndef WMATRIXDIAGRAMWIDGET_H
#define WMATRIXDIAGRAMWIDGET_H

#include "WWidget.h"

#include <QGridLayout>
#include <QMenu>
#include <QMouseEvent>

enum ArrowDirection
{
    Right = 1,
    Left = 2
};

class MenuLabel : public QLabel
{
    Q_OBJECT
public:
    MenuLabel(QWidget* parent = nullptr)
        : QLabel(parent)
    {
        setAlignment(Qt::AlignCenter);

        // 发送点击的枚举值
        connect(&mMenu, &QMenu::triggered, this, [=](QAction* action) { emit triggered(action->data()); });
    }

    void setId(const QString& attrId) { mAttrId = attrId; }

    QString getId() const { return mAttrId; }

    void setActions(const QMap<int, QString>& attrEnumMap)
    {
        mAttrEnumMap = attrEnumMap;
        mMenu.clear();

        for (auto key : attrEnumMap.keys())
        {
            auto text = QString("%1:%2").arg(key).arg(attrEnumMap.value(key));
            auto action = new QAction(text, this);
            action->setData(key);
            mMenu.addAction(action);
        }
    }

signals:
    void triggered(const QVariant& uValue);

protected:
    void mouseReleaseEvent(QMouseEvent* event)
    {
        if (event->button() == Qt::RightButton)
        {
            mMenu.exec(cursor().pos());
        }
    }

private:
    QMenu mMenu;
    QString mAttrId;
    QMap<int, QString> mAttrEnumMap;
};

class ArrowWidget;
class WMatrixDiagramWidget : public WWidget
{
    Q_OBJECT
public:
    WMatrixDiagramWidget(QWidget* parent = nullptr);

    void init(const QList<QPair<ParameterAttribute, QMap<int, QString>>>& attrEnumList);
    void setArrowDirection(ArrowDirection dire = Right);
    void resetDefaultStatus();
    void setValue(const QVariant& value);
    QVariant value();

signals:
    void triggered(const QString& attrId, const QVariant& uValue);

private:
    QGridLayout* mGridLayout;
    QMap<QString, MenuLabel*> mMenuLabMap;
    QList<ArrowWidget*> mArrowWidgetList;
};

#endif  // WMATRIXDIAGRAMWIDGET_H
