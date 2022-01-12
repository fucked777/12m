#include "TcpServer.h"

TcpServer::TcpServer(QObject* parent)
    : QTcpServer(parent)
{
}
TcpServer::~TcpServer() {}

void TcpServer::incomingConnection(qintptr handle) { emit sg_newConnect(handle); }
