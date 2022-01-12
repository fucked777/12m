#ifndef WCOMBOBOX_H
#define WCOMBOBOX_H

#include "WWidget.h"

class WComboBoxImpl;
class WComboBox : public WWidget
{
    Q_OBJECT

public:
    explicit WComboBox(QWidget* parent = nullptr);
    ~WComboBox() override;

    void setAttr(const ParameterAttribute& attr, const QMap<QString, QVariant>& map) override;
    void setAttrOrder(const ParameterAttribute& attr, const QMap<int, QPair<QString, QVariant>>& map) override;
    void addItem(const QVariant&, const QVariant&) override;
    void setItem(const QMap<QString, QVariant>&) override;
    void setItemOrder(const QMap<int, QPair<QString, QVariant>>& map) override;

    void resetValue() override;
    virtual void setValue(const QVariant& value) override;
    virtual QVariant value() override;
    QVariant value(bool& isValid) override;
    virtual QString text() override;
    void setName(const QString& name) override;
    void setParamStatus(DataBuilder::FormsStatus status) override;
    void setValueStyleSheet(const QString& qss) override;
    void setNameWidth(int w) override;
    void setValueWidth(int w) override;
    void setControlHeight(int h) override;

public slots:
    void setEnabled(bool enable);

private slots:
    void currentTextChanged(const QString& arg1);

private:
    WComboBoxImpl* m_impl;
};

#endif  // WCOMBOBOX_H
