#ifndef BASEUNPACKER_H
#define BASEUNPACKER_H

#include <QObject>

#include "CustomPacketMessageDefine.h"
#include "ProtocolXmlTypeDefine.h"

class BaseUnpacker : public QObject
{
    Q_OBJECT
public:
    explicit BaseUnpacker(QObject* parent = nullptr);
    virtual ~BaseUnpacker();

    bool unpack(const QByteArray& data, UnpackMessage& message);
    virtual DevMsgType messageType() const = 0;

protected:
    virtual bool unpackBody(const QByteArray& bodyBytes, UnpackMessage& message) = 0;

    // 解包上报参数
    virtual bool unpackParams(const QList<ParameterAttribute>& params, const QByteArray& paramBytes, QMap<QString, QVariant>& paramVals, int& index);
    virtual bool unpackParam(const ParameterAttribute& param, const QByteArray& paramBytes, QVariant& paramVal, int& index);
    // 解包bit类型参数
    virtual bool unpackBitParams(const QList<ParameterAttribute>& params, QMap<QString, QVariant>& paramVals, int& paramIndex,
                                 const QByteArray& paramBytes, int& index) const;

private:
    quint64 getBitValue(quint64 value, int startIndex, int length) const;
};

#endif  // BASEUNPACKER_H
