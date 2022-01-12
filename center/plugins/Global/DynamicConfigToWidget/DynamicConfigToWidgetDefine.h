#ifndef DYNAMICCONFIGTOWIDGETDEFINE_H
#define DYNAMICCONFIGTOWIDGETDEFINE_H
#include <QGroupBox>
#include <QString>
#include <QVariant>
#include <tuple>
#include <type_traits>

/*
 * 当前的工程其实已经有一个创建ui界面的类了
 * 但是此类是从4413来的,为了一个结构可以创建出很多的控件,
 * 在当前结构中包含了很多的参数
 * 非常不好分辨
 * 并且此控件用于监控界面，监控界面随时改动,因为公用的结构导致你一动结构可能别的控件也出bug
 */

enum class DynamicConfigToWidgetType
{
    Unknown,
    Enum,          // 下拉列表
    TextLine,      // 单行文本
    TextMultiple,  // 多行文本
    Integer,       // 整数
    Double,        // 浮点
    HexNum,        // 16进制数字,获取到的值是数字 最大为 quint64 max 的值
    HexStr,        // 16进制文本,获取到的值是文本,与HexNum的区别是HexStr有可能转换为数字会溢出
    IP,            // IP地址
    Label,         // 只读文本
};
struct CommonToWidgetInfo
{
    QString id;             /* 需要人机交互的控件的id,不需要的话此值无效 */
    QString name;           /* 一般是一个label 一个控件 如果有那么这个name就是显示的文本 */
    QString desc;           /* 对此控件的描述可以是功能也可以是其他的,根据用途使用 */
    bool important{ true }; /* 当为false时,即使数据无效数据不合法也无影响 */
    bool onlyOne{ false };  /* 有些选项会会重复出现,且内容不能重复 */
    DynamicConfigToWidgetType type{ DynamicConfigToWidgetType::Unknown };
    QVariant toWidgetInfo; /* 对应下面的ToWidgetInfo结构体 */
};
/* 单个控件的描述信息 */
struct DynamicWidgetInfo
{
    DynamicWidgetInfo(QWidget* widget_, const CommonToWidgetInfo& info_)
        : widget(widget_)
        , widgetInfo(info_)
    {
    }
    DynamicWidgetInfo()
        : widget(nullptr)
    {
    }
    QWidget* widget{ nullptr };
    CommonToWidgetInfo widgetInfo;
};

struct EnumToWidgetInfo
{
    using type = std::integral_constant<DynamicConfigToWidgetType, DynamicConfigToWidgetType::Enum>;
    QVariantMap enumMap; /* enum<文本,值> */
    int initValue{ 0 };  /* 默认选中第几个小于0认为就是第一个,越界也是第一个 */
};
struct TextLineToWidgetInfo
{
    using type = std::integral_constant<DynamicConfigToWidgetType, DynamicConfigToWidgetType::TextLine>;
    QString initValue;       /* 初始化默认值 */
    QString regConstraints;  /* 正则约束 */
    QString placeholderText; /* 占位符字符 */
    int length{ -1 };        /* 最大长度 */
};
struct TextMultipleToWidgetInfo
{
    using type = std::integral_constant<DynamicConfigToWidgetType, DynamicConfigToWidgetType::TextMultiple>;
    QString initValue;       /* 初始化默认值 */
    QString placeholderText; /* 占位符字符 */
    int length{ -1 };        /* 最大长度 */
};
struct IntegerToWidgetInfo
{
    using type = std::integral_constant<DynamicConfigToWidgetType, DynamicConfigToWidgetType::Integer>;
    int minValue{ 0 };        /* 最小值 */
    int maxValue{ 0 };        /* 最大值 */
    int initValue{ 0 };       /* 初始化默认值 */
    bool limitValid{ false }; /* 最大值最小值是否有效,这个是自动赋值的 */
};
struct DoubleToWidgetInfo
{
    using type = std::integral_constant<DynamicConfigToWidgetType, DynamicConfigToWidgetType::Double>;
    double minValue{ 0.0 };   /* 最小值 */
    double maxValue{ 0.0 };   /* 最大值 */
    double initValue{ 0.0 };  /* 初始化默认值 */
    int decimals{ -1 };       /* 精度,小于0则为默认值 */
    bool limitValid{ false }; /* 最大值最小值是否有效,这个是自动赋值的 */
};
struct HexNumToWidgetInfo
{
    using type = std::integral_constant<DynamicConfigToWidgetType, DynamicConfigToWidgetType::HexNum>;
    quint64 initValue{ 0 };  /* 初始化默认值 */
    int length{ -1 };        /* 数据长度,小于等于0则为默认值 */
    QString placeholderText; /* 占位符字符 */
};
struct HexStrToWidgetInfo  // 这个可以认为是TextLine的正则约束版本
{
    using type = std::integral_constant<DynamicConfigToWidgetType, DynamicConfigToWidgetType::HexStr>;
    QString initValue;       /* 初始化默认值 */
    QString placeholderText; /* 占位符字符 */
    int length{ -1 };        /* 最大长度 */
};
struct IPToWidgetInfo  // 这个可以认为是TextLine的正则约束版本
{
    using type = std::integral_constant<DynamicConfigToWidgetType, DynamicConfigToWidgetType::IP>;
    QString initValue;       /* 初始化默认值 */
    QString placeholderText; /* 占位符字符 */
};
struct LabelToWidgetInfo  // 只读文本
{
    using type = std::integral_constant<DynamicConfigToWidgetType, DynamicConfigToWidgetType::Label>;
    QString initValue; /* 初始化默认值 */
};

Q_DECLARE_METATYPE(CommonToWidgetInfo)
Q_DECLARE_METATYPE(DynamicWidgetInfo)
Q_DECLARE_METATYPE(EnumToWidgetInfo)
Q_DECLARE_METATYPE(TextLineToWidgetInfo)
Q_DECLARE_METATYPE(TextMultipleToWidgetInfo)
Q_DECLARE_METATYPE(IntegerToWidgetInfo)
Q_DECLARE_METATYPE(DoubleToWidgetInfo)
Q_DECLARE_METATYPE(HexNumToWidgetInfo)
Q_DECLARE_METATYPE(HexStrToWidgetInfo)
Q_DECLARE_METATYPE(IPToWidgetInfo)
Q_DECLARE_METATYPE(LabelToWidgetInfo)
#endif  // DYNAMICCONFIGTOWIDGETDEFINE_H
