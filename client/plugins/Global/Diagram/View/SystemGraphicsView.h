#ifndef SYSTEMGRAPHICSVIEW_H
#define SYSTEMGRAPHICSVIEW_H
#include "GraphFactory.h"
#include <QGraphicsView>
#include <QObject>

//系统框图
struct CreateBlockDiagramVar;
class SystemGraphicsViewImpl;
class SystemGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    SystemGraphicsView(QWidget* parent = nullptr);
    ~SystemGraphicsView() override;
    /* 重新加载框图 */
    Optional<GraphicsItemInfo> reload(const QString& configPath);

    /* 这个ID是图元ID */
    void setItemData(const QString& id, const QVariant&);

    // 平移速度
    void setTranslateSpeed(qreal speed);
    qreal translateSpeed() const;

    // 缩放的增量
    void setZoomDelta(qreal delta);
    qreal zoomDelta() const;

    void setScaleValue(qreal sx, qreal sy);

    void wheelEvent(QWheelEvent* event) Q_DECL_OVERRIDE;
    // 上/下/左/右键向各个方向移动、加/减键进行缩放、空格/回车键旋转
    void keyPressEvent(QKeyEvent* event) Q_DECL_OVERRIDE;
    // 平移
    void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
public Q_SLOTS:
    void zoomIn();                  // 放大
    void zoomOut();                 // 缩小
    void zoom(float scaleFactor);   // 缩放 - scaleFactor：缩放的比例因子
    void translate(QPointF delta);  // 平移
signals:
    void sg_singleClick(const QString& id, const QVariant& value);
    void sg_doubleClick(const QString& id, const QVariant& value);

private:
    Qt::MouseButton m_translateButton;  // 平移按钮
    qreal m_translateSpeed;             // 平移速度
    qreal m_zoomDelta;                  // 缩放的增量
    bool m_bMouseTranslate;             // 平移标识
    QPoint m_lastMousePos;              // 鼠标最后按下的位置
    qreal m_scale;                      // 缩放值
    SystemGraphicsViewImpl* m_impl;
};

#endif  // SYSTEMGRAPHICSVIEW_H
