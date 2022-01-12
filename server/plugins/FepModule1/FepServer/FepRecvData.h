#ifndef FEPRECVDATA_H
#define FEPRECVDATA_H
#include "IStateRecv.h"

class DataPack;
class FepRecvData : public IStateRecv
{
public:
    explicit FepRecvData(RecvContent* content, QObject* parent = nullptr);
    ~FepRecvData() override;
    void sendRequest() override;
    void recvResponse(const QByteArray& rawData) override; /* 特殊处理,废弃 */
    void recvResponse(const DataPack& tempPack);
};

#endif  // CFSENDDATA_H
