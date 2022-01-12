#ifndef ZMQMSG_H
#define ZMQMSG_H
#include "Utility.h"
#include "zmq.h"
#include <QtGlobal>

class ZMQMsg
{
    DIS_COPY_MOVE(ZMQMsg)

public:
    ZMQMsg(const void *data_, size_t size);
    ZMQMsg();
    ~ZMQMsg();

    bool empty() const noexcept;
    zmq_msg_t *handle() noexcept;
    const zmq_msg_t *handle() const noexcept;
    size_t size() const noexcept;
    void *data() noexcept;
    const void *data() const noexcept;
    bool valid() const noexcept;
    void rebuild();
    bool isMore() const;

private:
    zmq_msg_t m_msg;
    bool m_valid;
};

#endif  // ZMQMSG_H
