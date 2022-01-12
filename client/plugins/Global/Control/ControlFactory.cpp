#include "ControlFactory.h"

#include "WComboBox.h"
#include "WComboBoxOnly.h"
#include "WConverterWdBm.h"
#include "WDateEdit.h"
#include "WDateTimeEdit.h"
#include "WHexLineEdit.h"
#include "WHexLineEditOnly.h"
#include "WHexStringEdit.h"
#include "WIPLineEdit.h"
#include "WLED.h"
#include "WMatrixWidget.h"
#include "WNumber.h"
#include "WNumberOnly.h"
#include "WPlainTextEdit.h"
#include "WStatusCombobox.h"
#include "WStatusLabel.h"
#include "WStringEdit.h"
#include "WTimeEdit.h"
#include "WTracksNumberLineEdit.h"

template<DisplayFormat format>
struct FormatToSet
{
};
template<>
struct FormatToSet<DisplayFormat_Enum>
{
    using type = WComboBox;
};
template<>
struct FormatToSet<DisplayFormat_UInt8>
{
    using type = WUNumber;
};
template<>
struct FormatToSet<DisplayFormat_UInt16>
{
    using type = WUNumber;
};
template<>
struct FormatToSet<DisplayFormat_UInt32>
{
    using type = WUNumber;
};
template<>
struct FormatToSet<DisplayFormat_UInt64>
{
    using type = WUNumber;
};
template<>
struct FormatToSet<DisplayFormat_Int8>
{
    using type = WINumber;
};
template<>
struct FormatToSet<DisplayFormat_Int16>
{
    using type = WINumber;
};
template<>
struct FormatToSet<DisplayFormat_Int32>
{
    using type = WINumber;
};
template<>
struct FormatToSet<DisplayFormat_Int64>
{
    using type = WINumber;
};
template<>
struct FormatToSet<DisplayFormat_Float>
{
    using type = WDouble;
};
template<>
struct FormatToSet<DisplayFormat_Double>
{
    using type = WDouble;
};
template<>
struct FormatToSet<DisplayFormat_Hex>
{
    using type = WHexLineEdit;
};
template<>
struct FormatToSet<DisplayFormat_HexString>
{
    using type = WHexStringEdit;
};
template<>
struct FormatToSet<DisplayFormat_String>
{
    using type = WStringEdit;
};
template<>
struct FormatToSet<DisplayFormat_Bit>
{
    using type = WWidgetEmpty;
};
template<>
struct FormatToSet<DisplayFormat_IP>
{
    using type = WIPLineEdit;
};
template<>
struct FormatToSet<DisplayFormat_StrIP>
{
    using type = WIPLineEdit;
};
template<>
struct FormatToSet<DisplayFormat_AccumDate>
{
    using type = WDateEdit;
};
template<>
struct FormatToSet<DisplayFormat_BCDDate>
{
    using type = WDateEdit;
};
template<>
struct FormatToSet<DisplayFormat_BCDTime>
{
    using type = WTimeEdit;
};
template<>
struct FormatToSet<DisplayFormat_StrDate>
{
    using type = WDateEdit;
};
template<>
struct FormatToSet<DisplayFormat_StrTime>
{
    using type = WTimeEdit;
};
template<>
struct FormatToSet<DisplayFormat_StrDateTime>
{
    using type = WDateTimeEdit;
};
template<>
struct FormatToSet<DisplayFormat_WdBm>
{
    using type = WConverterWdBm;
};
template<>
struct FormatToSet<DisplayFormat_TracksNumber>
{
    using type = WTracksNumberLineEdit;
};

