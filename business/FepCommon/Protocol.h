#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QDataStream>
#include <QFile>
#include <QString>
#include <QTcpSocket>

/* constexpr linux qt5.6无法加载符号 */
// class Def
//{
// public:
//    static constexpr int typeIDLen = 1;      /* 类型标识长度 */
//    static constexpr int fileNameLen = 64;   /* 文件名长度 */
//    static constexpr int fileContentLen = 4; /* 文件内容大小长度 */
//    static constexpr int dataUnitIDLen = 4;  /* 数据单元号长度 */
//    static constexpr int fileIDLen = 2;      /* 文件标识长度 */

//    static constexpr int dataMaxLen = 4096; /* 数据内容长度 */
//};

class DefLen
{
public:
    static int typeIDLen;      /* 类型标识长度 */
    static int fileNameLen;    /* 文件名长度 */
    static int fileContentLen; /* 文件内容大小长度 */
    static int dataUnitIDLen;  /* 数据单元号长度 */
    static int fileIDLen;      /* 文件标识长度 */

    static int dataMaxLen; /* 数据内容长度 */

    /******************************************************************************/
    static int packLenSendREQ;
    static int packLenAnserREQ;
    static int packLenFinish;

    static int packLenData;
    static int packMaxLenData;
    /* 数据包是动态的,但是前面的标识字段是必须完整的 */
    static int packMinLenData;
    static int maxBufferSize;
};

//需要发送的文件的结构
struct SendFileInfo
{
    QString absoluteFilePath; /* 文件全路径 */
    QString fileName;         /* 文件名 */
    qint32 length{ 0 };       /* 文件总长 */
    qint32 sendLength{ 0 };   /* 已经发送的长度 */
    qint32 curUnitNum{ 0 };   /* 当前单元编号 */
    quint16 fileID{ 0 };      /* 文件标识 */
};
//接收的文件信息结构
struct RecvFileInfo
{
    QString absoluteFilePath; /* 文件全路径 */
    QString fileName;         /* 文件名 */
    quint16 fileID{ 0 };      /* 文件标识 */
    quint32 fileLength{ 0 };  /* 文件总长 */
};
/* 按照协议发送过程有以下的几种状态 */
enum class SendState
{
    InitSend = 0x00, /* 初始化发送信息 */
    SendFile,        /* 发送文件 */
    Close,           /* 正常关闭连接 */
    Next,            /* 单个文件发送完成 */
    Abort,           /* 异常 */
};
/* 接收的状态 */
enum class RecvState
{
    Wait = 0x00, /* 等待发送文件 */
    Abort,       /* 异常 断开连接 */
    Fault,       /* 故障 -2 */
    FullFile,    /* 完整的文件 */
};

struct SendContent
{
    QString dstIP;        /* 目的ip地址 */
    quint16 dstPort{ 0 }; /* 目的端口号 */
    QString workDir;      /* 目的文件的路径 */
    SendState state{ SendState::Close };
    QList<SendFileInfo> fileList;
    SendFileInfo* curFileInfo{ nullptr };
};
struct RecvContent
{
    QString srcIP;             /* ip地址 */
    qint32 curUnitNum{ 0 };    /* 当前预计的单元号 */
    quint32 recvFileSize{ 0 }; /* 当前已接收文件的大小 */
    quint16 srcPort{ 0 };      /* 端口号 */
    RecvState state{ RecvState::Wait };
    QString workDir; /* 工作目录 */
    RecvFileInfo curFileInfo;
    QFile file; /* 当前的写入文件的handle */
    RecvContent() {}
    ~RecvContent()
    {
        if (file.isOpen())
        {
            file.flush();
            file.close();
        }
    }

    RecvContent(const RecvContent&) = delete;
    RecvContent(RecvContent&&) = delete;
    RecvContent& operator=(const RecvContent&) = delete;
    RecvContent& operator=(RecvContent&&) = delete;

    void reset()
    {
        srcIP.clear();    /* ip地址 */
        curUnitNum = 0;   /* 当前预计的单元号 */
        recvFileSize = 0; /* 当前已接收文件的大小 */
        srcPort = 0;      /* 端口号 */
        state = RecvState::Wait;
        workDir.clear(); /* 工作目录 */
        curFileInfo = RecvFileInfo();
        if (file.isOpen())
        {
            file.flush();
            file.close();
        }
    }
};

/*
 * 包类型 占用1个字节
 */
enum class PackType : quint8
{
    SendREQ = 0x01,  /* 发送请求包 */
    AnserREQ = 0x02, /* 请求应答包 */
    Finish = 0x03,   /* 结束确认包 */
    Data = 0x04,     /* 数据包 */
};

/*
 * 请求包
 * 格式
 * 1字节类型 PackType
 * 64字节文件名
 * 4字节文件长度
 */
class REQPack
{
public:
    static constexpr auto type = PackType::SendREQ;
    quint32 fileLen{ 0 }; /* 文件内容长度 */
    QString fileName;     /* 文件名 */

public:
    bool toByteArray(QByteArray& array, QString& errMsg) const;
    bool fromByteArray(const QByteArray& array, QString& errMsg);
};
/*
 * 结束确认
 * 格式
 * 1字节类型 PackType
 * 64字节文件名
 * 4字节数据单元号
 * 2字节文件标识
 */
class FinishPack
{
public:
    static constexpr auto type = PackType::Finish;
    quint16 fileID{ 0 }; /* 文件标识 */

public:
    bool toByteArray(QByteArray& array, QString& errMsg) const;
    bool fromByteArray(const QByteArray& array, QString& errMsg);
};
/*
 * 请求应答包
 * 格式
 * 1字节类型 PackType
 * 2字节文件标识
 */
class AnserPack
{
public:
    static constexpr auto type = PackType::AnserREQ;
    qint32 dataUnitNum{ 0 }; /* 数据单元号 */
    quint16 fileID{ 0 };     /* 文件标识 */
    QString fileName;        /* 文件名 */

public:
    bool toByteArray(QByteArray& array, QString& errMsg) const;
    bool fromByteArray(const QByteArray& array, QString& errMsg);
};

/*
 * 数据包
 * 格式
 * 1字节类型 PackType
 * 4字节数据单元号
 * 2字节文件标识
 * n字节Data  Def::dataMaxLen
 */
class DataPack
{
public:
    static constexpr auto type = PackType::Data;
    qint32 dataUnitNum{ 0 }; /* 数据单元号 */
    quint16 fileID{ 0 };     /* 文件标识 */
    QByteArray data;         /* 数据 */

public:
    bool toByteArray(QByteArray& array, QString& errMsg) const;
    /*
     * 数据部分是动态长度,所以此处解包需要处理
     * readLen 返回的是实际读取的总字节数头+数据
     * return <0 发生错误
     * return =0 长度不够
     * return >0 ok
     */
    int fromByteArray(const QByteArray& array, const RecvContent&, quint32& readLen, QString& errMsg);
};

// constexpr int maxBufferSize()
//{
//    return qMax(StateInfo<PackType::SendREQ>::packLen,
//                qMax(StateInfo<PackType::AnserREQ>::packLen, qMax(StateInfo<PackType::Data>::packMaxLen, StateInfo<PackType::Finish>::packLen)));
//}

#endif  // PROTOCOL_H
