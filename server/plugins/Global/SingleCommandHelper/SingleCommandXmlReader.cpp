#include "SingleCommandXmlReader.h"

#include <QFile>
#include <QSettings>

#include "PlatformConfigTools.h"

#define SingleCommandConfigFileName "SingleCommand/SingleCommand.ini"

SingleCommandXmlReader::SingleCommandXmlReader() {}

QMap<QString, SingleCommand> SingleCommandXmlReader::getSingleCmdMap(QString& errorMsg)
{
    QMap<QString, SingleCommand> singleCmdMap;
    // 解析单命令配置文件
    parseSingleCommandConfigFile(PlatformConfigTools::configBusiness(SingleCommandConfigFileName), singleCmdMap, errorMsg);

    return singleCmdMap;
}

void SingleCommandXmlReader::parseSingleCommandConfigFile(const QString& filePath, QMap<QString, SingleCommand>& singleCmdMap, QString& errorMsg)
{
    if (!QFile::exists(filePath))
    {
        errorMsg = QString("不存在单命令配置文件：%1").arg(filePath);
        return;
    }

    QSettings setting(filePath, QSettings::IniFormat);
    setting.setIniCodec(QTextCodec::codecForName("UTF-8"));
    QStringList childList = setting.childGroups();

    for (int i = 0; i < childList.count(); i++)
    {
        QString groupName = childList.at(i);
        setting.beginGroup(groupName);
        if (groupName.startsWith("Step"))
        {
            SingleCommand singleCmd;
            /* 20210830 wp?? 不区分大小写了 */
            singleCmd.groupName = groupName.toUpper();
            for (auto key : setting.allKeys())
            {
                if (key == "StepType")
                {
                    auto cmdType = setting.value("StepType").toInt();
                    /* 1单命令 2单元命令 3更改多目标数据的单元命令(只能更改多目标) */
                    singleCmd.cmdTypeRaw = cmdType;
                    if (cmdType == 1)
                    {
                        singleCmd.cmdType = DevMsgType::ProcessControlCmd;
                    }
                    else if (cmdType == 2 || cmdType == 3)
                    {
                        singleCmd.cmdType = DevMsgType::UnitParameterSetCmd;
                    }
                    else
                    {
                        errorMsg = QString("文件:%1中，单命令Id:%2的命令类型错误").arg(filePath, groupName);
                        break;
                    }
                }
                else if (key == "StepDesc")
                {
                    singleCmd.cmdName = setting.value("StepDesc").toString();
                }
                else if (key == "DeviceID")
                {
                    singleCmd.deviceId = DeviceID(setting.value("DeviceID").toString().toInt(0, 16));
                }
                else if (key == "DeviceUnitOrCmd")
                {
                    singleCmd.cmdId = setting.value("DeviceUnitOrCmd").toString().toInt(0, 16);
                }
                else if (key == "DeviceMode")
                {
                    singleCmd.modeId = setting.value("DeviceMode").toString().toInt(0, 16);
                }
                else if (key == "DeviceName")
                {
                    singleCmd.deviceName = setting.value("DeviceName").toString();
                }
                else if (key == "DeviceInfo")
                {
                    singleCmd.deviceInfo = setting.value("DeviceInfo").toString();
                }
                else if (key == "TargetNo")
                {
                    /* 通道号 */
                    singleCmd.targetNo = setting.value("TargetNo").toInt();
                }
                // else if (key == "ValidIdent")
                // {
                //     /* 通道有效标识 */
                //     singleCmd.channelValidIdent = setting.value("ValidIdent").toInt();
                // }
                else
                {
                    QStringList valDescList = setting.value(key).toString().split(":");
                    if (valDescList.size() == 2)
                    {
                        singleCmd.paramMap[key] = valDescList[0];
                    }
                    else
                    {
                        singleCmd.paramMap[key] = setting.value(key);
                    }

                    if (singleCmd.paramMap[key].toString().startsWith("0x", Qt::CaseInsensitive))
                    {
                        singleCmd.paramMap[key] = singleCmd.paramMap[key].toString().toInt(nullptr, 16);
                    }
                }
            }

            if (singleCmdMap.contains(groupName))
            {
                errorMsg = QString("文件:%1中，单命令Id:%2重复").arg(filePath, groupName);
            }

            singleCmdMap[singleCmd.groupName] = singleCmd;
        }

        setting.endGroup();
    }
}
