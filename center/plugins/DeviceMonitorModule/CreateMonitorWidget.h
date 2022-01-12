#ifndef CREATEMONITORWIDGET_H
#define CREATEMONITORWIDGET_H
#include <QDomDocument>
#include <QWidget>
#include <memory>

class DeviceProcessService;
class CreateMonitorWidget
{
public:
    static QWidget* createWidget(const QString& subWidgetName, std::shared_ptr<DeviceProcessService> service);

private:
    static QMap<QString, QMap<QString, int>> getSystemNameMap();
    static void parseXMl(QDomElement* docElem, QString& currentProjectName, QMap<QString, int>& systemMap);
};

#endif  // CREATEMONITORWIDGET_H
