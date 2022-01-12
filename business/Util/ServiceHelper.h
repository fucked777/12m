#ifndef SERVICEHELPER_H
#define SERVICEHELPER_H

#include "CppMicroServicesUtility.h"
#include <QString>

template<typename S>
static std::shared_ptr<S> getService(cppmicroservices::BundleContext context)
{
    if (!context)
    {
        return nullptr;
    }
    auto s = context.GetServiceReference<S>();
    return s ? context.GetService(s) : nullptr;
}
template<typename S>
std::shared_ptr<S> getService()
{
    static auto context = cppmicroservices::GetBundleContext();
    if (!context)
    {
        return nullptr;
    }
    auto s = context.GetServiceReference<S>();
    return s ? context.GetService(s) : nullptr;
}

template<typename S>
std::shared_ptr<S> getService(const QString& moduleName)
{
    static auto context = cppmicroservices::GetBundleContext();
    if (!context)
    {
        return nullptr;
    }

    auto serviceRefs = context.GetServiceReferences<S>();
    for (auto serviceRef : serviceRefs)
    {
        if (QString(serviceRef.GetBundle().GetSymbolicName().c_str()) == moduleName)
        {
            return context.GetService(serviceRef);
        }
    }
    return nullptr;
}
#endif  // SERVICEHELPER_H
