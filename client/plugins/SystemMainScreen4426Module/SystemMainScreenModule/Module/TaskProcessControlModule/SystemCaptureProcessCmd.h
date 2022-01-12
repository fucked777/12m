#ifndef SYSTEMCAPTUREPROCESSCMD_H
#define SYSTEMCAPTUREPROCESSCMD_H

#include "DevProtocol.h"
#include "DeviceProcessMessageDefine.h"
#include "SystemCaptureProcessXmlReader.h"

#include <QDialog>
#include <QDomDocument>

struct DeviceID;
class QComboBox;

class SystemCaptureProcessXmlImpl;
class SystemCaptureProcessXml : public QObject
{
    Q_OBJECT
public:
    SystemCaptureProcessXml();
    ~SystemCaptureProcessXml() {}

    void setModeID(int modeID);
    void start(QWidget* comBox_MacId, QWidget* widget, const QString& widgetName);

signals:
    void signalsCmdDeviceJson(const QString& json);
    void signalsUnitDeviceJson(const QString& json);

    // isCheckMasterSlave是否检查主备
    void signalsCmdRequest(const CmdRequest& cmdRequest, bool isCheckMasterSlave = true);
    void signalsUnitParamRequest(const UnitParamRequest& unitParamRequest, bool isCheckMasterSlave = true);

private:
    int getCurrentDeviceId(QComboBox* combox);
    int getCurrentModeId(QComboBox* combox);

    void dealMasterSlaveSwitch(const CommandTemplate& temp);

    void getSlaveCKDeviceIDAndValue(QVariant& value, DeviceID& secondDevID);

    //这个函数是专门处理发射开关矩阵的，所以里面没有考虑有效标识
    void unitSetting(int unitID, int modeID, const DeviceID& devID, const QMap<QString, QVariant>& paramMap);
    //这个函数是专门处理主备机切换命令的，所以最后触发命令信号时加了个false参数
    void cmdSetting(int cmdID, int modeID, const DeviceID& devID, const QMap<QString, QVariant>& paramMap);

private:
    //只有扩二模式才需要判断目标数
    bool targetNumCheck(const int deviceId, const int curTargetNum);

private:
    SystemCaptureProcessXmlImpl* mImpl = nullptr;
};

#endif  // SYSTEMCAPTUREPROCESSCMD_H
