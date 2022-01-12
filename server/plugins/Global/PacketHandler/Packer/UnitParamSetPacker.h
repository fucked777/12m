#ifndef UNITPARAMSETPACKER_H
#define UNITPARAMSETPACKER_H

#include "BasePacker.h"

// 单元参数设置命令组包器
class UnitParamSetPacker : public BasePacker
{
    Q_OBJECT
public:
    explicit UnitParamSetPacker(QObject* parent = nullptr);
    virtual ~UnitParamSetPacker();

    virtual DevMsgType messageType() const override;

protected:
    bool packBody(const PackMessage& message, QByteArray& bodyBytes, QString& errorMsg) const override;

private:
    bool packUnitParams(const MessageAddress& targetAddr, const UnitParamSetMessage& unitParamSetMsg, QByteArray& unitBytes, QString& errorMsg) const;

    // 组标准多目标单元参数（完全按照标准协议进行组包）
    bool packStandardMultiUnitParams(const UnitParamSetMessage& unitParamSetMsg, const Unit& unit, QByteArray& bodyBytes) const;

    // 组一体化单元参数
    bool packYTHUnitParams(const UnitParamSetMessage& unitParamSetMsg, const Unit& unit, QByteArray& bodyBytes) const;

    // 组一体化 扩频 遥测和测量通道
    bool packYTH_KPUnitParams(const UnitParamSetMessage& unitParamSetMsg, const Unit& unit, QByteArray& bodyBytes) const;
};

#endif  // UNITPARAMSETPACKER_H
