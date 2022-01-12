#ifndef UPLINKHANDLER_H
#define UPLINKHANDLER_H

#include "BaseParamMacroHandler.h"

// 上行链路处理
// 此类为内部使用类 单独使用一定会出错
enum class CKQDSendSource
{
    JD,
    QD,
};
enum class UpLinkDeviceControl
{
    All,  // 所有
    CK,   // 测控
    SC,   // 数传
    Ignore,
};
enum class RFOutputMode
{
    TX,  // 天线
    FZ,  // 负载
};
enum class EnableMode
{
    Enable,  //
    Disable,
    Ignore,
};
enum class JDOutPut
{
    All,                // 载波输出+遥控加调+测距加调
    Carrier,            // 载波输出
    RangAdd,            // 测距加调
    RemoteAdd,          // 遥控加调
    CarrierRangAdd,     // 载波输出测距加调
    CarrierRemoteAdd,   // 载波输出遥控加调
};
/*
 * 本来是要走配置文件的
 * 但是此链路会获取一些当前设备的值
 * 且是在参数宏已经下完的情况下
 * 这个不好处理
 * 关闭和打开是独立的
 */
class UpLinkHandler : public BaseParamMacroHandler
{
    Q_OBJECT
public:
    explicit UpLinkHandler(QObject* parent = nullptr);
    /* 启用或者禁用上行链路
     * 启用上行链路需要设置
     * setLinkLine  只能是测控的模式 数传是单独的只有低速
     */
    void setEnableHpa(EnableMode enable);  // 启用 禁用 忽略
    void setEnableJD(EnableMode enable);  // 启用 禁用 忽略
    void setEnableQD(EnableMode enable);  // 启用 禁用 忽略
    void setIgnoreXLDevice(bool ignore);  // 启用 禁用 忽略
    void setIgnoreOtherDevice(bool ignore);  // 启用 禁用 忽略

    void enableControl(UpLinkDeviceControl);                                         // 控制测控还是数传 或者所有
    void setQDSendSource(CKQDSendSource source);                                     // 前端去是基带送还是前端送
    void setRFOutputMode(RFOutputMode mode);                                         // 去天线还是去负载
    void setAdd(JDOutPut mode);                                                      // 基带输出模式
    bool handle() override;
    bool handleSpecific();
    SystemWorkMode getSystemWorkMode() override;

private:
    // 测控基带
    void ckjdEnable();
    void zbsc(const DeviceID& deviceID);
    void ykjt(const DeviceID& deviceID);
    void cljt(const DeviceID& deviceID);
    void ckjdDisable(const QList<DeviceID>&);

    // 低速基带
    void dsjdEnable();
    void dsjdDisable(const QList<DeviceID>&);

    void jdEnable();
    void jdDisable();
    void jdDisableSpecific();

    // 测控前端
    void qdEnableImpl(const DeviceID& masterDeviceID, SystemWorkMode workMode);
    void ckqdEnable();
    void ckqdDisable();

    // 功放
    void hpaEnable();
    void hpaDisable();
    void hpaDisableSpecific();

    void sHpaDisable();
    void kackHpaDisable();
    void kascHpaDisable();

    // 校零变频器
    void xlDisable();
    // 其他关联设备关闭
    void otherDisable();

    void disableDevice(const QString&cmdKey, int unitID, const QString& powerKey, const DeviceID&deviceID, int powerOffValue = 1, const QVariantMap&replaceMap = QVariantMap());
    void disableDevice(const QString&cmdKey, int unitID, const QString& powerKey, int powerOffValue = 1, const QVariantMap&replaceMap = QVariantMap());
    void disableDevice(const QString&cmdKey, const QString& powerKey, int powerOffValue = 1, const QVariantMap&replaceMap = QVariantMap());
    void disableDevice(const QString&cmdKey, const QVariantMap&replaceMap = QVariantMap());
    void disableDevice(const QString&cmdKey, const QString&powerKey, const DeviceID&deviceID, int powerOffValue = 1);

private:
    EnableMode m_enableHpa{ EnableMode::Ignore };
    EnableMode m_enableJD{ EnableMode::Ignore };
    EnableMode m_enableQD{ EnableMode::Ignore };
    EnableMode m_enableXL{ EnableMode::Disable };
    EnableMode m_enableOther{ EnableMode::Disable };
    CKQDSendSource m_ckqdSource{ CKQDSendSource::JD };
    UpLinkDeviceControl m_control{ UpLinkDeviceControl::All };
    RFOutputMode m_outputMode{ RFOutputMode::TX };
    JDOutPut m_jdOutPut{ JDOutPut::All };
};
#endif  // UPLINKHANDLER_H
