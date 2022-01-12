#ifndef DATATYPEDEFINE_H
#define DATATYPEDEFINE_H

#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QtCore>
// add cyx
#include <stdlib.h>
#define MIN_VALUE 1e-8
#define IS_DOUBLE_ZERO(d) (abs(d) < 1e-8)

namespace DataBuilder
{
//数据类型
enum DataType
{
    Invalid = -1,  //无效
    Char,
    UChar,
    Double,
    Float,
    Int8,
    Int16,
    Int32,
    Int64,
    UInt8,
    UInt16,
    UInt32,
    UInt64,
    Combo,    //下拉框
    Checked,  //复选框
    Radio,    //单选框
    Date,
    Time,
    DateTime,
    Hex,     // 16进制
    Bit,     //按位存取 cyx V20
    IP,      // cyx V21
    Text,    //富文本框 //jyt 20190903
    Button,  //按钮控件 //jyt 20190905
    Lamp,
    LampGR,
    LampRG,
    LampGGR,  //状态灯控件 chehd 20191108
    LampGRG,
    LampGRYG,
    LampGGYR
};

//数据定义
struct Data
{
    Data()
    {
        pid = "";
        step = 1;
        bytesCount = -1;
        selfLoopNumber = -1;  // cyx V16
        sysID = -1;           // jyt 20190912
        devID = -1;
        extenID = -1;
        modelID = 0xFFFF;
        unitID = -1;
        comboCurrentValue = -1;  // FIX 20191012  wjy
    }
    QString pid;                 // pid
    QString id;                  // id
    QString title;               //标题
    QString desc;                //标题
    DataType type;               //数据类型
    QVariant defaultValue;       //值(如果是非原子数据类型, 则是枚举ID)
    QVariant comboCurrentValue;  //当前枚举值，用于初始化Enum控件     // FIX 20191012 wjy
    QVariant min;                //最小值
    QVariant max;                //最大值
    double step;
    QString unit;        //单位
    quint64 bytesCount;  //字节数
    int selfLoopNumber;  //字段自循环次数 cyx V16
    QString groupId;

    int sysID;  // jyt 20190912
    int devID;
    int extenID;
    int modelID;
    int unitID;
};

typedef QList<Data> DataList;  //数据列表

//枚举数据
struct EnumData
{
    int index;      //索引
    QVariant data;  //值
    QString desc;   //描述值
};

//配置宏设备枚举数据 liuym
struct DeviceItemEnumData
{
    qint32 index;      //索引
    qint32 system;     //系统号
    qint32 device;     //设备号
    qint32 extension;  //分机号
    qint32 unit;       //单元号
    QString name;      //名称
    QString desc;      //描述

    DeviceItemEnumData()
    {
        index = 0;
        system = -1;
        device = -1;
        extension = -1;
        unit = -1;
        name = "";
        desc = "";
    }
};

struct QWidgetPsotion
{
    quint8 rowindex;
    quint16 layoutX;
    quint16 layoutY;
    quint16 rowSpan;
    quint16 columnSpan;
    QWidget* qwidgest;
    QLabel* qlable;
    QString pid;  //控件ID
    QWidgetPsotion()
    {
        qwidgest = NULL;
        qlable = NULL;
        pid = "";
        rowSpan = 1;
        columnSpan = 1;
        rowindex = 0;
        layoutX = 0;
        layoutY = 0;
    }
};

struct QGroupBoxAndlayout
{
    QGroupBox* groupBoxItem;
    QGridLayout* groupLayout;
    quint8 rowindex;
    quint16 layoutX;
    quint16 layoutY;
    quint16 rowSpan;
    quint16 columnSpan;
    bool isShow;
    QGroupBoxAndlayout()
    {
        rowSpan = 1;
        columnSpan = 2;
        groupBoxItem = nullptr;
        groupLayout = nullptr;
        rowindex = 0;
        layoutX = 0;
        layoutY = 0;
        isShow = true;
    }
};
typedef QList<EnumData> EnumDatas;               //数据列表
typedef QMap<QString, EnumDatas> EnumIDToDatas;  //枚举ID-数据列表
typedef QList<QWidget*> Widgets;
typedef QMap<QString, QWidgetPsotion> PosList;
typedef QMap<QString, QGroupBoxAndlayout*> GroupBoxs;
typedef QList<QLabel*> Labellist;  // cyx V11
#define WIDGETWIDTH 200            //默认创建的参数或状态窗体大小 cyx V13
#define LABELWIDTH 140             //默认创建的参数或状态label大小
#define SPACE 50                   //默认创建的参数或状态间隔

#define DATATYPE_ID "DataID"                //数据ID
#define DATATYPE_TITLE "DataTitle"          //数据标题
#define DATATYPE_TYPE "DataType"            //数据类型
#define DATATYPE_EXTENDNUM "DataExtendNum"  //数据扩增数
#define DATATYPE_LABELPAR "LabelPar"        //数据对应的label
#define DATATYPE_GROUP "GroupID"
#define DATATYPE_SYSID "SysID"  // jyt 20190912
#define DATATYPE_DEVID "DevID"
#define DATATYPE_EXTENID "ExtenID"
#define DATATYPE_MODELID "ModelID"
#define DATATYPE_UNITID "UnitID"

enum FormsStatus
{
    Normal,           //正常
    WaitingResponse,  //等待响应
    ResponseError,    //响应结果错误
    ResponseTimeOut,  //响应超时
    Success,          //设置成功
};
}  // namespace DataBuilder
#endif  // DATATYPEDEFINE_H
