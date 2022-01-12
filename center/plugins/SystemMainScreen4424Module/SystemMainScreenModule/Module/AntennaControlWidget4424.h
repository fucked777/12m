#ifndef AntennaControlWidget4424_H
#define AntennaControlWidget4424_H

#include <QComboBox>
#include <QDialog>
#include <QMutex>
#include <QThread>
#include <QTimer>
#include <QVariant>

namespace Ui
{
    class AntennaControlWidget4424;
}

struct SatelliteManagementData;
struct AutorunPolicyData;
// 天线控制模块
class AntennaControlWidget4424 : public QDialog
{
    Q_OBJECT

public:
    explicit AntennaControlWidget4424(QWidget* parent = nullptr);
    ~AntennaControlWidget4424();

private:
    void initUI();
    void initArrow();

private:
    void standby();     /* 待机 */
    void point();       /* 指向 */
    void tower();       /* 对塔 */
    void waitPoint();   /* 等待转点 */
    void numberLead();  /* 数字引导 */
    void programLead(); /* 程序引导 */
    void collection();  /* 收藏不插锁 */
    void unlock();      /* 拔锁 */

    //    void waitPoint(); /* 等待转点 */
    //    void tower();     /* 对塔 */
    void taskStart(); /* 任务开始 */
    void sTrack1();   /* s跟踪 */
    // void kaMeasureTrack1();  /* ka测控跟踪 */
    void kaDataTransTrack(); /* ka数传跟踪   */

    void acuRunMode();    /* acu运行方式 */
    void taskTimeQuery(); /* 任务时间查询 */
    void gdybSet();       /* 轨道预报 */
    void arrowTop();
    void arrowBottom();
    void arrowRight();
    void arrowLeft();
    void arrowZero();

    /* 服务器，分控判断 */
    bool isOnline();
    // void setupPAImpl(QVariant value);
    // void setupDirectImpl(double value);

    void setupDirectImpl(double az, double pa, double azSpeed, double paSpeed);
    void comboxRedal(QComboBox* combox);  // combox处理
    void search();                        /* 查找 */
    void solt();                          /* 刷新 */

    void slotOneKeyXXS();    /* 一键校相S频段 */
    void slotOneKeyXXKaYc(); /* 一键校相Ka遥测 */
    void slotOneKeyXXKaSc(); /* 一键校相Ka数传 */

    void cmdSetting(int cmdID, QMap<QString, QVariant> paramMap = {});

signals:
    // void signalRefreshUI();
    void signalsCmdDeviceJson(const QString& json);
    void signalsUnitDeviceJson(const QString& json);

protected:
    void timerEvent(QTimerEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    Ui::AntennaControlWidget4424* ui;
    /* 当前方位俯仰 */
    QMap<QString, QString> m_currentWorkMode;
    QMap<QString, QStringList> mWorkModeDescMap;
    //所有卫星信息
    QMap<QString, SatelliteManagementData> m_taskCodeSateliteMap;
    //    double m_zeroAZ{ 0.0 };        /* 方位 */
    //    double m_zeroPA{ 0.0 };        /* 俯仰 */
    double m_currentAZ{ 0.0 }; /* 方位 */
    double m_currentPA{ 0.0 }; /* 俯仰 */
    double m_AZspeed{ 0.0 };   /* 方位速度 */
    double m_PAspeed{ 0.0 };   /* 俯仰速度 */
    //    bool m_syncAZControl{ false }; /* 同步方位控制按钮的数据 */
    //    bool m_syncPAControl{ false }; /* 同步方位控制按钮的数据 */
};

#endif  // AntennaControlWidget4424_H
