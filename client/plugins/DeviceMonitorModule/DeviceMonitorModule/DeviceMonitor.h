#ifndef DEVICEMONITOR_H
#define DEVICEMONITOR_H

#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QWidget>

#include "CustomPacketMessageDefine.h"
#include "DeviceProcessMessageDefine.h"
#include "ParamRelationOtherManager.h"
#include "ProtocolXmlTypeDefine.h"
#include "WWidget.h"
#include "WidgetDealWith.h"

namespace Ui
{
    class DeviceMonitor;
}

class QFlowLayout;
class DeviceMonitor : public QWidget
{
    Q_OBJECT

public:
    enum class PackResult
    {
        Success,
        Failed,         //正常的组包失败
        RangeFailed,    //因为范围不对的组包失败
        RelationFailed  //因为关联关系不对的组包失败
    };

public:
    explicit DeviceMonitor(QWidget* parent = nullptr);
    ~DeviceMonitor();

    void setDeviceID(DeviceID deviceID);  //设置当前设备的ID
    const DeviceID& getDeviceID() const;

    void setModeCtrl(ModeCtrl mode);  //设置当前设备的模式
    ModeCtrl getModeCtrl() const;

    QMap<int, QWidget*> getUnitIDWidgetMap();
    QMap<int, QWidget*> getCmdIDWidgetMap();
    QMap<int, QMap<QString, WWidget*>> getAllWidgetMap();
    QMap<QString, WWidget*> getUnitWidgetMap(int unitID);

    //获取当前模式的ID
    int currentModeID();

    ///////单元初始化/////////
    void initWidget();

    //无分组
    QWidget* initUnitStatusWidget(Unit& unit);
    QWidget* initUnitSetsWidget(Unit& unit, int unitID);
    //有分组
    QWidget* initUnitStatusGroupWidget(Unit& unit);
    QWidget* initUnitSetsGroupWidget(Unit& unit, int unitID);

    // 创建矩阵
    QWidget* createMatrix(Unit& unit);

    //用于一体化和扩频里公共单元的修改设置两个按钮
    QHBoxLayout* getCommonYTHAndKPBtnLayout(int unitID, Unit unit, QGroupBox* groupBox);
    //用于一体化和扩频里一体化和扩频部分修改设置两个按钮
    QHBoxLayout* getMultiYTHAndKPBtnLayout(int unitID, Unit unit, QGroupBox* groupBox);

    ///////命令初始化/////////
    void initCmdWidget();

    //无分组
    QWidget* initCmdStatusWidget(CmdAttribute cmdAttribute);
    QWidget* initCmdSetWidget(CmdAttribute cmdAttribute);
    //有分组
    QWidget* initCmdStatusGroupWidget(CmdAttribute cmdAttribute);
    QWidget* initCmdSetGroupWidget(CmdAttribute cmdAttribute);

    void timerEvent(QTimerEvent* event);
    void showEvent(QShowEvent*);
    void hideEvent(QHideEvent*);

    //设置公共参数面板状态
    void setCommonSettingPanel(bool state, QGroupBox* groupBox);
    //设置一体化参数面板状态
    void setMultiSettingPanel(bool state, QGroupBox* groupBox);
    //设置参数设置面板状态
    void setSettingPanel(bool state, QWidget* widget);
    //设备离线时所有数据恢复默认状态
    void setDeviceReduction();

    //单组包公共参数(限上面级和卫星等一体化模式)
    PackResult packUnitCommonParamSet(QWidget* widget, int unitID);
    //单组包一体化参数(限上面级和卫星等一体化模式)
    PackResult packUnitMultiParamSet(QWidget* widget, int unitID);
    //组包单元(标准和扩频)
    PackResult packUnitParamSet(QWidget* widget, int unitID);
    //组包命令
    void packCmdParamSet();
    //获取某个设备中指定单元的指定参数数据
    QVariant getDeviceWidgetParamData(int widgetIndex, QString attrName);
    //获取某个设备中指定单元的指定控件
    WWidget* getDeviceWidgetParamControl(int widgetIndex, QString attrName);
    QMap<int, WWidget*> getDeviceMultiWidgetParamControl(int widgetIndex, QString attrName);

    //组包单元(标准和扩频)(用于参数宏按钮)
    bool packMacroUnitParamSet(QWidget* widget, int unitID, UnitParamRequest& unitParamRequest);
    //自动映射的组包
    void packAutoMapUnitParamSet(int unitID, Unit unit, QWidget* widget);

    static quint64 createDeviceID(int sysID, int devID, int extenID);
signals:
    /*单元参数设置命令*/
    void signalsUnitDeviceJson(QString json);

    /*发送过程控制命令*/
    void signalsCmdDeviceJson(QString json);

    /*发送参数宏*/
    void signalsParaMacroDeviceJson(QString json);

    /*这个信号用于增加过程控制命令的关联关系*/
    void signalInitCmdFinished(DeviceMonitor* device);  // 命令界面初始化完成
public slots:
    void on_switchsBtn_clicked();

    //命令自动滚动
    void cmdCurrentIndexChanged(int index);

    /*分机状态上报的数据*/
    //    void slotsExtenStatusReport(const StatusReportingRequest& statusReport);        //服务器直接发给客户端的数据
    void slotsExtenStatusReport(const ExtensionStatusReportMessage& statusReport);  // redis里面的数据

    /*控制命令响应*/
    void slotsControlCmdResponse(const ControlCmdResponse& cmdResponce);

    /*控制结果上报*/
    void slotsCmdResult(const CmdResult& cmdResponce);
private slots:
    void on_macroBtn_clicked();
    //切换单元的槽函数
    void slot_unitChange(int index);

