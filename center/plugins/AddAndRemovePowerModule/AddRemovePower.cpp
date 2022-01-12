#include "AddRemovePower.h"
#include "CppMicroServicesUtility.h"

#include "DeviceProcessMessageDefine.h"
#include "DeviceProcessMessageSerialize.h"
#include "GlobalData.h"
#include "PlatformInterface.h"
#include "QssCommonHelper.h"
#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QDomDocument>
#include <QDomNodeList>
#include <QFile>
#include <QGridLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPushButton>
#include <QTextEdit>
#include <QUdpSocket>
#include <QVBoxLayout>
#include <QXmlStreamWriter>

enum class OperationStatus
{
    On,
    Off
};
enum class PlugStatus
{
    Unknown, /* 未知 */
    AllOn,   /* 全开 */
    AllOff,  /* 全关 */
    Part,    /* 部分关闭。。。或者说部分开启 */
};

enum class PowerFlow
{
    Exit,
    ShutdownDev,
    ShutdownPlug,
    OnPlug,
    Check,
    Wait,
};

class AutoCloseReply
{
public:
    explicit AutoCloseReply(QNetworkReply* reply)
        : m_reply(reply)
    {
        Q_ASSERT(reply != nullptr);
    }
    ~AutoCloseReply() { m_reply->deleteLater(); }

private:
    QNetworkReply* m_reply;
};

static constexpr int timerS = 1000;
class AddRemovePowerImpl
{
public:
    cppmicroservices::BundleContext context;
    QTextEdit* logWidget{ nullptr };
    QNetworkAccessManager networkManager;
    QMap<QString, PowerStripInfo> powerStripMap;
    QList<std::tuple<QCheckBox*, CenterBodyInfo>> centerBodyWidget; /* 中心体设备选择 */
    QMap<QString, PlugStatus> plugStatusMap;                        /* 存状态的 */
    QList<PlugInfo> curOperationPlugList;                           /* 排插接口的信息 */
    QStringList curPlugKeyList;
    QList<PlugDeviceInfo> curShutdownDeviceList; /* 排插设备的信息 */
    SHPPowerInfo shp;                            /* S功放信息 */

    int timerID{ -1 };                             /* 定时器ID */
    int devShutdownCount{ 0 };                     /* 关闭设备计数 */
    int plugCount{ 0 };                            /* 关闭、开启插排计数 */
    int queryCount{ 0 };                           /* 查询计数3此后queryResult为false */
    int waitCount{ 0 };                            /* 等待计数 */
    OperationStatus status{ OperationStatus::On }; /*  开还是关 */
    PowerFlow powerFlow{ PowerFlow::Exit };
    bool queryResult{ false }; /* 是否有查询结果 */

public:
    static void parseXml(const QString& filePath, QMap<QString, PowerStripInfo>& powerStripMap, QList<CenterBodyInfo>& centerList, SHPPowerInfo& shp);

    static void parseXmlItem(QDomElement& element, QMap<QString, PowerStripInfo>& powerStripMap, QList<CenterBodyInfo>& centerList,
                             SHPPowerInfo& shpInfo, QString& host, quint16& port);
    // static void saveXml(const QString& filePath, const QList<PowerStripInfo>& powerStripList);
    static QGroupBox* createItemGroup(const QString& name, QPushButton*& conditionsOnBtn, QPushButton*& conditionsOffBtn);
    QGroupBox* createItemGroup(const QList<CenterBodyInfo>& centerBody, QPushButton*& allSelect, QPushButton*& unSelect,
                               QPushButton*& conditionsOnBtn, QPushButton*& conditionsOffBtn);

    static QUrl createPlugOnUrl(const PlugInfo& plugInfo);
    static QUrl createPlugOffUrl(const PlugInfo& plugInfo);
    static QUrl createPlugStatusUrl(const PlugInfo& plugInfo);
    static QUrl createDeviceOffUrl(const PlugDeviceInfo& info);

    bool assemblingCmd(const CenterBodyInfo& info, int value, QString& json);
    bool assemblingUnitCmd(const CenterBodyInfo& info, int value, QString& json);

    void operationPlug(QUrl (*)(const PlugInfo&));

