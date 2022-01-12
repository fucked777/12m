#ifndef WSTRINGEDIT_H
#define WSTRINGEDIT_H

#include "WWidget.h"

// 字符串编辑控件
class WStringEditImpl;
class WStringEdit : public WWidget
{
    Q_OBJECT

public:
    explicit WStringEdit(QWidget* parent = nullptr);
    ~WStringEdit() override;

    void setAttr(const ParameterAttribute& attr, const QMap<QString, QVariant>& /* map */) override;
    void setValue(const QVariant& value) override;
    void resetValue() override;
    QVariant value() override;
    QVariant value(bool& isValid) override;
    QString text() override;
    void setName(const QString&) override;
    void setUnit(const QString&) override;
    /* 设置输入的最大长度 */
    void setMaxLength(int maxNum) override;
    void setParamStatus(DataBuilder::FormsStatus status) override;
    void setValueStyleSheet(const QString&) override;
    void setNameWidth(int w) override;
    void setValueWidth(int w) override;
    void setControlHeight(int h) override;

private:
    void updateWidgetShow();

public slots:
    void setEnabled(bool enable);
    void textChanged(const QString&);
    void editingFinished();

private:
    WStringEditImpl* m_impl;
};

#endif  // WSTRINGEDIT_H
