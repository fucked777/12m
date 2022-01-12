#ifndef PDXPMESSAGEDEFINE_H
#define PDXPMESSAGEDEFINE_H

#include <QDataStream>
#include <QDebug>
#include <QString>
enum PDXPMESSAGETYPE
{
    //    KCS_MESSAGE_TYPE = 0x602611A1,         //块参数设置命令
    //    DYCS_MESSAGE_TYPE = 0x602611A2,        //单元参数设置命令
    //    FJCS_MESSAGE_TYPE = 0x602611A3,        //分机参数设置命令
    //    SBQZJSXXCX_MESSAGE_TYPE = 0x602611A4,  //设备全帧监视信息查询命令
    //    XTGCKZ_MESSAGE_TYPE = 0x602611A5,      //系统过程控制命令
    //    FJGCKZ_MESSAGE_TYPE = 0x602611A6,      //分机过程控制命令
    //    JSXXYD_MESSAGE_TYPE = 0x602611B0,      //接收信息应答
    //    CSSZJGSB_MESSAGE_TYPE = 0x602611C1,    //参数设置结果上报
    //    SBQZJSXX_MESSAGE_TYPE = 0x602611C2,    //设备全帧监视信息上报
    //    KBZCJSXX_MESSAGE_TYPE = 0x602611C3,    //可变帧长监视信息上报
    //    LLZTJSXX_MESSAGE_TYPE = 0x602611C4,    //链路状态监视信息上报
    //    GCKZJGSB_MESSAGE_TYPE = 0x602611C5,    //过程控制结果上报
    //    JSXXSB_MESSAGE_TYPE = 0x602611C6,      //即时消息上报
    //    CKSBZHZT_MESSAGE_TYPE = 0x602611C7,    //测控设备综合状态上报
    //    SBKZML_DEVICE_TYPE = 0x6026FFF1,       //自定义设备类型

    //中心给监控
    ZWZX_YCKZML_TYPE = 0x60261000,  // 站网中心远程控制命令
                                    //回复中心命令接收成功
    ZWZX_YCKZMLYD_TYPE = 0x60050101,  // 站网中心远程控制命令应答
                                      //回复中心命令命令执行状态
    ZWZX_YCKZMLZXJG_TYPE = 0x60261001,  // 站网中心远程控制命令执行结果
    ZWZX_RWCSSZ_TYPE = 0X60E0021D,      // 站网中心参数设置命令帧
    ZWZX_RWCSZXJG_TYPE = 0X60E0020D,    // 任务参数执行结果
    ZWZX_SJSHCFML_TYPE = 0x00210515,    // 数据事后重发命令
    ZWZX_LLJSZ_TYPE = 0x00020101,       // 链路监视帧

    ERROR_TYPE = 0xCDCDCDCD,  //错误类型
};

#endif  // IMESSAGEDEFINE_H