    void sendRequest(const QUrl& url);

    void reset(OperationStatus stat);
    void writeLog(const QString& log) { logWidget->append(log); }
};
void AddRemovePowerImpl::operationPlug(QUrl (*func)(const PlugInfo&))
{
    /* 列表为空 */
    if (curOperationPlugList.isEmpty())
    {
        writeLog("未找到需要操作的排插信息");
        powerFlow = PowerFlow::Exit;
        return;
    }
    /* 发送排插关闭指令 */
    for (auto& item : curOperationPlugList)
    {
        if (!item.isEnable)
        {
            continue;
        }
        auto url = func(item);
        sendRequest(url);
    }
    powerFlow = PowerFlow::Wait;
}
void AddRemovePowerImpl::parseXmlItem(QDomElement& element, QMap<QString, PowerStripInfo>& powerStripMap, QList<CenterBodyInfo>& centerList,
                                      SHPPowerInfo& shpInfo, QString& host, quint16& port)
{
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
        if (name == "PowerStrip")
        {
            host = element.attribute("host").trimmed();
            auto& tempValue = powerStripMap[host];

            tempValue.name = element.attribute("name").trimmed();
            port = static_cast<quint16>(element.attribute("port").trimmed().toUInt());
        }
        else if (name == "Device")
        {
            auto& tempValue = powerStripMap[host];

            PlugDeviceInfo tempDev;
            tempDev.name = element.attribute("name").trimmed();
            tempDev.host = element.attribute("deviceIP").trimmed();
            tempDev.port = static_cast<quint16>(element.attribute("devicePort").trimmed().toUInt());
            tempValue.deviceList.append(tempDev);
        }
        else if (name == "Plug")
        {
            auto& tempValue = powerStripMap[host];

            PlugInfo tempPlug;
            tempPlug.name = element.attribute("name").trimmed();
            tempPlug.no = static_cast<quint8>(element.attribute("no").trimmed().toUInt());
            tempPlug.host = host;
            tempPlug.port = port;
            auto var = QVariant(element.attribute("isEnable").trimmed());
            tempPlug.isEnable = var.toBool();
            tempValue.plugList.append(tempPlug);
        }
        else if (name == "CenterBody")
        {
            CenterBodyInfo tempInfo;
            tempInfo.type = element.attribute("type").trimmed().toLower();
            tempInfo.name = element.attribute("name").trimmed();
            tempInfo.devID = element.attribute("devID").trimmed().toULongLong(nullptr, 16);
            tempInfo.cmdID = element.attribute("cmdID").trimmed().toInt(nullptr, 16);
            tempInfo.key = element.attribute("key").trimmed();
            tempInfo.on = element.attribute("on").trimmed().toInt(nullptr, 16);
            tempInfo.off = element.attribute("off").trimmed().toInt(nullptr, 16);
            if (tempInfo.type == "unit" || tempInfo.type == "process")
            {
                centerList.append(tempInfo);
            }
        }
        else if (name == "SHP")
        {
            shpInfo.ip = element.attribute("ip").trimmed();
            shpInfo.sendPort = element.attribute("sendPort").trimmed().toInt();
            shpInfo.recvPort = element.attribute("recvPort").trimmed().toInt();
        }
        parseXmlItem(element, powerStripMap, centerList, shpInfo, host, port);

        // else if (name == "Devices")
        // {
        // }
        // else if (name == "Interface")
        // {
        // }
    }
}
void AddRemovePowerImpl::parseXml(const QString& filePath, QMap<QString, PowerStripInfo>& powerStripMap, QList<CenterBodyInfo>& centerList,
                                  SHPPowerInfo& shp)
{
    QFile file(filePath);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return;

    QDomDocument doc;
    if (!doc.setContent(&file))
        return;

    auto rootEle = doc.documentElement();

    QString host;
    quint16 port = 0;
    parseXmlItem(rootEle, powerStripMap, centerList, shp, host, port);
}

