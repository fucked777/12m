#ifndef CONFIGMACROXMLWORKMODEDEFINE_H
#define CONFIGMACROXMLWORKMODEDEFINE_H

#include "DevProtocol.h"
#include "ProtocolXmlTypeDefine.h"
#include "SystemWorkMode.h"

struct EnumEntry
{
    QString desc;       // 枚举值描述
    QString value;      // 枚举值
    DeviceID deviceId;  // 选择该值对应的设备ID，比如测控前端、跟踪前端、下变频器（可以没有）
};

struct Item
{
    enum ConfigItemLinkTypeEnum
    {
        OnlyDownLink = 0,
        DownAndUpLink
    };

    QString id;                                      // id
    QString desc;                                    // 参数描述
    int startBit = -1;                               // 设备组合号里面的开始位
    int endBit = -1;                                 // 设备组合号里面的结束位
    int offset = 0;                                  // 设备组合号里面的偏移量
    ConfigItemLinkTypeEnum linkType = OnlyDownLink;  // 设备组合号类设备上下行区分
    bool isManualControl{ false };  // 是否是只在主界面任务流程控制中的手动控制界面中显示，为true只在手动控制界面显示，配置宏界面不显示
    QVariant value{ 0 };     // 当前参数值
    QList<EnumEntry> enums;  // 参数对应的枚举值 QMap<枚举值, 枚举值描述>
};

Q_DECLARE_METATYPE(Item)

struct ConfigMacroWorkMode
{
    SystemWorkMode workMode{ SystemWorkMode::NotDefine };
    QString desc;

    QList<Item> items;
};

using ConfigMacroWorkModeMap = QMap<SystemWorkMode, ConfigMacroWorkMode>;
using ConfigMacroWorkModeList = QList<ConfigMacroWorkMode>;

using ItemMap = QMap<QString, Item>;
using ItemList = QList<Item>;

using EnumEntryMap = QMap<QString, EnumEntry>;
using EnumEntryList = QList<EnumEntry>;

#endif  // CONFIGMACROXMLWORKMODEDEFINE_H
