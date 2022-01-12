#ifndef DOCKWIDGET_H
#define DOCKWIDGET_H

#include <QDockWidget>
#include <QObject>

class DockWidget : public QDockWidget
{
    Q_OBJECT
public:
    explicit DockWidget(QWidget* parent = nullptr);
    ~DockWidget();

protected:
    virtual void paintEvent(QPaintEvent* event) override;
};

#endif  // DOCKWIDGET_H
