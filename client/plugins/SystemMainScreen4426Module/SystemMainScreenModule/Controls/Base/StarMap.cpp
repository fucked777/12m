#include "StarMap.h"
#include "DeviceProcessMessageDefine.h"
#include "DeviceProcessMessageSerialize.h"
#include "JsonHelper.h"
#include "ProtocolXmlTypeDefine.h"
#include "RedisHelper.h"
#include <QElapsedTimer>
#include <QPainter>
#include <QtMath>

StarMap::StarMap(const QDomElement& domEle, QWidget* parent, const int deviceId, const int modeId)
    : BaseWidget(domEle, parent)
    , mDeviceId(deviceId)
{
    parseNode(domEle);

    if (m_starThread == nullptr)
    {
        m_starThread = new StarMapThread;

        connect(m_starThread, &StarMapThread::signalRefreshUI, this, &StarMap::slotsRefresh);
        m_starThread->setStarMapData(mDeviceId, mChannel);

        m_starThread->start();
    }
}

StarMap::~StarMap()
{
    if (m_starThread)
    {
        m_starThread->stopRunning();
        m_starThread->quit();
        m_starThread->wait();
        delete m_starThread;
        m_starThread = nullptr;
    }
}

void StarMap::setPoints(const QVector<QPointF>& points)
{
    if (points.empty())
    {
        mPoints.clear();
        update();
        return;
    }

    float xAvg = 0.0, yAvg = 0.0;
    for (const auto& point : points)
    {
        xAvg += qFabs(point.x());
        yAvg += qFabs(point.y());
    }
    xAvg /= points.size();
    yAvg /= points.size();

    xAvg = xAvg > yAvg ? xAvg : yAvg;

    float maxVal = xAvg * 2;
    float minVal = -1 * xAvg * 2;

    auto w = width();
    auto h = height();
    double rateX = (double)w / (maxVal - minVal);
    double rateY = (double)h / (maxVal - minVal);

    double centerX = w / 2;
    double centerY = h / 2;

    mPoints.clear();
    for (const auto& point : points)
    {
        mPoints << QPointF(centerX + point.x() * rateX, centerY - point.y() * rateY);
    }
    update();
}

void StarMap::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    // 启用反锯齿
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    // 绘制背景
    drawBackground(painter);
    // 画边框
    drawBorder(painter);
    // 画十字线
    drawCrossLine(painter);
    // 画文字
    drawText(painter);
    // 画点
    drawPoints(painter);
}

void StarMap::showEvent(QShowEvent* event) { mShow = true; }

void StarMap::hideEvent(QHideEvent* event) { mShow = false; }

void StarMap::slotsRefresh(const QVector<QPointF>& allPoints)
{
    if (!mShow)
        return;
    setPoints(allPoints);
}

void StarMap::parseNode(const QDomElement& domEle)
{
    if (domEle.isNull())
    {
        return;
    }

    if (domEle.hasAttribute("deviceId"))
    {
        QString strDeviceId = domEle.attribute("deviceId");
        mDeviceId = strDeviceId.toInt(nullptr, 16);
    }
    DeviceID id(mDeviceId);
    mExtenId = id.extenID;

    mText = domEle.attribute("text");
    mChannel = domEle.attribute("channel").toInt();
}

void StarMap::drawBackground(QPainter& painter)
{
    painter.save();

    QBrush brush;
    brush.setColor(Qt::black);
    brush.setStyle(Qt::SolidPattern);
    painter.setBrush(brush);

    QRect rect(0, 0, width(), height());
    painter.drawRect(rect);

    painter.restore();
}

void StarMap::drawBorder(QPainter& painter)
{
    painter.save();

    QPen pen;
    pen.setColor(Qt::green);
    pen.setWidth(1);

    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    QRect rect(mPadding, mPadding, width() - mPadding * 2, height() - mPadding * 2);
    painter.drawRect(rect);

    painter.restore();
}

void StarMap::drawCrossLine(QPainter& painter)
{
    painter.save();

    QPen pen;
    pen.setColor(Qt::green);
    pen.setWidth(1);

    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    QLineF horizontalLine(QPointF(width() / 2, mPadding), QPointF(width() / 2, height() - mPadding));
    QLineF verticalLine(QPointF(mPadding, height() / 2), QPointF(width() - mPadding, height() / 2));

    painter.drawLine(horizontalLine);
    painter.drawLine(verticalLine);

    painter.restore();
}

void StarMap::drawText(QPainter& painter)
{
    painter.save();

    QPen pen;
    pen.setColor(Qt::green);
    pen.setWidth(1);

    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    QFontMetrics metrics(font());
    auto textWidth = metrics.width(mText);
    auto textHeight = metrics.height();

    QPointF pos((width() - textWidth) / 2, textHeight + mPadding);
    painter.drawText(pos, mText);

    painter.restore();
}

void StarMap::drawPoints(QPainter& painter)
{
    painter.save();

    QPen pen;
    pen.setColor(Qt::green);
    pen.setWidth(2);

    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    painter.drawPoints(mPoints);

    painter.restore();
}
