#ifndef ZMQCONTEXT_H
#define ZMQCONTEXT_H
#include "Utility.h"
#include "zmq.h"
#include <QString>
#include <QtGlobal>

class ZMQContext
{
    DIS_COPY_MOVE(ZMQContext)

public:
    ZMQContext();
    ~ZMQContext();

    /* 重复调用init会返回false
     * 销毁之后可以重新初始化
     */
    bool init();
    bool init(int io_threads, int max_sockets = ZMQ_MAX_SOCKETS_DFLT);
    /* 销毁之后需要重新初始化 */
    void destroy();

    void shutdown() noexcept;
    void *handle() noexcept;
    bool valid() noexcept;

    QString getErrMsg() const;

private:
    void *m_context;
    QString m_errMsg;
};

#endif  // ZMQCONTEXT_H
