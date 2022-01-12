#include "StationNetCenterCmdTest.h"
#include "CConverter.h"
#include "NetInterfaceHelper.h"
#include "PDXPHeader.h"
#include "PDXPMessageDefine.h"
#include "PackPDXPHeader.h"
#include "ui_StationNetCenterCmdTest.h"
#include <QDate>
#include <QThread>
#include <QTime>
#include <QUdpSocket>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 8, 0))
#include <QNetworkDatagram>
#endif

class StationNetCenterCmdTestImpl
{
public:
    QUdpSocket* recvSocket{ nullptr };
    static uint getAccumDate(const QDate& date)
    {
        QDate startDate(2000, 1, 1);
        return startDate.daysTo(date) + 1;
    }
    static void createPack(quint8 paUI, quint8 jgUI, QByteArray& msg);
    static QString initSocket(QUdpSocket& socket, const QString& remote, const QString& local, int recvPort);
    void closeConnect();
};
QString StationNetCenterCmdTestImpl::initSocket(QUdpSocket& socket, const QString& remote, const QString& local, int recvPort)
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
        return QString("连接中心 失败: %1").arg(socket.errorString());
    }

    auto netInterfaces = NetInterfaceHelper::instance().ipMapToNetInterfaceList(local);

    for (auto& netInterface : netInterfaces)
    {
        result = socket.joinMulticastGroup(QHostAddress(remote), netInterface);
        if (!result)
        {
            return QString("Udp(组播) 加入失败 错误信息: %2").arg(socket.errorString());
        }
    }
    // socket.waitForConnected();
    return QString();
}

void StationNetCenterCmdTestImpl::createPack(quint8 paUI, quint8 jgUI, QByteArray& msg)
{
    PDXPHeader sendHeader;
    sendHeader.version = 0x1;
    sendHeader.mid = 0xEDCA;
    sendHeader.sid = 0xFEDCBA;
    sendHeader.did = 0x66041800;
    sendHeader.bid = 0x60261000;
    sendHeader.no = 0;
    sendHeader.flag = 0;
    sendHeader.reserve = 0;
    sendHeader.date = getAccumDate(QDate::currentDate());
    sendHeader.time = QTime::currentTime().msecsSinceStartOfDay() * 10;

    msg = PackPDXPHeader::packHead(sendHeader);
    quint8 v{ 0x02 };   // 版本号
    quint8 st{ 0x01 };  // 执行方式
    quint8 pa = paUI;   // 状态参数
    quint8 jg = jgUI;   // 执行结果
    quint32 res{ 0 };   // 保留
    sendHeader.len = sizeof(v) + sizeof(st) + sizeof(pa) + sizeof(jg) + sizeof(res);
    msg.append(CConverter::toByteArray<ushort>(sendHeader.len));

    msg.append(CConverter::toByteArray<quint8>(v));
    msg.append(CConverter::toByteArray<quint8>(st));
    msg.append(CConverter::toByteArray<quint8>(pa));
    msg.append(CConverter::toByteArray<quint8>(jg));
    msg.append(CConverter::toByteArray<quint32>(res));
}

StationNetCenterCmdTest::StationNetCenterCmdTest(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::StationNetCenterCmdTest)
    , m_impl(new StationNetCenterCmdTestImpl)
{
    ui->setupUi(this);
    connect(ui->send, &QPushButton::clicked, this, &StationNetCenterCmdTest::send);
    connect(ui->clean, &QPushButton::clicked, ui->log, &QTextEdit::clear);
    connect(ui->reset, &QPushButton::clicked, this, &StationNetCenterCmdTest::resetConnect);
}

