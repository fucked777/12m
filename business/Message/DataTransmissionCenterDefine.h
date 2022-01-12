
#ifndef DATATRANSMISSIONMESSAGE_H
#define DATATRANSMISSIONMESSAGE_H

#include <QDataStream>
#include <QList>
#include <QMap>
#include <QObject>

struct DataTransmissionCenterData
{
    QString centerName;                      // 中心名称
    QString centerIP;                        // 中心IP地址
    QString centerXS;                        // 中心信宿
    quint16 plaintextDestPortPDXP{ 1 };      // PDXP明文目的端口
    quint16 ciphertextDestPortPDXP{ 1 };     // PDXP密文目的端口
    quint16 plaintextSrcPortPDXP{ 1 };       // PDXP明文源端口
    quint16 ciphertextSrcPortPDXP{ 1 };      // PDXP密文源端口
    quint16 plaintextDestPortFep{ 1 };       // FEP明文目的端口
    quint16 ciphertextDestPortFep{ 1 };      // FEP密文目的端口
    quint16 plaintextSrcPortFep{ 1 };        // FEP明文源端口
    quint16 ciphertextSrcPortFep{ 1 };       // FEP密文源端口
    quint16 plaintextDestPortFepRECP{ 1 };   // FEP/RECP明文目的端口
    quint16 ciphertextDestPortFepRECP{ 1 };  // FEP/RECP密文目的端口
    quint16 plaintextSrcPortFepRECP{ 1 };    // FEP/RECP明文源端口
    quint16 ciphertextSrcPortFepRECP{ 1 };   // FEP/RECP密文源端口
    QString desc;                            // 描述
};

using DataTransmissionCenterMap = QMap<QString, DataTransmissionCenterData>;
using DataTransmissionCenterList = QList<DataTransmissionCenterData>;

Q_DECLARE_METATYPE(DataTransmissionCenterData);
#endif  // DATATRANSMISSIONMESSAGE_H
