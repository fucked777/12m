#ifndef ZMQSOCKET_H
#define ZMQSOCKET_H
#include "Utility.h"
#include <QString>
#include <QtGlobal>
#include <cstring>
#include <string>
#include <type_traits>

class ZMQSocketRes
{
public:
    ZMQSocketRes();
    explicit ZMQSocketRes(bool suc, bool e = false, bool isMore = false);
    ~ZMQSocketRes();
    bool success();
    void setSuccess(bool);
    /* 使用非阻塞方式接收消息的时候没有接收到任何消息 */
    bool eagain();
    void setEagain(bool);
    /* 是否还有更多数据 */
    bool isMore();
    void setIsMore(bool);

private:
    bool m_success;
    bool m_eagain;
    bool m_more;
};

class ZMQContext;
class ZMQMsg;
class QByteArray;
class QString;
class ZMQSocket
{
    DIS_COPY_MOVE(ZMQSocket)

public:
    ZMQSocket();
    ~ZMQSocket();

    /* 重复调用init会返回false
     * 销毁之后可以重新初始化
     */
    bool init(void *context, int type);
    bool init(ZMQContext &context, int type);
    void destroy();

    void *handle() noexcept;
    bool valid() noexcept;
    bool bind(const char *addr);
    void unbind(const char *addr);
    bool connect(const char *addr);
    void disconnect(const char *addr_);

    /*
     * true 读取数据成功
     * false + empty
     */
    ZMQSocketRes recv(ZMQMsg &msg, int flags = 0);
    ZMQSocketRes recv(QByteArray &msg, int flags = 0);
    ZMQSocketRes recv(QString &msg, int flags = 0);
    /* 接收一个空帧 */
    ZMQSocketRes recv(int flags = 0);

    ZMQSocketRes send(ZMQMsg &msg, int flags);
    ZMQSocketRes send(const QByteArray &msg, int flags);
    ZMQSocketRes send(const QString &msg, int flags);
    /* 发送空帧 */
    ZMQSocketRes send(int flags);
    QString getErrMsg() const;

    inline void unbind(const QString &addr)
    {
        auto stdStr = addr.toStdString();
        unbind(stdStr.c_str());
    }
    inline bool connect(const QString &addr)
    {
        auto stdStr = addr.toStdString();
        return connect(stdStr.c_str());
    }
    inline bool bind(const QString &addr)
    {
        auto stdStr = addr.toStdString();
        return bind(stdStr.c_str());
    }

    inline void disconnect(const QString &addr)
    {
        auto stdStr = addr.toStdString();
        disconnect(stdStr.c_str());
    }

public:
    template <class T,
              typename std::enable_if<std::is_integral<typename std::decay<T>::type>::value,
                                      int>::type = 0>
    bool set(int opt, const T &val)
    {
        return setOption(opt, &val, sizeof(val));
    }

    template <std::size_t N>
    bool set(int opt, const char (&buf)[N])
    {
        return setOption(opt, buf, buf[N - 1] == '\0' ? N - 1 : N);
    }
    bool set(int opt, const std::string &buf)
    {
        return setOption(opt, buf.data(), buf.size());
    }
    bool set(int opt, const QString &buf)
    {
        return set(opt, buf.toStdString());
    }
    bool set(int opt, const QByteArray &buf)
    {
        return setOption(opt, buf.data(), buf.size());
    }

private:
    bool setOption(int option, const void *optval, size_t optvallen);
    bool getOption(int option, void *optval, size_t *optvallen) const;

private:
    void *m_socket;
    void *m_context;
    QString m_errMsg;
};

#endif  // ZMQSOCKET_H
