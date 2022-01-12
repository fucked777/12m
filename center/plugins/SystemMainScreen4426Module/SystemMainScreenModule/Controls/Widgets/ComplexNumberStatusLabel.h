#ifndef COMPLEXNUMBERSTATUSLABEL_H
#define COMPLEXNUMBERSTATUSLABEL_H

#include "ProtocolXmlTypeDefine.h"
#include "StatusLabel.h"

#include <QVariant>

class ComplexNumberStatusLabel : public StatusLabel
{
    Q_OBJECT
    Q_PROPERTY(QVariant value READ getValue WRITE setValue)
    Q_PROPERTY(QString dealType READ getDealType WRITE setDealType)
    Q_PROPERTY(QString uniqueKey READ getUniqueKey WRITE setUniqueKey)
    Q_PROPERTY(QVariantList devids READ getDevids WRITE setDevids)

public:
    struct Info
    {
        DeviceID deviceId;
        int modeId;
        int unitId;
        int targetId;
        QString paramId;
        bool isValid{ false };
    };

    struct MainInfo
    {
        Info mainInfo;
        QString value;
    };

    struct ParamInfo
    {
        Info paramInfo;
        bool isMulti;
    };

    struct SumInfo
    {
        MainInfo mainInfo;
        Info targetInfo;
        ParamInfo paramInfo;
    };

    explicit ComplexNumberStatusLabel(const QDomElement& domEle, QWidget* parent = nullptr, const int deviceId = 0, const int modeId = 0);

    void setValue(const QVariant& value);
    QVariant getValue() const;

    void setDealType(const QString& dealType);
    QString getDealType() const;

    void setUniqueKey(const QString& uniqueKey);
    QString getUniqueKey() const;

    void setDevids(QVariantList& list);
    QVariantList getDevids();

    SumInfo getParamInfos();

private:
    void parseNode(const QDomElement& domEle);
    void parseMainInfoNode(const QDomNode& domEle, MainInfo& info);
    void parseTargetInfoNode(const QDomNode& domEle, Info& info);
    void parseParamInfoNode(const QDomNode& domEle, ParamInfo& info);
    void parseInfoNode(const QDomNode& domEle, Info& info);

private:
    SumInfo d_SumInfo;
    QString d_dealType;
    QString d_uniqueKey;
    QVariantList d_deviceIds;

    DeviceID d_deviceId;
    int d_modeId;
};

#endif  // COMPLEXNUMBERSTATUSLABEL_H
