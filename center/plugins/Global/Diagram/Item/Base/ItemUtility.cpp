#include "ItemUtility.h"
#include <QDir>
#include <QRegularExpression>
#include <QtMath>

double ItemUtility::directionToAngle(Direction direction)
{
    double angle = 0.0;
    switch (direction)
    {
    case Direction::Up:
    {
        angle = 270.0;
        break;
    }
    case Direction::Down:
    {
        angle = 90.0;
        break;
    }
    case Direction::Left:
    {
        angle = 180.0;
        break;
    }
    case Direction::Right:
    {
        angle = 0;
        break;
    }
    case Direction::Left_Up:
    {
        angle = 225.0;
        break;
    }
    case Direction::Left_Down:
    {
        angle = 135.0;
        break;
    }
    case Direction::Right_Up:
    {
        angle = 315.0;
        break;
    }
    case Direction::Right_Down:
    {
        angle = 45.0;
        break;
    }
    }
    return angle;
}
void ItemUtility::calcArrow(double rotateAngle, double w, double h, QPolygonF& point, SourcePointPosition sourcePoint, const QPointF& offset)
{
    point.resize(3);
    auto halfW = w / 2;
    auto halfH = h / 2;

    switch (sourcePoint)
    {
    case SourcePointPosition::Left_Up:
    {
        point[0] = ItemUtility::rotatePoint(QPointF(0, 0), rotateAngle) + offset;
        point[1] = ItemUtility::rotatePoint(QPointF(w, halfH), rotateAngle) + offset;
        point[2] = ItemUtility::rotatePoint(QPointF(0, h), rotateAngle) + offset;
        return;
    }
    case SourcePointPosition::Left_Down:
    {
        point[0] = ItemUtility::rotatePoint(QPointF(0, 0), rotateAngle) + offset;
        point[1] = ItemUtility::rotatePoint(QPointF(w, -halfH), rotateAngle) + offset;
        point[2] = ItemUtility::rotatePoint(QPointF(0, -h), rotateAngle) + offset;
        return;
    }
    case SourcePointPosition::Left_Middle:
    {
        break;
    }
    case SourcePointPosition::Up_Middle:
    case SourcePointPosition::Down_Middle:
    case SourcePointPosition::Center:
    {
        point[0] = ItemUtility::rotatePoint(QPointF(halfW, 0), rotateAngle) + offset;
        point[1] = ItemUtility::rotatePoint(QPointF(-halfW, -halfH), rotateAngle) + offset;
        point[2] = ItemUtility::rotatePoint(QPointF(-halfW, halfH), rotateAngle) + offset;
        return;
    }
    case SourcePointPosition::Right_Up:
    case SourcePointPosition::Right_Middle:
    case SourcePointPosition::Right_Down:
    {
        point[0] = ItemUtility::rotatePoint(QPointF(0, 0), rotateAngle) + offset;
        point[1] = ItemUtility::rotatePoint(QPointF(-w, -halfH), rotateAngle) + offset;
        point[2] = ItemUtility::rotatePoint(QPointF(-w, halfH), rotateAngle) + offset;
        return;
    }
    }
    point[0] = ItemUtility::rotatePoint(QPointF(0, halfH), rotateAngle) + offset;
    point[1] = ItemUtility::rotatePoint(QPointF(0, -halfH), rotateAngle) + offset;
    point[2] = ItemUtility::rotatePoint(QPointF(w, 0), rotateAngle) + offset;
}
void ItemUtility::calcArrow(double w, double h, QPolygonF& point, SourcePointPosition sourcePoint, const QPointF& offset)
{
    point.resize(3);
    auto halfW = w / 2;
    auto halfH = h / 2;

    switch (sourcePoint)
    {
    case SourcePointPosition::Left_Up:
    {
        point[0] = QPointF(0, 0) + offset;
        point[1] = QPointF(w, halfH) + offset;
        point[2] = QPointF(0, h) + offset;
        return;
    }
    case SourcePointPosition::Left_Down:
    {
        point[0] = QPointF(0, 0) + offset;
        point[1] = QPointF(w, -halfH) + offset;
        point[2] = QPointF(0, -h) + offset;
        return;
    }
    case SourcePointPosition::Left_Middle:
    {
        break;
    }
    case SourcePointPosition::Up_Middle:
    case SourcePointPosition::Down_Middle:
    case SourcePointPosition::Center:
    {
        point[0] = QPointF(halfW, 0) + offset;
        point[1] = QPointF(-halfW, -halfH) + offset;
        point[2] = QPointF(-halfW, halfH) + offset;
        return;
    }
    case SourcePointPosition::Right_Up:
    case SourcePointPosition::Right_Middle:
    case SourcePointPosition::Right_Down:
    {
        point[0] = QPointF(0, 0) + offset;
        point[1] = QPointF(-w, -halfW) + offset;
        point[2] = QPointF(-w, halfW) + offset;
        return;
    }
    }
    point[0] = QPointF(0, halfH) + offset;
    point[1] = QPointF(0, -halfH) + offset;
    point[2] = QPointF(w, 0) + offset;
}

