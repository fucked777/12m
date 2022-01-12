#ifndef SRMUTILITY_H
#define SRMUTILITY_H
#include "StationResManagementSerialize.h"
#include <QObject>
#include <QRegExpValidator>
#include <QString>

class SRMUtility
{
public:
    template<typename UI>
    static void uiInit(UI* ui)
    {
        ui->isUsable->addItem("否", false);
        ui->isUsable->addItem("是", true);

        QRegExp regExp(Regexp::hex(8));
        ui->stationIdentifying->setValidator(new QRegExpValidator(regExp, ui->stationIdentifying));

        regExp.setPattern(Regexp::hex(8));
        ui->ttcUACAddr->setValidator(new QRegExpValidator(regExp, ui->ttcUACAddr));
        ui->dataTransUACAddr->setValidator(new QRegExpValidator(regExp, ui->dataTransUACAddr));

        ui->stationLongitude->setRange(-180, 180);
        ui->stationLatitude->setRange(-90, 90);
        ui->stationHeight->setRange(0, 10000);

        ui->stationLongitude->setToolTip("参数范围: -180~180");
        ui->stationLatitude->setToolTip("参数范围: -90~90");
        ui->stationHeight->setToolTip("参数范围: 0~10000");

        ui->stationLongitude->setDecimals(6);
        ui->stationLatitude->setDecimals(6);
        ui->stationHeight->setDecimals(6);
    }

    template<typename UI>
    static void setUIEnable(UI* ui, bool enable)
    {
        ui->stationIdentifying->setEnabled(enable);
        ui->stationName->setEnabled(enable);
        ui->ttcUACAddr->setEnabled(enable);
        ui->dataTransUACAddr->setEnabled(enable);
        ui->desc->setEnabled(enable);
        ui->stationLongitude->setEnabled(enable);
        ui->stationLatitude->setEnabled(enable);
        ui->stationHeight->setEnabled(enable);
        ui->isUsable->setEnabled(enable);
    }

    template<typename UI>
    static StationResManagementData getUIData(UI* ui)
    {
        StationResManagementData tempData;
        tempData.stationIdentifying = ui->stationIdentifying->text().trimmed();
        tempData.stationName = ui->stationName->text().trimmed();
        tempData.ttcUACAddr = ui->ttcUACAddr->text().trimmed();
        tempData.dataTransUACAddr = ui->dataTransUACAddr->text().trimmed();
        tempData.stationLongitude = ui->stationLongitude->value();
        tempData.stationLatitude = ui->stationLatitude->value();
        tempData.stationHeight = ui->stationHeight->value();
        tempData.isUsable = ui->isUsable->currentData().toBool();
        tempData.descText = ui->desc->toPlainText();

        return tempData;
    }

    template<typename UI>
    static void setUIData(UI* ui, const StationResManagementData& tempData)
    {
        ui->stationIdentifying->setText(tempData.stationIdentifying);
        ui->stationName->setText(tempData.stationName);
        ui->ttcUACAddr->setText(tempData.ttcUACAddr);
        ui->dataTransUACAddr->setText(tempData.dataTransUACAddr);
        ui->desc->setPlainText(tempData.descText);
        ui->stationLongitude->setValue(tempData.stationLongitude);
        ui->stationLatitude->setValue(tempData.stationLatitude);
        ui->stationHeight->setValue(tempData.stationHeight);
        ui->isUsable->setCurrentText(SRMUtility::isUsed2Desc(tempData.isUsable));
    }

    template<typename UI>
    static void clearUIData(UI* ui)
    {
        ui->stationIdentifying->clear();
        ui->stationName->clear();
        ui->ttcUACAddr->clear();
        ui->dataTransUACAddr->clear();
        ui->desc->clear();
        ui->stationLongitude->setValue(0.0);
        ui->stationLatitude->setValue(0.0);
        ui->stationHeight->setValue(0.0);
        ui->isUsable->setCurrentIndex(0);
    }

    static inline QString isUsed2Desc(bool value) { return value ? "是" : "否"; }
    /* 这两个函数的顺序要一致 modeHeader taskCenterData2Mode */
    static inline QStringList modeHeader()
    {
        QStringList headList;
        headList << QString("站标识") << QString("站名") << QString("站址经度(°)") << QString("站址纬度(°)") << QString("站址高度(m)")
                 << QString("测控UAC地址") << QString("数传UAC地址") << QString("是否可用") << QString("站描述");
        return headList;
    }
    static inline QStringList data2Mode(const StationResManagementData& value)
    {
        QStringList listValue;
        listValue << value.stationIdentifying << value.stationName << QString::number(value.stationLongitude, 'f')
                  << QString::number(value.stationLatitude, 'f') << QString::number(value.stationHeight, 'f') << value.ttcUACAddr
                  << value.dataTransUACAddr << isUsed2Desc(value.isUsable) << value.descText;
        return listValue;
    }
    template<typename T>
    static bool dataCompare(const T& data1, const T& data2)
    {
        return data1.stationIdentifying == data2.stationIdentifying;
    }
};

#endif  // DTCUTILITY_H
