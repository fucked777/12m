#include "FepTcpServer.h"

FepTcpServer::FepTcpServer(QObject* parent)
    : QTcpServer(parent)
{
}
FepTcpServer::~FepTcpServer() {}

void FepTcpServer::incomingConnection(qintptr handle) { emit sg_newConnect(handle); }
