#include "DeviceCommunicationConfig.h"
#include "CommunicationDefine.h"
#include "CommunicationSerialize.h"
#include "CppMicroServicesUtility.h"
#include "DevProtocol.h"
#include "DevProtocolSerialize.h"
#include "GlobalData.h"
#include "LocalCommunicationAddr.h"
#include "PlatformInterface.h"
#include "Utility.h"
#include <QCoreApplication>
#include <QDomDocument>
#include <QFile>
#include <QTextStream>

static constexpr auto devInfoXmlPath = "DeviceCommunicationConfig.xml";
class DeviceCommunicationConfigImpl
{
    DIS_COPY_MOVE(DeviceCommunicationConfigImpl)
public:
    // CommunicationInfoMap infoMap;
    cppmicroservices::BundleContext context;

public:
    inline DeviceCommunicationConfigImpl() {}
    inline ~DeviceCommunicationConfigImpl() {}

    static Optional<QDomDocument> openXml();
    static QDomDocument createXml();
    static Optional<std::tuple<CommunicationInfoMap, MessageAddress>> reload();
    static Optional<CommunicationInfo> parsingItem(QDomElement& element);
    static void parsingGroup(QDomElement& element, CommunicationInfoMap& infoMap);
    static void parsingLocalAddr(QDomElement& element, MessageAddress& localAddr);
    static bool saveItem(QDomElement& element, const CommunicationInfo&);
    static OptionalNotValue save(const CommunicationInfoMap& value);
};

Optional<QDomDocument> DeviceCommunicationConfigImpl::openXml()
{
    using ResType = Optional<QDomDocument>;

    auto filePath = PlatformConfigTools::configBusiness(devInfoXmlPath);
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        auto errMsg = QString("%1:%2").arg(file.errorString(), filePath);
        return ResType(ErrorCode::OpenFileError, errMsg);
    }

    QDomDocument domDoc;
    QString errStr;
    int errLine{ 0 };
    int errcCol{ 0 };
    if (!domDoc.setContent(&file, &errStr, &errLine, &errcCol))
    {
        errStr = QString("设备配置信息解析失败:%1,错误行:%2,错误列:%2").arg(errStr).arg(errLine).arg(errcCol);
        return ResType(ErrorCode::XmlParseError, errStr);
    }
    return ResType(domDoc);
}

QDomDocument DeviceCommunicationConfigImpl::createXml()
{
    QDomDocument domDoc;

    auto instruction = domDoc.createProcessingInstruction("xml", R"(version="1.0" encoding="UTF-8")");
    domDoc.appendChild(instruction);
    return domDoc;
}
OptionalNotValue DeviceCommunicationConfigImpl::save(const CommunicationInfoMap& value)
{
    auto filePath = PlatformConfigTools::configBusiness(devInfoXmlPath);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return OptionalNotValue(ErrorCode::OpenFileError, file.errorString());
    }

    auto doc = DeviceCommunicationConfigImpl::createXml();

    auto root = doc.createElement("CommunicationInfo");
    root.setAttribute("ChannelNum", QString::number(value.channelNum));
    QMap<QString, QList<QDomElement>> groupMap;

    for (auto& item : value.infoMap)
    {
        auto element = doc.createElement("DevItem");
        if (!saveItem(element, item))
        {
            qWarning() << "跳过保存设备通讯信息:" << item;
            continue;
        }
        groupMap[item.localIP].append(element);
    }
    for (auto iter = groupMap.begin(); iter != groupMap.end(); ++iter)
    {
        auto& group = iter.value();
        auto element = doc.createElement("Group");
        element.setAttribute("IP", iter.key());

        for (auto& item : group)
        {
            element.appendChild(item);
        }
        root.appendChild(element);
    }

    doc.appendChild(root);
    doc.normalize();

    QTextStream out(&file);

    doc.save(out, 4);

    return OptionalNotValue();
}

