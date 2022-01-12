#ifndef ISTATERECV_H
#define ISTATERECV_H

#include <QObject>
struct RecvContent;
enum class RecvState;
class IStateRecv : public QObject
{
    Q_OBJECT
public:
    explicit IStateRecv(RecvContent* content, QObject* parent = nullptr);
    virtual ~IStateRecv() override;
    //在此状态应该做的事
    virtual void recvResponse(const QByteArray& rawData) = 0;
    virtual void sendRequest() = 0;
signals:
    void sg_sendData(const QByteArray&);
    void sg_stateChange(RecvState state, const QString& errMsg = QString());

protected:
    RecvContent* m_content;
};

#endif  // ISTATERECV_H
