#ifndef ADDPARAMMACRODIALOG_H
#define ADDPARAMMACRODIALOG_H

#include "ParamMacroMessage.h"

#include "ParamMacroRelationManager.h"
#include <QDialog>
#include <QTreeWidgetItem>
namespace Ui
{
    class AddParamMacroDialog;
}

class QMenu;
class WWidget;
struct ParamMacroModeInfo;
class AddParamMacroDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddParamMacroDialog(QWidget* parent = nullptr);
    ~AddParamMacroDialog();

    enum WidgetMode
    {
        Add = 0,  // 添加
        Edit,     // 编辑
        Look      // 查看
    };

    void initUI();
    void initSlot();

    void setWidgetMode(const WidgetMode& currentUIMode);

    // 设置生成参数界面数据
    void setParamMacroModeInfoList(const QList<ParamMacroModeInfo>& paramMacroModeInfoList);

    // 设置和获取参数宏数据
    void setParamMacroData(const ParamMacroData& paramMacro);
    ParamMacroData getParamMacroData() const;

protected:
    void closeEvent(QCloseEvent* event);

private:
    // 创建点频参数控件
    void createPointFreqControl(QTreeWidget* tree, QTreeWidgetItem* parentItem, SystemWorkMode workMode, int dpNum, const QString& deviceID,
                                const ParameterAttribute& attr, const QMap<QString, DevEnum>& devEnumMap);

    // 创建设备参数控件
    void createDeviceParamControl(QTreeWidget* tree, QTreeWidgetItem* parentItem, QTreeWidgetItem* deviceWidgetItem, SystemWorkMode workMode,
                                  const QString& deviceID, int unitID, const ParameterAttribute& attr, const QMap<QString, DevEnum>& devEnumMap);

private:
    void processRelation_4426_STTC();
    void processRelation_4426_SKuo2();
    void processRelation_4426_SKT();
    void processRelation_4426_KaDS();
    void processRelation_4426_KaGS();
    void processRelation_4426_XDS();

    //处理低速基带码速率控件的范围  第一个参数时要改范围的控件  第二个参数是中频接收单元的界面数据Map  第三个参数是解调单元的界面数据Map
    void dealDSWidgetMSLNumRangeJT(WWidget* control, const QMap<QString, QVariant>& setParamJSMap, const QMap<QString, QVariant>& setParamJTMap);
    void dealDSWidgetMSLNumRangeJT(const QMap<QString, QVariant>& setParamJSMap, const QMap<QString, QVariant>& setParamJTMap, int& maxValue,
                                   int& minValue);
    //处理低速基带码速率控件的范围  第一个参数时要改范围的控件  第二个参数是上行数传的界面数据Map
    void dealDSWidgetMSLNumRangeSC(WWidget* control, const QMap<QString, QVariant>& setParamSCMap);
    void dealDSWidgetMSLNumRangeSC(const QMap<QString, QVariant>& setParamSCMap, int& maxValue, int& minValue);

    // 计算码速率最大值及最小值
    void calcuCodeRateMinMaxValue(int initMinValue, int initMaxValue, double bValue, QString mark, int& minValue, int& maxValue);

    bool checkPolarRelation(QString& msg);
signals:
    void signalAddParamMacro(const QByteArray& json);
    void signalEditParamMacro(const QByteArray& json);

public slots:
    void slotOperationResult(bool result, const QString& msg);

private slots:
    // 参数宏左边树结构点击
    void slotParamMacroModeTreeWidgetClicked(QTreeWidgetItem* item, int column);
    // 确定按钮点击
    void slotOkBtnClicked();
    // 参数宏左边树结构右键点击
    void slotParamMacroModeTreeWidgetCustomContextMenuRequested(const QPoint& pos);
    // 复制点频确定按钮点击
    void slotCopyPointFreqClicked();
    // 同步参数宏和卫星的模式数据
    // 为了当卫星模式动态增加或者减少时参数宏数据也能动态的增加或者减少
    void syncParamData(const SatelliteManagementData& currentSatelliteData);

private:
    Ui::AddParamMacroDialog* ui;

    QList<ParamMacroModeInfo> mParamMacroModeInfoList;    // 生成界面的配置
    QMap<QTreeWidgetItem*, QTreeWidget*> mTreeWidgetMap;  // 树节点对应旁边的参数设置界面

    WidgetMode mCurrentMode = WidgetMode::Add;
    QString mTipsMessage;

    // 储存点频参数控件  QMap<模式, QMap<点频, QMap<4001_1/4002_1, QMap<参数id, WWidget*>>>>
    QMap<SystemWorkMode, QMap<int, QMap<QString, QMap<QString, WWidget*>>>> mModePointFreqControlMap;
    // 储存设备参数控件  QMap<模式, QMap<4001/4002, QMap<单元id, QMap<参数id, 控件>>>>
    QMap<SystemWorkMode, QMap<QString, QMap<int, QMap<QString, WWidget*>>>> mModeDeviceControlMap;
    // 动态参数按钮 QMap<模式, QMap<点频, QMap<4001_1/4002_1, 按钮>>>
    QMap<SystemWorkMode, QMap<int, QMap<QString, QPushButton*>>> mModeDynamicBtnMap;

    ParamMacroData mParamMacroData;  // 当前参数宏数据

    QTreeWidgetItem* mPointFreqGroupParamItem = nullptr;                           // 复制参数宏节点
    QMap<int, QMap<int, QMap<QString, QTreeWidgetItem*>>> mPointFreqUnitParamMap;  // 点频的单元参数 QMap<模式id,QMap<第几个点频,QMap<组名称,节点>>>
    QList<WWidget*> mSReceivPolarList;                                             // 接收极化控件

    QMap<SystemWorkMode, QMap<int, WWidget*>> m_currentSendPolarMap;    // 发射极化控件
    QMap<SystemWorkMode, QMap<int, WWidget*>> m_currentReceivPolarMap;  // 接收极化控件
    QMap<SystemWorkMode, QMap<int, WWidget*>> m_currentTrackPolarMap;   // 跟踪极化控件
    static ParamMacroRelationManager m_paraRelation;                    //关联关系
};

#endif  // ADDPARAMMACRODIALOG_H