QGroupBox* AddRemovePowerImpl::createItemGroup(const QString& name, QPushButton*& conditionsOnBtn, QPushButton*& conditionsOffBtn)
{
    auto currentBox = new QGroupBox(name);
    auto currentLayout = new QHBoxLayout;
    currentLayout->setContentsMargins(10, 10, 10, 10);
    currentLayout->setSpacing(15);

    conditionsOnBtn = new QPushButton("一键加电", currentBox);
    conditionsOffBtn = new QPushButton("一键去电", currentBox);

    currentLayout->addStretch();
    currentLayout->addWidget(conditionsOnBtn);
    currentLayout->addWidget(conditionsOffBtn);
    currentLayout->addStretch();

    currentBox->setLayout(currentLayout);

    return currentBox;
}
QGroupBox* AddRemovePowerImpl::createItemGroup(const QList<CenterBodyInfo>& centerBody, QPushButton*& allSelect, QPushButton*& unSelect,
                                               QPushButton*& conditionsOnBtn, QPushButton*& conditionsOffBtn)
{
    if (centerBody.isEmpty())
    {
        return nullptr;
    }
    auto currentBox = new QGroupBox("中心体设备");
    auto mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(15);

    /* checkbox的布局 */
    auto checkBoxLayout = new QGridLayout;
    checkBoxLayout->setContentsMargins(0, 0, 0, 0);
    checkBoxLayout->setHorizontalSpacing(15);
    checkBoxLayout->setVerticalSpacing(15);

    int row = 0;
    int col = 0;
    for (auto& item : centerBody)
    {
        auto checkBox = new QCheckBox(item.name, currentBox);
        checkBox->setChecked(false);
        centerBodyWidget.append(std::make_tuple(checkBox, item));
        checkBoxLayout->addWidget(checkBox, row, col);
        ++col;
        if (col >= 4)
        {
            ++row;
            col = 0;
        }
    }

    auto buttonLayout = new QHBoxLayout;
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(15);

    allSelect = new QPushButton("全选", currentBox);
    unSelect = new QPushButton("全不选", currentBox);
    conditionsOnBtn = new QPushButton("一键加电", currentBox);
    conditionsOffBtn = new QPushButton("一键去电", currentBox);

    buttonLayout->addStretch();
    buttonLayout->addWidget(allSelect);
    buttonLayout->addWidget(unSelect);
    buttonLayout->addWidget(conditionsOnBtn);
    buttonLayout->addWidget(conditionsOffBtn);

    mainLayout->addLayout(checkBoxLayout);
    mainLayout->addLayout(buttonLayout);

    currentBox->setLayout(mainLayout);

    return currentBox;
}
QUrl AddRemovePowerImpl::createPlugOnUrl(const PlugInfo& plugInfo)
{
    return QString("http://%1:%2/setswitch?a=%3&b=1&c=0").arg(plugInfo.host).arg(plugInfo.port).arg(plugInfo.no);
}
QUrl AddRemovePowerImpl::createPlugOffUrl(const PlugInfo& plugInfo)
{
    return QString("http://%1:%2/setswitch?a=%3&b=0&c=0").arg(plugInfo.host).arg(plugInfo.port).arg(plugInfo.no);
}
QUrl AddRemovePowerImpl::createPlugStatusUrl(const PlugInfo& plugInfo)
{
    return QString("http://%1:%2/getswitch?a=0&").arg(plugInfo.host).arg(plugInfo.port);
}

QUrl AddRemovePowerImpl::createDeviceOffUrl(const PlugDeviceInfo& info) { return QString("http://%1:%2/shutdown").arg(info.host).arg(info.port); }
void AddRemovePowerImpl::sendRequest(const QUrl& url)
{
    if (networkManager.networkAccessible() == QNetworkAccessManager::NotAccessible)
        networkManager.setNetworkAccessible(QNetworkAccessManager::Accessible);

    QNetworkRequest request(url);
    networkManager.get(request);
}
void AddRemovePowerImpl::reset(OperationStatus stat)
{
    status = stat;
    devShutdownCount = 0;
    plugCount = 0;
    curOperationPlugList.clear();  /* 排插接口的信息 */
    curShutdownDeviceList.clear(); /* 排插设备的信息 */
    curPlugKeyList.clear();
    queryResult = false;
    queryCount = 0;
    waitCount = 0;
    logWidget->clear();
    powerFlow = PowerFlow::ShutdownDev;
}

