#include "BlockDiagramConfig.h"
#include "PlatformInterface.h"
#include <QDomDocument>
#include <QFile>

BlockDiagramConfig::BlockDiagramConfig() {}
BlockDiagramConfig::~BlockDiagramConfig() {}

OptionalNotValue BlockDiagramConfig::reload()
{
    m_info = SystemBlockInfo();
    auto filePath = PlatformConfigTools::configBusiness("GraphConfig/DataConfig.xml");
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        auto errMsg = QString("框图配置解析失败 %1").arg(file.errorString());
        return OptionalNotValue(ErrorCode::OpenFileError, errMsg);
    }

    QDomDocument doc;
    QString error;
    int errorRow = -1;
    int errorCol = -1;
    if (!doc.setContent(&file, &error, &errorRow, &errorCol))
    {
        auto errMsg = QString("框图配置解析失败 %1,在%2第%3行第%4列").arg(error).arg(file.fileName()).arg(errorRow).arg(errorCol);
        return OptionalNotValue(ErrorCode::OpenFileError, errMsg);
    }

    auto root = doc.documentElement();
    return parseXML(root);
}
OptionalNotValue BlockDiagramConfig::parseXML(const QDomElement& eleValue)
{
    auto node = eleValue.firstChild();
    while (!node.isNull())
    {
        auto isComment = node.isComment();
        auto element = node.toElement();
        auto name = node.nodeName();
        node = node.nextSibling();
        /* 跳过注释 */
        if (isComment)
        {
            continue;
        }
        /* 默认的颜色 */
        if (name == "DefaultScale")
        {
            m_info.scaleX = element.attribute("scaleX", "1.0").trimmed().toDouble();
            m_info.scaleY = element.attribute("scaleY", "1.0").trimmed().toDouble();
        }
        if (name == "DefaultColorValue")
        {
            m_info.rgbOffLine = element.attribute("offLine", "0xFFD3D3D3").trimmed().toUInt(nullptr, 16);
            m_info.rgbSubControl = element.attribute("subControl", "0xFFDFBA1D").trimmed().toUInt(nullptr, 16);
            m_info.rgbFault = element.attribute("fault", "0xFFFF0000").trimmed().toUInt(nullptr, 16);
            m_info.rgbNormal = element.attribute("normal", "0xFF00FF00").trimmed().toUInt(nullptr, 16);
            m_info.rgbCurLink = element.attribute("curLink", "0xFFFFFF00").trimmed().toUInt(nullptr, 16);
            m_info.rgbNotCurLink = element.attribute("notCurLink", "0xFF000000").trimmed().toUInt(nullptr, 16);
            m_info.rgbShutdown = element.attribute("shutdown", "0xFF1D92DF").trimmed().toUInt(nullptr, 16);
            m_info.rgbSystem = element.attribute("system", "0xFFC71585").trimmed().toUInt(nullptr, 16);
            m_info.rgbNotFound = element.attribute("notFound", "0xFFD3D3D3").trimmed().toUInt(nullptr, 16);
            m_info.rgbDefault = element.attribute("default", "0xFF000000").trimmed().toUInt(nullptr, 16);
        }
        if (name == "DefaultIconValue")
        {
            QDomNamedNodeMap map = element.attributes();
            for (int i = 0; i < map.count(); i++)
            {
                QDomNode node = map.item(i);
                QDomAttr attr = node.toAttr();
                m_info.ribbonIcons.insert(attr.name(), attr.value());
            }
        }
        /* 分系统 */
        else if (name == "System")
        {
            auto result = parseSystemTypeInfo(element);
            if (result.success())
            {
                auto tupleValue = std::make_tuple(ItemShowType::System, QVariant::fromValue<SystemTypeInfo>(result.value()));
                m_info.dataConfigMap.insert(result->graphicsID, tupleValue);
            }
        }
        else if (name == "TwoValueSwitch" || name == "TwoValueCrossSwitch")
        {
            auto result = parseTwoValueSwitchTypeInfo(element);
            if (result.success())
            {
                auto tupleValue = std::make_tuple(ItemShowType::TwoValueSwitch, QVariant::fromValue<TwoValueSwitchTypeInfo>(result.value()));
                m_info.dataConfigMap.insert(result->graphicsID, tupleValue);
                m_info.switchTypeMap.insert(result->graphicsID, result.value());
            }
        }
        else if (name == "Status")
        {
            auto result = parseDeviceStatusTypeInfo(element);
            if (result.success())
            {
                auto tupleValue = std::make_tuple(ItemShowType::Status, QVariant::fromValue<DeviceStatusTypeInfo>(result.value()));
                m_info.dataConfigMap.insert(result->graphicsID, tupleValue);
                m_info.deviceStatusTypeMap.insert(result->graphicsID, result.value());
            }
        }
        parseXML(element);
    }
    return OptionalNotValue();
}
Optional<SystemTypeInfo> BlockDiagramConfig::parseSystemTypeInfo(const QDomElement& element)
{
    using ResType = Optional<SystemTypeInfo>;
    SystemTypeInfo info;
    info.graphicsID = element.attribute("graphicsID").trimmed();
    if (info.graphicsID.isEmpty())
    {
        return ResType(ErrorCode::DataNotEmpty, "图元ID不能为空");
    }
    info.statusTriggerModule = element.attribute("triggerModule").trimmed();
    if (info.statusTriggerModule.isEmpty())
    {
        return ResType(ErrorCode::DataNotEmpty, "触发模块名不能为空");
    }

    info.systemID = element.attribute("systemID").trimmed();
    if (info.systemID.isEmpty())
    {
        return ResType(ErrorCode::DataNotEmpty, "系统ID不能为空");
    }
    info.systemName = element.attribute("desc", "分系统").trimmed();
    return ResType(info);
}

