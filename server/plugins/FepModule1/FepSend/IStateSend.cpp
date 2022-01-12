#include "IStateSend.h"
#include "Protocol.h"

IStateSend::IStateSend(SendContent* content, QObject* parent)
    : QObject(parent)
    , m_content(content)
{
    Q_ASSERT(content != nullptr);
}

IStateSend::~IStateSend() {}
