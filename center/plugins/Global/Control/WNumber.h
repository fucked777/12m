
#ifndef WLINEEDIT_H
#define WLINEEDIT_H

#include "DevProtocol.h"
#include "WWidget.h"

/* 无符号 */
template<typename T>
class WNumberImpl;
class WUNumber : public WWidget
{
    Q_OBJECT
public:
    explicit WUNumber(QWidget* parent = nullptr);
    ~WUNumber() override;
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
    void setValueStyleSheet(const QString& qss) override;
    void setNameWidth(int w) override;
    void setValueWidth(int w) override;
    void setControlHeight(int h) override;

public slots:
    void setEnabled(bool enable);
signals:
    void sig_valueChanged(const QVariant& value);  // htf 这个信号主要是为了动态生成界面而加的

private:
    /* 20200619 出现一个问题假设一个值的范围是 10-20
     * 你在输入框中添加或者删除一个字符时这个数字的范围就超出了预设的范围
     * 感觉像无法输入的样子
     * 所以不能再字符改变时判断
     * 需要在字符完成输入时判断
     */
    void editingFinished();
    void textChanged(const QString&);

private:
    WNumberImpl<quint64>* m_impl;
};

/*  有符号 */
class WINumber : public WWidget
{
    Q_OBJECT
public:
    explicit WINumber(QWidget* parent = nullptr);
    ~WINumber() override;
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
    void setValueStyleSheet(const QString& qss) override;
    void setNameWidth(int w) override;
    void setValueWidth(int w) override;
    void setControlHeight(int h) override;

public slots:
    void setEnabled(bool enable);

private:
    /* 20200619 出现一个问题假设一个值的范围是 10-20
     * 你在输入框中添加或者删除一个字符时这个数字的范围就超出了预设的范围
     * 感觉像无法输入的样子
     * 所以不能再字符改变时判断
     * 需要在字符完成输入时判断
     */
    void editingFinished();
    void textChanged(const QString&);

private:
    WNumberImpl<qint64>* m_impl;
};

class WDoubleImpl;
class WDouble : public WWidget
{
    Q_OBJECT
public:
    explicit WDouble(QWidget* parent = nullptr);
    ~WDouble() override;
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
    /* 20200619 出现一个问题假设一个值的范围是 10-20
     * 你在输入框中添加或者删除一个字符时这个数字的范围就超出了预设的范围
     * 感觉像无法输入的样子
     * 所以不能再字符改变时判断
     * 需要在字符完成输入时判断
     */
    void textChanged(const QString&);
    void editingFinished();

private:
    WNumberImpl<double>* m_impl;
};

#endif  // WLINEEDIT_H
