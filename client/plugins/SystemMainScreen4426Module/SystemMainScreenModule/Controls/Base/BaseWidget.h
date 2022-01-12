#ifndef BASEWIDGET_H
#define BASEWIDGET_H

#include <QDebug>
#include <QDomElement>
#include <QWidget>

class BaseWidgetImpl;
class BaseWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BaseWidget(const QDomElement& domEle, QWidget* parent = nullptr);
    virtual ~BaseWidget();

protected:
    virtual void paintEvent(QPaintEvent* event) override;
    void timerEvent(QTimerEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;

private:
    void parseNode(const QDomElement& domEle);

private:
    BaseWidgetImpl* mImpl = nullptr;
};

#endif  // BASEWIDGET_H
