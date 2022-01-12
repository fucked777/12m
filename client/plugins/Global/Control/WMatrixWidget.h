#ifndef WMATRIXWIDGET_H
#define WMATRIXWIDGET_H

#include "WMatrixDiagramWidget.h"
#include "WWidget.h"

class WMatrixLEDWidget;

class WMatrixWidget : public WWidget
{
    Q_OBJECT
public:
    explicit WMatrixWidget(QWidget* parent = nullptr);
    ~WMatrixWidget();

signals:
    void actionTriggered(const QString& attrId, const int& uValue);

public:
    void setMatrixInfo(const QList<QPair<ParameterAttribute, QMap<int, QString>>>& attrEnumList);
    void setDiagramArrowDirection(ArrowDirection dire = Right);
    void resetDefaultStatus();
    void setValue(const QVariant& value);
    QVariant value();

private:
    WMatrixLEDWidget* mLedWidget;
    WMatrixDiagramWidget* mDiagramWidget;
};

#endif  // WMATRIXWIDGET_H
