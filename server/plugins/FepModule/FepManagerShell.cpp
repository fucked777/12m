#include "FepManagerShell.h"
#include "CConverter.h"
#include "CppMicroServicesUtility.h"
#include "FepSendUnit.h"
#include "FepServer.h"
#include "GlobalData.h"
#include "MessagePublish.h"
#include "PlatformInterface.h"
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QDomDocument>
#include <QSettings>

FepManagerShell::FepManagerShell(cppmicroservices::BundleContext context)
    : INetMsg()
    , m_fepServer{ new FepServer }
    , m_fepSend(new FepSendUnit)
{
    qRegisterMetaType<QMap<QString, QVariantMap>>("QMap<QString,QVariantMap>");
    qRegisterMetaType<QMap<QString, QVariantMap>>("const QMap<QString,QVariantMap>&");
    qRegisterMetaType<QMap<QString, QVariantMap>>("QMap<QString,QVariantMap>&");
    // connect(this, &FepManagerShell::signalRecvData, this, &FepManagerShell::slotRecvData);
    init();
}

FepManagerShell::~FepManagerShell()
{
    m_fepServer->stop();
    m_fepServer->setParent(nullptr);
    delete m_fepServer;
}
QMap<QString, QVariantMap> FepManagerShell::getCurIPCheckData(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return {};
    }
    QDomDocument doc;
    QString errorMsg;
    int errorLine{ 0 };
    int errorColumn{ 0 };
    if (!doc.setContent(&file, &errorMsg, &errorLine, &errorColumn))
    {
        auto errMsg = QString("Fep IP检查配置文件出错: %1:第%2行,第%3列").arg(errorMsg).arg(errorLine).arg(errorColumn);
        qWarning() << errMsg;
        return {};
    }

    QMap<QString, QVariantMap> resultData;
    auto root = doc.documentElement();
    auto node = root.firstChild();
    while (!node.isNull())
    {
        auto isComment = node.isComment();
        auto element = node.toElement();
        auto name = node.nodeName();
        node = node.nextSibling();
        /* 跳过注释 */
        if (isComment || name != "item")
        {
            continue;
        }

        QVariantMap tempData;
        auto extendNode = element.firstChild();
        while (!extendNode.isNull())
        {
            auto isComment = extendNode.isComment();
            auto configElement = extendNode.toElement();
            auto customKey = extendNode.nodeName();
            extendNode = extendNode.nextSibling();

            /* 跳过注释 */
            if (isComment)
            {
                continue;
            }

            tempData[customKey] = configElement.text().trimmed();
        }

        auto key = element.attribute("ip").trimmed();
        resultData[key] = tempData;
    }

    return resultData;
}
void FepManagerShell::init()
{
    QString dbConnnectPath = PlatformConfigTools::configBusiness("FepConfig/FepConfig.ini");
    QSettings setting(dbConnnectPath, QSettings::IniFormat);

    auto port = static_cast<quint16>(setting.value("FepConfig/port", 5555).toUInt());
    auto recvpath = PlatformConfigTools::configBusiness(setting.value("FepConfig/recvpath", "FepRecv").toString());
    recvpath = QDir(recvpath).absolutePath();

    auto despath = PlatformConfigTools::configBusiness(setting.value("FepConfig/despath", "DesRecv").toString());
    despath = QDir(despath).absolutePath();
    m_fepServer->setWorkDir(recvpath, despath);
    m_fepServer->setPort(port);

    /* 20210818 ip检查
     * 因为这个玩意儿不是必须的所以
     * 这里没有强制检查正确与否
     */
    auto ipcheckData = getCurIPCheckData(PlatformConfigTools::configBusiness("FepConfig/FepIPCheck.xml"));
    m_fepServer->setIPCheck(ipcheckData);

    connect(m_fepServer, &FepServer::sg_oneFileRecvSuccess, this, &FepManagerShell::slot_oneFileRecvSuccess);
    QString errMsg;
    if (!m_fepServer->start(errMsg))
    {
        qDebug() << errMsg;
    }
}

