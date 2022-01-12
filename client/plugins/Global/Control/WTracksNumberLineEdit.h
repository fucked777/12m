#ifndef WTRACKSNUMBERLINEEDIT_H
#define WTRACKSNUMBERLINEEDIT_H

#include "DevProtocol.h"
#include "WWidget.h"

// 轨道根数下发控件
template<typename T>
class WWTracksNumberLineEditImpl;
class WTracksNumberLineEdit : public WWidget
{
    Q_OBJECT
public:
    explicit WTracksNumberLineEdit(QWidget* parent = nullptr);
    ~WTracksNumberLineEdit() override;
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
    void setParamDesc(const QString& desc) override;
    void setUnit(const QString&) override;
    void setDecimal(int dec) override;
    void setValueStyleSheet(const QString& qss) override;
    void setNameWidth(int w) override;
    void setValueWidth(int w) override;
    void setControlHeight(int h) override;

public slots:
    void setEnabled(bool enable);

private:
    void textChanged(const QString&);
    void editingFinished();

private:
    WWTracksNumberLineEditImpl<double>* m_impl;
};
#endif  // WTRACKSNUMBERLINEEDIT_H
