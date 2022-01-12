#ifndef RESOURCERELEASEHANDLER_H
#define RESOURCERELEASEHANDLER_H

#include "BaseHandler.h"

// 资源释放处理
class BaseResourceReleaseHandler : public BaseHandler
{
    Q_OBJECT
public:
    explicit BaseResourceReleaseHandler(QObject* parent = nullptr);

    virtual SystemWorkMode getSystemWorkMode() = 0;
};

#endif  // RESOURCERELEASEHANDLER_H
