#ifndef GROUPPARAMSETPACKER_H
#define GROUPPARAMSETPACKER_H

#include "BasePacker.h"

// 组参数设置命令组包器
class GroupParamSetPacker : public BasePacker
{
    Q_OBJECT
public:
    explicit GroupParamSetPacker(QObject* parent = nullptr);
    virtual ~GroupParamSetPacker();

    virtual DevMsgType messageType() const override;

protected:
    bool packBody(const PackMessage& message, QByteArray& bodyBytes, QString& errorMsg) const override;

    bool packUnitParams(const MessageAddress& targetAddr, const UnitParamSetMessage& unitParamSetMsg, QByteArray& unitBytes) const;
};

#endif  // GROUPPARAMSETPACKER_H
