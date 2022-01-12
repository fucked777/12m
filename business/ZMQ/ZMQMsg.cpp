#include "ZMQMsg.h"
#include <QDebug>

ZMQMsg::ZMQMsg(const void *data_, size_t size_)
{
    m_valid = (0 == zmq_msg_init_size(&m_msg, size_));

    if (!valid()) {
        return;
    }

    if (size_) {
        // this constructor allows (nullptr, 0),
        // memcpy with a null pointer is UB
        memcpy(data(), data_, size_);
    }
}
ZMQMsg::ZMQMsg()
{
    m_valid = (0 == zmq_msg_init(&m_msg));
}

ZMQMsg::~ZMQMsg()
{
    if (valid()) {
        m_valid = false;
        int rc  = zmq_msg_close(&m_msg);
        Q_ASSERT(rc == 0);
    }
}
bool ZMQMsg::valid() const noexcept
{
    return m_valid;
}
bool ZMQMsg::empty() const noexcept
{
    return size() == 0u;
}
bool ZMQMsg::isMore() const
{
    return valid() ? zmq_msg_more(const_cast<zmq_msg_t *>(&m_msg)) : false;
}
void ZMQMsg::rebuild()
{
    if (valid()) {
        int rc = zmq_msg_close(&m_msg);
        Q_ASSERT(rc == 0);
    }

    m_valid = (0 == zmq_msg_init(&m_msg));
}
zmq_msg_t *ZMQMsg::handle() noexcept
{
    return valid() ? &m_msg : nullptr;
}
const zmq_msg_t *ZMQMsg::handle() const noexcept
{
    return valid() ? &m_msg : nullptr;
}
size_t ZMQMsg::size() const noexcept
{
    return valid() ? zmq_msg_size(const_cast<zmq_msg_t *>(&m_msg)) : 0u;
}
void *ZMQMsg::data() noexcept
{
    return valid() ? zmq_msg_data(&m_msg) : nullptr;
}

const void *ZMQMsg::data() const noexcept
{
    return valid() ? zmq_msg_data(const_cast<zmq_msg_t *>(&m_msg)) : nullptr;
}
