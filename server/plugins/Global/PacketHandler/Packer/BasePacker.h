#ifndef BASEPACKER_H
#define BASEPACKER_H

#include <QObject>

#include "CustomPacketMessageDefine.h"
#include "ProtocolXmlTypeDefine.h"

class BasePacker : public QObject
{
    Q_OBJECT
public:
    explicit BasePacker(QObject* parent = nullptr);
    virtual ~BasePacker();

    bool pack(const PackMessage& message, QByteArray& data, QString& errorMsg) const;

    virtual DevMsgType messageType() const = 0;

protected:
    virtual bool packBody(const PackMessage& message, QByteArray& bodyBytes, QString& errorMsg) const = 0;

    // 组包参数
    virtual bool packParams(const QList<ParameterAttribute>& params, const QMap<QString, QVariant>& paramVals, QByteArray& paramBytes) const;

    // 组bit类型的参数
    virtual bool packBitParams(const QList<ParameterAttribute>& params, const QMap<QString, QVariant>& paramVals, int& index,
                               QByteArray& paramBytes) const;
};

#endif  // BASEPACKER_H
