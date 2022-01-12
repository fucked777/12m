#ifndef SYSTEMMONITOR4426_H
#define SYSTEMMONITOR4426_H

#include "ParamRelationManager.h"
#include "ProtocolXmlTypeDefine.h"

#include <QTreeWidgetItem>
#include <QWidget>

namespace Ui
{
    class SystemMonitor4426;
}
#include "DeviceProcessMessageDefine.h"
class SystemMonitor4426 : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int deviceID READ getDeviceID WRITE setDeviceID)

public:
    explicit SystemMonitor4426(QWidget* parent = nullptr);
    ~SystemMonitor4426();
    void setSystem(System currentSystem);  //每个分系统对应的数据
    void initSystemWidget();

    void setDeviceID(const int deviceID);
    int getDeviceID() const;

signals:
    /*单元参数设置命令*/
    void signalsUnitDeviceJson(QString json);

    /*发送过程控制命令*/
    void signalsCmdDeviceJson(QString json);

    /*发送参数宏*/
    void signalsParaMacroDeviceJson(QString json);

public:
    void modeChanged(const QString& text);
    /*控制命令响应*/
    void slotsControlCmdResponseJson(ControlCmdResponse cmdRespnce);

    /*分机状态上报的数据*/
    //    void slotsStatusReportJson(const StatusReportingRequest& statusReport);

    /*过程控制命令结果上报*/
    void slotsCmdResultJson(const CmdResult& cmdReport);

private:
    void initDeviceList();  //初始化设备的list列表，列表上是当前分系统所有的分机名称

    void initDeviceStackWidget();  //初始化每个分机对应的stackwidget，里面是各个单元以及过程控制命令界面

    void createOtherExtentWidget(DeviceID deviceID);
    /* 点击列表,切换设备 */
    void deviceChange(QTreeWidgetItem* item, int column);

    /* 计时器的开关 */
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void timerEvent(QTimerEvent* event) override;  //这里才是现在用的分机状态上报的数据

    void createDeviceWidget(DeviceID deviceID, ModeCtrl modeCtrl);

private:
    Ui::SystemMonitor4426* ui;

    int m_timerID{ -1 };
    System m_currentSystem;

    DeviceID m_currentDeviceID;

    //  QMap<DeviceID, QString> m_deviceNameMap;  //设备ID 和 当前设备名称的对应
    // m_deviceModeNameMap是专门给测控基带这种设备设计的，而m_deviceTreeItem是给普通设备设计的
    QMap<DeviceID, QMap<QString, QMap<int, QString>>> m_deviceModeNameMap;  //设备ID 设备名称 和当前模式以及对应的名称
    QMap<QString, QMap<DeviceID, bool>> m_relationNameIsShowDevice;  // key是关联设备的名称，第二个key是设备ID，bool是分机是否显示
    QMap<DeviceID, QMap<QTreeWidgetItem*, int>> m_deviceModeTreeItem;  //存取测控基带的设备id的模式的树节点  会根据数据上报来显示对应模式的树节点
    QMap<DeviceID, QTreeWidgetItem*> m_deviceTreeItem;  //存取测控基带的设备id的模式的树节点  会根据数据上报来显示对应模式的树节点

    QMap<DeviceID, QMap<int, QWidget*>> m_deviceWidgetMaps;  //参数设置界面 第二个Map key 作为模式
    QMap<DeviceID, int> m_deviceMode;                        //每个设备ID对应的模式
    static ParamRelationManager m_paraRelation;              //关联关系
};

#endif  // SYSTEMMONITOR4426_H
