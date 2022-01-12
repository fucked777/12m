#include "StatusQueryCmdPacker.h"

#include "CConverter.h"

StatusQueryCmdPacker::StatusQueryCmdPacker(QObject* parent)
    : BasePacker(parent)
{
}
StatusQueryCmdPacker::~StatusQueryCmdPacker() {}

DevMsgType StatusQueryCmdPacker::messageType() const { return DevMsgType::StatusQueryCmd; }

bool StatusQueryCmdPacker::packBody(const PackMessage& /*message*/, QByteArray& bodyBytes, QString& /*errorMsg*/) const
{
    // 添加顺序号
    bodyBytes.append(CConverter::toByteArray<ushort>(0));

    return true;
}
