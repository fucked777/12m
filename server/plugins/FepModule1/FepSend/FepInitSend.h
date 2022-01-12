#ifndef FEPINITSEND_H
#define FEPINITSEND_H
#include "IStateSend.h"
#include <QTimer>

class FepInitSend : public IStateSend
{
public:
    explicit FepInitSend(SendContent* content, QObject* parent = nullptr);
    ~FepInitSend() override;
    void sendRequest() override;
    void recvResponse(const QByteArray& rawData) override;
};

#endif  // FEPINITSEND_H
