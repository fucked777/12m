#ifndef CONTROLCMDRESPONSEUNPACKER_H
#define CONTROLCMDRESPONSEUNPACKER_H

#include "BaseUnpacker.h"

// 控制命令响应解包器
class ControlCmdResponseUnpacker : public BaseUnpacker
{
public:
    explicit ControlCmdResponseUnpacker(QObject* parent = nullptr);
    ~ControlCmdResponseUnpacker();

    DevMsgType messageType() const override;

protected:
    virtual bool unpackBody(const QByteArray& bodyBytes, UnpackMessage& message) override;
};

#endif  // CONTROLCMDRESPONSEUNPACKER_H
