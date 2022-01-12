#ifndef COMPLEXLEDSTATUSLABEL_H
#define COMPLEXLEDSTATUSLABEL_H

#include "BaseWidget.h"
#include "StatusController.h"

#include "ProtocolXmlTypeDefine.h"
#include <QDomElement>
#include <QLabel>
#include <QVariant>

class ComplexLEDStatusLabel : public BaseWidget, public StatusController
{
    Q_OBJECT
    Q_PROPERTY(QVariant value READ getValue WRITE setValue)
    Q_PROPERTY(QString dealType READ getDealType WRITE setDealType)
    Q_PROPERTY(QString uniqueKey READ getUniqueKey WRITE setUniqueKey)
    Q_PROPERTY(QVariantList devids READ getDevids WRITE setDevids)

    struct LEDItem
    {
        QVariant value;
        Status status{ Unknown };
        QString toolTip{ "未知" };
    };

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

    struct TargetInfo
    {
        Info targetInfo;
    };

    struct ParamInfo
    {
        Info paramInfo;
        bool isMulti{ false };
    };

    struct SumInfo
    {
        MainInfo mainInfo;
        TargetInfo targetInfo;
        ParamInfo paramInfo;
        QMap<QVariant, LEDItem> ledInfo;
    };

    explicit ComplexLEDStatusLabel(const QDomElement& domEle, QWidget* parent = nullptr, const int deviceId = 0, const int modeId = 0);

    void setValue(const QVariant& value);
    QVariant getValue() const;

    void setDealType(const QString& dealType);
    QString getDealType() const;

    void setUniqueKey(const QString& uniqueKey);
    QString getUniqueKey() const;

    void setDevids(QVariantList& list);
    QVariantList getDevids();

    SumInfo& getSumInfo();

protected:
    virtual void statusChanged(Status status) override;

private:
    void initLayout();
    void parseNode(const QDomElement& domEle);
    void parseMainInfo(const QDomElement& domEle, MainInfo& mainInfo);
    void parseTargetInfo(const QDomElement& domEle, TargetInfo& targetInfo);
    void parseParamInfo(const QDomElement& domEle, ParamInfo& paramInfo);
    void parseLedInfo(const QDomElement& domEle, QMap<QVariant, LEDItem>& ledInfo);
    void parseInfo(const QDomElement& domEle, Info& info);

private:
    QLabel* mLabel = nullptr;

    SumInfo d_sumInfo;
    //有模式
    //    QMap<int, QList<QMap<QVariant, LEDItem>>> d_hasModeLedItems;
    //无模式，但有主备信息
    //    QList<QMap<QVariant, LEDItem>> d_noModeLedItems;
    //无模式,无主备信息
    QString d_dealType;
    QString d_uniqueKey;
    QVariantList d_deviceIds;

    DeviceID d_deviceId;
    int d_modeId;
};

#endif  // COMPLEXLEDSTATUSLABEL_H
