#ifndef EDUTILITY_H
#define EDUTILITY_H
#include "EphemerisDataSerialize.h"
#include "GlobalData.h"
#include "SatelliteManagementDefine.h"
#include <QDoubleSpinBox>
#include <QObject>
#include <QRegExpValidator>
#include <QString>

class EDUtility
{
public:
    template<typename UI>
    static void uiInit(UI* ui)
    {
        ui->satelliteIdentification->setEnabled(false);
        ui->satelliteName->setEnabled(false);

        ui->dateOfEpoch->setCorrectionMode(QDoubleSpinBox::CorrectToNearestValue);
        ui->dateOfEpoch->setCalendarPopup(true);
        ui->dateOfEpoch->setDisplayFormat("yyyy年MM月dd日");
        ui->dateOfEpoch->setDate(GlobalData::currentDate());

        ui->timeOfEpoch->setDisplayFormat("hh时mm分ss秒");
        ui->timeOfEpoch->setTime(GlobalData::currentTime());

        ui->semimajorAxis->setRange(0.0, 429496729.5);
        ui->eccentricity->setRange(0.0, 1.0);
        ui->dipAngle->setRange(0.0, 180.0);
        ui->rightAscension->setRange(0.0, 360.0);
        ui->argumentOfPerigee->setRange(0.0, 360.0);
        ui->meanAnomaly->setRange(0.0, 360.0);

        ui->semimajorAxis->setToolTip(QString("参数范围: 0~429496729.5"));
        ui->eccentricity->setToolTip(QString("参数范围: 0~1"));
        ui->dipAngle->setToolTip(QString("参数范围: 0~180"));
        ui->rightAscension->setToolTip(QString("参数范围: 0~360"));
        ui->argumentOfPerigee->setToolTip(QString("参数范围: 0~360"));
        ui->meanAnomaly->setToolTip(QString("参数范围: 0~360"));

        ui->semimajorAxis->setDecimals(5);
        ui->eccentricity->setDecimals(5);
        ui->dipAngle->setDecimals(5);
        ui->rightAscension->setDecimals(5);
        ui->argumentOfPerigee->setDecimals(5);
        ui->meanAnomaly->setDecimals(5);
    }
    template<typename UI>
    static void setUIEnable(UI* ui, bool enable)
    {
        ui->satellitCode->setEnabled(enable);            //卫星代号,任务代号
        ui->satelliteIdentification->setEnabled(false);  //卫星标识
        ui->satelliteName->setEnabled(false);            //卫星名称
        ui->dateOfEpoch->setEnabled(enable);             //瞬跟历元日期
        ui->timeOfEpoch->setEnabled(enable);             //瞬跟历元时刻
        ui->semimajorAxis->setEnabled(enable);           //半长轴
        ui->eccentricity->setEnabled(enable);            //偏心率
        ui->dipAngle->setEnabled(enable);                //倾角(度)
        ui->rightAscension->setEnabled(enable);          //升交点赤经(度)
        ui->argumentOfPerigee->setEnabled(enable);       //近地点幅角(度)
        ui->meanAnomaly->setEnabled(enable);             //平近点角(度)
    }

    /* 星历数据依赖卫星管理 */
    template<typename UI>
    static void setUIData(UI* ui, const QMap<QString, SatelliteManagementData>& smm, const EphemerisDataData& data)
    {
        ui->satellitCode->setCurrentText(data.satellitCode);
        if (smm.contains(data.satellitCode))
        {
            ui->satelliteIdentification->setText(smm[data.satellitCode].m_satelliteIdentification);
            ui->satelliteName->setText(smm[data.satellitCode].m_satelliteName);
        }
        ui->dateOfEpoch->setDate(data.dateOfEpoch);               //瞬跟历元日期
        ui->timeOfEpoch->setTime(data.timeOfEpoch);               //瞬跟历元时刻
        ui->semimajorAxis->setValue(data.semimajorAxis);          //半长轴
        ui->eccentricity->setValue(data.eccentricity);            //偏心率
        ui->dipAngle->setValue(data.dipAngle);                    //倾角(度)
        ui->rightAscension->setValue(data.rightAscension);        //升交点赤经(度)
        ui->argumentOfPerigee->setValue(data.argumentOfPerigee);  //近地点幅角(度)
        ui->meanAnomaly->setValue(data.meanAnomaly);              //平近点角(度)
    }
    template<typename UI>
    static void clearUIData(UI* ui)
    {
        ui->satellitCode->setCurrentIndex(0);
        ui->satelliteIdentification->clear();
        ui->satelliteName->clear();
        ui->dateOfEpoch->setDate(GlobalData::currentDate());  //瞬跟历元日期
        ui->timeOfEpoch->setTime(GlobalData::currentTime());  //瞬跟历元时刻
        ui->semimajorAxis->setValue(0.0);                     //半长轴
        ui->eccentricity->setValue(0.0);                      //偏心率
        ui->dipAngle->setValue(0.0);                          //倾角(度)
        ui->rightAscension->setValue(0.0);                    //升交点赤经(度)
        ui->argumentOfPerigee->setValue(0.0);                 //近地点幅角(度)
        ui->meanAnomaly->setValue(0.0);                       //平近点角(度)
    }

    template<typename UI>
    static EphemerisDataData getUIData(UI* ui)
    {
        EphemerisDataData tempData;
        tempData.satellitCode = ui->satellitCode->currentText();
        tempData.dateOfEpoch = ui->dateOfEpoch->date();
        tempData.timeOfEpoch = ui->timeOfEpoch->time();
        tempData.semimajorAxis = ui->semimajorAxis->value();
        tempData.eccentricity = ui->eccentricity->value();
        tempData.dipAngle = ui->dipAngle->value();
        tempData.rightAscension = ui->rightAscension->value();
        tempData.argumentOfPerigee = ui->argumentOfPerigee->value();
        tempData.meanAnomaly = ui->meanAnomaly->value();

        return tempData;
    }

    /* 这两个函数的顺序要一致 modeHeader taskCenterData2Mode */
    static inline QStringList modeHeader()
    {
        QStringList headList;
        headList << QString("任务代号") << QString("瞬跟历元日期") << QString("瞬跟历元时刻") << QString("半长轴") << QString("偏心率")
                 << QString("倾角(度)") << QString("升交点赤经(度)") << QString("近地点幅角(度)") << QString("平近点角(度)");
        return headList;
    }
    static inline QStringList data2Mode(const EphemerisDataData& value)
    {
        QStringList listValue;
        listValue << value.satellitCode << value.dateOfEpoch.toString("yyyy-MM-dd") << value.timeOfEpoch.toString("hh:mm:ss")
                  << QString::number(value.semimajorAxis, 'f', 5) << QString::number(value.eccentricity, 'f', 5)
                  << QString::number(value.dipAngle, 'f', 5) << QString::number(value.rightAscension, 'f', 5)
                  << QString::number(value.argumentOfPerigee, 'f', 5) << QString::number(value.meanAnomaly, 'f', 5);
        return listValue;
    }
    template<typename T>
    static bool dataCompare(const T& data1, const T& data2)
    {
        return data1.satellitCode == data2.satellitCode;
    }
};

#endif  // EDUTILITY_H
