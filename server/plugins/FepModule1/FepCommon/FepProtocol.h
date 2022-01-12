#ifndef FEPPROTOCOL_H
#define FEPPROTOCOL_H

#include <QByteArray>
#include <QDataStream>
#include <QFile>
#include <QString>
#include <QVariant>

/* 协议长度定义 */
namespace ProtocolLen
{
    /* Fep */
    static constexpr qint32 fepTypeID = 1;                                                         /* 类型标识长度 */
    static constexpr qint32 fepFileName = 64;                                                      /* 文件名长度 */
    static constexpr qint32 fepFileContentLen = 4;                                                 /* 文件内容大小长度 */
    static constexpr qint32 fepDataUnitID = 4;                                                     /* 数据单元号长度 */
    static constexpr qint32 fepFileID = 2;                                                         /* 文件标识长度 */
    static constexpr qint32 fepDataMaxLen = 4096;                                                  /* 数据内容长度 */
    static constexpr qint32 fepPackSendREQ = fepTypeID + fepFileName + fepFileContentLen;          /* FEP发送请求包长度 */
    static constexpr qint32 fepPackAnserREQ = fepTypeID + fepFileName + fepDataUnitID + fepFileID; /* FEP请求应答包长度 */
    static constexpr qint32 fepPackFinish = fepTypeID + fepFileID;                                 /* FEP结束确认包长度 */
    static constexpr qint32 fepPackData = fepTypeID + fepDataUnitID + fepFileID + fepDataMaxLen;   /* FEP数据包长度 */
    static constexpr qint32 fepPackMaxData = fepPackData;                                          /* FEP数据包最大有效长度 */
    static constexpr qint32 fepPackMinData = fepTypeID + fepDataUnitID + fepFileID + 0;            /* FEP数据包最小有效长度 */

    /* RECP */
    static constexpr qint32 recpTypeID = 1;                                                                                /* 类型标识长度 */
    static constexpr qint32 recpSrcAddr = 4;                                                                               /* 源地址长度 */
    static constexpr qint32 recpSN = 4;                                                                                    /* 序号长度 */
    static constexpr qint32 recpRetain = 4;                                                                                /* 保留位长度 */
    static constexpr qint32 recpAbstractLength = 4;                                                                        /* 摘要长度长度 */
    static constexpr qint32 recpAbstract = 56;                                                                             /* 摘要长度 */
    static constexpr qint32 recpHead = recpTypeID + recpSrcAddr + recpSN + recpRetain + recpAbstractLength + recpAbstract; /* RECP头长度 */

}  // namespace ProtocolLen

/*
 * 包类型 占用1个字节
 */
enum class RECPPackType : quint8
{
    SYN = 0x01,  /* 连接包 */
    ACK = 0x02,  /* 应答包 */
    Data = 0x04, /* 数据包 */
    FIN = 0x08,  /* 结束连接包 */
};
/* RECP协议头 */
struct RECPProtocolHead
{
    RECPPackType recpPackType{ RECPPackType::SYN }; /* 标志 1字节 */
    QString srcIP;                                  /* 源IP地址 需要转换成数字 4字节 */
    qint32 sn{ 0 };                                 /* 序号 4字节 */
    quint8 retain[ProtocolLen::recpRetain]{};       /* 保留 4字节 */
    qint32 abstractLength{ 0 };                     /* 摘要长度(可选) 4字节 */
    quint8 abstract[ProtocolLen::recpAbstract]{};   /* 摘要长度(可选) 56字节 */
};

