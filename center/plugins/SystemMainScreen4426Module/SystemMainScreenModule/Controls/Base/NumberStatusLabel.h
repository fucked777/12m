#ifndef NUMBERSTATUSLABEL_H
#define NUMBERSTATUSLABEL_H

#include "SatelliteManagementSerialize.h"
#include "StatusLabel.h"

#include <QVariant>

class NumberStatusLabel : public StatusLabel
{
    Q_OBJECT
    Q_PROPERTY(QVariant value READ getValue WRITE setValue)
    Q_PROPERTY(QString spacialData_ READ getSpacialData WRITE setSpacialData)
public:
    explicit NumberStatusLabel(const QDomElement& domEle, QWidget* parent = nullptr);

    void setValue(const QVariant& value);
    QVariant getValue() const;

    void setSpacialData(const QString& value);
    QString getSpacialData() const;

    QString getSatePolarString(const SystemOrientation& orientation);

    void dealPolarGetWay(const LinkLine& link, QString& sendPolar, QString& receivePolar);

    void dealSpcialData();

private:
    void parseNode(const QDomElement& domEle);
    //所有卫星信息
    QMap<QString, SatelliteManagementData> m_taskCodeSateliteMap;

    QString m_sSendPolar;
    QString m_kaCKSendPolar;
    QString m_kaSCSendPolar;
    QString m_sReceivePolar;
    QString m_kaCKReceivePolar;
    QString m_kaSCReceivePolar;
};

#endif  // NUMBERSTATUSLABEL_H