void FepManagerShell::sendInfoRecv(const QString& filename) {}

void FepManagerShell::getPlan(QString workDir)
{
    if (workDir.isEmpty())
    {
        // RedisHelper::getInstance().publistSystemLogError("无法生成索取的计划文件");
        return;
    }
    QString dbConnnectPath = QApplication::applicationDirPath() + "/resources/config/FepConfig.ini";
    QSettings setting(dbConnnectPath, QSettings::IniFormat);

    auto addr = setting.value("FepConfig/sendaddr", "192.20.1.156").toString();
    auto file = setting.value("FepConfig/file", false).toBool();
    auto port = static_cast<quint16>(setting.value("FepConfig/sendport", 5556).toUInt());
    //    auto workDir = setting.value("FepConfig/getplanpath", "FepSend").toString();
    m_fepSend->setServerAddr(addr, port);

    /* 发送文件 */
    if (file)
    {
        QFileInfo info(workDir);
        QString errMsg;
        if (!m_fepSend->startSendFile(workDir, errMsg))
        {
            // QMessageBox::information(this, "发送错误", errMsg);
        }
        return;
    }

    QString errMsg;
    if (!m_fepSend->startSendDir(workDir, errMsg))
    {
        // QMessageBox::information(this, "发送错误", errMsg);
        return;
    }
}

void FepManagerShell::slotRecvData(const QByteArray& bArrayData) { getPlan(initGetPlanFile()); }

void FepManagerShell::slot_oneFileRecvSuccess(const QVariantMap& recvMap)
{
    auto ip = recvMap.value("ip").toString();
    auto port = recvMap.value("port").toUInt();
    auto path = recvMap.value("path").toString();
}

void FepManagerShell::initRecvFile() {}

QString FepManagerShell::initGetPlanFile()
{
    QDomDocument doc;
    //元素创建
    QDomProcessingInstruction instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\"");
    QDomElement root = doc.createElement("计划索取指令");
    QDomElement time = doc.createElement("时间");
    QDomElement applyNum = doc.createElement("申请流水号");
    QDomElement deviceCode = doc.createElement("设备代号");
    QDomElement taskType = doc.createElement("计划类型");

    QDateTime currentDateTime = GlobalData::currentDateTime();
    QString timeStr = currentDateTime.toString("yyyyMMddhhmmss");

    QString fileName = QString("04_") + QString("JHSQZL") + currentDateTime.toString("_ByyyyMMdd_hhmmss_%1").arg(number, 4, 10, QLatin1Char('0'));
    QString appleID = currentDateTime.toString("yyyyMMdd%1").arg(number, 4, 10, QLatin1Char('0'));
    number++;
    QDomText timeText = doc.createTextNode(timeStr);
    time.appendChild(timeText);
    QDomText applyNumText = doc.createTextNode(appleID);
    applyNum.appendChild(applyNumText);
    QDomText deviceCodeText = doc.createTextNode("TK-4413");
    deviceCode.appendChild(deviceCodeText);
    QDomText taskTypeText = doc.createTextNode("1");
    taskType.appendChild(taskTypeText);

    root.appendChild(time);
    root.appendChild(applyNum);
    root.appendChild(deviceCode);
    root.appendChild(taskType);
    doc.appendChild(root);

    //保存为xml文件
    QString pwd = QApplication::applicationDirPath();
    QString default_out = pwd + QString("/System/TaskCache/ftpfile/");
    QString filePath = QString("%1/%2").arg(default_out).arg(fileName);  //构建返回xml
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    {
        qWarning() << QString("%1 open(%2) fail").arg(__FUNCTION__).arg(default_out);
        return "";
    }
    QTextStream out(&file);
    out.setCodec("UTF-8");
    doc.save(out, 4, QDomNode::EncodingFromTextStream);
    file.close();
    return filePath;
}