template<DisplayFormat format>
struct FormatToQuery
{
};
template<>
struct FormatToQuery<DisplayFormat_Enum>
{
    using type = WStatusCombobox;
};
template<>
struct FormatToQuery<DisplayFormat_UInt8>
{
    using type = WStatusLabel;
};
template<>
struct FormatToQuery<DisplayFormat_UInt16>
{
    using type = WStatusLabel;
};
template<>
struct FormatToQuery<DisplayFormat_UInt32>
{
    using type = WStatusLabel;
};
template<>
struct FormatToQuery<DisplayFormat_UInt64>
{
    using type = WStatusLabel;
};
template<>
struct FormatToQuery<DisplayFormat_Int8>
{
    using type = WStatusLabel;
};
template<>
struct FormatToQuery<DisplayFormat_Int16>
{
    using type = WStatusLabel;
};
template<>
struct FormatToQuery<DisplayFormat_Int32>
{
    using type = WStatusLabel;
};
template<>
struct FormatToQuery<DisplayFormat_Int64>
{
    using type = WStatusLabel;
};
template<>
struct FormatToQuery<DisplayFormat_Float>
{
    using type = WStatusLabel;
};
template<>
struct FormatToQuery<DisplayFormat_Double>
{
    using type = WStatusLabel;
};
template<>
struct FormatToQuery<DisplayFormat_Hex>
{
    using type = WStatusLabel;
};
template<>
struct FormatToQuery<DisplayFormat_HexString>
{
    using type = WStatusLabel;
};
template<>
struct FormatToQuery<DisplayFormat_String>
{
    using type = WStatusLabel;
};
template<>
struct FormatToQuery<DisplayFormat_LED>
{
    using type = WLED;
};
template<>
struct FormatToQuery<DisplayFormat_Bit>
{
    using type = WWidgetEmpty;
};
template<>
struct FormatToQuery<DisplayFormat_IP>
{
    using type = WStatusLabel;
};
template<>
struct FormatToQuery<DisplayFormat_StrIP>
{
    using type = WStatusLabel;
};
template<>
struct FormatToQuery<DisplayFormat_AccumDate>
{
    using type = WStatusLabel;
};
template<>
struct FormatToQuery<DisplayFormat_BCDDate>
{
    using type = WStatusLabel;
};
template<>
struct FormatToQuery<DisplayFormat_BCDTime>
{
    using type = WStatusLabel;
};
template<>
struct FormatToQuery<DisplayFormat_StrDate>
{
    using type = WStatusLabel;
};
template<>
struct FormatToQuery<DisplayFormat_StrTime>
{
    using type = WStatusLabel;
};
template<>
struct FormatToQuery<DisplayFormat_StrDateTime>
{
    using type = WStatusLabel;
};
template<>
struct FormatToQuery<DisplayFormat_WdBm>
{
    using type = WConverterWdBm;
};
template<>
struct FormatToQuery<DisplayFormat_TracksNumber>
{
    using type = WStatusLabel;
};

WWidget* ControlFactory::createStatusControl(const ParameterAttribute& attr, const QMap<QString, QVariant>& map)
{
    auto widget = createStatusTypeControl(attr.displayFormat);
    if (widget != nullptr)
    {
        widget->setAttr(attr, map);
    }
    return widget;
}

