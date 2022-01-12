#ifndef DMPARAMINPUTORDISPLAY_H
#define DMPARAMINPUTORDISPLAY_H
#include "DMMessageDefine.h"
#include "Utility.h"
#include <QWidget>

struct DynamicWidgetInfo;
struct UIDataParameter;
class DMParamInputOrDisplayImpl;
class DMParamInputOrDisplay : public QWidget
{
    Q_OBJECT
public:
    explicit DMParamInputOrDisplay(const UIDataParameter& parameter, QWidget* parent, bool hideEmptyAttr);
    ~DMParamInputOrDisplay();
    void setFromOnlyOneDisable();
    void setReadOnly();
    Optional<DMDataItem> getUIValue() const;  /* 获取当前ui交互的数据 */
    void setUIValue(const DMDataItem& value); /* 设置当前ui交互的数据 */
    QString type() const;
    bool dataExist(const QString& devID) const;
    void clearInput();
    QList<DynamicWidgetInfo> fromWidgetInfo() const;

private:
    void groupAddRemove();
    void refreshAttr(uint channelNum);

private:
    DMParamInputOrDisplayImpl* m_impl;
};

#endif  // DMPARAMINPUTORDISPLAY_H
