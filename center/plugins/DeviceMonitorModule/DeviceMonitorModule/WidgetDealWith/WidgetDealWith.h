#ifndef WIDGETDEALWITH_H
#define WIDGETDEALWITH_H

#include "CustomPacketMessageDefine.h"
#include "DeviceProcessMessageDefine.h"
#include "ProtocolXmlTypeDefine.h"
#include "WWidget.h"
#include <QGroupBox>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QWidget>

/////////////////这个CPP文件用于Widget的各个处理（如新控件的添加，界面上的值获取）/////////////////////

class WidgetDealWith : public QWidget
{
    Q_OBJECT
public:
    explicit WidgetDealWith(QWidget* parent = nullptr);

    void setTargetID(const QString& targetID);

    // htf 为需要的命令添加QPlainTextEdit
    QWidget* initCmdStatusPlaintextWidget(int cmdID, int modeID, CmdAttribute cmdAttribute, DeviceID deviceID);

    //多中心数状态面板设置
    QWidget* initDataStatusMultiWidget(Unit unit, int centerNum);

    //初始化动态界面(设置)
    QHBoxLayout* initMultiWidgetStatusHLayout(CmdAttribute cmdAttribute);
    QWidget* initCmdMulitiStatusWidgets(CmdAttribute cmdAttribute, const QString& name);

    // AGC状态面板数据上报处理
    void dealWithAGCMultiWidgets(QWidget* widget, CmdAttribute cmdAttribute, const CmdResult& cmdResponce);
    //初始化动态界面(设置)
    QHBoxLayout* initMultiWidgetSetsHLayout(CmdAttribute cmdAttribute);
    QWidget* initCmdMulitiSetWidgets(CmdAttribute cmdAttribute, const QString& name);
    //初始化ACU的AGC曲线界面
    QWidget* initCmdMulitiACUAGCSetWidgets(CmdAttribute cmdAttribute, const QString& name);

    //动态添加虚拟信道和挑帧个数 以及基带数量
    void dealCmdMulitiSetsWidgets(const QString& objName, QVariant value, CmdAttribute cmdAttribute, QMap<int, QWidget*> cmdIDWidgetMap);
    void dealCmdMulitiSetVirWidgets(const QString& objName, QVariant value, QWidget* widget, CmdAttribute cmdAttribute);
    void dealCmdMulitiSetFrameWidgets(const QString& objName, QVariant value, QWidget* widget, CmdAttribute cmdAttribute);
    void dealCmdMulitiSetJDNumWidgets(const QString& objName, QVariant value, QWidget* widget, CmdAttribute cmdAttribute);
    void dealCmdMulitiSetAGCWidgets(const QString& objName, QVariant value, QWidget* widget, CmdAttribute cmdAttribute);
    void dealCmdMulitiSetACUAGCWidgets(const QString& objName, QVariant value, QWidget* widget, CmdAttribute cmdAttribute);
    //多中心数状态面板数据上报处理
    void dealWithDataStatusMultiWidgets(QGroupBox* groupBox, Unit unit, UnitReportMessage reportMessage);

    //取得界面某个参数的值
    void getWidgetParamValue(const QString& paramName, QVariant& paramValue, QScrollArea* scrollArea);
    //取得界面某个控件
    WWidget* getWidgetParamControl(const QString& paramName, QScrollArea* scrollArea);
    QMap<int, WWidget*> getMultiWidgetParamControl(const QString& paramName, QScrollArea* scrollArea);

    //处理测控基带跨单元的参数关联关系  如扩频单目标时，接收单元就有三个参数 左旋 右旋 左右旋同时
    void dealWidgetControlMultiRelation(int recycle, const QString& objName, QVariant value, QMap<int, WWidget*> controlMap);

    //低速基带第一个界面的工作模式取值
    QString getDSWidgetWorkModeText(Unit unit, UnitReportMessage reportMessage);

    //处理低速基带跨单元的参数关联关系
    void dealWidgetControlRelation(WWidget* trigger, WWidget* receiverI, WWidget* receiverQ);

    void dealWidgetRelation(int unitID, const QString& objName, const QVariant value, QGroupBox* groupBox);
    //处理低速基带的界面关联关系
    void dealWithDSRelation(const QString& objName, const QVariant value, QGroupBox* groupBox);

    //处理跟踪基带标准+低速 扩频+低速的界面关联关系
    void dealWithGZDSRelation(const QString& objName, const QVariant value, QGroupBox* groupBox);

