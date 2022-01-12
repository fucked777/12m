#include "PlatformObjectTools.h"
#include "PlatformDefine.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QMutex>
#include <QMutexLocker>
#include <QPointer>
#include <QRegularExpression>
#include <QVariant>

class PlatformObjectToolsImpl
{
public:
    struct PlatformObjectItem
    {
        PlatformCreateObj createObj;
        PlatformDestroyObj destroyObj;
        void* obj{ nullptr };
    };
    QMutex mutex;
    QMap<QString, PlatformObjectItem> platformObjectMap;
    void* addObj(const QString& indexKey, PlatformCreateObj createObj, PlatformDestroyObj destroyObj);
    void removeObj(const QString& indexKey);
    void removeAllObj();
};
Q_GLOBAL_STATIC(PlatformObjectTools, platformThreadPoolInstance);

void* PlatformObjectToolsImpl::addObj(const QString& indexKey, PlatformCreateObj createObj, PlatformDestroyObj destroyObj)
{
    if (indexKey.isEmpty())
    {
        return nullptr;
    }
    if (platformObjectMap.contains(indexKey))
    {
        return platformObjectMap.value(indexKey).obj;
    }

    PlatformObjectItem item;
    auto tempObj = createObj();
    if (tempObj == nullptr)
    {
        return nullptr;
    }
    item.createObj = createObj;
    item.destroyObj = destroyObj;
    item.obj = tempObj;
    platformObjectMap[indexKey] = item;
    return tempObj;
}

void PlatformObjectToolsImpl::removeObj(const QString& indexKey)
{
    auto find = platformObjectMap.find(indexKey);
    if (find == platformObjectMap.end())
    {
        find->destroyObj(find->obj);
        platformObjectMap.erase(find);
    }
}
void PlatformObjectToolsImpl::removeAllObj()
{
    for (auto& item : platformObjectMap)
    {
        item.destroyObj(item.obj);
    }
    platformObjectMap.clear();
}

/***************************************************************************************/
/***************************************************************************************/
/***************************************************************************************/
/***************************************************************************************/
/***************************************************************************************/
PlatformObjectTools* PlatformObjectTools::instance() { return platformThreadPoolInstance; }

PlatformObjectTools::PlatformObjectTools()
    : m_impl(new PlatformObjectToolsImpl)
{
}
PlatformObjectTools::~PlatformObjectTools()
{
    for (auto& item : m_impl->platformObjectMap)
    {
        item.destroyObj(item.obj);
    }
    m_impl->platformObjectMap.clear();
}
bool PlatformObjectTools::contains(const QString& indexKey) const
{
    QMutexLocker locker(&(m_impl->mutex));
    return containsImpl(indexKey);
}
bool PlatformObjectTools::containsImpl(const QString& indexKey) const { return m_impl->platformObjectMap.contains(indexKey); }
void* PlatformObjectTools::addObj(const QString& indexKey, PlatformCreateObj createObj, PlatformDestroyObj destroyObj)
{
    QMutexLocker locker(&(m_impl->mutex));
    return addObjImpl(indexKey, createObj, destroyObj);
}
void* PlatformObjectTools::addObjImpl(const QString& indexKey, PlatformCreateObj createObj, PlatformDestroyObj destroyObj)
{
    return m_impl->addObj(indexKey, createObj, destroyObj);
}

void PlatformObjectTools::removeObj(const QString& indexKey)
{
    QMutexLocker locker(&(m_impl->mutex));
    m_impl->removeObj(indexKey);
}
void PlatformObjectTools::removeAllObj()
{
    QMutexLocker locker(&(m_impl->mutex));
    m_impl->removeAllObj();
}
void* PlatformObjectTools::getObjImpl(const QString& indexKey)
{
    auto find = m_impl->platformObjectMap.find(indexKey);
    if (find == m_impl->platformObjectMap.end())
    {
        return nullptr;
    }
    return find->obj;
}
void* PlatformObjectTools::getObj(const QString& indexKey)
{
    QMutexLocker locker(&(m_impl->mutex));
    return getObjImpl(indexKey);
}
QThreadPool* PlatformObjectTools::poolInstance()
{
    static QPointer<QThreadPool> theInstance;
    static QBasicMutex theMutex;

    const QMutexLocker locker(&theMutex);
    if (theInstance.isNull() && !QCoreApplication::closingDown())
        theInstance = new QThreadPool();
    return theInstance;
}
void PlatformObjectTools::lock() { m_impl->mutex.lock(); }
void PlatformObjectTools::unlock() { m_impl->mutex.unlock(); }