Optional<PlusCmdInfo> parseTwoValueSwitchValueInfo(const QDomElement& element)
{
    using ResType = Optional<PlusCmdInfo>;

    // int value = element.attribute("value").toInt();

    PlusCmdInfo plusCmdInfo;
    auto domNode = element.firstChild();
    while (!domNode.isNull())
    {
        auto domEle = domNode.toElement();
        if (domEle.isNull())
        {
            continue;
        }

        if (domEle.tagName() == "UnitParam")
        {
            int unitId = domEle.attribute("unitId").toInt();
            QString strParamInfo = domEle.attribute("paramInfo");
            QMap<QString, QVariant> paramInfo;
            auto tempList = strParamInfo.split(" ");
            for (auto& p : tempList)
            {
                if (p.contains("|"))
                {
                    paramInfo.insert(p.split("|").at(0), p.split("|").at(1).toInt());
                }
            }

            plusCmdInfo.unitParam.insert(unitId, paramInfo);
        }

        if (domEle.tagName() == "ProcessControl")
        {
            int cmdId = domEle.attribute("cmdId").toInt();
            QString strParamInfo = domEle.attribute("paramInfo");
            QMap<QString, QVariant> paramInfo;
            auto tempList = strParamInfo.split(" ");
            for (auto& p : tempList)
            {
                if (p.contains("|"))
                {
                    paramInfo.insert(p.split("|").at(0), p.split("|").at(1).toInt());
                }
            }

            plusCmdInfo.processControl.insert(cmdId, paramInfo);
        }
        domNode = domNode.nextSibling();
    }

    return ResType(plusCmdInfo);
}

