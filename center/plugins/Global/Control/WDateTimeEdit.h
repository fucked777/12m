#ifndef WSTRDATETIMEEDIT_H
#define WDATETIMEEDIT_H

#include "WWidget.h"
#include <QWidget>

// 日期时间编辑控件
class WDateTimeEditImpl;
class WDateTimeEdit : public WWidget
{
    Q_OBJECT

public:
    explicit WDateTimeEdit(QWidget* parent = nullptr);
    ~WDateTimeEdit() override;

    void setAttr(const ParameterAttribute& attr, const QMap<QString, QVariant>& /* map */) override;
    void setValue(const QVariant& value);
    QVariant value() override;
    QVariant value(bool& isValid) override;
    QString text() override;
    void setName(const QString&) override;
    void setParamStatus(DataBuilder::FormsStatus status) override;
    void setValueStyleSheet(const QString& qss) override;
    void setNameWidth(int w) override;
    void setValueWidth(int w) override;
    void setControlHeight(int h) override;

    void setDisplayFormat(const QString& format);

public slots:
    void setEnabled(bool enable);

private slots:
    void dateTimeChanged(const QDateTime& datetime);

private:
    WDateTimeEditImpl* m_impl;
};
#endif  // WDATETIMEEDIT_H
