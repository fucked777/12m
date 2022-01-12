#ifndef ERRORCODE_H
#define ERRORCODE_H
#include <QObject>

/*
 * 0为无错误
 * [1-0x1000)通用性错误码
 * 然后每过0x100为一组
 *
 * [0x1000-0x1100) ZMQ错误
 * [0x1100-0x1200) Redis
 * [0x1300-0x1400) 配置宏
 * [0x1400-0x1500) AGC曲线标定
 * ....
 */
/* 错误码以及错误码对应的字符串 */
#define ERROR_CODE(X)                                                                                                                                \
    X(UnknownError, 0x01, "未知的错误")                                                                                                              \
    X(OpenFileError, 0x02, "文件打开错误")                                                                                                           \
    X(XmlParseError, 0x03, "Xml文件解析错误")                                                                                                        \
    X(DataItemIsMissing, 0x04, "数据项缺失")                                                                                                         \
    X(DataLoadFailed, 0x05, "数据载入失败")                                                                                                          \
    X(NotInit, 0x06, "未初始化")                                                                                                                     \
    X(InitFailed, 0x07, "初始化失败")                                                                                                                \
    X(DataSendFailed, 0x08, "数据发送错误")                                                                                                          \
    X(DataRecvFailed, 0x09, "数据接收错误")                                                                                                          \
    X(NotFound, 0x0A, "未查找到指定的数据")                                                                                                          \
    X(InvalidArgument, 0x0B, "无效的参数")                                                                                                           \
    X(NotImplemented, 0x0C, "未实现")                                                                                                                \
    X(UseOverride, 0x0D, "使用此方法需要重写")                                                                                                       \
    X(DataNotEmpty, 0x0E, "数据不能为空")                                                                                                            \
    X(DataBaseConnectFailed, 0x0F, "数据库连接失败")                                                                                                 \
    X(DataBaseOpenFailed, 0x10, "数据库打开失败")                                                                                                    \
    X(SqlExecFailed, 0x11, "Sql语句执行失败")                                                                                                        \
    X(InitDataBaseTableFailed, 0x12, "初始化数据库表失败")                                                                                           \
    X(DataExist, 0x13, "数据已存在")                                                                                                                 \
    X(InvalidData, 0x14, "无效的数据")                                                                                                               \
    X(NotSupported, 0x15, "不支持")                                                                                                                  \
    X(CreateCatalogueFailed, 0x16, "目录创建失败")                                                                                                   \
    X(OperatingModeAbnormal, 0x17, "当前工作模式异常")                                                                                               \
    X(DevOffline, 0x18, "设备离线")                                                                                                                  \
    X(ZMQSocketInitFailed, 0x1001, "ZMQSocket初始化失败")                                                                                            \
    X(ZMQConnectFailed, 0x1002, "ZMQSocket连接失败")                                                                                                 \
    X(RedisDisconnect, 0x1100, "Redis未连接")                                                                                                        \
    X(RedisSendDataFailed, 0x1101, "Redis数据发送错误")                                                                                              \
    X(RedisRecvDataFailed, 0x1102, "Redis接收数据错误")                                                                                              \
    X(RedisQueryFailed, 0x1103, "Redis查询错误")                                                                                                     \
    X(ConfigAddFailed, 0x1301, "配置宏添加失败")                                                                                                     \
    X(ConfigEditFailed, 0x1302, "配置宏编辑失败")                                                                                                    \
    X(ConfigDelFailed, 0x1303, "配置宏删除失败")                                                                                                     \
    X(AutomateTestFailed, 0x1401, "自动化测试失败")

#define X(a, b, c) a = b,
enum class ErrorCode : quint32
{
    /* 0为无错误 */
    Success = 0,
    OK = Success,
    Normal = Success,
    ERROR_CODE(X)
};
#undef X

Q_DECLARE_METATYPE(ErrorCode);
#endif  // ERRORCODE_H