bool AddRemovePowerImpl::assemblingCmd(const CenterBodyInfo& info, int value, QString& json)
{
    DeviceID deviceID(info.devID);
    int modeID = 65535;

    CmdRequest cmdRequest;
    cmdRequest.m_systemNumb = deviceID.sysID;
    cmdRequest.m_deviceNumb = deviceID.devID;
    cmdRequest.m_extenNumb = deviceID.extenID;

    cmdRequest.m_modeID = modeID;
    cmdRequest.m_cmdID = info.cmdID;
    auto cmdAttr = GlobalData::getCmd(deviceID, modeID, info.cmdID);
    if (!cmdAttr.isValid())
    {
        writeLog(QString("未找到%1设备的命令").arg(info.name));
        return false;
    }
    cmdRequest.m_paramdataMap[info.key] = value;

    JsonWriter writer;
    writer& cmdRequest;

    json = writer.GetString();

    return true;
}

bool AddRemovePowerImpl::assemblingUnitCmd(const CenterBodyInfo& info, int value, QString& json)
{
    DeviceID deviceID(info.devID);
    int modeID = 65535;

    UnitParamRequest unitParamRequest;
    unitParamRequest.m_systemNumb = deviceID.sysID;
    unitParamRequest.m_deviceNumb = deviceID.devID;
    unitParamRequest.m_extenNumb = deviceID.extenID;

    unitParamRequest.m_modeID = modeID;
    unitParamRequest.m_unitID = info.cmdID;
    //目前没有哪个设备的单元ID从0开始
    if (unitParamRequest.m_unitID <= 0)
    {
        writeLog(QString("未找到%1设备的单元命令").arg(info.name));
        return false;
    }
    unitParamRequest.m_paramdataMap[info.key] = value;

    JsonWriter writer;
    writer& unitParamRequest;

    json = writer.GetString();

    return true;
}

/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/
AddRemovePower::AddRemovePower(cppmicroservices::BundleContext context, QWidget* parent)
    : QWidget(parent)
    , m_impl(new AddRemovePowerImpl)
{
    m_impl->context = context;

    QList<CenterBodyInfo> centerBody;
    QString path = PlatformConfigTools::configBusiness("AddAndRemovePowerConfig/PowerConfig.xml");
    AddRemovePowerImpl::parseXml(path, m_impl->powerStripMap, centerBody, m_impl->shp);

    connect(&(m_impl->networkManager), &QNetworkAccessManager::finished, this, &AddRemovePower::plugStatus);

    initUI(centerBody);
}

AddRemovePower::~AddRemovePower() { delete m_impl; }