    //初始化QPlainTextEdit界面
    void initPlainTextEdit();

private:
    //这里是获取当前界面修改之前或者设置之后界面的当前值以及控件
    void getSetParaAllMap(QWidget* widget);
    void comPareSetParaValue();

    void comPareWidgetSetValue();  //值比较，只有值更改成功了的控件才设置为绿色，不然全部设置为红色

    // 设置设置面板QLabel颜色
    void setSettingPanelColor(QWidget* control, QColor color);

    // 通过参数id查找分组名
    QString findAttrGroupDesc(Unit unit, const QString& attrId);
    // 矩阵框图中菜单项切换的时候改变对于的设置参数值，并发送设置
    void matrixDiagramMenuTriggered(const int& unitCode, const QString& attrId, const int& uValue);

    // 检查一些特殊的关联关系（需要参数满足特殊公式的） 限单目标的（无多目标的情况）
    bool checkParamRelation(QMap<QString, QVariant>& setParamData, QString& errorMsg);
    // 检查一些特殊的关联关系（需要参数满足特殊公式的）  限扩频模式（有多目标的情况）
    bool checkParamRelation2(const QList<QMap<QString, QVariant>>& paraTargerMap, QString& errorMsg);
    //处理低速基带码速率的关联关系
    void dealWithDSMSLNumRange(int unitID, const QString& objName);
    //处理测控基带码速率的关联关系
    void dealWithCKTTCRcuNumRange(int unitID, const QString& objName);
    //处理跟踪基带标准+低速 扩频+低速码速率的关联关系
    void dealWithGZMSLNumRange(int unitID, const QString& objName);
    //处理测控基带扩频/一体化码速率的关联关系
    void dealWithCKMultiRcuNumRange(int recycle, int unitID, int modeID, const QString& objName);

    //检查一些过程控制命令的关联关系
    bool checkParamCmdRelation(const QMap<QString, QVariant>& setParamData, int cmdID, QString& errorMsg);

private:
    Ui::DeviceMonitor* ui;

    DeviceID m_currentDeviceID;  //当前设备ID
    ModeCtrl m_currentMode;      //当前设备模式

    QScrollArea* m_cmdscrollArea = nullptr;

    const QString STATUS_TITLE = QString("状态");
    const QString SET_TITLE = QString("设置");
    const QString MULTI_TARGET = QString("目标");
    const QString CMD_TEST = QString("测试");
    const QString CMD_FILENAME = QString("文件名称");

    int m_isOnline = 0;                        //判断在线状态
    bool m_switchFlag = true;                  //切换单元以及过程控制命令的flag
    bool m_isShow = false;                     //这里判断界面是否前端显示
    bool initCmdUI = true;                     //第一次初始化命令界面
    bool m_colorFlag = false;                  //颜色标识
    QGroupBox* m_cmdCurrGroupBox = nullptr;    //这个是用于当前的某个命令界面来改变样式的
    QScrollArea* m_currentScroArea = nullptr;  //这个是用于存储当前改变了参数的界面  用于颜色处理的
    ExtensionStatusReportMessage m_currentStatusReport;

    QMap<int, QWidget*> m_cmdIDWidgetMap;            //存放选中命令的wdiget和对应的命令号
    QMap<QWidget*, int> m_cmdWidgetIDMap;            //存放选中命令的wdiget和对应的命令号
    QMap<int, QWidget*> m_unitIDWidgetMap;           //存放选中单元的widget和对应的单元ID
    QMap<QWidget*, int> m_unitWidgetIDMap;           //存放选中单元的widget和对应的单元ID
    QMap<int, QMap<QString, WWidget*>> m_widgetMap;  // 单元号对应的所有控件

    QMap<int, bool> m_unitChangeStatus;  //用于界面处于修改状态时不刷新界面值

    QMap<int, QPushButton*> m_changeBtnMap;   //这里记录了每个单元的修改按钮
    QMap<int, QPushButton*> m_settingBtnMap;  //这里记录了每个单元的设置按钮   测控的一体化两个按钮不记录在内
    QMap<int, QPushButton*> m_changeBtnMapYTHCommon;  //根据测控基带一体化的区别设置两套修改设置按钮的Map  Common代表普通的一体化参数按钮集合
    QMap<int, QPushButton*> m_settingBtnMapYTHCommon;  // Multi代表有目标的一体化参数按钮集合
    QMap<int, QPushButton*> m_changeBtnMapYTHMulti;
    QMap<int, QPushButton*> m_settingBtnMapYTHMulti;

    quint64 m_deviceID = 0;                  //这个是当前设备的ID号如0x4001等
    QList<WWidget*> m_targetNumControlList;  // 储存控制多目标数的控件

    bool m_click = false;

    QMap<WWidget*, QVariant> m_commonMapBefore;  //这里是修改之前界面上的参数值以及控件
    QMap<int, QMap<WWidget*, QVariant>> m_multiMapBefore;

    QMap<WWidget*, QVariant> m_commonMapAfter;  //这里是设置之后界面上的参数值以及控件
    QMap<int, QMap<WWidget*, QVariant>> m_multiMapAfter;

    QMap<WWidget*, QVariant> m_needUpdateColorControlCommon;            //这里是记录了需要改变颜色的普通控件值
    QMap<int, QMap<WWidget*, QVariant>> m_needUpdateColorControlMulti;  //这里是记录了需要改变颜色的多目标控件值

    QTime m_setTime;  //记录设置参数时的时间

    QString m_currentProject;                //当前项目  4424 or  4426
    ParamRelationOtherManager m_paramOther;  //其它关联关系
    WidgetDealWith m_widgetDeal;             //以后创其它的自定义控件就用这里面的函数
};

#endif  // DEVICEMONITOR_H
