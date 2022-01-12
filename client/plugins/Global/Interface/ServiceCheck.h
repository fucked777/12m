#ifndef SERVICECHECK_H
#define SERVICECHECK_H

#include "Utility.h"
#define SERVICEONLINE ServiceCheck::serviceOnline()

#define SERVICEONLINECHECK()                                                                                                                         \
    do                                                                                                                                               \
    {                                                                                                                                                \
        if (!SERVICEONLINE)                                                                                                                          \
        {                                                                                                                                            \
            QMessageBox::information(this, QString("提示"), "当前监控服务器离线", QString("确定"));                                                  \
            return;                                                                                                                                  \
        }                                                                                                                                            \
    } while (false)

#define SERVICEONLINECHECKNOPARENT()                                                                                                                 \
    do                                                                                                                                               \
    {                                                                                                                                                \
        if (!SERVICEONLINE)                                                                                                                          \
        {                                                                                                                                            \
            QMessageBox::information(nullptr, QString("提示"), "当前监控服务器离线", QString("确定"));                                               \
            return;                                                                                                                                  \
        }                                                                                                                                            \
    } while (false)

class ServiceCheck
{
public:
    static bool serviceOnline();
};

#endif  // SERVICECHECK_H
