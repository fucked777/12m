#include "SystemGraphicsView.h"
#include "GraphFactory.h"
#include <QApplication>
#include <QDebug>
#include <QMouseEvent>
#include <QScrollBar>

#define VIEW_CENTER viewport()->rect().center()
#define VIEW_WIDTH  viewport()->rect().width()
#define VIEW_HEIGHT viewport()->rect().height()

class SystemGraphicsViewImpl
{
public:
    // QPoint lastMousePos;
    // qreal zoom{ 0.0 };
    QGraphicsScene* pScene{ nullptr };
    GraphicsItemInfo info;
};

SystemGraphicsView::SystemGraphicsView(QWidget* parent)
    : QGraphicsView(parent)
    , m_translateButton(Qt::LeftButton)
    , m_translateSpeed(1.0)
    , m_zoomDelta(0.1)
    , m_bMouseTranslate(false)
    , m_scale(1.0)
    , m_impl(new SystemGraphicsViewImpl)
{
    m_impl->pScene = new QGraphicsScene(this);
    m_impl->pScene->setBackgroundBrush(Qt::white);
    m_impl->pScene->setItemIndexMethod(QGraphicsScene::NoIndex);
    setRenderHint(QPainter::Antialiasing, true); /* 反走样 */
    setOptimizationFlags(QGraphicsView::DontSavePainterState);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setCacheMode(QGraphicsView::CacheBackground);
    setScene(m_impl->pScene);

    // 去掉滚动条
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setCursor(Qt::PointingHandCursor);
    setRenderHint(QPainter::Antialiasing);

    //    setSceneRect(-500, -500, 1000, 1000);
    //    centerOn(500, 500);

    setSceneRect(-INT_MIN, -INT_MIN, 0, 0);
    centerOn(-500, -500);
}

SystemGraphicsView::~SystemGraphicsView()
{
    GraphFactory::destroyBlockDiagram(m_impl->pScene, m_impl->info);
    delete m_impl;
}
Optional<GraphicsItemInfo> SystemGraphicsView::reload(const QString& configPath)
{
    GraphFactory::destroyBlockDiagram(m_impl->pScene, m_impl->info);
    auto res = GraphFactory::createBlockDiagram(configPath);
    if (res.success())
    {
        m_impl->info = res.value();
        for (auto& info : m_impl->info.graphicsItemMap)
        {
            m_impl->pScene->addItem(info.item);
            connect(info.item, &ItemBase::sg_singleClick, this, &SystemGraphicsView::sg_singleClick);
            connect(info.item, &ItemBase::sg_doubleClick, this, &SystemGraphicsView::sg_doubleClick);
        }
    }

    return res;
}

void SystemGraphicsView::setItemData(const QString& id, const QVariant& data)
{
    auto find = m_impl->info.graphicsItemMap.find(id);
    if (find != m_impl->info.graphicsItemMap.end())
    {
        auto& info = find.value();
        (info.setItemData)(info, data);
    }
}

// 平移速度
void SystemGraphicsView::setTranslateSpeed(qreal speed)
{
    // 建议速度范围
    Q_ASSERT_X(speed >= 0.0 && speed <= 2.0, "InteractiveView::setTranslateSpeed", "Speed should be in range [0.0, 2.0].");
    m_translateSpeed = speed;
}

qreal SystemGraphicsView::translateSpeed() const { return m_translateSpeed; }

// 缩放的增量
void SystemGraphicsView::setZoomDelta(qreal delta)
{
    // 建议增量范围
    Q_ASSERT_X(delta >= 0.0 && delta <= 1.0, "InteractiveView::setZoomDelta", "Delta should be in range [0.0, 1.0].");
    m_zoomDelta = delta;
}

qreal SystemGraphicsView::zoomDelta() const { return m_zoomDelta; }

