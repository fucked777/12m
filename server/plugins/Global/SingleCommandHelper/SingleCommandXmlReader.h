#ifndef SINGLECOMMANDXMLREADER_H
#define SINGLECOMMANDXMLREADER_H

#include "BusinessMacroCommon.h"
#include "CustomPacketMessageDefine.h"

// 配置文件的单命令
struct SingleCommand
{
    QString groupName;
    DeviceID deviceId;                          // 设备ID
    int modeId{ -1 };                           // 模式id
    int cmdId{ -1 };                            // 单元或者命令id
    DevMsgType cmdType{ DevMsgType::Unknown };  // 命令类型
    /* 多目标处理 20210828 wp??
     * 通道有效标识是代表基带收到数据之后数据的解析范围 比如Skuo2是4个目标有效标识是2则只会解析公共单元和目标1的数据
     */
    int cmdTypeRaw{ 0 };  // ini配置中的命令类型原始值
    int targetNo{ 0 };    // 目标号 此目标号在 扩频是1-4 在一体化是1-2 为0则是公共单元的数据

    QString cmdName;
    QString deviceName;
    QString deviceInfo;
    QMap<QString, QVariant> paramMap;
    /* 20211022 写这个是用于单命令的 比如DTE的任务加载 */
    QMap<int, QList<QPair<QString, QVariant>>> multiParamMap;
};

// 单命令文件读取
class SingleCommandXmlReader
{
public:
    SingleCommandXmlReader();

    QMap<QString, SingleCommand> getSingleCmdMap(QString& errorMsg);

private:
    // 解析单命令文件
    void parseSingleCommandConfigFile(const QString& filePath, QMap<QString, SingleCommand>& singleCmdMap, QString& errorMsg);
};

#endif  // SINGLECOMMANDXMLREADER_H
