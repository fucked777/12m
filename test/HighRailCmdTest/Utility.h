#ifndef UTILITY_H
#define UTILITY_H
#include "ErrorCode.h"
#include <QByteArray>
#include <QDataStream>
#include <QObject>
#include <QString>
#include <assert.h>
#include <atomic>
#include <exception>
#include <type_traits>

class QTableView;

template<std::size_t>
struct EnumSizeType
{
};
template<>
struct EnumSizeType<1>
{
    using type = quint8;
};
template<>
struct EnumSizeType<2>
{
    using type = quint16;
};
template<>
struct EnumSizeType<4>
{
    using type = quint32;
};
template<>
struct EnumSizeType<8>
{
    using type = quint64;
};

class QGridLayout;
class Utility
{
public:
    template<typename T, typename U, typename std::enable_if<std::is_pointer<typename std::decay<T>::type>::value, int>::type = 0>
    static inline void nullAssignment(T&& obj, U&& value)
    {
        if (obj == nullptr)
        {
            return;
        }
        *obj = value;
    }

    /* 停止QObject的定时器 */
    static void killTimer(QObject* object, int& timerID);
    static void startTimer(QObject* object, int& timerID, int interval);
    /* 获取错误码对应的字符串 */
    static QString errorCodeStr(ErrorCode);
    /* 目录拼接/文件拼接 */
    static QString dirStitchingQt(const QString& dir1, const QString& dir2);
    static QString dirStitchingWin(const QString& dir1, const QString& dir2);
    static QString dirStitchingLinux(const QString& dir1, const QString& dir2);
};

/* 常用正则 */
class Regexp
{
    /* Full Word Match  FWM全词匹配*/
public:
    static QString ip();
    static QString CodeString();
    static QString hex(int maxLen = -1);
    static QString hexFWM(int fixLen = -1);
    static QString englishChar(int maxLen = -1);
    static QString englishCharFWM(int fixLen = -1);
    static QString english(int len = -1);
    /* 正则验证 */
    static bool matching(const QString& regStr, const QString& str);
};

/* 取走就什么都干
 * 不取值走就会delete
 */
template<typename T>
class GetValueHelper
{
public:
    GetValueHelper(T* type)
        : m_type(type)
    {
    }
    ~GetValueHelper()
    {
        if (m_type != nullptr)
        {
            delete m_type;
            m_type = nullptr;
        }
    }
    T* get()
    {
        auto temp = m_type;
        m_type = nullptr;
        return temp;
    }

private:
    T* m_type;
};
/*
 * 此类很简单希望返回值与错误信息好处理一些
 * 20210223扩展此类的功能现在此类可用于
 * 对外的API接口上的
 * {
 *     "code": 100,  错误码
 *     "message": "错误信息",
 *     T:value  数据部分可有可无
 * }
 */
template<typename T>
class Optional
{
public:
    using ValueType = T;

    inline explicit Optional(ErrorCode code, const QString& msg_, const T& value = T())
        : m_errorCode(code)
        , m_value(value)
        , m_success(code == ErrorCode::Success)
        , m_msg(msg_)
    {
    }
    inline explicit Optional(const T& value) noexcept
        : m_errorCode(ErrorCode::Success)
        , m_value(value)
        , m_success(true)
        , m_msg()
    {
    }
    inline explicit Optional(T&& value) noexcept
        : m_errorCode(ErrorCode::Success)
        , m_value(value)
        , m_success(true)
        , m_msg()
    {
    }
    static Optional<T> emptyOptional() { return Optional<T>(ErrorCode::UnknownError, "未知错误"); }
    const T* operator->() const noexcept
    {
        assert(m_success);
        return &m_value;
    }
    T* operator->() noexcept
    {
        assert(m_success);
        return &m_value;
    }

    const T& operator*() const noexcept
    {
        assert(m_success);
        return m_value;
    }
    T& operator*() noexcept
    {
        assert(m_success);
        return m_value;
    }

    T& value()
    {
        if (!m_success)
            throw std::exception();
        return m_value;
    }
    const T& value() const
    {
        if (!m_success)
            throw std::exception();
        return m_value;
    }
    T notCheckValue() const { return m_value; }

    explicit operator bool() const noexcept { return m_success; }
    bool success() const noexcept { return m_success; }

    QString msg() const { return m_msg; }
    ErrorCode errorCode() const { return m_errorCode; }

private:
    ErrorCode m_errorCode;
    ValueType m_value;
    bool m_success;
    QString m_msg;
};
/* 没有值返回,只有成功与否和错误信息 */
template<>
class Optional<void>
{
public:
    inline explicit Optional(ErrorCode code = ErrorCode::Success, const QString& msg_ = QString())
        : m_success(code == ErrorCode::Success)
        , m_errorCode(code)
        , m_msg(msg_)
    {
    }

    explicit operator bool() const noexcept { return m_success; }
    bool success() const noexcept { return m_success; }

    QString msg() { return m_msg; }
    ErrorCode errorCode() const { return m_errorCode; }
    static Optional<void> emptyOptional() { return Optional<void>(ErrorCode::UnknownError, "未知错误"); }

private:
    bool m_success{ false };
    ErrorCode m_errorCode;
    QString m_msg;
};
using OptionalNotValue = Optional<void>;

class BlockSignalsHelper
{
public:
    BlockSignalsHelper(QObject* obj)
        : m_obj(obj)
    {
        Q_ASSERT(obj != nullptr);
        m_obj->blockSignals(true);
    };
    ~BlockSignalsHelper() { m_obj->blockSignals(false); }

private:
    QObject* m_obj;
};
struct Limits
{
    template<typename T, typename std::enable_if<std::is_unsigned<T>::value, std::size_t>::type = 0>
    static constexpr inline T maxValue() noexcept
    {
        return ~static_cast<T>(0);
    }
    template<typename T, typename std::enable_if<std::is_signed<T>::value, std::size_t>::type = 0>
    static constexpr inline T maxValue() noexcept
    {
        return static_cast<T>(Limits::maxValue<typename std::make_unsigned<T>::type>() >> 1);
    }

    template<typename T, typename std::enable_if<std::is_unsigned<T>::value, std::size_t>::type = 0>
    static constexpr inline T minValue() noexcept
    {
        return static_cast<T>(0);
    }
    template<typename T, typename std::enable_if<std::is_signed<T>::value, std::size_t>::type = 0>
    static constexpr inline T minValue() noexcept
    {
        return ~Limits::maxValue<T>();
    }
};

class AtomicFlagHelper
{
public:
    AtomicFlagHelper(std::atomic_bool& flag, bool init)
        : m_flag(flag)
    {
        m_flag = init;
        destroyValue = !init;
    }
    AtomicFlagHelper(std::atomic_bool& flag)
        : m_flag(flag)
    {
        destroyValue = !flag;
    }
    ~AtomicFlagHelper() { m_flag = destroyValue; }

private:
    std::atomic_bool& m_flag;
    bool destroyValue;
};

template<typename T>
struct EnumInfo
{
    T enumValue;
    QString str;
    QString desStr;

    EnumInfo(T t, const QString& s, const QString& ds)
        : enumValue(t)
        , str(s)
        , desStr(ds)
    {
    }
    EnumInfo()
        : enumValue(T::Unknown)
        , str("Unknown")
        , desStr("未知")
    {
    }
};

#endif  // UTILITY_H
