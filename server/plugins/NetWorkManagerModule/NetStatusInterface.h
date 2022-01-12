#ifndef NETSTATUSINTERFACE_H
#define NETSTATUSINTERFACE_H
#include <QAbstractSocket>
#include <QByteArray>

struct NetStatusItemInfo
{
    int index{ -1 };
    quint64 count{ 0 };
    quint64 failCount{ 0 };
    QAbstractSocket::SocketState state{ QAbstractSocket::UnconnectedState };
    QByteArray data;

    void reset()
    {
        index = -1;
        count = 0;
        failCount = 0;
        state = QAbstractSocket::UnconnectedState;
        data.clear();
    }
};

#endif  // NETSTATUSINTERFACE_H
