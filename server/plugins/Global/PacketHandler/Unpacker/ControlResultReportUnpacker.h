#ifndef CONTROLRESULTREPORTUNPACKER_H
#define CONTROLRESULTREPORTUNPACKER_H

#include "BaseUnpacker.h"

// 控制结果上报解包器
class ControlResultReportUnpacker : public BaseUnpacker
{
public:
    explicit ControlResultReportUnpacker(QObject* parent = nullptr);
    ~ControlResultReportUnpacker();

    DevMsgType messageType() const override;

protected:
    virtual bool unpackBody(const QByteArray& bodyBytes, UnpackMessage& message) override;

    // 特殊处理动态上报命令
    bool unpackMultiBody(const QByteArray& cmdParamsBytes, CmdAttribute& cmdAttr, ControlResultReportMessage& controlResultReportMsg);
};

#endif  // CONTROLRESULTREPORTUNPACKER_H