void AddRemovePower::start()
{
    //    auto packMgr = getPacketManagerInterface();
    //    if (packMgr != nullptr)
    //    {
    //        //设置参数控制命令
    //        connect(this, &AddRemovePower::sendMessage, packMgr, &IPacketManager::signal_packCmdPackMessage);
    //    }
}
void AddRemovePower::initUI(const QList<CenterBodyInfo>& centerBody)
{
    //初始化qss
    QString qssPath = QApplication::applicationDirPath() + "/resources/newqss/common.qss";
    QssCommonHelper::setWidgetStyle(this, qssPath);

    QPushButton* conditionsOnBtn = nullptr;
    QPushButton* conditionsOffBtn = nullptr;
    QPushButton* allSelect = nullptr;
    QPushButton* unSelect = nullptr;

    auto mainLayout = new QGridLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(15);

    /* 日志窗体 */
    m_impl->logWidget = new QTextEdit(this);
    m_impl->logWidget->setReadOnly(true);
    m_impl->logWidget->setMinimumHeight(200);
    mainLayout->addWidget(m_impl->logWidget, 0, 0, 1, 2);

    /* 排插设备 */
    int row = 1;
    int col = 0;
    for (auto iter = m_impl->powerStripMap.begin(); iter != m_impl->powerStripMap.end(); ++iter)
    {
        auto& item = iter.value();
        auto tempGroup = AddRemovePowerImpl::createItemGroup(item.name, conditionsOnBtn, conditionsOffBtn);
        connect(conditionsOnBtn, &QAbstractButton::clicked, [=]() { itemPlugConditionsOn(iter.key()); });
        connect(conditionsOffBtn, &QAbstractButton::clicked, [=]() { itemPlugConditionsOff(iter.key()); });

        mainLayout->addWidget(tempGroup, row, col, 1, 1);
        row += col;
        ++col;
        if (col >= 2)
        {
            col = 0;
        }
    }
    /* 功放机柜 */
    row = m_impl->powerStripMap.size() % 2 == 0 ? row : row + 1;
    auto tempGroup = AddRemovePowerImpl::createItemGroup("S功放机柜", conditionsOnBtn, conditionsOffBtn);
    if (tempGroup == nullptr)
    {
        return;
    }
    mainLayout->addWidget(tempGroup, row, 0, 1, 1);
    /* 1加电 2去电 */
    connect(conditionsOnBtn, &QAbstractButton::clicked, [=]() { shpOnOff(1); });
    connect(conditionsOffBtn, &QAbstractButton::clicked, [=]() { shpOnOff(2); });

    /* 中心体设备 */
    ++row;
    tempGroup = m_impl->createItemGroup(centerBody, allSelect, unSelect, conditionsOnBtn, conditionsOffBtn);
    if (tempGroup == nullptr)
    {
        return;
    }

    mainLayout->addWidget(tempGroup, row, 0, 1, 2);
    connect(conditionsOnBtn, &QAbstractButton::clicked, this, &AddRemovePower::allCenterOn);
    connect(conditionsOffBtn, &QAbstractButton::clicked, this, &AddRemovePower::allCenterOff);
    connect(allSelect, &QAbstractButton::clicked, [=]() {
        /* 全选 */
        for (auto& item : m_impl->centerBodyWidget)
        {
            std::get<0>(item)->setChecked(true);
        }
    });
    connect(unSelect, &QAbstractButton::clicked, [=]() {
        /* 反选 */
        for (auto& item : m_impl->centerBodyWidget)
        {
            std::get<0>(item)->setChecked(false);
        }
    });
}

