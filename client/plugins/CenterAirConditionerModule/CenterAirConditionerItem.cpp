#include "CenterAirConditionerItem.h"
#include "Endian.h"
#include "GlobalData.h"
#include "ProtocolXmlTypeHelper.h"
#include "QssCommonHelper.h"
#include "Utility.h"
#include "ui_CenterAirConditionerItem.h"
#include <QCloseEvent>
#include <QThread>
#include <QUdpSocket>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 8, 0))
#include <QNetworkDatagram>
#endif

class CenterAirConditionerItemImpl
{
public:
    int timerID{ -1 };
    QString desIP{ "0.0.0.0" };
    QString localIP{ "0.0.0.0" };
    quint16 sendPort{ 65534 };
    quint16 recvPort{ 65533 };
    QUdpSocket* socket{ nullptr };
    QByteArray statusQuery; /* 状态查询指令 */
    QByteArray recvData;
    CenterAirConditionerItemImpl()
    {
        statusQuery.append(char(0x40));
        statusQuery.append(char(0x04));
        statusQuery.append(char(0x00));
        statusQuery.append(char(0x00));
        statusQuery.append(char(0x00));
        statusQuery.append(char(0x0e));
        addCRC(statusQuery);
    }

public:
    static void addCRC(QByteArray& data);
    static quint16 calcCRC(const QByteArray& data, int len);
    static bool checkCRC(const QByteArray& data);
    static void bgRedOrGreen(QWidget* widget, bool red);
    static void bgReset(QLabel* widget, bool changeText);
};
quint16 CenterAirConditionerItemImpl::calcCRC(const QByteArray& data, int len)
{
    quint16 crc = 0xFFFF;
    if (data.size() < len)
    {
        return crc;
    }
    for (int i = 0; i < len; ++i)
    {
        crc ^= static_cast<quint8>(data.at(i));
        for (int i = 0; i < 8; ++i)
        {
            if ((crc & 0x01) == 0x01)
            {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }
    return crc;
}
void CenterAirConditionerItemImpl::addCRC(QByteArray& data)
{
    auto crc = calcCRC(data, data.size());
    /* 空调协议CRC校验码传输格式是先传低8位,后传高8位。
     * 例：传送0xAA55 先传0x55 再传0xAA
     */
#if !IS_LITTLEENDIAN
    crc = Endian::bSwap<quint16>(crc);
#endif
    auto item = QByteArray(reinterpret_cast<char*>(&crc), sizeof(crc));
    data.append(item);
}
bool CenterAirConditionerItemImpl::checkCRC(const QByteArray& data)
{
    auto crc = calcCRC(data, data.size() - 2);
    auto crcByte = data.right(2);

    auto crcCheck = *(static_cast<quint16*>(static_cast<void*>(crcByte.data())));
#if !IS_LITTLEENDIAN
    crcCheck = Endian::bSwap<quint16>(crcCheck);
#endif

    return crc == crcCheck;
}
void CenterAirConditionerItemImpl::bgRedOrGreen(QWidget* widget, bool red)
{
    widget->setAutoFillBackground(true);
    auto palette = widget->palette();
    palette.setColor(QPalette::Background, red ? Qt::red : Qt::green);
    widget->setPalette(palette);
}
void CenterAirConditionerItemImpl::bgReset(QLabel* widget, bool changeText)
{
    widget->setAutoFillBackground(false);
    auto palette = widget->palette();
    palette.setColor(QPalette::Background, Qt::white);
    widget->setPalette(palette);
    if (changeText)
    {
        widget->setText("-");
    }
}
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/

CenterAirConditionerItem::CenterAirConditionerItem(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::CenterAirConditionerItem)
    , m_impl(new CenterAirConditionerItemImpl)
{
    ui->setupUi(this);
    //    QString qssPath = QApplication::applicationDirPath() + "/resources/newqss/common.qss";
    //    QssCommonHelper::setWidgetStyle(this, qssPath);
    reset();

    connect(ui->kj, &QPushButton::clicked, this, [=]() { setControlCmd(0x00, 0x00, 0x00, 0xAA); });
    connect(ui->gj, &QPushButton::clicked, this, [=]() { setControlCmd(0x00, 0x00, 0x00, 0x55); });
    connect(ui->sf, &QPushButton::clicked, this, [=]() { setControlCmd(0x00, 0x01, 0x00, 0x01); });
    connect(ui->zl, &QPushButton::clicked, this, [=]() { setControlCmd(0x00, 0x01, 0x00, 0x02); });
    connect(ui->zr, &QPushButton::clicked, this, [=]() { setControlCmd(0x00, 0x01, 0x00, 0x05); });
    connect(ui->cs, &QPushButton::clicked, this, [=]() { setControlCmd(0x00, 0x01, 0x00, 0x06); });

    connect(ui->zdong, &QPushButton::clicked, this, [=]() { setControlCmd(0x00, 0x02, 0x00, 0x01); });
    connect(ui->ddang, &QPushButton::clicked, this, [=]() { setControlCmd(0x00, 0x02, 0x00, 0x02); });
    connect(ui->zdang, &QPushButton::clicked, this, [=]() { setControlCmd(0x00, 0x02, 0x00, 0x03); });
    connect(ui->gdang, &QPushButton::clicked, this, [=]() { setControlCmd(0x00, 0x02, 0x00, 0x04); });

    connect(ui->szwd, &QPushButton::clicked, this, [=]() { setControlCmd(0x00, 0x03, 0x00, static_cast<quint8>(ui->wdz->value())); });
}
void CenterAirConditionerItem::setAddr(const QString& localAddr, const QString& desAddr, quint16 recvPort, quint16 sendPort)
{
    m_impl->desIP = desAddr;
    m_impl->localIP = localAddr;
    m_impl->recvPort = recvPort;
    m_impl->sendPort = sendPort;
}
CenterAirConditionerItem::~CenterAirConditionerItem()
{
    delete ui;
    if (m_impl->socket != nullptr)
    {
        m_impl->socket->close();
        delete m_impl->socket;
    }
    delete m_impl;
}
bool CenterAirConditionerItem::initSocket()
{
    //增大缓存区大小
    static constexpr int bufferSize = 128 * 1024 * 1024;
    if (m_impl->socket != nullptr)
    {
        if (m_impl->socket->state() == QAbstractSocket::BoundState)
        {
            return true;
        }
        closeConnect();
    }

    auto socket = new QUdpSocket;
    GetValueHelper<QUdpSocket> helper(socket);

    //增大发送和接收缓存区大小
    // socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, bufferSize);
    socket->setSocketOption(QAbstractSocket::SendBufferSizeSocketOption, bufferSize);
    /* 禁用自回环 */
    socket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, false);
    /* 通过UDP通讯时要过两个交换机，如果你不进行TTL设置，在第二台交换机就已经收不到你的UDP报文，导致你发的报文中途丢掉了
     * 所以要进行TTL设置(就像游戏中你有255条命一样)
     */
    socket->setSocketOption(QAbstractSocket::MulticastTtlOption, 0XFF);
    // QHostAddress(m_impl->desIP)
    auto result = socket->bind(QHostAddress::AnyIPv4, m_impl->recvPort, QAbstractSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    if (!result)
    {
        qWarning() << QString("连接空调失败: %1").arg(socket->errorString());
        return false;
    }

    QThread::msleep(10);
    connect(socket, &QIODevice::readyRead, this, &CenterAirConditionerItem::onReadyRead);
    m_impl->socket = helper.get();
    return true;
}
void CenterAirConditionerItem::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    m_impl->recvData.clear();

    if (m_impl->timerID == -1)
    {
        m_impl->timerID = startTimer(1000);
    }
}
void CenterAirConditionerItem::hideEvent(QHideEvent* event)
{
    QWidget::hideEvent(event);

    if (m_impl->timerID != -1)
    {
        killTimer(m_impl->timerID);
        m_impl->timerID = -1;
    }
    closeConnect();
}
void CenterAirConditionerItem::timerEvent(QTimerEvent* /*event*/)
{
    if (!initSocket())
    {
        return;
    }
    /* 定时发送状态查询命令 */
    auto& socket = m_impl->socket;
    auto sendRet = socket->writeDatagram(m_impl->statusQuery, QHostAddress(m_impl->desIP), m_impl->sendPort);

    if (sendRet < 0)
    {
        qWarning() << QString("空调状态查询指令发送失败:%1").arg(socket->errorString());
    }
}
void CenterAirConditionerItem::closeConnect()
{
    if (m_impl->socket != nullptr)
    {
        m_impl->socket->close();
        delete m_impl->socket;
        m_impl->socket = nullptr;
    }
    m_impl->recvData.clear();
    reset();
}
void CenterAirConditionerItem::reset()
{
    CenterAirConditionerItemImpl::bgReset(ui->sf_0, false);
    CenterAirConditionerItemImpl::bgReset(ui->sf_1, false);
    CenterAirConditionerItemImpl::bgReset(ui->sf_2, false);
    CenterAirConditionerItemImpl::bgReset(ui->sf_3, false);
    CenterAirConditionerItemImpl::bgReset(ui->sf_4, false);
    CenterAirConditionerItemImpl::bgReset(ui->sf_5, false);
    CenterAirConditionerItemImpl::bgReset(ui->sf_6, false);
    CenterAirConditionerItemImpl::bgReset(ui->sf_7, false);
    CenterAirConditionerItemImpl::bgReset(ui->sf_8, false);

    CenterAirConditionerItemImpl::bgReset(ui->dsf_0, false);
    CenterAirConditionerItemImpl::bgReset(ui->dsf_1, false);
    CenterAirConditionerItemImpl::bgReset(ui->dsf_2, false);

    CenterAirConditionerItemImpl::bgReset(ui->dy, true);
    CenterAirConditionerItemImpl::bgReset(ui->ms, true);
    CenterAirConditionerItemImpl::bgReset(ui->nfjzt, true);
    CenterAirConditionerItemImpl::bgReset(ui->wfjzt, true);
    CenterAirConditionerItemImpl::bgReset(ui->ysj, true);
    CenterAirConditionerItemImpl::bgReset(ui->fzdjr, true);
    CenterAirConditionerItemImpl::bgReset(ui->gyyl, true);
    CenterAirConditionerItemImpl::bgReset(ui->dyyl, true);
    CenterAirConditionerItemImpl::bgReset(ui->sdwd, true);
    CenterAirConditionerItemImpl::bgReset(ui->hfwd, true);
    CenterAirConditionerItemImpl::bgReset(ui->swwd, true);
    CenterAirConditionerItemImpl::bgReset(ui->nfjdw, true);
}

void CenterAirConditionerItem::setSFData(quint8 data1, quint8 data2)
{
    /* 0 高压保护 1红 0绿 */
    CenterAirConditionerItemImpl::bgRedOrGreen(ui->sf_0, (data1 >> 0) & 0x01);
    /* 1 低压保护 1红 0绿 */
    CenterAirConditionerItemImpl::bgRedOrGreen(ui->sf_1, (data1 >> 1) & 0x01);
    /* 2 排气高温保护 1红 0绿 */
    CenterAirConditionerItemImpl::bgRedOrGreen(ui->sf_2, (data1 >> 2) & 0x01);
    /* 3 内管感温包故障 1红 0绿 */
    CenterAirConditionerItemImpl::bgRedOrGreen(ui->sf_3, (data1 >> 3) & 0x01);
    /* 4 排气感温包故障 1红 0绿 */
    CenterAirConditionerItemImpl::bgRedOrGreen(ui->sf_4, (data1 >> 4) & 0x01);
    /* 5 吸气感温包故障 1红 0绿 */
    CenterAirConditionerItemImpl::bgRedOrGreen(ui->sf_5, (data1 >> 5) & 0x01);
    /* 6 高压传感器故障 1红 0绿 */
    CenterAirConditionerItemImpl::bgRedOrGreen(ui->sf_6, (data1 >> 6) & 0x01);
    /* 7 低压传感器故障 1红 0绿 */
    CenterAirConditionerItemImpl::bgRedOrGreen(ui->sf_7, (data1 >> 7) & 0x01);
    /* 8 内外机通讯故障 1红 0绿 */
    CenterAirConditionerItemImpl::bgRedOrGreen(ui->sf_8, (data2 >> 0) & 0x01);
}
void CenterAirConditionerItem::setDSFData(quint8 data1, quint8 /*data2*/)
{
    /* 0 内机与显示板通讯故障 1红 0绿 */
    CenterAirConditionerItemImpl::bgRedOrGreen(ui->dsf_0, (data1 >> 0) & 0x01);
    /* 1 回风感温包故障 1红 0绿 */
    CenterAirConditionerItemImpl::bgRedOrGreen(ui->dsf_1, (data1 >> 1) & 0x01);
    /* 2 室外感温包故障 1红 0绿 */
    CenterAirConditionerItemImpl::bgRedOrGreen(ui->dsf_2, (data1 >> 2) & 0x01);
}
void CenterAirConditionerItem::setDYData(quint8 data1, quint8 data2)
{
    quint16 data = (static_cast<quint16>(data1) << 8 | data2);
    /* 开关机状态 0关机 1关机 索引是1+1+1+4 1+1+1+5 */
    bool dy = data != 0x0001;
    ui->dy->setText(dy ? "关机" : "开机");
    CenterAirConditionerItemImpl::bgRedOrGreen(ui->dy, dy);
}
void CenterAirConditionerItem::setMSData(quint8 data1, quint8 data2)
{
    /* 0X01 送风 0X02 制冷 0X05 制热 0X06 除湿 */
    quint16 data = (static_cast<quint16>(data1) << 8 | data2);
    switch (data)
    {
    case 0x01:
    {
        ui->ms->setText("送风");
        return;
    }
    case 0x02:
    {
        ui->ms->setText("制冷");
        return;
    }
    case 0x05:
    {
        ui->ms->setText("制热");
        return;
    }
    case 0x06:
    {
        ui->ms->setText("除湿");
        return;
    }
    }
    ui->ms->setText("-");
}
void CenterAirConditionerItem::setNFJZTData(quint8 data1, quint8 data2)
{
    /* 000 关闭 0X01 低挡 0X02 中档 0X03 高档 */
    quint16 data = (static_cast<quint16>(data1) << 8 | data2);
    switch (data)
    {
    case 0x00:
    {
        ui->nfjzt->setText("关闭");
        return;
    }
    case 0x01:
    {
        ui->nfjzt->setText("低挡");
        return;
    }
    case 0x02:
    {
        ui->nfjzt->setText("中档");
        return;
    }
    case 0x03:
    {
        ui->nfjzt->setText("高档");
        return;
    }
    }
    ui->nfjzt->setText("-");
}
void CenterAirConditionerItem::setNFJDWData(quint8 data1, quint8 data2)
{
    /* 001 自动 0X02 低挡 0X03 中档 0X04 高档 */
    quint16 data = (static_cast<quint16>(data1) << 8 | data2);
    switch (data)
    {
    case 0x01:
    {
        ui->nfjdw->setText("自动");
        return;
    }
    case 0x02:
    {
        ui->nfjdw->setText("低挡");
        return;
    }
    case 0x03:
    {
        ui->nfjdw->setText("中档");
        return;
    }
    case 0x04:
    {
        ui->nfjdw->setText("高档");
        return;
    }
    }
    ui->nfjdw->setText("-");
}
void CenterAirConditionerItem::setWFJZTData(quint8 data1, quint8 data2)
{
    /* 000 关闭 0X01 低挡 0X02 中档 0X03 高档 */
    quint16 data = (static_cast<quint16>(data1) << 8 | data2);
    switch (data)
    {
    case 0x00:
    {
        ui->wfjzt->setText("关闭");
        return;
    }
    case 0x01:
    {
        ui->wfjzt->setText("低挡");
        return;
    }
    case 0x02:
    {
        ui->wfjzt->setText("中档");
        return;
    }
    case 0x03:
    {
        ui->wfjzt->setText("高档");
        return;
    }
    }
    ui->wfjzt->setText("-");
}
void CenterAirConditionerItem::setYSJZTData(quint8 data1, quint8 data2)
{
    quint16 data = (static_cast<quint16>(data1) << 8 | data2);
    /* 压缩机状态 0关闭 1开启 */
    bool dy = data != 0x0001;
    ui->ysj->setText(dy ? "关闭" : "开启");
    CenterAirConditionerItemImpl::bgRedOrGreen(ui->ysj, dy);
}
void CenterAirConditionerItem::setFZDJRZTData(quint8 data1, quint8 data2)
{
    quint16 data = (static_cast<quint16>(data1) << 8 | data2);
    /* 辅助电加热状态 0关闭 1开启 */
    bool dy = data != 0x0001;
    ui->fzdjr->setText(dy ? "关闭" : "开启");
    CenterAirConditionerItemImpl::bgRedOrGreen(ui->fzdjr, dy);
}
void CenterAirConditionerItem::setNumData(QLabel* label, quint8 data1, quint8 data2)
{
    quint16 data = (static_cast<quint16>(data1) << 8 | data2);
    label->setText(QString::number(data));
}
void CenterAirConditionerItem::setNumDataDev10(QLabel* label, quint8 data1, quint8 data2)
{
    qint16 data = (static_cast<qint16>(data1) << 8 | data2);
    label->setText(QString::number(data / 10.0, 'f', 1));
}
void CenterAirConditionerItem::onReadyRead()
{
    auto& socket = m_impl->socket;
    if (!socket->hasPendingDatagrams())
    {
        return;
    }

#if (QT_VERSION >= QT_VERSION_CHECK(5, 8, 0))
    auto datagram = socket->receiveDatagram();
    auto data = datagram.data();
#else
    auto count = socket->pendingDatagramSize();
    QByteArray data;
    data.resize(count);
    socket->readDatagram(data.data(), count);
#endif

    m_impl->recvData += data;
    /* 收到数据 */
    /* 状态
     * 设备地址 功能码 字节数 有效数据 CRC校验
     *  0x40    0x04      1     n      2
     *
     *  设置
     * 设备地址  功能码    起始地址  数据数量    CRC校验
     *  0x40     0x10       2       2       2
     *
     *  错误
     * 设备地址  功能码        异常码   CRC校验
     *  0x40   0x84|0x90      1       2
     *
     *
     *  读取状态是 0x4004
     *  写入数据是 0x4010
     *  错误码是 0x4084或者0x4090
     *
     *  不管什么数据至少有5个字节
     */
    if (m_impl->recvData.size() < 5)
    {
        return;
    }
    /* 头不对,乱包了,重来 */
    if (m_impl->recvData.at(0) != char(0x40))
    {
        qWarning() << "中心体空调数据包错误";
        closeConnect();
        return;
    }
    switch (quint8(m_impl->recvData.at(1)))
    {
    /* 这个是读取状态的 */
    case 0x04:
    {
        /* 读取数据长度 */
        auto dataLen = static_cast<quint8>(m_impl->recvData.at(2));
        auto total = 1 + 1 + 1 + dataLen + 2;
        /* 数据长度不够,再等等 */
        if (m_impl->recvData.size() < total)
        {
            return;
        }
        auto realData = m_impl->recvData.left(total);
        m_impl->recvData.remove(0, total);
        /* 校验失败,数据错误,丢弃数据,2021018这个校验有问题 */
        if (!CenterAirConditionerItemImpl::checkCRC(realData))
        {
            return;
        }

        // 40041c 0000 0000 0001 0002 0001 0002 0002 0001 0000 0388 0132 0011 00bf 0095 139d
        /* 校验成功,解析数据 */
        /* 空调机组停机故障 索引是1+1+1+0  1+1+1+1 */
        setSFData(realData.at(3), realData.at(4));
        /* 空调机组不停机故障 索引是1+1+1+2 1+1+1+3 */
        setDSFData(realData.at(5), realData.at(6));
        /* 开关机状态 0关机 1关机 索引是1+1+1+4 1+1+1+5 */
        setDYData(realData.at(7), realData.at(8));
        /* 模式状态 */
        setMSData(realData.at(9), realData.at(10));
        /* 内风机风挡 */
        setNFJDWData(realData.at(11), realData.at(12));
        /* 内风机状态 */
        setNFJZTData(realData.at(13), realData.at(14));
        /* 外风机状态 */
        setWFJZTData(realData.at(15), realData.at(16));
        /* 压缩机状态 */
        setYSJZTData(realData.at(17), realData.at(18));
        /* 辅助电加热状态 */
        setFZDJRZTData(realData.at(19), realData.at(20));
        /* 高压压力 */
        setNumData(ui->gyyl, realData.at(21), realData.at(22));
        /* 低压压力 */
        setNumData(ui->dyyl, realData.at(23), realData.at(24));
        /* 设定温度16 35 */
        setNumData(ui->sdwd, realData.at(25), realData.at(26));
        /* 回风温度-45 100 */
        setNumDataDev10(ui->hfwd, realData.at(27), realData.at(28));
        /* 室外温度-45 100 */
        setNumDataDev10(ui->swwd, realData.at(29), realData.at(30));

        return;
    }
    /* 设置数据的返回值 */
    case 0x10:
    {
        /* 不做处理丢弃数据就行 */
        /* 数据长度不够,再等等 */
        auto total = 1 + 1 + 2 + 2 + 2;
        if (m_impl->recvData.size() < total)
        {
            return;
        }
        // auto realData = m_impl->recvData.left(total);
        m_impl->recvData.remove(0, total);
        return;
    }
    /* 错误1 */
    case 0x84:
    /* 错误2 */
    case 0x90:
    {
        /* 不做处理丢弃数据就行 */
        // auto realData = m_impl->recvData.left(total);
        m_impl->recvData.remove(0, 5);
        return;
    }
    }

    /* 数据错误没有这个选项 */
    qWarning() << "未知的功能码";
    closeConnect();
}
void CenterAirConditionerItem::setControlCmd(quint8 addr1, quint8 addr2, quint8 data1, quint8 data2)
{
    QByteArray cmd;
    cmd.append(char(0x40));
    cmd.append(char(0x10));
    cmd.append(char(addr1));
    cmd.append(char(addr2));
    cmd.append(char(0x00));
    cmd.append(char(0x01));
    cmd.append(char(0x02));
    cmd.append(char(data1));
    cmd.append(char(data2));
    CenterAirConditionerItemImpl::addCRC(cmd);
    setWindowTitle(QString(cmd.toHex()));

    if (!initSocket())
    {
        return;
    }
    QThread::msleep(50);
    /* 定时发送状态查询命令 */
    auto& socket = m_impl->socket;
    auto sendRet = socket->writeDatagram(cmd, QHostAddress(m_impl->desIP), m_impl->sendPort);

    if (sendRet < 0)
    {
        qWarning() << QString("空调控制指令发送失败:%1").arg(socket->errorString());
    }
}
