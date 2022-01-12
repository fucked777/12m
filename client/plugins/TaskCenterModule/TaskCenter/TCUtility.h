#ifndef TCUTILITY_H
#define TCUTILITY_H
#include "TaskCenterSerialize.h"
#include <QObject>
#include <QRegExpValidator>
#include <QString>

class TCUtility
{
public:
    template<typename UI>
    static void uiInit(UI* ui)
    {
        auto frameFormatInfo = TCConvert::tcffListInfo();
        for (auto& item : frameFormatInfo)
        {
            ui->frameFormatComboBox->addItem(item.desStr, item.str);
        }

        auto direTypeInfo = TCConvert::tcdcListInfo();
        for (auto& item : direTypeInfo)
        {
            ui->direTypeComboBox->addItem(item.desStr, item.str);
        }

        ui->isUsedLinkMonitorComboBox->addItem("否", false);
        ui->isUsedLinkMonitorComboBox->addItem("是", true);
        ui->isUsedComboBox->addItem("否", false);
        ui->isUsedComboBox->addItem("是", true);

        QRegExp regExp(Regexp::hex(6));
        QRegExp reg(Regexp::CodeString());  //中心代号  只能是6个数字 字母 _
        ui->centerCodeLineEdit->setMaxLength(6);
        ui->centerCodeLineEdit->setValidator(new QRegExpValidator(reg, ui->centerCodeLineEdit));

        regExp.setPattern(Regexp::hex(8));
        ui->UACLineEdit->setValidator(new QRegExpValidator(regExp, ui->UACLineEdit));
    }
    template<typename UI>
    static void setUIEnable(UI* ui, bool enable)
    {
        ui->centerNameLineEdit->setEnabled(enable);
        ui->centerCodeLineEdit->setEnabled(enable);
        ui->frameFormatComboBox->setEnabled(enable);
        ui->direTypeComboBox->setEnabled(enable);
        ui->isUsedLinkMonitorComboBox->setEnabled(enable);
        ui->isUsedComboBox->setEnabled(enable);
        ui->UACLineEdit->setEnabled(enable);
    }

    template<typename UI>
    static void setUIData(UI* ui, const TaskCenterData& data)
    {
        ui->centerNameLineEdit->setText(data.centerName);
        ui->centerCodeLineEdit->setText(data.centerCode);
        ui->frameFormatComboBox->setCurrentText(TCConvert::tcffToDesStr(data.frameFormat));
        ui->direTypeComboBox->setCurrentText(TCConvert::tcdcToDesStr(data.direType));
        ui->isUsedLinkMonitorComboBox->setCurrentText(TCUtility::isUsedLinkMonitor2Desc(data.isUsedLinkMonitor));
        ui->isUsedComboBox->setCurrentText(TCUtility::isUsed2Desc(data.isUsed));
        ui->UACLineEdit->setText(data.uacAddr);  // 描述
    }
    template<typename UI>
    static void clearUIData(UI* ui)
    {
        ui->centerNameLineEdit->clear();
        ui->centerCodeLineEdit->clear();
        ui->frameFormatComboBox->setCurrentIndex(0);
        ui->direTypeComboBox->setCurrentIndex(0);
        ui->isUsedLinkMonitorComboBox->setCurrentIndex(0);
        ui->isUsedComboBox->setCurrentIndex(0);
        ui->UACLineEdit->clear();
    }

    template<typename UI>
    static TaskCenterData getUIData(UI* ui)
    {
        TaskCenterData tempData;
        tempData.centerName = ui->centerNameLineEdit->text().trimmed();
        tempData.centerCode = ui->centerCodeLineEdit->text().trimmed();
        tempData.frameFormat = TCConvert::tcffFromStr(ui->frameFormatComboBox->currentData().toString());
        tempData.direType = TCConvert::tcdcFromStr(ui->direTypeComboBox->currentData().toString());
        tempData.isUsedLinkMonitor = ui->isUsedLinkMonitorComboBox->currentData().toBool();
        tempData.isUsed = ui->isUsedComboBox->currentData().toBool();
        tempData.uacAddr = ui->UACLineEdit->text().trimmed();
        return tempData;
    }

    static inline QString isUsedLinkMonitor2Desc(bool value) { return value ? "是" : "否"; }
    static inline QString isUsed2Desc(bool value) { return value ? "是" : "否"; }

    /* 这两个函数的顺序要一致 modeHeader taskCenterData2Mode */
    static inline QStringList modeHeader()
    {
        QStringList headList;
        headList << QString("中心名称") << QString("帧格式") << QString("方向类型") << QString("是否发链监") << QString("是否使用")
                 << QString("UAC(Hex)");
        return headList;
    }
    static inline QStringList data2Mode(const TaskCenterData& value)
    {
        QStringList listValue;
        listValue << value.centerName << TCConvert::tcffToDesStr(value.frameFormat) << TCConvert::tcdcToDesStr(value.direType)
                  << isUsedLinkMonitor2Desc(value.isUsedLinkMonitor) << isUsed2Desc(value.isUsed) << value.uacAddr;
        return listValue;
    }
    template<typename T>
    static bool dataCompare(const T& data1, const T& data2)
    {
        return data1.uacAddr == data2.uacAddr;
    }
};

#endif  // TCUTILITY_H
