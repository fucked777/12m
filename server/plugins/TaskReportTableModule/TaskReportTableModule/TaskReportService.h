#ifndef TaskReportSERVICE_H
#define TaskReportSERVICE_H

#include "INetMsg.h"

namespace cppmicroservices
{
    class BundleContext;
}

class TaskReportServiceImpl;
class TaskReportService : public INetMsg
{
public:
    TaskReportService(cppmicroservices::BundleContext context);
    ~TaskReportService();

private:
    //程序只需要用到以下数据库操作，可以扩充修改删除
    void insertData(const ProtocolPack& pack);
    void getDataByUUID(const ProtocolPack& pack);

private:
    TaskReportServiceImpl* mImpl;
};

#endif  // TaskReportSERVICE_H
