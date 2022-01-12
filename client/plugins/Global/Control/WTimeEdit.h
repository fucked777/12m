#ifndef WTIMEEDIT_H
#define WTIMEEDIT_H

#include "WWidget.h"

// 时间编辑控件
class WTimeEditImpl;
class WTimeEdit : public WWidget
{
    Q_OBJECT

public:
    explicit WTimeEdit(QWidget* parent = nullptr);
    ~WTimeEdit() override;

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
    void timeChanged(const QTime& time);

private:
    WTimeEditImpl* m_impl;
};

#endif  // WTIMEEDIT_H
