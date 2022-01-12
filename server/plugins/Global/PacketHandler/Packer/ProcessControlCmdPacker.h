#ifndef PROCESSCONTROLCMDPACKER_H
#define PROCESSCONTROLCMDPACKER_H

#include "BasePacker.h"

// 过程控制命令设置组包器
class ProcessControlCmdPacker : public BasePacker
{
    Q_OBJECT
public:
    explicit ProcessControlCmdPacker(QObject* parent = nullptr);
    virtual ~ProcessControlCmdPacker();

    virtual DevMsgType messageType() const override;

protected:
    bool packBody(const PackMessage& message, QByteArray& bodyBytes, QString& errorMsg) const override;

private:
    // 组过程控制命令标识
    bool packProcessCtrlCmdIdent(int systemId, int deviceId, int cmdId, QByteArray& cmdIdentBytes) const;
};

#endif  // PROCESSCONTROLCMDPACKER_H
