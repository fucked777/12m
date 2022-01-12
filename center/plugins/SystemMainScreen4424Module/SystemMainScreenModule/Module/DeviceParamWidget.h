#ifndef DEVICEPARAMWIDGET_H
#define DEVICEPARAMWIDGET_H

#include "ComplexLEDStatusLabel.h"
#include "ComplexNumberStatusLabel.h"
#include "CustomPacketMessageDefine.h"
#include "LEDStatusDecodeLabel.h"
#include "LEDStatusStimuLabel.h"
#include "NumberStatusFreqLabel.h"
#include "NumberStatusPowerLabel.h"
#include "NumberStatusRotationLabel.h"
#include "ProtocolXmlTypeDefine.h"
#include <QMutex>
#include <QThread>
#include <QWidget>
class QString;
// 系统参数监视模块
class DeviceParamWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DeviceParamWidget(QWidget* parent = nullptr);
    ~DeviceParamWidget();

signals:
    void signalsCmdDeviceJson(QString json);
    void signalsUnitDeviceJson(QString json);

    void signalRefreshStaticUI();
    void signalRefreshDynamicUI();

private:
    //各个模式下下行频率的参数ID都不一致，因此没办法统一处理
    QString getSBandDownFreqId_24(int modeId) const;
    int isMultiTarget_24(int modeId) const;
    QVariant getSBandDownFreqValue_24(int deviceId, int modeId, int unitId, int onlineSta);



private slots:
    void slotsRefreshStatic();
    void slotsRefreshDynamic();

protected:
    void timerEvent(QTimerEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;

private:
    bool mIsShow = false;
    QMutex m_refreshMutex;
    QMap<DeviceID, ExtensionStatusReportMessage> currentDataMap;
    QMap<DeviceID, ExtensionStatusReportMessage> dynamicDataMap;

    /*key为uuid,value为动态控件的取值*/
    QMap<QString, QVariant> d_currentComplextDataMap;
    /*key为uuid,value为动态控件所需要用到的设备id列表*/
    QMap<QString, QVariantList> d_currentComplexDevids;
    /*key为uuid,value为译码控件的配置信息*/
    QMap<QString, QList<LEDStatusDecodeLabel::LEDParam>> d_decodeInfos;
    /*key为uuid,value为发射功率控件的配置信息*/
    QMap<QString, QList<NumberStatusPowerLabel::SumInfo>> d_powerInfos;
    /*key为uuid,value为激励控件的配置信息*/
    QMap<QString, QList<LEDStatusStimuLabel::SumInfo>> d_stimuInfos;
    /*key为uuid,value为发射或接收频率控件的配置信息*/
    QMap<QString, QList<NumberStatusFreqLabel::SumInfo>> d_freqInfos;
    /*key为uuid,value为多目标发射功率控件的配置信息*/
    QMap<QString, QMap<int, QList<NumberStatusFreqLabel::MultiTargetSumInfo>>> d_multiTargetFreqInfos;
    /*key为uuid,value为旋向控件的配置信息*/
    QMap<QString, QList<NumberStatusRotationLabel::ParamInfo>> d_rotationInfos;
    /*key为uuid,value为26跟踪基带扩频相关模式的配置信息*/
    QMap<QString, ComplexNumberStatusLabel::SumInfo> d_26TrkBBENumberInfos;
    /*key为uuid,value为26跟踪基带扩频相关模式的配置信息*/
    QMap<QString, ComplexLEDStatusLabel::SumInfo> d_26TrkBBELedInfos;

    static uint staticCount;
    static uint dynamicCount;

};

#endif  // DEVICEPARAMWIDGET_H
