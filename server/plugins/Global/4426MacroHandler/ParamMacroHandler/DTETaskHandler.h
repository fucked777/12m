#ifndef DTETASKHANDLER_H
#define DTETASKHANDLER_H
#include "BaseParamMacroHandler.h"

/* 默认是下3年的计划 */
class DTETaskHandler : public BaseParamMacroHandler
{
    Q_OBJECT
public:
    explicit DTETaskHandler(QObject* parent = nullptr);
    void setDTETaskTime(const QDateTime& start, const QDateTime& end);
    bool handle() override;
    SystemWorkMode getSystemWorkMode() override;

private:
    QDateTime m_dteTraceStartDateTime;
    QDateTime m_dteTaskEndDateTime;
};
#endif  // DTETASKHANDLER_H