bool DeviceCommunicationConfigImpl::saveItem(QDomElement& element, const CommunicationInfo& info)
{
    QString typeStr;
    typeStr << info.type;
    if (typeStr.isEmpty())
    {
        return false;
    }

    element.setAttribute("Desc", info.desc);
    element.setAttribute("ID", info.id);
    element.setAttribute("Type", typeStr);
    element.setAttribute("DesAddress", info.desIP);
    element.setAttribute("SendPort", quint32(info.sendPort));
    element.setAttribute("RecvPort", quint32(info.recvPort));
    element.setAttribute("ChannelNo", info.channelNo);
    element.setAttribute("IsCenterDevice", quint32(info.isCenterDevice));  // add wjy ,用于判断是否是中心体设备
    element.setAttribute("onlyRecvTargetData", info.onlyRecvTargetData ? "true" : "false");

    return true;
}
Optional<CommunicationInfo> DeviceCommunicationConfigImpl::parsingItem(QDomElement& element)
{
    using ResType = Optional<CommunicationInfo>;
    CommunicationInfo info;
    if (!element.hasAttribute("ID"))
    {
        return ResType(ErrorCode::DataItemIsMissing, "ID不存在");
    }
    if (!element.hasAttribute("DesAddress"))
    {
        return ResType(ErrorCode::DataItemIsMissing, "目标地址不存在");
    }
    if (!element.hasAttribute("SendPort"))
    {
        return ResType(ErrorCode::DataItemIsMissing, "发送端口不存在");
    }
    if (!element.hasAttribute("RecvPort"))
    {
        return ResType(ErrorCode::DataItemIsMissing, "接收端口不存在");
    }
    if (!element.hasAttribute("Type"))
    {
        return ResType(ErrorCode::DataItemIsMissing, "通讯类型不存在");
    }

    if (element.hasAttribute("ChannelNo"))
    {
        info.channelNo = element.attribute("ChannelNo", "1").toUInt();
        info.channelNo = info.channelNo <= 0 ? 1 : info.channelNo;
    }
    else
    {
        info.channelNo = 1;
    }

    info.id = element.attribute("ID").toUInt(nullptr, 16);
    info.desc = element.attribute("Desc");
    info.desIP = element.attribute("DesAddress");
    info.sendPort = element.attribute("SendPort").toUShort();
    info.recvPort = element.attribute("RecvPort").toUShort();
    info.isCenterDevice = element.attribute("IsCenterDevice").toUInt();
    QVariant var = element.attribute("OnlyRecvTargetData", "true");
    info.onlyRecvTargetData = var.toBool();

    auto typeStr = element.attribute("Type");
    typeStr >> info.type;

    info.recvDataDesModule = element.attribute("RecvDataDesModule");

    var = element.attribute("DevProtocolCheck", "true");
    info.devProtocolCheck = var.toBool();

    return ResType(info);
}

void DeviceCommunicationConfigImpl::parsingGroup(QDomElement& element, CommunicationInfoMap& infoMap)
{
    /* 跳过无IP的分组 */
    auto localIP = element.attribute("IP");
    if (localIP.isEmpty())
    {
        return;
    }

    /* 20210804 wp?? 接收数据的模块 */
    auto recvDataDesModule = element.attribute("RecvDataDesModule");

    /* 遍历分组 */
    auto node = element.firstChild();
    while (!node.isNull())
    {
        auto isComment = node.isComment();
        auto devItem = node.toElement();
        node = node.nextSibling();
        /* 跳过注释 */
        if (isComment)
        {
            continue;
        }

        auto item = parsingItem(devItem);
        if (!item)
        {
            qWarning() << "设备通讯配置解析失败:" << item.msg();
            continue;
        }

        if (infoMap.infoMap.contains(item->id))
        {
            qWarning() << "设备通讯配置重复:" << item->id;
            continue;
        }
        item->localIP = localIP;
        if (item->recvDataDesModule.isEmpty())
        {
            item->recvDataDesModule = recvDataDesModule;
        }
        infoMap.infoMap.insert(item->id, item.value());
    }
}
void DeviceCommunicationConfigImpl::parsingLocalAddr(QDomElement& element, MessageAddress& localAddr)
{
    /* 遍历分组 */
    auto node = element.firstChild();
    while (!node.isNull())
    {
        auto name = node.nodeName();
        auto isComment = node.isComment();
        auto element = node.toElement();
        node = node.nextSibling();
        /* 跳过注释 */
        if (isComment)
        {
            continue;
        }

        auto value = static_cast<quint8>(element.text().toUInt(nullptr, 16));
        if (name == "BaseID")
        {
            localAddr.baseNumb = value;
        }
        else if (name == "StationID")
        {
            localAddr.stationNumb = value;
        }
        else if (name == "DeviceID")
        {
            localAddr.equipmentNumb = value;
        }
        else if (name == "ExtenName")
        {
#ifdef COMPATIBLE
            localAddr.systemNumb = value / 0x10;
            localAddr.deviceNumb = value % 0x10;
#else
            localAddr.extenName = value;
#endif
        }
        else if (name == "ExtenNumb")
        {
            localAddr.extenNumb = value;
        }
    }
}
Optional<std::tuple<CommunicationInfoMap, MessageAddress>> DeviceCommunicationConfigImpl::reload()
{
    using ResType = Optional<std::tuple<CommunicationInfoMap, MessageAddress>>;
    auto doc = openXml();
    if (!doc.success())
    {
        return ResType(doc.errorCode(), doc.msg());
    }

    auto root = doc->documentElement();

    std::tuple<CommunicationInfoMap, MessageAddress> result;
    auto& retMap = std::get<0>(result);
    auto& retAddr = std::get<1>(result);

    retMap.channelNum = root.attribute("ChannelNum", "1").toInt();
    retMap.channelNum = retMap.channelNum <= 0 ? 1 : retMap.channelNum;
    auto node = root.firstChild();
    while (!node.isNull())
    {
        auto isComment = node.isComment();
        auto name = node.nodeName();
        auto element = node.toElement();
        node = node.nextSibling();
        /* 跳过注释 */
        if (isComment)
        {
            continue;
        }
        if (name == "Group")
        {
            parsingGroup(element, retMap);
        }
        else if (name == "LocalCommunicationAddr")
        {
            parsingLocalAddr(element, retAddr);
        }
    }
    return ResType(result);
}
/****************************************************************************************/
/****************************************************************************************/
/****************************************************************************************/
/****************************************************************************************/
/****************************************************************************************/
/****************************************************************************************/
DeviceCommunicationConfig::DeviceCommunicationConfig(cppmicroservices::BundleContext context)
    : INetMsg(nullptr)
    , m_impl(new DeviceCommunicationConfigImpl)
{
    m_impl->context = context;
    registerSubscribe("reload", &DeviceCommunicationConfig::reload, this);
    registerSubscribe("getAll", &DeviceCommunicationConfig::getAll, this);
    registerSubscribe("refresh", &DeviceCommunicationConfig::refresh, this);
    registerSubscribe("changeAll", &DeviceCommunicationConfig::changeAll, this);
    registerSubscribe("changeInfoWithID", &DeviceCommunicationConfig::changeInfoWithID, this);

    init();
}

