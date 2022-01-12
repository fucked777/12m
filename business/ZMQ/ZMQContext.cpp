#include "ZMQContext.h"
#include "ZMQHelper.h"
#include <QDebug>

ZMQContext::ZMQContext() : m_context(nullptr) {}
QString ZMQContext::getErrMsg() const
{
    return m_errMsg;
}
bool ZMQContext ::init()
{
    if (valid()) {
        m_errMsg = "当前context已经初始化";
        return false;
    }
    m_context = zmq_ctx_new();
    if (valid()) {
        return true;
    }

    m_errMsg = ZMQHelper::errMsg();
    return false;
}
bool ZMQContext::init(int io_threads, int max_sockets)
{
    if (!init()) {
        return false;
    }

    int rc = zmq_ctx_set(m_context, ZMQ_IO_THREADS, io_threads);
    Q_ASSERT(rc == 0);

    rc = zmq_ctx_set(m_context, ZMQ_MAX_SOCKETS, max_sockets);
    Q_ASSERT(rc == 0);
    return true;
}
ZMQContext::~ZMQContext()
{
    shutdown();
    destroy();
}

void ZMQContext::shutdown() noexcept
{
    if (!valid()) {
        return;
    }
    int rc = zmq_ctx_shutdown(m_context);
    Q_ASSERT(rc == 0);
}
void ZMQContext::destroy()
{
    if (!valid()) {
        return;
    }
    int rc = 0;
    do {
        // rc = zmq_ctx_destroy(m_context);
        rc = zmq_ctx_term(m_context);
    } while (rc == -1 && errno == EINTR);

    m_context = nullptr;
    Q_ASSERT(rc == 0);
}

void *ZMQContext::handle() noexcept
{
    return m_context;
}
bool ZMQContext::valid() noexcept
{
    return m_context != nullptr;
}