Optional<TwoValueSwitchTypeInfo> BlockDiagramConfig::parseTwoValueSwitchTypeInfo(const QDomElement& element)
{
    using ResType = Optional<TwoValueSwitchTypeInfo>;
    TwoValueSwitchTypeInfo info;
    info.graphicsID = element.attribute("graphicsID").trimmed();
    if (info.graphicsID.isEmpty())
    {
        return ResType(ErrorCode::DataNotEmpty, "图元ID不能为空");
    }
    auto tempList = element.attribute("statusInfo").trimmed().split(' ', QString::SkipEmptyParts);
    if (tempList.size() != 3)
    {
        return ResType(ErrorCode::InvalidArgument, "错误的状态信息");
    }
    bool isOK1 = false;
    info.redisKey = tempList[0];
    info.unitID = tempList[1].toInt(&isOK1, 16);
    info.statusKey = tempList[2];
    if (!isOK1)
    {
        return ResType(ErrorCode::InvalidArgument, "单元ID错误");
    }

    tempList = element.attribute("cmdInfo").trimmed().split(' ', QString::SkipEmptyParts);
    if (tempList.size() != 4)
    {
        return ResType(ErrorCode::InvalidArgument, "错误的命令信息");
    }
    auto deviceID = tempList[0].toInt(&isOK1, 16);
    if (!isOK1)
    {
        return ResType(ErrorCode::InvalidArgument, "错误的设备ID");
    }
    DeviceID tempDeviceID(deviceID);
    info.systemNum = tempDeviceID.sysID;          /* 系统号 */
    info.deviceNum = tempDeviceID.devID;          /* 设备号 */
    info.extenNum = tempDeviceID.extenID;         /* 分机号 */
    info.modeNum = tempList[1].toInt(&isOK1, 16); /* 当前设备模式号 */
    if (!isOK1)
    {
        return ResType(ErrorCode::InvalidArgument, "错误的设备模式号");
    }

    info.cmdNum = tempList[2].toInt(&isOK1, 16); /* 命令号 */
    if (!isOK1)
    {
        return ResType(ErrorCode::InvalidArgument, "错误的命令号");
    }
    info.cmdKey = tempList[3];
    /* 数据映射 */
    tempList = element.attribute("valueMap").trimmed().split(' ', QString::SkipEmptyParts);
    /* 二值开关不等于2就是错误 */
    if (tempList.size() != 2)
    {
        return ResType(ErrorCode::InvalidArgument, "错误的值映射");
    }
    QStringList itemTupleList;
    bool isOK2 = false;
    for (int i = 0; i < 2; ++i)
    {
        itemTupleList = tempList.at(i).split('|', QString::SkipEmptyParts);
        if (itemTupleList.size() != 3)
        {
            return ResType(ErrorCode::InvalidArgument, "错误的值映射");
        }
        info.twoValueRelational[i] = std::make_tuple(itemTupleList[0].toInt(&isOK1, 16), itemTupleList[1].toDouble(&isOK2), itemTupleList[2]);
        if (!(isOK1 & isOK2))
        {
            return ResType(ErrorCode::InvalidArgument, "错误的值映射");
        }
    }

    auto domNode = element.firstChild();
    while (!domNode.isNull())
    {
        auto domEle = domNode.toElement();
        if (domEle.isNull())
        {
            continue;
        }
        if (domEle.tagName() == "Value")
        {
            Optional<PlusCmdInfo> plusCmdInfo = parseTwoValueSwitchValueInfo(domEle);
            if (plusCmdInfo.success())
            {
                int unitId = domEle.attribute("value").toInt();
                info.addtionalInfo.addtionalInfo.insert(unitId, plusCmdInfo.value());
            }
            else
            {
                return ResType(ErrorCode::InvalidArgument, "错误的二值开关补充信息");
            }
        }
        domNode = domNode.nextSibling();
    }

    return ResType(info);
}

/* 开关电源 */
static Optional<ShutdownInfo> parseShutdown(const QDomElement& element)
{
    using ResType = Optional<ShutdownInfo>;
    ShutdownInfo info;

    auto tempList = element.attribute("shutdownID").split('|', QString::SkipEmptyParts);
    if (tempList.size() != 3)
    {
        return ResType(info);
    }
    bool isOK1 = false;
    bool isOK2 = false;
    info.unitID = tempList.at(0).toInt(&isOK1, 16);
    info.indexKey = tempList.at(1);
    info.shutdownValue = tempList.at(2).toDouble(&isOK2);
    if (!(isOK1 & isOK2))
    {
        return ResType(ErrorCode::InvalidArgument, "加去电信息错误");
    }
    return ResType(info);
}