WWidget* ControlFactory::createStatusTypeControl(DisplayFormat displayFormat)
{
    WWidget* widget = nullptr;
    switch (displayFormat)
    {
    case DisplayFormat_Enum:
    {
        widget = new typename FormatToQuery<DisplayFormat_Enum>::type;
        break;
    }
    case DisplayFormat_Int8:
    {
        widget = new typename FormatToQuery<DisplayFormat_Int8>::type;
        break;
    }
    case DisplayFormat_UInt8:
    {
        widget = new typename FormatToQuery<DisplayFormat_UInt8>::type;
        break;
    }
    case DisplayFormat_Int16:
    {
        widget = new typename FormatToQuery<DisplayFormat_Int16>::type;
        break;
    }
    case DisplayFormat_UInt16:
    {
        widget = new typename FormatToQuery<DisplayFormat_UInt16>::type;
        break;
    }
    case DisplayFormat_Int32:
    {
        widget = new typename FormatToQuery<DisplayFormat_Int32>::type;
        break;
    }
    case DisplayFormat_UInt32:
    {
        widget = new typename FormatToQuery<DisplayFormat_UInt32>::type;
        break;
    }
    case DisplayFormat_Float:
    {
        widget = new typename FormatToQuery<DisplayFormat_Float>::type;
        break;
    }
    case DisplayFormat_Double:
    {
        widget = new typename FormatToQuery<DisplayFormat_Double>::type;
        break;
    }
    case DisplayFormat_Int64:
    {
        widget = new typename FormatToQuery<DisplayFormat_Int64>::type;
        break;
    }
    case DisplayFormat_UInt64:
    {
        widget = new typename FormatToQuery<DisplayFormat_UInt64>::type;
        break;
    }
    case DisplayFormat_String:
    {
        widget = new typename FormatToQuery<DisplayFormat_String>::type;
        break;
    }
    case DisplayFormat_LED:
    {
        widget = new typename FormatToQuery<DisplayFormat_LED>::type;
        break;
    }
    case DisplayFormat_Hex:
    {
        widget = new typename FormatToQuery<DisplayFormat_Hex>::type;
        break;
    }
    case DisplayFormat_HexString:
    {
        widget = new typename FormatToQuery<DisplayFormat_HexString>::type;
        break;
    }
    case DisplayFormat_Bit:
    {
        widget = new typename FormatToQuery<DisplayFormat_Bit>::type;
        break;
    }
    case DisplayFormat_IP:
    {
        widget = new typename FormatToQuery<DisplayFormat_IP>::type;
        break;
    }
    case DisplayFormat_StrIP:
    {
        widget = new typename FormatToQuery<DisplayFormat_StrIP>::type;
        break;
    }
    case DisplayFormat_AccumDate:
    {
        widget = new typename FormatToQuery<DisplayFormat_AccumDate>::type;
        break;
    }
    case DisplayFormat_BCDDate:
    {
        widget = new typename FormatToQuery<DisplayFormat_BCDDate>::type;
        break;
    }
    case DisplayFormat_BCDTime:
    {
        widget = new typename FormatToQuery<DisplayFormat_BCDTime>::type;
        break;
    }
    case DisplayFormat_StrDate:
    {
        widget = new typename FormatToQuery<DisplayFormat_StrDate>::type;
        break;
    }
    case DisplayFormat_StrTime:
    {
        widget = new typename FormatToQuery<DisplayFormat_StrTime>::type;
        break;
    }
    case DisplayFormat_StrDateTime:
    {
        widget = new typename FormatToQuery<DisplayFormat_StrDateTime>::type;
        break;
    }
    case DisplayFormat_WdBm:
    {
        widget = new typename FormatToQuery<DisplayFormat_WdBm>::type;
        break;
    }
    case DisplayFormat_TracksNumber:
    {
        widget = new typename FormatToQuery<DisplayFormat_TracksNumber>::type;
        break;
    }
    case DisplayFormat_Invalid:
    {
        widget = nullptr;
        break;
    }
    }
    return widget;
}

WWidget* ControlFactory::createSetControl(const ParameterAttribute& attr, const QMap<QString, QVariant>& map)
{
    auto widget = createSetTypeControl(attr.displayFormat);
    if (widget != nullptr)
    {
        widget->setAttr(attr, map);
    }
    return widget;
}

