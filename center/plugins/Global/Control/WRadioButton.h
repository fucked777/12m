
#ifndef WRADIOBUTTON_H
#define WRADIOBUTTON_H
#include "DevProtocol.h"
#include "WWidget.h"
#include <QWidget>

class QAbstractButton;
class WRadioButtonImpl;
class WRadioButton : public WWidget
{
    Q_OBJECT

public:
    explicit WRadioButton(QWidget *parent = nullptr);
    ~WRadioButton() override;

    void setAttr(const ParameterAttribute &attr, const QMap<QString, QVariant> &map) override;
    void setItem(const QMap<QString, QVariant> &itemMap) override;
    void resetValue() override;
    void setValue(const QVariant &value) override;
    QVariant value() override;
    QVariant value(bool &isValid) override;
    QString text() override;
    void setName(const QString &) override;
    void setParamStatus(DataBuilder::FormsStatus status) override;
    void setValueStyleSheet(const QString &qss) override;
    void setNameWidth(int w) override;
    void setValueWidth(int w) override;
    void setControlHeight(int h) override;

public:
    void setEnabled(bool enable);

private:
    void valueChange(QAbstractButton *);

private:
    WRadioButtonImpl *m_impl;
};

#endif  // WRADIOBUTTON_H