DeviceCommunicationConfig::~DeviceCommunicationConfig() { delete m_impl; }
void DeviceCommunicationConfig::init()
{
    auto res = DeviceCommunicationConfigImpl::reload();
    if (!res.success())
    {
        qCritical() << "设备通讯配置模块初始失败:" << res.msg();
        return;
    }
    auto& retMap = std::get<0>(res.value());
    auto& retAddr = std::get<1>(res.value());

    LocalCommunicationAddr::setDevAddrMessage(retAddr);
    GlobalData::setCommunicationInfo(retMap);
}
/* 重载配置文件 */
void DeviceCommunicationConfig::reload(const ProtocolPack& pack)
{
    using ResType = Optional<CommunicationInfoMap>;
    auto resPack = pack;
    resPack.data.clear();
    auto res = DeviceCommunicationConfigImpl::reload();

    if (res.success())
    {
        auto& retMap = std::get<0>(res.value());
        auto& retAddr = std::get<1>(res.value());

        LocalCommunicationAddr::setDevAddrMessage(retAddr);
        GlobalData::setCommunicationInfo(retMap);

        resPack.data << ResType(retMap);
    }
    else
    {
        /* 重新加载失败原来的值不变 */
        qCritical() << "设备通讯配置模块重新加载失败:" << res.msg();
        resPack.data << ResType(ErrorCode::DataLoadFailed, res.msg());
    }
    silenceSendACK(resPack);
}
void DeviceCommunicationConfig::refresh(const ProtocolPack& pack)
{
    auto resPack = pack;
    resPack.data.clear();
    auto res = DeviceCommunicationConfigImpl::reload();

    if (res.success())
    {
        auto& retMap = std::get<0>(res.value());
        auto& retAddr = std::get<1>(res.value());

        LocalCommunicationAddr::setDevAddrMessage(retAddr);
        GlobalData::setCommunicationInfo(retMap);

        resPack.data << OptionalNotValue();
    }
    else
    {
        /* 重新加载失败原来的值不变 */
        qCritical() << "设备通讯配置模块重新加载失败:" << res.msg();
        resPack.data << OptionalNotValue(ErrorCode::DataLoadFailed, res.msg());
    }
    silenceSendACK(resPack);
}
/* 获取所有的设备信息 */
void DeviceCommunicationConfig::getAll(const ProtocolPack& pack)
{
    using ResType = Optional<CommunicationInfoMap>;

    auto resPack = pack;
    auto tempInfo = GlobalData::communicationInfo();
    resPack.data.clear();
    resPack.data << ResType(tempInfo);

    silenceSendACK(resPack);
}

/* 更新所有,会写入文件 */
void DeviceCommunicationConfig::changeAll(const ProtocolPack& pack)
{
    auto resPack = pack;

    /* 反序列化数据 */
    CommunicationInfoMap recvValue;
    resPack.data >> recvValue;
    resPack.data.clear();

    GlobalData::setCommunicationInfo(recvValue);
    resPack.data << DeviceCommunicationConfigImpl::save(recvValue);

    silenceSendACK(resPack);
}
/* 更新指定ID的设备信息,会写入文件 */
void DeviceCommunicationConfig::changeInfoWithID(const ProtocolPack& pack)
{
    auto resPack = pack;

    /* ID为16进制字符串 */
    CommunicationInfo replace;
    resPack.data >> replace;
    resPack.data.clear();

    auto tempInfo = GlobalData::communicationInfo();

    auto find = tempInfo.infoMap.find(replace.id);
    if (find == tempInfo.infoMap.end())
    {
        resPack.data << OptionalNotValue(ErrorCode::NotFound, "未找到对应的ID信息");
    }
    else
    {
        find.value() = replace;
        GlobalData::setCommunicationInfo(tempInfo);
        resPack.data << DeviceCommunicationConfigImpl::save(tempInfo);
    }

    silenceSendACK(resPack);
}
