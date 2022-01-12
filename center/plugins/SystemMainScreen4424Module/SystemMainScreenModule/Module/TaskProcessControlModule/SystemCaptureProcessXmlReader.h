#ifndef SYSTEMCAPTUREPROCESSXMLREADER_H
#define SYSTEMCAPTUREPROCESSXMLREADER_H

#include "DevProtocol.h"

#include <QDomDocument>

//只考虑过程控制命令的打包
typedef struct CommandTemplate
{
    QString defaultDeviceId;  //设备Id,从控件的下拉框中选择
    QString defaultModeId;    //模式Id,从控件的下拉框中选择
    QString defaultTargetId;  //通道号
    QString name;             //唯一标识
    QString cmdIdOrUnitId;
    QString validIdent;
    DevMsgType cmdType;

    QMap<QString, QVariant> staticParams;   //不需要动态获取的参数，可以直接配置的参数
    QMap<QString, QVariant> dynamicParams;  //需要从控件中动态获取值的参数

    QList<QDomElement> domELems;
} CmdTemp;

class SystemCaptureProcessXmlReader
{
public:
    SystemCaptureProcessXmlReader();

    bool getCmdList(QList<CommandTemplate>& cmdList, QString* errorMsg = nullptr);

private:
    void parseXML(const QString& path);

private:
    QList<CommandTemplate> mCmdList;
    QString mErrorMsg;
};

#endif  // SYSTEMCAPTUREPROCESSXMLREADER_H
