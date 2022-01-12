#ifndef STARMAP_H
#define STARMAP_H

#include "BaseWidget.h"
#include "StarMapThread.h"
#include <QPointF>

// 星座图
class StarMap : public BaseWidget
{
    Q_OBJECT
public:
    explicit StarMap(const QDomElement& domEle, QWidget* parent = nullptr, const int deviceId = 0, const int modeId = 0);
    ~StarMap();
    void setPoints(const QVector<QPointF>& points);

protected:
    void paintEvent(QPaintEvent* event);
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;

private slots:
    void slotsRefresh(const QVector<QPointF>& allPoints);

private:
    void parseNode(const QDomElement& domEle);

    // 绘制背景
    void drawBackground(QPainter& painter);
    // 画边框
    void drawBorder(QPainter& painter);
    // 画十字线
    void drawCrossLine(QPainter& painter);
    // 画文字
    void drawText(QPainter& painter);
    // 画点
    void drawPoints(QPainter& painter);

private:
    QString mText;
    int mPadding = 3;  // 内边距
    QPolygonF mPoints;
    bool mShow;
    int mChannel;
    int mDeviceId;
    int mExtenId;

    StarMapThread* m_starThread = nullptr;
};

#endif  // STARMAP_H