/* 单个插排 */
void AddRemovePower::itemPlugConditionsOn(const QString& key)
{
    if (m_impl->timerID != -1)
    {
        QMessageBox::information(this, "提示", "当前操作未执行完成");
        return;
    }
    auto find = m_impl->powerStripMap.find(key);
    if (find == m_impl->powerStripMap.end())
    {
        QMessageBox::information(this, "配置文件错误", "未找到当前的控制插排");
        return;
    }
    m_impl->reset(OperationStatus::On);
    m_impl->curOperationPlugList = find->plugList;
    m_impl->curPlugKeyList.append(key);
    m_impl->powerFlow = PowerFlow::OnPlug;
    m_impl->timerID = startTimer(timerS);
}
void AddRemovePower::itemPlugConditionsOff(const QString& key)
{
    if (m_impl->timerID != -1)
    {
        QMessageBox::information(this, "提示", "当前操作未执行完成");
        return;
    }
    auto find = m_impl->powerStripMap.find(key);
    if (find == m_impl->powerStripMap.end())
    {
        QMessageBox::information(this, "配置文件错误", "未找到当前的控制插排");
        return;
    }
    m_impl->reset(OperationStatus::Off);
    m_impl->curOperationPlugList = find->plugList;
    m_impl->curShutdownDeviceList = find->deviceList;
    m_impl->curPlugKeyList.append(key);
    m_impl->powerFlow = PowerFlow::ShutdownDev;
    m_impl->timerID = startTimer(timerS);
}
void AddRemovePower::allCenterOn()
{
    for (auto& item : m_impl->centerBodyWidget)
    {
        /* 未选中跳过 */
        if (!std::get<0>(item)->isChecked())
        {
            continue;
        }
        auto& itemInfo = std::get<1>(item);
        if (itemInfo.type == "process")
        {
            QString json;
            if (m_impl->assemblingCmd(itemInfo, itemInfo.on, json))
            {
                emit signalsCmdDeviceJson(json);
            }
        }
        else if (itemInfo.type == "unit")
        {
            QString json;
            if (m_impl->assemblingUnitCmd(itemInfo, itemInfo.on, json))
            {
                emit signalsUnitDeviceJson(json);
            }
        }
    }
    m_impl->writeLog("中心体加电指令下发完成");
}
void AddRemovePower::allCenterOff()
{
    for (auto& item : m_impl->centerBodyWidget)
    {
        /* 未选中跳过 */
        if (!std::get<0>(item)->isChecked())
        {
            continue;
        }
        auto& itemInfo = std::get<1>(item);
        if (itemInfo.type == "process")
        {
            QString json;
            if (m_impl->assemblingCmd(itemInfo, itemInfo.off, json))
            {
                emit signalsCmdDeviceJson(json);
            }
        }
        else if (itemInfo.type == "unit")
        {
            QString json;
            if (m_impl->assemblingUnitCmd(itemInfo, itemInfo.off, json))
            {
                emit signalsUnitDeviceJson(json);
            }
        }
    }
    m_impl->writeLog("中心体去电指令下发完成");
}
/* 本来是应答的,改为状态查询 */
void AddRemovePower::plugStatus(QNetworkReply* reply)
{
    AutoCloseReply helper(reply);
    if (reply->error() != QNetworkReply::NoError)
    {
        return;
    }
    /* 这里直接忽略所有的控制命令,只管状态查询 */

    auto host = reply->url().host();
    /* 插板状态查询是115字节 */
    auto data = reply->readAll();
    if (data.length() != 115)
    {
        return;
    }

    m_impl->queryResult = true;
    m_impl->queryCount = 0;

    auto& plugStatusValue = (m_impl->plugStatusMap)[host];
    auto find = m_impl->powerStripMap.find(host);
    if (find == m_impl->powerStripMap.end())
    {
        m_impl->writeLog(QString("未查找到当前的设备:%1").arg(host));
        return;
    }
    QString dataStr = data.mid(11, 32);
    auto statusList = dataStr.split("?", QString::SkipEmptyParts);

    int len = find->plugList.size() > statusList.size() ? statusList.size() : find->plugList.size();
    /*
     * 1代表关
     * 2代表开
     */
    int onCount = 0;
    int offCount = 0;
    int total = 0;
    for (int i = 0; i < len; ++i)
    {
        auto& statusValue = statusList.at(i);
        auto& plugInfo = find->plugList.at(i);
        if (!plugInfo.isEnable)
        {
            continue;
        }
        ++total;
        if (statusValue == "1")
            ++offCount;
        else if (statusValue == "2")
            ++onCount;
    }
    plugStatusValue = (onCount == total ? PlugStatus::AllOn : (offCount == total ? PlugStatus::AllOff : PlugStatus::Part));
}

void AddRemovePower::offDev()
{
    /* 列表为空 */
    if (m_impl->curShutdownDeviceList.isEmpty())
    {
        m_impl->powerFlow = PowerFlow::ShutdownPlug;
        return;
    }
    /* 发送关机指令 */
    for (auto& item : m_impl->curShutdownDeviceList)
    {
        auto url = AddRemovePowerImpl::createDeviceOffUrl(item);
        m_impl->sendRequest(url);
    }

    /* 计数器 */
    ++(m_impl->devShutdownCount);
    if (m_impl->devShutdownCount >= 2)
    {
        m_impl->powerFlow = PowerFlow::ShutdownPlug;
        return;
    }
}
void AddRemovePower::queryPlug()
{
    /* 发送排插状态指令 */
    for (auto& item : m_impl->curOperationPlugList)
    {
        auto url = AddRemovePowerImpl::createPlugStatusUrl(item);
        m_impl->sendRequest(url);
    }
}
void AddRemovePower::offPlug() { m_impl->operationPlug(AddRemovePowerImpl::createPlugOffUrl); }
void AddRemovePower::onPlug() { m_impl->operationPlug(AddRemovePowerImpl::createPlugOnUrl); }