    //处理低速基带码速率控件的范围  第一个参数时要改范围的控件  第二个参数是中频接收单元的界面数据Map  第三个参数是解调单元的界面数据Map
    void dealDSWidgetMSLNumRangeJT(WWidget* control, const QMap<QString, QVariant>& setParamJSMap, const QMap<QString, QVariant>& setParamJTMap);
    //处理低速基带码速率控件的范围  第一个参数时要改范围的控件  第二个参数是模拟源单元的界面数据
    void dealDSWidgetMSLNumRangeMNY(WWidget* controlI, WWidget* controlQ, const QMap<QString, QVariant>& setParamData);
    //处理低速基带码速率控件的范围  第一个参数时要改范围的控件  第二个参数是上行数传的界面数据Map
    void dealDSWidgetMSLNumRangeSC(WWidget* control, const QMap<QString, QVariant>& setParamSCMap);

    //处理跟踪基带码速率控件的范围  第一个参数时要改范围的控件  第二个参数是中频接收单元的界面数据Map  第三个参数是解调单元的界面数据Map
    void dealGZWidgetMSLNumRangeJT(WWidget* control, const QMap<QString, QVariant>& setParamJSMap, const QMap<QString, QVariant>& setParamJTMap);

    //处理测控基带标准TTC码速率控件的范围  第一个参数时要改范围的控件  第二个参数是遥控单元的界面数据
    void dealCkWidgetMSLNumRangeTTCRCU(WWidget* control, const QMap<QString, QVariant>& setParamData);
    //处理测控基带扩频/一体化码速率控件的范围  第一个参数是当前recycle所在tabWidget界面  第二个是要修改范围的控件集合  第三个参数是遥控单元的界面数据
    void dealCkWidgetMSLMultiNumRangeRCU(int recycle, QString trigParam1, QString trigParam2, QMap<int, WWidget*> controlMap,
                                         const QList<QMap<QString, QVariant>>& paraTargerMap);

    // 计算码速率最大值及最小值
    void calcuCodeRateMinMaxValue(int initMinValue, int initMaxValue, double bValue, QString mark, int& minValue, int& maxValue);

    void setMode(ModeCtrl mode);

    void setDeviceID(DeviceID deviceID);

    // private:
    //获取当前界面的设置参数值(标准和扩频)
    bool getSetParamMapOther(QMap<QString, QVariant>& map, QWidget* widget);
    //获取当前界面的多目标参数值（标准和扩频）
    bool getSetMultiTargetParamMapOther(QList<QMap<QString, QVariant>>& list, QWidget* widget);
    //获取当前界面的设置参数值(一体化)
    bool getSetParamMapYTH(QMap<QString, QVariant>& map, QWidget* widget);
    //获取当前界面的多目标参数值（一体化）
    bool getSetMultiTargetParamMapYTH(QList<QMap<QString, QVariant>>& list, UnitParamRequest& unitParamRequest, QWidget* widget);

    //通过ID 和单元查找枚举值，然后将枚举值返回后赋值到控件本身 按协议排序
    bool getEnumData(QString id, Unit& unit, QMap<QString, QVariant>& map);
    //通过ID 和命令数据查找枚举值，然后将枚举值返回后赋值到控件本身 按协议排序
    bool getCmdEnumData(QString id, QMap<QString, QVariant>& map);

    //通过ID 和单元查找枚举值，然后将枚举值返回后赋值到控件本身  按svalue排序
    bool getEnumDataOrder(QString id, Unit& unit, QMap<int, QPair<QString, QVariant>>& map);
    //通过ID 和命令数据查找枚举值，然后将枚举值返回后赋值到控件本身  svalue排序
    bool getCmdEnumDataOrder(QString id, QMap<int, QPair<QString, QVariant>>& map);

    //获取存储转发/DTE当前界面的扩展参数值
    bool getCmdMultiParamMap(QMap<int, QList<QPair<QString, QVariant>>>& map, QWidget* widget);

    //获取当前界面的设置参数值--命令
    bool getCmdSetParamMap(QMap<QString, QVariant>& map, QWidget* widget);

    //获取当前界面的设置参数值(标准和扩频) 关联关系
    void getSetParamMapOtherRelation(QMap<QString, QVariant>& map, QWidget* widget);

private:
    const QString SET_TITLE = QString("设置");
    const QString CMD_TEST = QString("测试");
    const QString CMD_FILENAME = QString("文件名称");
    const QString CENTERNUM_TITLE = QString("中心数");
    const QString CMD_SET_MULTI_VIRTITLE = QString("识别信道个数");
    const QString CMD_SET_MULTI_FRAMETITLE = QString("挑帧个数");
    const QString CMD_SET_MULTI_JDNUM = QString("基带数量");
    const QString CMD_SET_MULTI_AGC = QString("AGC曲线点数");

    DeviceID m_currentDeviceID;  //当前设备ID
    ModeCtrl m_currentMode;      //当前设备模式

    QString m_targetID;  //这个用于记住每个模式里目标数的参数ID

    QList<WWidget*> m_tfsAGCWidget;  //用于存放ACU里面的AGC界面动态参数控件
signals:
};

#endif  // WIDGETDEALWITH_H
