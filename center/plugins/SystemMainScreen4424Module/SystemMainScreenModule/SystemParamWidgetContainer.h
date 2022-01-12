#ifndef SYSTEMPARAMWIDGETCONTAINER_H
#define SYSTEMPARAMWIDGETCONTAINER_H

#include <QMainWindow>
#include <QMap>

class QDockWidget;
class QStackedWidget;
class QListWidgetItem;

class NavBarWidget;

class SystemParamWidgetContainerImpl;
class SystemParamWidgetContainer : public QMainWindow
{
    Q_OBJECT

public:
    explicit SystemParamWidgetContainer(QWidget* parent = nullptr);
    ~SystemParamWidgetContainer();

    void initWindowState();
    void saveWindowState();

protected:
    void showEvent(QShowEvent* event) override;

private:
    void initNavBarWidget();
    void initStackedWidget();

signals:
    void signalsCmdDeviceJson(QString json);
    void signalsUnitDeviceJson(QString json);

private slots:
    void slotNavBarItemClicked(QListWidgetItem* item);

private:
    SystemParamWidgetContainerImpl* mImpl = nullptr;
};
#endif  // SYSTEMPARAMWIDGETCONTAINER_H
