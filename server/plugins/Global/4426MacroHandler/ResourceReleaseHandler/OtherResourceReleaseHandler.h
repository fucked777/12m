#ifndef OTHERRESOURCERELEASEHANDLER_H
#define OTHERRESOURCERELEASEHANDLER_H

#include "BaseResourceReleaseHandler.h"

class OtherResourceReleaseHandler : public BaseResourceReleaseHandler
{
    Q_OBJECT
public:
    explicit OtherResourceReleaseHandler(QObject* parent = nullptr);

    bool handle() override;
    SystemWorkMode getSystemWorkMode() override;

private:
    /* 清空存转任务 */
    void cleanCZTask();
    /* ACU停止 */
    void cleanACUTask();
    /* 停止测试设备 */
    void stopTestDevice();
};

#endif  // OTHERRESOURCERELEASEHANDLER_H
