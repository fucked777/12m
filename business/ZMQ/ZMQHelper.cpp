#include "ZMQHelper.h"
#include "PlatformInterface.h"
#include "zmq.h"

QString ZMQHelper::errMsg() { return QString(zmq_strerror(zmq_errno())); }
QString ZMQHelper::syntheticAddress(const QString& protocol, const QString& ip, quint16 port)
{
    auto des = QString("%1://%2:%3").arg(protocol, ip).arg(port);
    return des;
}
QString ZMQHelper::syntheticAddress(const QString& protocol, const QString& file)
{
    auto des = QString("%1://%2").arg(protocol, file);
    return des;
}
QString ZMQHelper::procAddress(const QString& name)
{
    auto des = QString("inproc://%1").arg(name);
    return des;
}
QString ZMQHelper::deaultLocalAddress()
{
    static auto defaultInprocID = QString("%1_%2").arg(PlatformConfigTools::instanceID(), "zmq_inproc");
    return procAddress(defaultInprocID);
}
QString ZMQHelper::defaultContextObjName() { return "DefaultZMQContext"; }