static QString initSocket(QUdpSocket& socket, int recvPort)
{
    static constexpr int bufferSize = 128 * 1024 * 1024;

    //增大发送和接收缓存区大小
    socket.setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, bufferSize);
    socket.setSocketOption(QAbstractSocket::SendBufferSizeSocketOption, bufferSize);
    /* 禁用自回环 */
    socket.setSocketOption(QAbstractSocket::MulticastLoopbackOption, false);
    /* 通过UDP通讯时要过两个交换机，如果你不进行TTL设置，在第二台交换机就已经收不到你的UDP报文，导致你发的报文中途丢掉了
     * 所以要进行TTL设置(就像游戏中你有255条命一样)
     */
    socket.setSocketOption(QAbstractSocket::MulticastTtlOption, 0XFF);
    // QHostAddress(m_impl->desIP)
    auto result = socket.bind(QHostAddress::AnyIPv4, recvPort, QAbstractSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    if (!result)
    {
        return QString("功放控制 失败: %1").arg(socket.errorString());
    }

    // socket.waitForConnected();
    return QString();
}

void AddRemovePower::shpOnOff(int value)
{
    /* 1加电 2去电 */
    QByteArray data(32, 0);
    data[8] = 0x02;
    data[9] = 0xF0;
    data[24] = 0x04;
    data[28] = 0x01;
    data[29] = 0xFF;
    data[30] = 0xFF;
    data[31] = static_cast<qint8>(value);

    QUdpSocket socket;
    auto str = initSocket(socket, m_impl->shp.recvPort);
    if (!str.isEmpty())
    {
        qWarning() << str;
        return;
    }
    socket.writeDatagram(data, QHostAddress(m_impl->shp.ip), m_impl->shp.sendPort);
}

void AddRemovePower::timerEvent(QTimerEvent* /*event*/)
{
    /*
     * 定时器启动
     * 1. 首先将curShutdownMap中的数据依次取出, 向设备发送关机命令, 关机命令发送两次
     * 2. 向插排发送关闭电源指令
     * 3. 查询插排的电源状态
     * 4. 如果查询到有插排没有关闭则重复2,3 3此之后 报警告
     */
    ++(m_impl->queryCount);
    if (m_impl->queryCount > 3)
    {
        m_impl->queryResult = false;
        m_impl->powerFlow = PowerFlow::Exit;
        m_impl->writeLog("操作超时");
    }
    queryPlug();
    switch (m_impl->powerFlow)
    {
    case PowerFlow::Exit:
    {
        break;
    }
    case PowerFlow::ShutdownDev:
    {
        m_impl->writeLog("发送关闭设备指令");
        offDev();
        return;
    }
    case PowerFlow::ShutdownPlug:
    {
        m_impl->writeLog("发送关闭排插指令");
        offPlug();
        return;
    }
    case PowerFlow::Check:
    {
        ++(m_impl->plugCount);
        if (m_impl->plugCount > 3)
        {
            m_impl->writeLog("操作执行超时");
            break;
        }
        if (!m_impl->queryResult)
        {
            m_impl->powerFlow = m_impl->status == OperationStatus::Off ? PowerFlow::ShutdownPlug : PowerFlow::OnPlug;
            return;
        }
        bool isOK = true;
        for (auto& key : m_impl->curPlugKeyList)
        {
            auto& value = (m_impl->plugStatusMap)[key];

            if (m_impl->status == OperationStatus::Off)
            {
                isOK &= (value == PlugStatus::AllOff);
            }
            else
            {
                isOK &= (value == PlugStatus::AllOn);
            }
        }

        if (!isOK)
        {
            m_impl->powerFlow = m_impl->status == OperationStatus::Off ? PowerFlow::ShutdownPlug : PowerFlow::OnPlug;
            return;
        }

        /* 全部关电或者全部开电了 */
        m_impl->writeLog("操作完成");
        break;
    }
    case PowerFlow::OnPlug:
    {
        m_impl->writeLog("发送排插开启指令");
        onPlug();
        return;
    }
    case PowerFlow::Wait:
    {
        m_impl->writeLog("等待控制执行");
        ++(m_impl->waitCount);
        if (m_impl->waitCount >= 3)
        {
            m_impl->waitCount = 0;
            m_impl->powerFlow = PowerFlow::Check;
        }
        return;
    }
    }

    killTimer(m_impl->timerID);
    m_impl->timerID = -1;
}
