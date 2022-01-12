#ifndef MIRCOSERVICEINFO_H
#define MIRCOSERVICEINFO_H
#include <QString>

class MircoServiceInfo
{
public:
    QString m_serviceID;           // current Macroservice ID  ,systemid group by 0
    QString m_symbolicName;        //
    QString m_serviceName;         // current Macroservice Name
    QString m_serviceVersion;      //
    QString m_serviceAuthors;      //
    QString m_serviceDescription;  //
    QString m_serviceLog;          // code update log
};

#endif  // MIRCOSERVICEINFO_H
