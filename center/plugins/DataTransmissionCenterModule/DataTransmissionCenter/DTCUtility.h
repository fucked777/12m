#ifndef DTCUTILITY_H
#define DTCUTILITY_H
#include "DataTransmissionCenterSerialize.h"
#include <QObject>
#include <QRegExpValidator>
#include <QString>

class DTCUtility
{
public:
    template<typename UI>
    static void uiInit(UI* ui)
    {
        QRegExp regExp(Regexp::ip());
        ui->centerIP->setValidator(new QRegExpValidator(regExp, ui->centerIP));

        regExp.setPattern(Regexp::hex(8));
        ui->centerXS->setValidator(new QRegExpValidator(regExp, ui->centerXS));

        regExp.setPattern(Regexp::englishChar(40));
        ui->centerName->setValidator(new QRegExpValidator(regExp, ui->centerName));

        ui->plaintextDestPortPDXP->setRange(1, 65535);      // PDXP明文目的端口
        ui->ciphertextDestPortPDXP->setRange(1, 65535);     // PDXP密文目的端口
        ui->plaintextSrcPortPDXP->setRange(1, 65535);       // PDXP明文源端口
        ui->ciphertextSrcPortPDXP->setRange(1, 65535);      // PDXP密文源端口
        ui->plaintextDestPortFep->setRange(1, 65535);       // FEP明文目的端口
        ui->ciphertextDestPortFep->setRange(1, 65535);      // FEP密文目的端口
        ui->plaintextSrcPortFep->setRange(1, 65535);        // FEP明文源端口
        ui->ciphertextSrcPortFep->setRange(1, 65535);       // FEP密文源端口
        ui->plaintextDestPortFepRECP->setRange(1, 65535);   // FEP/RECP明文目的端口
        ui->ciphertextDestPortFepRECP->setRange(1, 65535);  // FEP/RECP密文目的端口
        ui->plaintextSrcPortFepRECP->setRange(1, 65535);    // FEP/RECP明文源端口
        ui->ciphertextSrcPortFepRECP->setRange(1, 65535);   // FEP/RECP密文源端口

        ui->plaintextDestPortPDXP->setToolTip("参数范围: 1~65535");
        ui->ciphertextDestPortPDXP->setToolTip("参数范围: 1~65535");     // PDXP密文目的端口
        ui->plaintextSrcPortPDXP->setToolTip("参数范围: 1~65535");       // PDXP明文源端口
        ui->ciphertextSrcPortPDXP->setToolTip("参数范围: 1~65535");      // PDXP密文源端口
        ui->plaintextDestPortFep->setToolTip("参数范围: 1~65535");       // FEP明文目的端口
        ui->ciphertextDestPortFep->setToolTip("参数范围: 1~65535");      // FEP密文目的端口
        ui->plaintextSrcPortFep->setToolTip("参数范围: 1~65535");        // FEP明文源端口
        ui->ciphertextSrcPortFep->setToolTip("参数范围: 1~65535");       // FEP密文源端口
        ui->plaintextDestPortFepRECP->setToolTip("参数范围: 1~65535");   // FEP/RECP明文目的端口
        ui->ciphertextDestPortFepRECP->setToolTip("参数范围: 1~65535");  // FEP/RECP密文目的端口
        ui->plaintextSrcPortFepRECP->setToolTip("参数范围: 1~65535");    // FEP/RECP明文源端口
        ui->ciphertextSrcPortFepRECP->setToolTip("参数范围: 1~65535");   // FEP/RECP密文源端口
    }

    template<typename UI>
    static void setUIEnable(UI* ui, bool enable)
    {
        ui->centerName->setEnabled(enable);                 // 中心名称
        ui->centerIP->setEnabled(enable);                   // 中心IP地址
        ui->centerXS->setEnabled(enable);                   // 中心信宿
        ui->plaintextDestPortPDXP->setEnabled(enable);      // PDXP明文目的端口
        ui->ciphertextDestPortPDXP->setEnabled(enable);     // PDXP密文目的端口
        ui->plaintextSrcPortPDXP->setEnabled(enable);       // PDXP明文源端口
        ui->ciphertextSrcPortPDXP->setEnabled(enable);      // PDXP密文源端口
        ui->plaintextDestPortFep->setEnabled(enable);       // FEP明文目的端口
        ui->ciphertextDestPortFep->setEnabled(enable);      // FEP密文目的端口
        ui->plaintextSrcPortFep->setEnabled(enable);        // FEP明文源端口
        ui->ciphertextSrcPortFep->setEnabled(enable);       // FEP密文源端口
        ui->plaintextDestPortFepRECP->setEnabled(enable);   // FEP/RECP明文目的端口
        ui->ciphertextDestPortFepRECP->setEnabled(enable);  // FEP/RECP密文目的端口
        ui->plaintextSrcPortFepRECP->setEnabled(enable);    // FEP/RECP明文源端口
        ui->ciphertextSrcPortFepRECP->setEnabled(enable);   // FEP/RECP密文源端口
        ui->desc->setEnabled(enable);                       // 描述
    }

