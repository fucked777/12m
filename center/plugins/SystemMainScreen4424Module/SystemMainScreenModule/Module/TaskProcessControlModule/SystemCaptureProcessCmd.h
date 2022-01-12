#ifndef SYSTEMCAPTUREPROCESSCMD_H
#define SYSTEMCAPTUREPROCESSCMD_H

#include "DevProtocol.h"
#include "DeviceProcessMessageDefine.h"

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

    void start(QWidget* comBox_MacId, QWidget* comBox_MacMode, QWidget* widget, const QString& widgetName);

signals:
    void signalsCmdDeviceJson(const QString& json);
    void signalsUnitDeviceJson(const QString& json);

    // isCheckMasterSlave是否检查主备
    void signalsCmdRequest(const CmdRequest& cmdRequest, bool isCheckMasterSlave = true);
    void signalsUnitParamRequest(const UnitParamRequest& unitParamRequest, bool isCheckMasterSlave = true);

private:
    int getCurrentDeviceId(QComboBox* combox);
    int getCurrentModeId(QComboBox* combox);

private:
    //只有扩二模式才需要判断目标数
    bool targetNumCheck(const int deviceId, const int curTargetNum);

private:
    SystemCaptureProcessXmlImpl* mImpl = nullptr;
};

#endif  // SYSTEMCAPTUREPROCESSCMD_H
