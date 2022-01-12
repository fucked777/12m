#ifndef CFSENDDATA_H
#define CFSENDDATA_H
#include "IStateSend.h"

struct SendContent;
class FepSendData : public IStateSend
{
    Q_OBJECT
public:
    explicit FepSendData(SendContent* content, QObject* parent = nullptr);
    ~FepSendData() override;
    void sendRequest() override;
    void recvResponse(const QByteArray& rawData) override;
signals:
    void sg_timeReset();
};

#endif  // CFSENDDATA_H