WWidget* ControlFactory::createSetTypeControl(DisplayFormat displayFormat)
{
    WWidget* widget = nullptr;
    switch (displayFormat)
    {
    case DisplayFormat_Enum:
    {
        widget = new typename FormatToSet<DisplayFormat_Enum>::type;
        break;
    }
    case DisplayFormat_Int8:
    {
        widget = new typename FormatToSet<DisplayFormat_Int8>::type;
        break;
    }
    case DisplayFormat_UInt8:
    {
        widget = new typename FormatToSet<DisplayFormat_UInt8>::type;
        break;
    }
    case DisplayFormat_Int16:
    {
        widget = new typename FormatToSet<DisplayFormat_Int16>::type;
        break;
    }
    case DisplayFormat_UInt16:
    {
        widget = new typename FormatToSet<DisplayFormat_UInt16>::type;
        break;
    }
    case DisplayFormat_Int32:
    {
        widget = new typename FormatToSet<DisplayFormat_Int32>::type;
        break;
    }
    case DisplayFormat_UInt32:
    {
        widget = new typename FormatToSet<DisplayFormat_UInt32>::type;
        break;
    }
    case DisplayFormat_Float:
    {
        widget = new typename FormatToSet<DisplayFormat_Float>::type;
        break;
    }
    case DisplayFormat_Double:
    {
        widget = new typename FormatToSet<DisplayFormat_Double>::type;
        break;
    }
    case DisplayFormat_Int64:
    {
        widget = new typename FormatToSet<DisplayFormat_Int64>::type;
        break;
    }
    case DisplayFormat_UInt64:
    {
        widget = new typename FormatToSet<DisplayFormat_UInt64>::type;
        break;
    }
    case DisplayFormat_String:
    {
        widget = new typename FormatToSet<DisplayFormat_String>::type;
        break;
    }
    case DisplayFormat_Hex:
    {
        widget = new typename FormatToSet<DisplayFormat_Hex>::type;
        break;
    }
    case DisplayFormat_HexString:
    {
        widget = new typename FormatToSet<DisplayFormat_HexString>::type;
        break;
    }
    case DisplayFormat_Bit:
    {
        widget = new typename FormatToSet<DisplayFormat_Bit>::type;
        break;
    }
    case DisplayFormat_IP:
    {
        widget = new typename FormatToSet<DisplayFormat_IP>::type;
        break;
    }
    case DisplayFormat_StrIP:
    {
        widget = new typename FormatToSet<DisplayFormat_StrIP>::type;
        break;
    }
    case DisplayFormat_AccumDate:
    {
        widget = new typename FormatToSet<DisplayFormat_AccumDate>::type;
        break;
    }
    case DisplayFormat_BCDDate:
    {
        widget = new typename FormatToSet<DisplayFormat_BCDDate>::type;
        break;
    }
    case DisplayFormat_BCDTime:
    {
        widget = new typename FormatToSet<DisplayFormat_BCDTime>::type;
        break;
    }
    case DisplayFormat_StrDate:
    {
        widget = new typename FormatToSet<DisplayFormat_StrDate>::type;
        break;
    }
    case DisplayFormat_StrTime:
    {
        widget = new typename FormatToSet<DisplayFormat_StrTime>::type;
        break;
    }
    case DisplayFormat_StrDateTime:
    {
        widget = new typename FormatToSet<DisplayFormat_StrDateTime>::type;
        break;
    }
    case DisplayFormat_WdBm:
    {
        widget = new typename FormatToSet<DisplayFormat_WdBm>::type;
        break;
    }
    case DisplayFormat_TracksNumber:
    {
        widget = new typename FormatToSet<DisplayFormat_TracksNumber>::type;
        break;
    }
    case DisplayFormat_Invalid:
    {
        widget = nullptr;
        break;
    }
    }
    return widget;
}

WWidget* ControlFactory::createStatusControlOrder(const ParameterAttribute& attr, const QMap<int, QPair<QString, QVariant>>& map)
{
    auto widget = createStatusTypeControl(attr.displayFormat);
    if (widget != nullptr)
    {
        widget->setAttrOrder(attr, map);
    }
    return widget;
}

WWidget* ControlFactory::createSetControlOrder(const ParameterAttribute& attr, const QMap<int, QPair<QString, QVariant>>& map)
{
    auto widget = createSetTypeControl(attr.displayFormat);
    if (widget != nullptr)
    {
        widget->setAttrOrder(attr, map);
    }
    return widget;
}

WWidget* ControlFactory::createSetControlOnly(const ParameterAttribute& attr, const QMap<QString, QVariant>& map)
{
    WWidget* widget = nullptr;
    switch (attr.displayFormat)
    {
    case DisplayFormat_Hex:
    {
        widget = new WHexLineEditOnly();
    }
    break;
    case DisplayFormat_Double:
    {
        widget = new WDoubleOnly();
    }
    break;
    case DisplayFormat_UInt8:
    case DisplayFormat_UInt16:
    case DisplayFormat_UInt32:
    case DisplayFormat_UInt64:
    {
        widget = new WUNumberOnly();
    }
    break;
    case DisplayFormat_Int8:
    case DisplayFormat_Int16:
    case DisplayFormat_Int32:
    case DisplayFormat_Int64:
    {
        widget = new WINumberOnly();
    }
    break;
    case DisplayFormat_Enum:
    {
        widget = new WComboBoxOnly();
    }
    break;
    default: break;
    }

    if (widget != nullptr)
    {
        widget->setAttr(attr, map);
    }

    return widget;
}

QWidget* ControlFactory::createMatrix(const int& unitCode, const QList<QPair<ParameterAttribute, QMap<int, QString>>>& attrEnumList)
{
    auto widget = new WMatrixWidget;
    widget->setMatrixInfo(attrEnumList);

    return widget;
}

QWidget* ControlFactory::createPlainText(const QVector<QString>& attr, CmdAttribute cmdAttribute)
{
    WPlainTextEdit* widget = new WPlainTextEdit;
    widget->init(attr, cmdAttribute);
    return widget;
}
