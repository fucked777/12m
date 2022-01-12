#include "ZMQSocket.h"
#include "ZMQContext.h"
#include "ZMQHelper.h"
#include "ZMQMsg.h"
#include <QByteArray>
#include <QString>
#include <QtDebug>

ZMQSocketRes::ZMQSocketRes()
    : m_success(false)
    , m_eagain(false)
    , m_more(false)
{
}
ZMQSocketRes::ZMQSocketRes(bool suc, bool e, bool isMore)
    : m_success(suc)
    , m_eagain(e)
    , m_more(isMore)
{
}
ZMQSocketRes::~ZMQSocketRes() {}
bool ZMQSocketRes::success() { return m_success; }
bool ZMQSocketRes::eagain() { return m_eagain; }
void ZMQSocketRes::setSuccess(bool suc) { m_success = suc; }
void ZMQSocketRes::setEagain(bool e) { m_eagain = e; }
bool ZMQSocketRes::isMore() { return m_more; }
void ZMQSocketRes::setIsMore(bool more) { m_more = more; }

/************************************************************************************/
/************************************************************************************/
/************************************************************************************/
/************************************************************************************/
/************************************************************************************/
/************************************************************************************/

ZMQSocket::ZMQSocket()
    : m_socket(nullptr)
    , m_context(nullptr)
{
}

ZMQSocket::~ZMQSocket() { destroy(); }
QString ZMQSocket::getErrMsg() const { return m_errMsg; }
bool ZMQSocket::init(void* context, int type)
{
    if (valid())
    {
        m_errMsg = "当前socket已经初始化";
        return false;
    }
    m_socket = zmq_socket(context, type);
    if (valid())
    {
        m_context = context;
        return true;
    }

    m_errMsg = ZMQHelper::errMsg();
    return false;
}
bool ZMQSocket::init(ZMQContext& context, int type) { return init(context.handle(), type); }
void ZMQSocket::destroy()
{
    if (!valid())
    {
        return;
    }

    int rc = zmq_close(m_socket);
    m_socket = nullptr;
    m_context = nullptr;
    Q_ASSERT(rc == 0);
}

void* ZMQSocket::handle() noexcept { return m_socket; }
bool ZMQSocket::valid() noexcept { return m_socket != nullptr; }
bool ZMQSocket::bind(const char* addr)
{
    auto res = (0 == zmq_bind(m_socket, addr));
    if (!res)
    {
        m_errMsg = ZMQHelper::errMsg();
    }
    return res;
}

void ZMQSocket::unbind(const char* addr)
{
    zmq_unbind(m_socket, addr);

    //    auto res = (0 == zmq_unbind(m_socket, addr));

    //    if (!res) {
    //        m_errMsg = ZMQHelper::zmqErrMsg();
    //    }
}

bool ZMQSocket::connect(const char* addr)
{
    auto res = (0 == zmq_connect(m_socket, addr));
    if (!res)
    {
        m_errMsg = ZMQHelper::errMsg();
    }
    return res;
}

void ZMQSocket::disconnect(const char* addr)
{
    zmq_disconnect(m_socket, addr);

    //    auto res = (0 == zmq_disconnect(m_socket, addr));

    //    if (!res) {
    //        m_errMsg = ZMQHelper::zmqErrMsg();
    //    }
}

ZMQSocketRes ZMQSocket::recv(ZMQMsg& msg, int flags)
{
    const int nbytes = zmq_msg_recv(msg.handle(), m_socket, flags);
    if (nbytes >= 0)
    {
        Q_ASSERT(msg.size() == static_cast<size_t>(nbytes));
        return ZMQSocketRes(true, false, msg.isMore());
    }
    if (zmq_errno() == EAGAIN)
    {
        return ZMQSocketRes(false, true);
    }
    m_errMsg = ZMQHelper::errMsg();
    return ZMQSocketRes(false, false);
}
ZMQSocketRes ZMQSocket::recv(QByteArray& msg, int flags)
{
    ZMQMsg tempMsg;
    const int nbytes = zmq_msg_recv(tempMsg.handle(), m_socket, flags);
    if (nbytes >= 0)
    {
        Q_ASSERT(tempMsg.size() == static_cast<size_t>(nbytes));
        msg = QByteArray(static_cast<const char*>(tempMsg.data()), static_cast<int>(tempMsg.size()));
        return ZMQSocketRes(true, false, tempMsg.isMore());
    }
    msg = QByteArray();
    if (zmq_errno() == EAGAIN)
    {
        return ZMQSocketRes(false, true);
    }
    m_errMsg = ZMQHelper::errMsg();
    return ZMQSocketRes(false, false);
}
ZMQSocketRes ZMQSocket::recv(QString& msg, int flags)
{
    ZMQMsg tempMsg;
    const int nbytes = zmq_msg_recv(tempMsg.handle(), m_socket, flags);
    if (nbytes >= 0)
    {
        Q_ASSERT(tempMsg.size() == static_cast<size_t>(nbytes));
        msg = QString::fromUtf8(static_cast<const char*>(tempMsg.data()), static_cast<int>(tempMsg.size()));
        return ZMQSocketRes(true, false, tempMsg.isMore());
    }
    msg = QString();
    if (zmq_errno() == EAGAIN)
    {
        return ZMQSocketRes(false, true);
    }
    m_errMsg = ZMQHelper::errMsg();
    return ZMQSocketRes(false, false);
}
ZMQSocketRes ZMQSocket::recv(int flags)
{
    ZMQMsg tempMsg;
    return recv(tempMsg, flags);
}

ZMQSocketRes ZMQSocket::send(ZMQMsg& msg, int flags)
{
    int nbytes = zmq_msg_send(msg.handle(), m_socket, flags);
    if (nbytes >= 0)
        return ZMQSocketRes(true);
    if (zmq_errno() == EAGAIN)
        return ZMQSocketRes(false, true);

    m_errMsg = ZMQHelper::errMsg();
    return ZMQSocketRes(false, false);
}
ZMQSocketRes ZMQSocket::send(const QByteArray& msg, int flags)
{
    const int nbytes = zmq_send(m_socket, msg.constData(), static_cast<size_t>(msg.size()), flags);
    if (nbytes >= 0)
        return ZMQSocketRes(true);

    if (zmq_errno() == EAGAIN)
        return ZMQSocketRes(false, true);

    m_errMsg = ZMQHelper::errMsg();
    return ZMQSocketRes(false, false);
}
ZMQSocketRes ZMQSocket::send(const QString& msg, int flags)
{
    auto array = msg.toUtf8();
    return send(array, flags);
}
ZMQSocketRes ZMQSocket::send(int flags)
{
    const int nbytes = zmq_send(m_socket, "", 0, flags);
    if (nbytes >= 0)
        return ZMQSocketRes(true);

    if (zmq_errno() == EAGAIN)
        return ZMQSocketRes(false, true);

    m_errMsg = ZMQHelper::errMsg();
    return ZMQSocketRes(false, false);
}

bool ZMQSocket::setOption(int option, const void* optval, size_t optvallen)
{
    auto res = (0 == zmq_setsockopt(m_socket, option, optval, optvallen));
    if (!res)
    {
        m_errMsg = ZMQHelper::errMsg();
    }
    return res;
}
bool ZMQSocket::getOption(int option, void* optval, size_t* optvallen) const
{
    auto res = (0 == zmq_getsockopt(m_socket, option, optval, optvallen));
    if (!res)
    {
        *const_cast<QString*>(&m_errMsg) = ZMQHelper::errMsg();
    }
    return res;
}
