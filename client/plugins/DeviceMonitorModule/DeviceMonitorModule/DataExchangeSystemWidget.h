#ifndef DATAEXCHANGESYSTEMWIDGET_H
#define DATAEXCHANGESYSTEMWIDGET_H

#include <QTableView>
#include <QWidget>

#include "CenterDataControlModel.h"
#include "CommunicateStatusModel.h"
#include "DataExchangeWidgetDelegate.h"
#include "HostInfoModel.h"
#include "MissionInfoModel.h"
#include "NetCardModel.h"
#include "SoftUnitHostModel.h"
#include "SoftUnitModel.h"
#include "TaskRunBaseNumModel.h"
#include "TaskRunCenterDataModel.h"
#include "TcpStatusModel.h"

#include "CustomPacketMessageDefine.h"
#include "DeviceProcessMessageDefine.h"
#include "WWidget.h"
#include "WidgetDealWith.h"
#include <QComboBox>
#include <QMessageBox>
#include <QScrollArea>
#include <QScrollBar>
#include <QTime>
#include <QVBoxLayout>

namespace Ui
{
    class DataExchangeSystemWidget;
}

class DataExchangeSystemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DataExchangeSystemWidget(QWidget* parent = 0);
    ~DataExchangeSystemWidget();

    void setModeCtrl(ModeCtrl modeCtrl);
    //设置当前的设备模式
    void setDeviceID(DeviceID deviceId);
    const DeviceID& getDeviceID() const;
    ///////单元初始化/////////
    void initWidget();

    void initUnitStatusWidget(Unit& unit);  // DTE只有状态

    ///////命令初始化/////////
    void initCmdWidget();

    //无分组
    QWidget* initCmdStatusWidget(CmdAttribute cmdAttribute);
    QWidget* initCmdSetWidget(CmdAttribute cmdAttribute);
    //有分组
    QWidget* initCmdStatusGroupWidget(CmdAttribute cmdAttribute);
    QWidget* initCmdSetGroupWidget(CmdAttribute cmdAttribute);
    //这是添加那些动态界面又嵌套动态界面的标签
    void initOtherRecycleTabWidget(Unit unit);

    // htf
    void timerEvent(QTimerEvent* event);
    void showEvent(QShowEvent* event);
    void hideEvent(QHideEvent* event);

    //组包命令
    void packCmdParamSet();
public slots:
    void on_switchBtn_2_clicked();

    //命令自动滚动
    void cmdCurrentIndexChanged(int index);

    /*分机状态上报的数据*/
    void slotsExtenStatusReport(const ExtensionStatusReportMessage& statusReport);

    /*控制命令响应*/
    void slotsControlCmdResponse(const ControlCmdResponse& cmdResponce);

    /*控制结果上报*/
    void slotsCmdResult(const CmdResult& cmdResponce);

signals:
    /*发送过程控制命令*/
    void signalsCmdDeviceJson(QString json);

private:
    void initTable(const QMap<QString, DevEnum>& enumMap, const QList<ParameterAttribute>& attrList, QStringList& headers, QTableView* tableView);
    void updateStatusGroupBox(const QMap<QString, QVariant>& reportMap);

private:
    Ui::DataExchangeSystemWidget* ui;
    ModeCtrl m_currentMode;

    MissionInfoModel* mMissionInfoModel = nullptr;
    CommunicateStatusModel* mCommStatusModel = nullptr;
    SoftUnitModel* mSoftUnitModel = nullptr;
    HostInfoModel* mHostInfoMode = nullptr;
    NetCardModel* mNetCardModel = nullptr;
    TcpStatusModel* mTcpStatusModel = nullptr;
    CenterDataControlModel* mCenterDataControlModel = nullptr;
    SoftUnitHostModel* mSoftUnitHostModel = nullptr;
    TaskRunCenterDataModel* mTaskRunCenterDataModel = nullptr;
    TaskRunBaseNumModel* mTaskRunBaseNumModel = nullptr;
    QMap<QTableView*, int> mTableNameMap;

    int isOnline = 0;
    bool m_isShow = false;  //这里判断界面是否前端显示
    bool m_isParamSet = true;
    bool initCmdUI = true;  //第一次初始化命令界面
    QGroupBox* m_cmdCurrGroupBox = nullptr;

    QMap<int, QWidget*> m_cmdIDWidgetMap;  //存放选中命令的wdiget和对应的命令号
    QMap<QWidget*, int> m_cmdWidgetIDMap;  //存放选中命令的wdiget和对应的命令号
    const QString STATUS_TITLE = QString("状态");
    const QString SET_TITLE = QString("设置");
    const QString COMMON_PARA = QString("普通参数");

    DeviceID m_currentDeviceID;  //当前监控软件的ID,这里判断是不是当前设备，如果是就更新界面

    QScrollArea* m_cmdscrollArea = nullptr;

    WidgetDealWith m_widgetDeal;  //以后创其它的自定义控件就用这里面的函数
};

#endif  // DATAEXCHANGESYSTEMWIDGET_H