/* 解析主备信息 */
static Optional<QList<MainPreparationInfo>> parseMainPreparation(const QDomElement& elem)
{
    //    QString mainRedisKey;  /* 主机的redisKey 这里是考虑有些设备判断主备不是在同一台设备上 */
    //    QString redisKey;      /* 从redis中查找上报数据的key */
    //    QString indexKey;      /* 主备的Key */
    //    quint32 deviceID{ 0 }; /* 设备ID */
    //    qint32 unitID{ -1 };   /* 单元号 */
    //    qint32 mainValue;      /* 主机的值 */

    using ResType = Optional<QList<MainPreparationInfo>>;
    QList<MainPreparationInfo> listInfo;
    auto tempStrList = elem.attribute("mainPreparation").split(' ', QString::SkipEmptyParts);
    if (tempStrList.isEmpty())
    {
        return ResType(listInfo);
    }
    bool isOK1 = false;
    bool isOK2 = false;
    for (auto& item : tempStrList)
    {
        MainPreparationInfo info;
        auto tempList = item.split('|', QString::KeepEmptyParts);
        if (tempList.size() != 5)
        {
            return ResType(ErrorCode::InvalidArgument, "主备信息错误");
        }
        info.redisKey = tempList[0];
        info.unitID = tempList[1].toInt(&isOK1, 16);
        info.indexKey = tempList[2];
        info.mainValue = tempList[3].toInt(&isOK2, 16);
        info.mainRedisKey = tempList[4];
        if (!(isOK1 & isOK2))
        {
            return ResType(ErrorCode::InvalidArgument, "主备信息错误");
        }
        listInfo << info;
    }
    return ResType(listInfo);
}

Optional<DeviceStatusTypeInfo> BlockDiagramConfig::parseDeviceStatusTypeInfo(const QDomElement& element)
{
    using ResType = Optional<DeviceStatusTypeInfo>;
    DeviceStatusTypeInfo info;
    info.graphicsID = element.attribute("graphicsID").trimmed();
    if (info.graphicsID.isEmpty())
    {
        return ResType(ErrorCode::DataNotEmpty, "图元ID不能为空");
    }
    info.statusTriggerModule = element.attribute("triggerModule").trimmed();
    if (info.statusTriggerModule.isEmpty())
    {
        return ResType(ErrorCode::DataNotEmpty, "触发模块名不能为空");
    }

    info.systemName = element.attribute("desc", "分系统").trimmed();
    info.systemID = element.attribute("systemID").trimmed();
    if (info.systemID.isEmpty())
    {
        return ResType(ErrorCode::DataNotEmpty, "系统ID不能为空");
    }

    info.redisKey = element.attribute("statusRedisKey").trimmed();
    if (info.redisKey.isEmpty())
    {
        return ResType(ErrorCode::InvalidArgument, "状态的Redis键读取错误");
    }

    auto tempList = element.attribute("statusID").split(' ', QString::SkipEmptyParts);
    if (tempList.size() != 3)
    {
        return ResType(ErrorCode::DataNotEmpty, "状态信息为空");
    }
    bool isOK1 = false;
    bool isOK2 = false;
    info.comprehensiveStateInfo.unitID = tempList.at(0).toInt(&isOK1, 16);
    info.comprehensiveStateInfo.indexKey = tempList.at(1);
    info.comprehensiveStateInfo.normal = tempList.at(2).toInt(&isOK2, 16);
    if (!(isOK1 & isOK2))
    {
        return ResType(ErrorCode::InvalidArgument, "状态信息错误");
    }

    if (info.redisKey.isEmpty())
    {
        return ResType(ErrorCode::InvalidArgument, "状态的Redis键读取错误");
    }

    /* 主备 */
    auto mainPreparationRes = parseMainPreparation(element);
    if (!mainPreparationRes.success())
    {
        return ResType(mainPreparationRes.errorCode(), mainPreparationRes.msg());
    }
    info.mainPreparationList = mainPreparationRes.value();

    /* 加去电 */
    auto shutdownRes = parseShutdown(element);
    if (!shutdownRes.success())
    {
        return ResType(shutdownRes.errorCode(), shutdownRes.msg());
    }
    info.shutdownInfo = shutdownRes.value();

    /* 本分控 */

    tempList = element.attribute("controlID").split('|', QString::SkipEmptyParts);
    QString controlID = element.attribute("controlID");
    if (!controlID.isEmpty())
    {
        if (tempList.size() != 3)
        {
            return ResType(ErrorCode::InvalidArgument, "错误的本分控信息");
        }
        else
        {
            info.controlInfo.unitID = tempList.at(0).toInt(&isOK1, 16);
            info.controlInfo.indexKey = tempList.at(1);
            info.controlInfo.subControl = tempList.at(2).toInt(&isOK2, 16);
            if (!(isOK1 & isOK2))
            {
                return ResType(ErrorCode::InvalidArgument, "错误的本分控信息");
            }
        }
    }
    else
    {
        //部分模块允许本分控信息为空
    }
    return ResType(info);
}
