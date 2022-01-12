#ifndef PLATFORMOBJECTTOOLS_H
#define PLATFORMOBJECTTOOLS_H

#include "PlatformGlobal.h"
#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <QThreadPool>
#include <functional>
/*
 * 平台对象工具
 * 用途 这里之前约定了一般不在模块里面调用模块
 * 所以这个类就不会去存储模块对象
 *
 * 这个类一般存储一些全局的数据对象,线程对象等
 */
using PlatformCreateObj = std::function<void*()>;
using PlatformDestroyObj = std::function<void(void*)>;
class PlatformObjectToolsImpl;
class PLATFORMSHARED_EXPORT PlatformObjectTools
{
public:
    /*
     * 获取当前平台全局的线程池
     *
     * 20210809 这代码是直接抄的Qt的源码
     * 因为这里跨dll了 直接调用Qt自带全局函数
     * 有部分虽然是全局的但是可能会出现每个dll里面会出现不同的对象的情况
     */
    static QThreadPool* poolInstance();
    static PlatformObjectTools* instance();
    PlatformObjectTools();
    ~PlatformObjectTools();

    template<typename T>
    T* addObj(const QString& indexKey)
    {
        auto createObj = []() { return new T; };
        auto destroyObj = [](void* obj) {
            auto tempObj = reinterpret_cast<T*>(obj);
            delete tempObj;
        };
        return reinterpret_cast<T*>(addObj(indexKey, createObj, destroyObj));
    }
    /* 添加一个自定义对象 当索引对应的对象存在的话什么也不会发生 返回的是当前的对象 */
    void* addObj(const QString& indexKey, PlatformCreateObj createObj, PlatformDestroyObj destroyObj);
    bool contains(const QString& indexKey) const;
    void removeObj(const QString& indexKey);
    void removeAllObj();

    void* getObj(const QString& indexKey);
    template<typename T>
    T* getObj(const QString& indexKey)
    {
        return reinterpret_cast<T*>(getObj(indexKey));
    }
    template<typename T>
    T getObjCp(const QString& indexKey)
    {
        lock();
        auto obj = reinterpret_cast<T*>(getObjImpl(indexKey));
        T result = {};
        if (obj != nullptr)
        {
            result = *obj;
        }
        unlock();
        return result;
    }

    void lock();
    void unlock();

    template<typename T>
    void updataObj(const QString& indexKey, const T& newObj)
    {
        lock();
        auto createObj = []() { return new T; };
        auto destroyObj = [](void* obj) {
            auto tempObj = reinterpret_cast<T*>(obj);
            delete tempObj;
        };
        auto obj = reinterpret_cast<T*>(addObjImpl(indexKey, createObj, destroyObj));

        if (obj != nullptr)
        {
            *obj = newObj;
        }

        unlock();
    }

private:
    void* getObjImpl(const QString& indexKey);
    bool containsImpl(const QString& indexKey) const;
    void* addObjImpl(const QString& indexKey, PlatformCreateObj createObj, PlatformDestroyObj destroyObj);

private:
    QScopedPointer<PlatformObjectToolsImpl> m_impl;
};

#endif  // PLATFORMOBJECTTOOLS_H
