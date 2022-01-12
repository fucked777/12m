#include "IStateRecv.h"

IStateRecv::IStateRecv(RecvContent* content, QObject* parent)
    : QObject(parent)
    , m_content(content)
{
    Q_ASSERT(content != nullptr);
}

IStateRecv::~IStateRecv() {}
