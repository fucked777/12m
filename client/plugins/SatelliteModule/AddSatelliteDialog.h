#ifndef ADDSATELLITEDIALOG_H
#define ADDSATELLITEDIALOG_H

#include "SatelliteManagementDefine.h"
#include "SatelliteXMLReader.h"
#include <QComboBox>
#include <QDialog>
#include <QTreeWidget>
namespace Ui
{
    class AddSatelliteDialog;
}

class XComboBox;
class AddSatelliteDialog : public QDialog
{
    Q_OBJECT

public:
    enum AddSatelliteDialogEnum
    {
        TK4424 = 4424,
        TK4426 = 4426
    };
    explicit AddSatelliteDialog(QWidget* parent = nullptr);
    ~AddSatelliteDialog();
    void setSatelliteDataList(const QList<SalliteModeInfo>& salliteModeInfoList);

    /* 更新配置宏界面 */
    void updateUI(SatelliteManagementData request);

    SatelliteManagementData currentsatelliteData() const;
    enum STATUS
    {
        ADD,
        EDIT,
        SHOW
    };
    void setCurrentUIMode(const STATUS& currentUIMode);
    void setMaxWorkMode(int maxWorkMode);

    void setStationBz(int stationBz);

signals:

    /* 添加卫星数据 */
    void signalAddSatellite(QString json);

    /* 编辑卫星数据 */
    void signalEditSatellite(QString json);
public slots:

    /*添加配置宏响应*/
    void addConfigMacroDialogSuccess();

    void slotAddSatelliteResponce(bool flag, SatelliteManagementData request);

    void slotEditSatelliteResponce(bool flag, SatelliteManagementData request);

    /* 初始化界面 */
    void initUI();

    void dpChangeValue(QString);

private slots:

    /* 工作模式变化的时候界面刷新 */
    void onWorkModeChanged(int);

    /* 添加配置宏事件响应 */
    void on_save_clicked();
    /* 退出添加配置宏事件响应 */
    void on_cancel_clicked();

    void on_desc_textChanged();

private:
    Ui::AddSatelliteDialog* ui;
    XComboBox* m_workmodeBox{ nullptr };                                               //工作模式下拉复选框
    XComboBox* m_sczxbz = nullptr;                                                     //工作模式下拉复选框
    QMap<int, QTreeWidget*> m_worksystemTreeMap;                                       //工作体制树对象集合
    QMap<SystemWorkMode, QComboBox*> m_defaultDpControlMap;                            //
    QMap<SystemWorkMode, QMap<int, QMap<QString, QWidget*>>> m_currentModeControlMap;  //工作模式下 点频几 下的参数 当前模式的控件
    QMap<SystemWorkMode, QMap<int, QMap<QString, QVariant>>> m_currentModeControlValueMap;  //工作模式下 点频几 下的参数 当前模式的控件值
    QString m_edit;
    STATUS m_currentUIMode = STATUS::ADD;  // 0:添加 1:编辑模式 2:查看

    QList<SalliteModeInfo> m_satelliteModeInfoList;

    SatelliteManagementData m_currentSalliteData;

    QMap<int, int> m_workModeMap;  // 重新打开时候的创建控件数量

    int m_maxWorkMode = 0;     //用于约束最大的集合
    int m_stationBz = 0;       //用于判断当前是什么站
    QString m_currentProject;  //当前项目  4424 or  4426
};

#endif  // ADDSATELLITEDIALOG_H
