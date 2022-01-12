#ifndef WHEXLINEEDIT_H
#define WHEXLINEEDIT_H

#include "DevProtocol.h"
#include "WWidget.h"

class WHexLineEditImpl;
class WHexLineEdit : public WWidget
{
    Q_OBJECT

public:
    explicit WHexLineEdit(QWidget* parent = nullptr);
    ~WHexLineEdit() override;

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
    WHexLineEditImpl* m_impl;
};

#endif  // WHEXLINEEDIT_H
