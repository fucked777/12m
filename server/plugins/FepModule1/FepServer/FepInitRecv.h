#ifndef FEPINITRECV_H
#define FEPINITRECV_H
#include "IStateRecv.h"
class AnserPack;
class REQPack;
class QFileInfo;
class FepInitRecv : public IStateRecv
{
    Q_OBJECT
public:
    explicit FepInitRecv(RecvContent* content, QObject* parent = nullptr);
    ~FepInitRecv() override;
    void sendRequest() override;
    void recvResponse(const QByteArray& rawData) override;
    QString fileName() const { return m_fileName; }

private:
    void fileExistsOpt(QFileInfo& info, const REQPack& pack);

private:
    QString m_fileName;
};

#endif  // FEPINITSEND_H
