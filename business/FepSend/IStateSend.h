#ifndef ISTATESEND_H
#define ISTATESEND_H

#include <QObject>
enum class SendState;
struct SendContent;
class IStateSend : public QObject
{
    Q_OBJECT
public:
    explicit IStateSend(SendContent* content, QObject* parent = nullptr);
    virtual ~IStateSend() override;
    //在此状态应该做的事
    virtual void recvResponse(const QByteArray& rawData) = 0;
    virtual void sendRequest() = 0;
signals:
    void sg_sendData(const QByteArray&);
    void sg_stateChange(SendState state, const QString& errMsg = QString());

protected:
    SendContent* m_content;
};

#endif  // ISTATESEND_H
