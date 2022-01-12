#ifndef WHEXSTRINGEDIT_H
#define WHEXSTRINGEDIT_H

#include "DevProtocol.h"
#include "WWidget.h"
#include <QWidget>

// 十六进制字符串控件，解决范围超出最大数字类型的问题
class WHexStringEditImpl;
class WHexStringEdit : public WWidget
{
    Q_OBJECT

public:
    explicit WHexStringEdit(QWidget* parent = nullptr);
    ~WHexStringEdit() override;

    void setAttr(const ParameterAttribute& attr, const QMap<QString, QVariant>& /* map */) override;
    void resetValue() override;
    void setValue(const QVariant& value) override;
    QVariant value() override;
    QVariant value(bool& isValid) override;
    QString text() override;
    void setMinNum(const QVariant& minNum) override;
    void setMaxNum(const QVariant& maxNum) override;
    void setName(const QString&) override;
    void setUnit(const QString&) override;
    void setNumRange(const QVariant& minNum, const QVariant& maxNum) override;
    void setParamStatus(DataBuilder::FormsStatus status) override;
    void setValueStyleSheet(const QString& qss) override;
    void setNameWidth(int w) override;
    void setValueWidth(int w) override;
    void setControlHeight(int h) override;
    /* 用来补零 */
    void setMaxLength(int len) override;
    void setReadOnly(bool) override;

public slots:
    void setEnabled(bool enable);

private:
    void editingFinished();
    void textChanged(const QString&);

private:
    WHexStringEditImpl* m_impl;
};

#endif  // WHEXSTRINGEDIT_H
