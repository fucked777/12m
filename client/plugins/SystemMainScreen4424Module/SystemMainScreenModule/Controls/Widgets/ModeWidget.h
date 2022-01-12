#ifndef MODEWIDGET_H
#define MODEWIDGET_H

#include "BaseWidget.h"

// 设备模式控件
class ModeWidget : public BaseWidget
{
    Q_OBJECT
public:
    explicit ModeWidget(const QDomElement& domEle, QWidget* parent = nullptr);
    ~ModeWidget();

    // 获取模式id
    int getModeId() const;
    // 获取模式名称
    QString getModeName() const;

private:
    void initLayout();
    void parseNode(const QDomElement& domEle);

private:
    int mModeId = -1;
    QString mModeName;
};

#endif  // MODEWIDGET_H
