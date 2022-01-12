#ifndef HP_H
#define HP_H

#include "WConverterWdBm.h"
#include <QWidget>

namespace Ui
{
    class HP;
}

class HP : public QWidget
{
    Q_OBJECT

public:
    explicit HP(QWidget* parent = nullptr);
    ~HP();

public:
    enum HPTYPE
    {
        SCK,   // S测控功放
        KaCK,  // Ka测控功放
        KaSC   // Ka数传功放
    };

    void setType(HPTYPE type);

signals:
    void signalsCmdDeviceJson(const QString& json);
    void signalsUnitDeviceJson(const QString& json);

private:
    //控制测控基带AB同时上天线发射或不发射
    void ctrlCKBBESTX(bool stx = true);

    //创建功率转化控件
    WWidget* createFreqContol(const int deviceId, const int modeId, const int unitId, const QString& paramId);

private:
    void initUI();

private:
    Ui::HP* ui;
    HPTYPE d_type;
    QString m_currentProject;  //当前项目  4424 or  4426
    WWidget* d_power_S;
    WWidget* d_power_KaCKA;
    WWidget* d_power_KaCKB;
    WWidget* d_power_KaSCA;
    WWidget* d_power_KaSCB;
};

#endif  // HP_H
