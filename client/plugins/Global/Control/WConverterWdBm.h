#ifndef WCONVERTERWDBM_H
#define WCONVERTERWDBM_H

#include "WWidget.h"

#include "ProtocolXmlTypeDefine.h"
#include "WWidget.h"
#include <QWidget>

// 瓦特(W)换算为功率   dBm = 30 + 10lgP
static double WdBmConversion(const double& value, bool direction)
{
    auto tempVal = value;
    if (direction)  // P => dBm
        tempVal = 30 + 10 * log10(value);
    else  // dBm => P
        tempVal = pow(10, (value - 30) / 10);

    return QString::number(tempVal, 'f', 1).toDouble();
}

class WConverterWdBmImpl;
class WConverterWdBm : public WWidget
{
    Q_OBJECT
public:
    explicit WConverterWdBm(QWidget* parent = nullptr);
    ~WConverterWdBm() override;
    void setAttr(const ParameterAttribute& attr, const QMap<QString, QVariant>& /* map */) override;

    void resetValue() override;
    void setValue(const QVariant& value) override;
    QVariant value() override;
    QVariant value(bool& isValid) override;
    QString text() override;
    void setMinNum(const QVariant& minNum) override;
    void setMaxNum(const QVariant& maxNum) override;
    void setNumRange(const QVariant& minNum, const QVariant& maxNum) override;
    void setParamStatus(DataBuilder::FormsStatus status) override;
    void setName(const QString&) override;
    void setUnit(const QString&) override;
    void setDecimal(int dec) override;
    void setValueStyleSheet(const QString& qss) override;
    void setNameWidth(int w) override;
    void setValueWidth(int w) override;
    void setControlHeight(int h) override;

public slots:
    void setEnabled(bool enable);
    void slotSwitchBtnClicked();

private:
    void textChanged(const QString&);
    void editingFinished();

private:
    WConverterWdBmImpl* m_impl;
};

#endif  // WCONVERTERWDBM_H
