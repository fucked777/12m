#ifndef WLED_H
#define WLED_H

#include "WWidget.h"

// 状态灯控件
class WLEDImpl;
class WLED : public WWidget
{
    Q_OBJECT
public:
    explicit WLED(QWidget* parent = nullptr);
    ~WLED() override;
    void setAttr(const ParameterAttribute& attr, const QMap<QString, QVariant>& map) override;
    void setValue(const QVariant& value) override;
    QVariant value() override;
    QVariant value(bool& isValid) override;
    void setName(const QString&) override;
    void setNameWidth(int w) override;
    void setValueWidth(int w) override;
    void setControlHeight(int h) override;

private:
    WLEDImpl* m_impl;
};
#endif  // WLED_H
