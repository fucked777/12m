#include "testInterface.h"
#include "Utility.h"
#include <QDebug>
#include <QFile>
#include <QTextStream>

TestInterface::TestInterface()
    : INetMsg(nullptr)
{
    registerSubscribe("test1", &TestInterface::test1, this);
    registerSubscribe("test2", &TestInterface::test2, this);
    registerSubscribe("test3", &TestInterface::test3, this);
    registerSubscribe("test4", &TestInterface::test4, this);
    registerSubscribe("test5", &TestInterface::test5, this);
}
TestInterface::~TestInterface() {}
void TestInterface::test1(const ProtocolPack& pack) { emit sg_testPack(pack); }
void TestInterface::test2(const ProtocolPack& pack) { emit sg_testPack(pack); }
void TestInterface::test3(const ProtocolPack& pack) { emit sg_testPack(pack); }
void TestInterface::test4(const ProtocolPack& pack) { emit sg_testPack(pack); }
void TestInterface::test5(const ProtocolPack& pack) { emit sg_testPack(pack); }
