#ifndef SINGLEMODULELOAD_H
#define SINGLEMODULELOAD_H
#include "CppMicroServicesUtility.h"
#include <QDir>
#include <QString>

class SingleModuleLoad
{
public:
    SingleModuleLoad();
    ~SingleModuleLoad();
    bool loadModule(const QString& moduleName);
    inline QString errMsg() { return m_errMsg; }

    template<typename T>
    std::shared_ptr<T> instance()
    {
        if (!m_context)
        {
            return nullptr;
        }
        auto ref = m_context.GetServiceReference<T>();
        return m_context.GetService(ref);
    }

private:
    cppmicroservices::Framework m_framework;
    cppmicroservices::BundleContext m_context;
    QString m_errMsg;
};

#endif  // SINGLEMODULELOAD_H
