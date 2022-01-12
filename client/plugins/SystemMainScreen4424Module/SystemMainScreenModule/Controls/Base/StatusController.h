#ifndef STATUSCONTROLLER_H
#define STATUSCONTROLLER_H
#include <QtGlobal>
class StatusTimer;
// 状态控制器
class StatusController
{
    Q_DISABLE_COPY(StatusController)
public:
    enum Status
    {
        Normal,    // 正常
        Abnormal,  // 异常
        Unknown    // 未知
    };

    explicit StatusController();
    virtual ~StatusController();

    Status getStatus() const;
    void setStatus(Status status);

protected:
    virtual void statusChanged(Status status);

private:
    Status mStatus;
    StatusTimer* mTimer;
};

#endif  // STATUSCONTROLLER_H
