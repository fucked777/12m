#ifndef NUMBERSTATUSROTATIONLABEL_H
#define NUMBERSTATUSROTATIONLABEL_H

#include "ProtocolXmlTypeDefine.h"
#include "StatusLabel.h"

#include <QVariant>

class NumberStatusRotationLabel : public StatusLabel
{
    Q_OBJECT
    Q_PROPERTY(QVariant value READ getValue WRITE setValue)
    Q_PROPERTY(QString dealType READ getDealType WRITE setDealType)
    Q_PROPERTY(QString uniqueKey READ getUniqueKey WRITE setUniqueKey)
    Q_PROPERTY(QString connector READ getConnector WRITE setConnector)
    Q_PROPERTY(QVariantList devids READ getDevids WRITE setDevids)
public:
    struct ParamInfo
    {
        DeviceID deviceId;
        int modeId;
        int unitId;
        int targetId;
        QString paramId;

        QMap<QString, QString> mapping;
    };

    explicit NumberStatusRotationLabel(const QDomElement& domEle, QWidget* parent = nullptr);

    void setValue(const QVariant& value);
    QVariant getValue() const;

    void setDealType(const QString& dealType);
    QString getDealType() const;

    void setUniqueKey(const QString& uniqueKey);
    QString getUniqueKey() const;

    void setConnector(const QString& connector);
    QString getConnector() const;

    void setDevids(QVariantList& list);
    QVariantList getDevids();

    QList<ParamInfo> getParamInfos();

private:
    void parseNode(const QDomElement& domEle);
    void parseEnumNode(const QDomElement& domEle, QMap<QString, QString>& mapping);

private:
    QList<ParamInfo> d_SumInfos;
    QString d_dealType;
    QString d_uniqueKey;
    QVariantList d_deviceIds;
    QString d_connector;
};

#endif  // NUMBERSTATUSROTATIONLABEL_H