    template<typename UI>
    static DataTransmissionCenterData getUIData(UI* ui)
    {
        DataTransmissionCenterData tempData;
        tempData.centerName = ui->centerName->text().trimmed();                       // 中心名称
        tempData.centerIP = ui->centerIP->text().trimmed();                           // 中心IP地址
        tempData.centerXS = ui->centerXS->text().trimmed();                           // 中心信宿
        tempData.plaintextDestPortPDXP = ui->plaintextDestPortPDXP->value();          // PDXP明文目的端口
        tempData.ciphertextDestPortPDXP = ui->ciphertextDestPortPDXP->value();        // PDXP密文目的端口
        tempData.plaintextSrcPortPDXP = ui->plaintextSrcPortPDXP->value();            // PDXP明文源端口
        tempData.ciphertextSrcPortPDXP = ui->ciphertextSrcPortPDXP->value();          // PDXP密文源端口
        tempData.plaintextDestPortFep = ui->plaintextDestPortFep->value();            // FEP明文目的端口
        tempData.ciphertextDestPortFep = ui->ciphertextDestPortFep->value();          // FEP密文目的端口
        tempData.plaintextSrcPortFep = ui->plaintextSrcPortFep->value();              // FEP明文源端口
        tempData.ciphertextSrcPortFep = ui->ciphertextSrcPortFep->value();            // FEP密文源端口
        tempData.plaintextDestPortFepRECP = ui->plaintextDestPortFepRECP->value();    // FEP/RECP明文目的端口
        tempData.ciphertextDestPortFepRECP = ui->ciphertextDestPortFepRECP->value();  // FEP/RECP密文目的端口
        tempData.plaintextSrcPortFepRECP = ui->plaintextSrcPortFepRECP->value();      // FEP/RECP明文源端口
        tempData.ciphertextSrcPortFepRECP = ui->ciphertextSrcPortFepRECP->value();    // FEP/RECP密文源端口
        tempData.desc = ui->desc->toPlainText();                                      // 描述
        return tempData;
    }

    template<typename UI>
    static void setUIData(UI* ui, const DataTransmissionCenterData& tempData)
    {
        ui->centerName->setText(tempData.centerName);                                 // 中心名称
        ui->centerIP->setText(tempData.centerIP);                                     // 中心IP地址
        ui->centerXS->setText(tempData.centerXS);                                     // 中心信宿
        ui->plaintextDestPortPDXP->setValue(tempData.plaintextDestPortPDXP);          // PDXP明文目的端口
        ui->ciphertextDestPortPDXP->setValue(tempData.ciphertextDestPortPDXP);        // PDXP密文目的端口
        ui->plaintextSrcPortPDXP->setValue(tempData.plaintextSrcPortPDXP);            // PDXP明文源端口
        ui->ciphertextSrcPortPDXP->setValue(tempData.ciphertextSrcPortPDXP);          // PDXP密文源端口
        ui->plaintextDestPortFep->setValue(tempData.plaintextDestPortFep);            // FEP明文目的端口
        ui->ciphertextDestPortFep->setValue(tempData.ciphertextDestPortFep);          // FEP密文目的端口
        ui->plaintextSrcPortFep->setValue(tempData.plaintextSrcPortFep);              // FEP明文源端口
        ui->ciphertextSrcPortFep->setValue(tempData.ciphertextSrcPortFep);            // FEP密文源端口
        ui->plaintextDestPortFepRECP->setValue(tempData.plaintextDestPortFepRECP);    // FEP/RECP明文目的端口
        ui->ciphertextDestPortFepRECP->setValue(tempData.ciphertextDestPortFepRECP);  // FEP/RECP密文目的端口
        ui->plaintextSrcPortFepRECP->setValue(tempData.plaintextSrcPortFepRECP);      // FEP/RECP明文源端口
        ui->ciphertextSrcPortFepRECP->setValue(tempData.ciphertextSrcPortFepRECP);    // FEP/RECP密文源端口
        ui->desc->setPlainText(tempData.desc);                                        // 描述
    }
    template<typename UI>
    static void clearUIData(UI* ui)
    {
        ui->centerName->clear();                     // 中心名称
        ui->centerIP->clear();                       // 中心IP地址
        ui->centerXS->clear();                       // 中心信宿
        ui->plaintextDestPortPDXP->setValue(1);      // PDXP明文目的端口
        ui->ciphertextDestPortPDXP->setValue(1);     // PDXP密文目的端口
        ui->plaintextSrcPortPDXP->setValue(1);       // PDXP明文源端口
        ui->ciphertextSrcPortPDXP->setValue(1);      // PDXP密文源端口
        ui->plaintextDestPortFep->setValue(1);       // FEP明文目的端口
        ui->ciphertextDestPortFep->setValue(1);      // FEP密文目的端口
        ui->plaintextSrcPortFep->setValue(1);        // FEP明文源端口
        ui->ciphertextSrcPortFep->setValue(1);       // FEP密文源端口
        ui->plaintextDestPortFepRECP->setValue(1);   // FEP/RECP明文目的端口
        ui->ciphertextDestPortFepRECP->setValue(1);  // FEP/RECP密文目的端口
        ui->plaintextSrcPortFepRECP->setValue(1);    // FEP/RECP明文源端口
        ui->ciphertextSrcPortFepRECP->setValue(1);   // FEP/RECP密文源端口
        ui->desc->clear();                           // 描述
    }

    /* 这两个函数的顺序要一致 modeHeader taskCenterData2Mode */
    static inline QStringList modeHeader()
    {
        QStringList headList;
        headList << QString("中心名称") << QString("中心IP") << QString("中心信宿") << QString("描述");
        return headList;
    }
    static inline QStringList data2Mode(const DataTransmissionCenterData& value)
    {
        QStringList listValue;
        listValue << value.centerName << value.centerIP << value.centerXS << value.desc;
        return listValue;
    }
    template<typename T>
    static bool dataCompare(const T& data1, const T& data2)
    {
        return data1.centerName == data2.centerName;
    }
};

#endif  // DTCUTILITY_H