/* Fep包类型 */
enum class FepPackType : quint8
{
    SendREQ = 0x01,  /* 发送请求包 */
    AnserREQ = 0x02, /* 请求应答包 */
    Finish = 0x03,   /* 结束确认包 */
    Data = 0x04,     /* 数据包 */
};
// struct FepProtocol
//{
//    FepPackType fepPackType{ FepPackType::SendREQ }; /* 类型标识 */
//    QVariant variant;
//};
/* FEP发送请求包 */
struct FepSendREQ
{
    const FepPackType fepPackType{ FepPackType::SendREQ }; /* 类型标识 1字节 */
    QString fileName;                                      /* 文件名   64字节 */
    quint32 fileLen{ 0 };                                  /* 文件长度 4字节 */
};
/* FEP发送请求包 */
struct FepAnserREQ
{
    const FepPackType fepPackType{ FepPackType::AnserREQ }; /* 类型标识 1字节 */
    QString fileName;                                       /* 文件名   64字节 */
    qint32 unitNum{ 0 };                                    /* 数据单元号 4字节 */
    quint16 fileID{ 0 };                                    /* 文件标识 2字节 */
};
/* FEP结束确认包 */
struct FepFinish
{
    const FepPackType fepPackType{ FepPackType::Finish }; /* 类型标识 1字节 */
    quint16 fileID{ 0 };                                  /* 文件标识 2字节 */
};
/* FEP数据包 */
struct FepData
{
    const FepPackType fepPackType{ FepPackType::Data }; /* 类型标识 1字节 */
    qint32 unitNum{ 0 };                                /* 数据单元号 4字节 */
    quint16 fileID{ 0 };                                /* 文件标识 2字节 */
    QByteArray data;                                    /* 数据 */
};

/* 接收的状态 */
enum class FepRecvState
{
    Wait = 0x00, /* 等待发送文件 */
    Abort,       /* 异常 断开连接 */
    Fault,       /* 故障 -2 */
    FullFile,    /* 完整的文件 */
};
enum class FepSendState
{
    InitSend = 0x00, /* 初始化发送信息 */
    SendFile,        /* 发送文件 */
    Close,           /* 正常关闭连接 */
    Next,            /* 单个文件发送完成 */
    Abort,           /* 异常 */
};
//需要发送的文件的结构
struct FepSendFileInfo
{
    QString absoluteFilePath; /* 文件全路径 */
    QString fileName;         /* 文件名 */
    qint32 length{ 0 };       /* 文件总长 */
    qint32 sendLength{ 0 };   /* 已经发送的长度 */
    qint32 curUnitNum{ 0 };   /* 当前单元编号 */
    quint16 fileID{ 0 };      /* 文件标识 */
};
//接收的文件信息结构
struct FepRecvFileInfo
{
    QString absoluteFilePath; /* 文件全路径 */
    QString fileName;         /* 文件名 */
    quint16 fileID{ 0 };      /* 文件标识 */
    quint32 fileLength{ 0 };  /* 文件总长 */
};
/* 发送接收上下文 */
struct FepSendContent
{
    QString dstIP;        /* 目的ip地址 */
    quint16 dstPort{ 0 }; /* 目的端口号 */
    QString workDir;      /* 目的文件的路径 */
    FepSendState state{ FepSendState::Close };
    QList<FepSendFileInfo> fileList;
    int curInfoIndex{ -1 };
};
struct FepRecvContent
{
    QString srcIP;                            /* ip地址 */
    qint32 curUnitNum{ 0 };                   /* 当前预计的单元号 */
    quint32 recvFileSize{ 0 };                /* 当前已接收文件的大小 */
    quint16 srcPort{ 0 };                     /* 端口号 */
    FepRecvState state{ FepRecvState::Wait }; /* 接收状态 */
    QString workDir;                          /* 工作目录 */
    FepRecvFileInfo curFileInfo;              /* 接收的文件信息 */
    QFile file;                               /* 当前的写入文件的handle */
    FepRecvContent();
    ~FepRecvContent();

    FepRecvContent(const FepRecvContent&) = delete;
    FepRecvContent(FepRecvContent&&) = delete;
    FepRecvContent& operator=(const FepRecvContent&) = delete;
    FepRecvContent& operator=(FepRecvContent&&) = delete;

    void reset();
};

Q_DECLARE_METATYPE(FepAnserREQ);
Q_DECLARE_METATYPE(FepFinish);
Q_DECLARE_METATYPE(FepData);
Q_DECLARE_METATYPE(FepSendREQ);
// Q_DECLARE_METATYPE(FepProtocol);
Q_DECLARE_METATYPE(RECPProtocolHead);

#endif  // FEPPROTOCOL_H
