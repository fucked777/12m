#ifndef WDATEEDIT_H
#define WDATEEDIT_H

#include "WWidget.h"

// 日期编辑控件
class WDateEditImpl;
class WDateEdit : public WWidget
{
    Q_OBJECT

public:
    explicit WDateEdit(QWidget* parent = nullptr);
    ~WDateEdit() override;

    void setAttr(const ParameterAttribute& attr, const QMap<QString, QVariant>& map) override;
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
    void dateChanged(const QDate& date);

private:
    WDateEditImpl* m_impl;
};
#endif  // WDATEEDIT_H
