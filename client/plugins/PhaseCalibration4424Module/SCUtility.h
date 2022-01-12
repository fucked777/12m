#ifndef SCUTILITY_H
#define SCUTILITY_H

#include <QFrame>
#include <QIcon>
#include <QLabel>
#include <functional>
#include <tuple>

struct WidgetValueInfo;
using SetStatusFunc = std::function<void(const QVariant&, WidgetValueInfo&)>;

struct WidgetValueInfo
{
    QString key;
    int unit{ 0 };
    QWidget* widget{ nullptr };
    SetStatusFunc setStatusFunc;
};

/* 设置widget值的 */
struct ExtensionStatusReportMessage;
class SetWidgetValue
{
public:
    static void setLabelText(const QVariant&, WidgetValueInfo&);
    static void setLineEditText(const QVariant&, WidgetValueInfo&);
    static void setDoubleLabelText(const QVariant&, WidgetValueInfo&);
    /* 状态灯 0灰 1红  2绿 */
    static void setLampGRG(const QVariant&, WidgetValueInfo&);
    /* 状态灯 0灰 1绿  2红 */
    static void setLampGGR(const QVariant&, WidgetValueInfo&);
    /* 状态灯 0灰 1红  2,3绿色 */
    static void setLampGRYG(const QVariant&, WidgetValueInfo&);
};

#define MACBE_CLEAN_ICO(a)                                                                                                                           \
    {                                                                                                                                                \
        MBasicParamStateMonitorWidget::resetLedStatus(ui->a##_1);                                                                                    \
        MBasicParamStateMonitorWidget::resetLedStatus(ui->a##_2);                                                                                    \
        MBasicParamStateMonitorWidget::resetLedStatus(ui->a##_3);                                                                                    \
        MBasicParamStateMonitorWidget::resetLedStatus(ui->a##_4);                                                                                    \
    }

#define SET_WIDGET_STYLE(className)                                                                                                                  \
    {                                                                                                                                                \
        static constexpr const char* qss = #className "{"                                                                                            \
                                                      "border:2px solid rgb(151, 172, 187);"                                                         \
                                                      "border-radius:5px;"                                                                           \
                                                      "}"                                                                                            \
                                                      ".QGroupBox{"                                                                                  \
                                                      "font: 10pt;"                                                                                  \
                                                      "border:1px solid #C0DCF2;"                                                                    \
                                                      "border-radius:5px;"                                                                           \
                                                      "margin-top:5px;"                                                                              \
                                                      "}"                                                                                            \
                                                      ".QGroupBox::title{"                                                                           \
                                                      "subcontrol-origin:margin;"                                                                    \
                                                      "subcontrol-position :top left;"                                                               \
                                                      "position:5 5px;"                                                                              \
                                                      "font-size:18pt;"                                                                              \
                                                      "left:10px;"                                                                                   \
                                                      "}";                                                                                           \
        setStyleSheet(qss);                                                                                                                          \
    }

class SCUtility
{
public:
    static void registerValueInfo(const QString& key, QWidget*, int unit, SetStatusFunc, QList<WidgetValueInfo>&);
    static WidgetValueInfo createValueInfo(const QString& key, QWidget*, int unit, SetStatusFunc);
    static void setDataS(const ExtensionStatusReportMessage& message, QList<WidgetValueInfo>& info);

public:
    static void resetLedStatus(QLabel*);
    static void bgBlackTextGreenLabel(QWidget*);
};

#endif  // SCUTILITY_H
