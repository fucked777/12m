#ifndef STATUSQUERYCMDPACKER_H
#define STATUSQUERYCMDPACKER_H

#include "BasePacker.h"

// 状态参数命令组包器
class StatusQueryCmdPacker : public BasePacker
{
    Q_OBJECT
public:
    explicit StatusQueryCmdPacker(QObject* parent = nullptr);
    virtual ~StatusQueryCmdPacker();

    DevMsgType messageType() const override;

protected:
    bool packBody(const PackMessage& message, QByteArray& bodyBytes, QString& errorMsg) const override;
};

#endif  // STATUSQUERYCMDPACKER_H
