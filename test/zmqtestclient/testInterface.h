#ifndef DEVICECOMMUNICATIONCONFIG_H
#define DEVICECOMMUNICATIONCONFIG_H

#include "INetMsg.h"
class TestInterface : public INetMsg
{
    Q_OBJECT
public:
    TestInterface();
    ~TestInterface();

private:
    void test1(const ProtocolPack& pack);
    void test2(const ProtocolPack& pack);
    void test3(const ProtocolPack& pack);
    void test4(const ProtocolPack& pack);
    void test5(const ProtocolPack& pack);

signals:
    void sg_testPack(const ProtocolPack& pack);
};
#endif  // DEVICECONFIGCOMMUNICATION_H
