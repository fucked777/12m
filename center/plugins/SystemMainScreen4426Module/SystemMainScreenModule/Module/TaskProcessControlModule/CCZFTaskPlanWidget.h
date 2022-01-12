#ifndef CCZFTASKPLANWIDGET_H
#define CCZFTASKPLANWIDGET_H

#include "GlobalData.h"
#include "SatelliteManagementSerialize.h"
#include <QWidget>

namespace Ui
{
    class CCZFTaskPlanWidget;
}

class XComboBox;
class CCZFTaskPlanWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CCZFTaskPlanWidget(QWidget* parent = nullptr);
    ~CCZFTaskPlanWidget();

    void resetUIData();

protected:
    void showEvent(QShowEvent* event) override;

signals:
    /*单元参数设置命令*/
    void signalsUnitDeviceJson(QString json);

    /*发送过程控制命令*/
    void signalsCmdDeviceJson(QString json);

public slots:
    void refreUIData();

private slots:
    //一键实时事后命令
    void slotOnkeySSSHSetting();
    //一键事后命令
    void slotOnkeySHSetting();
    //一键只接收命令
    void slotOnkeyJSSetting();
    void slotUnitAndCmdSetting(int workWay);

    void slotSatellitCodeChange(const QString& arg1);  //任务代号切换 任务标识跟着变化

    void slotRefreshTime();

    void slotJDNameRelation(int index);

private:
    void initUI();
    void initSlot();

    bool checkDeviceStatus(QString& errorMsg);  //设备和服务器在线检测
    bool checkRelation(QString& errorMsg);      //下面命令时的关联检测

    // 设置设置面板QLabel颜色
    void setSettingPanelColor(QWidget* control, QColor color);

private:
    Ui::CCZFTaskPlanWidget* ui;

    //所有卫星信息
    QMap<QString, SatelliteManagementData> m_taskCodeSateliteMap;

    XComboBox* m_target_center = nullptr;  //数传中心下拉复选框
};

#endif  // CCZFTASKPLANWIDGET_H
