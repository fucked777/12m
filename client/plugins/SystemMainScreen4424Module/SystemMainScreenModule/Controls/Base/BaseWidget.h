#ifndef BASEWIDGET_H
#define BASEWIDGET_H

#include <QDomElement>
#include <QWidget>
#include <QDebug>

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
    bool mIsShow = false;
};

#endif  // BASEWIDGET_H