void SystemGraphicsView::setScaleValue(qreal sx, qreal sy) { scale(sx, sy); }
void SystemGraphicsView::wheelEvent(QWheelEvent* event)
{
    if (QApplication::keyboardModifiers() == Qt::ControlModifier)
    {
        // 获取当前鼠标相对于view的位置;
        QPointF cursorPoint = event->pos();
        // 获取当前鼠标相对于scene的位置;
        QPointF scenePos = this->mapToScene(QPoint(cursorPoint.x(), cursorPoint.y()));

        // 获取view的宽高;
        qreal viewWidth = this->viewport()->width();
        qreal viewHeight = this->viewport()->height();

        // 获取当前鼠标位置相当于view大小的横纵比例;
        qreal hScale = cursorPoint.x() / viewWidth;
        qreal vScale = cursorPoint.y() / viewHeight;

        // 当前放缩倍数;
        qreal scaleFactor = this->matrix().m11();
        int wheelDeltaValue = event->delta();

        // 向上滚动，放大;
        if (wheelDeltaValue > 0)
        {
            this->scale(1.1, 1.1);
        }
        // 向下滚动，缩小;
        else
        {
            this->scale(1.0 / 1.1, 1.0 / 1.1);
        }

        // 将scene坐标转换为放大缩小后的坐标;
        QPointF viewPoint = this->matrix().map(scenePos);
        // 通过滚动条控制view放大缩小后的展示scene的位置;
        horizontalScrollBar()->setValue(int(viewPoint.x() - viewWidth * hScale));
        verticalScrollBar()->setValue(int(viewPoint.y() - viewHeight * vScale));
    }
    else
    {
        int wheelDeltaValue = event->delta();
        // 向上滚动，放大;
        if (wheelDeltaValue > 0)
        {
            translate(QPointF(0, 30));  // 上移
        }
        // 向下滚动，缩小;
        else
        {
            translate(QPointF(0, -30));  // 下移
        }
    }
}
void SystemGraphicsView::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
    case Qt::Key_Up:
        translate(QPointF(0, -2));  // 上移
        break;
    case Qt::Key_Down:
        translate(QPointF(0, 2));  // 下移
        break;
    case Qt::Key_Left:
        translate(QPointF(-2, 0));  // 左移
        break;
    case Qt::Key_Right:
        translate(QPointF(2, 0));  // 右移
        break;
    case Qt::Key_Plus:  // 放大
        zoomIn();
        break;
    case Qt::Key_Minus:  // 缩小
        zoomOut();
        break;
        /*case Qt::Key_Space:  // 逆时针旋转
            rotate(-5);
            break;
        case Qt::Key_Enter:  // 顺时针旋转
        case Qt::Key_Return: rotate(5);
        break;*/
    default: QGraphicsView::keyPressEvent(event);
    }
}

// 平移
void SystemGraphicsView::mouseMoveEvent(QMouseEvent* event)
{
    if (m_bMouseTranslate)
    {
        QPointF mouseDelta = mapToScene(event->pos()) - mapToScene(m_lastMousePos);
        translate(mouseDelta);
    }

    m_lastMousePos = event->pos();

    QGraphicsView::mouseMoveEvent(event);
}

void SystemGraphicsView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == m_translateButton)
    {
        // 当光标底下没有 item 时，才能移动
        QPointF point = mapToScene(event->pos());
        if (scene()->itemAt(point, transform()) == NULL)
        {
            m_bMouseTranslate = true;
            m_lastMousePos = event->pos();
        }
    }

    QGraphicsView::mousePressEvent(event);
}

void SystemGraphicsView::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == m_translateButton)
        m_bMouseTranslate = false;

    QGraphicsView::mouseReleaseEvent(event);
}

void SystemGraphicsView::zoomIn() { zoom(1 + m_zoomDelta); }

// 缩小
void SystemGraphicsView::zoomOut() { zoom(1 - m_zoomDelta); }

// 缩放 - scaleFactor：缩放的比例因子
void SystemGraphicsView::zoom(float scaleFactor)
{
    // 防止过小或过大
    qreal factor = transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
    if (factor < 0.07 || factor > 100)
        return;

    scale(scaleFactor, scaleFactor);
    m_scale *= scaleFactor;
}

// 平移
void SystemGraphicsView::translate(QPointF delta)
{
    // 根据当前 zoom 缩放平移数
    delta *= m_scale;
    delta *= m_translateSpeed;

    // view 根据鼠标下的点作为锚点来定位 scene
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    QPoint newCenter(VIEW_WIDTH / 2 - delta.x(), VIEW_HEIGHT / 2 - delta.y());
    centerOn(mapToScene(newCenter));

    // scene 在 view 的中心点作为锚点
    setTransformationAnchor(QGraphicsView::AnchorViewCenter);
}