QPointF ItemUtility::rotatePoint(const QPointF& ptSrc, const QPointF& ptRotationCenter, double angle)
{
    //???????????????a,b),  ??????????????????(x0,y0), ??????????????   ,
    //???????????????????????? x=a+(x0-a)cos??-(y0-b)sin?? ,    y=b+(x0-a)sin??+(y0-b)cos??
    QPointF ptDst;
    qreal a = ptRotationCenter.x();
    qreal b = ptRotationCenter.y();
    qreal x0 = ptSrc.x();
    qreal y0 = ptSrc.y();
    ptDst.rx() = a + (x0 - a) * cos(angle * M_PI / 180) - (y0 - b) * sin(angle * M_PI / 180);
    ptDst.ry() = b + (x0 - a) * sin(angle * M_PI / 180) + (y0 - b) * cos(angle * M_PI / 180);
    return ptDst;
}

QPointF ItemUtility::rotatePoint(const QPointF& ptSrc, double angle)
{
    //???????????????a,b),  ??????????????????(x0,y0), ??????????????   ,
    //???????????????????????? x=a+(x0-a)cos??-(y0-b)sin?? ,    y=b+(x0-a)sin??+(y0-b)cos??
    // cos(a+b)???cosa??cosb???sina??sinb
    // tan(??+??)=(tan??+tan??)/(1-tan??tan??)
    // tan(??-??)=(tan??-tan??)/(1+tan??tan??)
    // cos(??+??)=cos??cos??-sin??sin??
    // cos(??-??)=cos??cos??+sin??sin??
    // sin(??+??)=sin??cos??+cos??sin??
    // sin(??-??)=sin??cos?? -cos??sin??
    QPointF ptDst;
    qreal x0 = ptSrc.x();
    qreal y0 = ptSrc.y();
    ptDst.rx() = x0 * cos(angle * M_PI / 180) - y0 * sin(angle * M_PI / 180);
    ptDst.ry() = x0 * sin(angle * M_PI / 180) + y0 * cos(angle * M_PI / 180);
    return ptDst;
}

QRectF ItemUtility::calcRectFPos(SourcePointPosition mode, const QPointF& ptSrc, double w, double h)
{
    auto halfW = w / 2.0;
    auto halfH = h / 2.0;
    auto x = ptSrc.x();
    auto y = ptSrc.y();

    /* ???????????? */
    auto result = QRectF(-halfW + x, -halfH + y, w, h);
    switch (mode)
    {
    case SourcePointPosition::Up_Middle /* ?????? */:
    {
        result = QRectF(-halfW + x, 0 + y, w, h);
        break;
    }
    case SourcePointPosition::Down_Middle /* ?????? */:
    {
        result = QRectF(-halfW + x, -h + y, w, h);
        break;
    }
    case SourcePointPosition::Left_Middle /* ?????? */:
    {
        result = QRectF(0 + x, -halfH + y, w, h);
        break;
    }
    case SourcePointPosition::Right_Middle /* ?????? */:
    {
        result = QRectF(-w + x, -halfH + y, w, h);
        break;
    }
    case SourcePointPosition::Left_Up /* ?????? */:
    {
        result = QRectF(0 + x, 0, w + y, h);
        break;
    }
    case SourcePointPosition::Left_Down /* ?????? */:
    {
        result = QRectF(0 + x, -h + y, w, h);
        break;
    }
    case SourcePointPosition::Right_Up /* ?????? */:
    {
        result = QRectF(-w + x, 0 + y, w, h);
        break;
    }
    case SourcePointPosition::Right_Down /* ?????? */:
    {
        result = QRectF(-w + x, -h + y, w, h);
        break;
    }
    case SourcePointPosition::Center /* ?????? */:
    {
        break;
    }
    }
    return result;
}
QString ItemUtility::joinPath(const QString& v1, const QString& v2)
{
    auto path = QString("%1/%2").arg(v1, v2);
    /* ???????????????/  ???????????? */
    path = QDir::fromNativeSeparators(path);
    return path.replace(QRegularExpression("/+"), "/");
}