StationNetCenterCmdTest::~StationNetCenterCmdTest()
{
    delete ui;
    delete m_impl;
}
void StationNetCenterCmdTest::send()
{
    QUdpSocket socket;
    auto res = StationNetCenterCmdTestImpl::initSocket(socket, ui->sendIP->text(), ui->localIP->text(), ui->sendPort->value());
    if (!res.isEmpty())
    {
        log(res);
        return;
    }
    QByteArray sendMsg;
    StationNetCenterCmdTestImpl::createPack(ui->pa->value(), ui->jg->value(), sendMsg);
    /* 连续发3次 */
    bool sendRet = true;
    sendRet = (0 < socket.writeDatagram(sendMsg, QHostAddress(ui->sendIP->text()), ui->sendPort->value()));
    sendRet &= (0 < socket.writeDatagram(sendMsg, QHostAddress(ui->sendIP->text()), ui->sendPort->value()));
    sendRet &= (0 < socket.writeDatagram(sendMsg, QHostAddress(ui->sendIP->text()), ui->sendPort->value()));
    socket.waitForBytesWritten();

    if (!sendRet)
    {
        log(QString("发送失败:%1").arg(socket.errorString()));
        return;
    }
    log("发送成功");
}

void StationNetCenterCmdTest::log(const QString& msg) { ui->log->append(msg); }

void StationNetCenterCmdTest::resetConnect()
{
    if (m_impl->recvSocket != nullptr)
    {
        m_impl->recvSocket->close();
        delete m_impl->recvSocket;
        m_impl->recvSocket = nullptr;
    }
    auto socket = new QUdpSocket;
    GetValueHelper<QUdpSocket> getValueHelper(socket);
    auto res = StationNetCenterCmdTestImpl::initSocket(*socket, ui->recvIP->text(), ui->localIP->text(), ui->recvPort->value());
    if (!res.isEmpty())
    {
        log(res);
        return;
    }
    // socket->open(QAbstractSocket::ReadWrite);
    m_impl->recvSocket = getValueHelper.get();
    connect(m_impl->recvSocket, &QUdpSocket::readyRead, this, &StationNetCenterCmdTest::onReadyRead);
}
void StationNetCenterCmdTest::onReadyRead()
{
    /* 等待数据 */
    if (!m_impl->recvSocket->hasPendingDatagrams())
    {
        return;
    }
#if (QT_VERSION >= QT_VERSION_CHECK(5, 8, 0))
    auto datagram = m_impl->recvSocket->receiveDatagram();
    auto data = datagram.data();
#else
    auto count = m_impl->recvSocket->pendingDatagramSize();
    QByteArray data;
    data.resize(count);
    m_impl->recvSocket->readDatagram(data.data(), count);
#endif
    /* 收到ACK */
    parseData(data);
}

bool StationNetCenterCmdTest::parseData(const QByteArray& data)
{
    log(data.toHex());
    /* 解包 */
    auto headerResult = PackPDXPHeader::unpackHead(data);
    if (!headerResult)
    {
        log("解包失败");
        log(headerResult.msg());
        return false;
    }

    auto header = headerResult.value();
    auto type = PDXPMESSAGETYPE(header.bid);
    auto realData = data.mid(PDXPHeaderLength::value);
    switch (type)
    {
    case ZWZX_YCKZMLYD_TYPE:  // 站网中心远程控制命令应答
    {
        log("站网中心远程控制命令应答");
        if (realData.size() < 5)
        {
            log("数据包长度不符");
            return false;
        }
        log(QString("信息类型：0x%1").arg(QString(realData.mid(0, 4).toHex())));
        auto res = realData.mid(4, 1).toInt();
        if (res == 0x33)
        {
            log("控制命令应答:错误");
            return false;
        }
        log("控制命令应答:正确");
        return true;
    }
    case ZWZX_YCKZMLZXJG_TYPE:  // 站网中心远程控制命令执行结果
    {
        log("站网中心远程控制命令执行结果");
        if (realData.size() < 11)
        {
            log("数据包长度不符");
            return false;
        }
        log(QString("设备UAC：0x%1").arg(QString(realData.mid(0, 4).toHex())));
        log(QString("时间:%1").arg(realData.mid(4, 4).toUInt() / 10));
        log(QString("命令状态参数:0x%1").arg(QString(realData.mid(8, 1).toHex())));
        log(QString("命令要求执行结果:0x%1").arg(QString(realData.mid(9, 1).toHex())));
        log(QString("命令实际执行结果:0x%1").arg(QString(realData.mid(10, 1).toHex())));
        return false;
    }
    default: break;
    }

    log("未知指令");
    return false;
}
