#ifndef ENDIAN_H
#define ENDIAN_H
#include <QObject>
#include <cstring>
#include <iostream>

#if defined(_MSC_VER)
#define U_BUILD_MSVC
#elif defined(__GNUC__)
#define U_BUILD_GNUC
#else
#error "Unsupported compilers"
#endif

#if defined(Q_OS_WIN)
#if defined(__MINGW32__)
#define U_ENV_MINGW32
#endif
#if defined(__MINGW64__)
#define U_ENV_MINGW64
#endif

#if defined(U_ENV_MINGW32) || defined(U_ENV_MINGW64)
#define U_ENV_MINGW
#endif
#endif

#if defined(U_BUILD_MSVC)
/* msvc中没有直接判断的方法,直接来宏定义 */
#if defined(U_ENDIAN_BE)
#define IS_LITTLEENDIAN false
#else
#define IS_LITTLEENDIAN true
#endif
#else
#define IS_LITTLEENDIAN (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#endif

#if defined(U_BUILD_MSVC) || defined(U_ENV_MINGW)

#include <cstdlib>
#define u_bswap16(x) _byteswap_ushort(static_cast<unsigned short>(x))
#define u_bswap32(x) _byteswap_ulong(static_cast<unsigned long>(x))
#define u_bswap64(x) _byteswap_uint64(static_cast<unsigned __int64>(x))

#elif defined(Q_OS_LINUX)
/*
 * https://man7.org/linux/man-pages/man3/bswap.3.html
 * linux中uint**_t 是 __uint**_t的别名
 */
#include <byteswap.h>
#define u_bswap16(x) bswap_16(static_cast<__uint16_t>(x))
#define u_bswap32(x) bswap_32(static_cast<__uint32_t>(x))
#define u_bswap64(x) bswap_64(static_cast<__uint64_t>(x))

#endif

class Endian
{
    template<typename T>
    using CustomConditions =
        std::integral_constant<bool, std::is_integral<typename std::decay<T>::type>::value || std::is_enum<typename std::decay<T>::type>::value>;

public:
    template<typename T, typename std::enable_if<sizeof(T) == 2 && CustomConditions<T>::value, std::size_t>::type = 0>
    static inline typename std::decay<T>::type bSwap(T value)
    {
        return static_cast<typename std::decay<T>::type>(u_bswap16(value));
    }

    template<typename T, typename std::enable_if<sizeof(T) == 4 && CustomConditions<T>::value, std::size_t>::type = 0>
    static inline typename std::decay<T>::type bSwap(T value)
    {
        return static_cast<typename std::decay<T>::type>(u_bswap32(value));
    }
    template<typename T, typename std::enable_if<sizeof(T) == 8 && CustomConditions<T>::value, std::size_t>::type = 0>
    static inline typename std::decay<T>::type bSwap(T value)
    {
        return static_cast<typename std::decay<T>::type>(u_bswap64(value));
    }

    /* 大小端 */
    //    static constexpr const auto isLittleEndian = IS_LITTLEENDIAN;
    //    static constexpr const auto isBigEndian = !isLittleEndian;

    /* 交换数字的高低位(倒叙) 一般用在大小端转换时 */
    //    static qint16 bSwap(qint16);
    //    static quint16 bSwap(quint16);
    //    static qint32 bSwap(qint32);
    //    static quint32 bSwap(quint32);
    //    static qint64 bSwap(qint64);
    //    static quint64 bSwap(quint64);
    //    static float bSwap(float);
    //    static double bSwap(double);

    /* 主机转网络 */
    static quint16 hton(quint16 hvalue);
    static qint16 hton(qint16 hvalue);
    static quint32 hton(quint32 hvalue);
    static qint32 hton(qint32 hvalue);
    static quint64 hton(quint64 hvalue);
    static qint64 hton(qint64 hvalue);

    /* 网络转主机 */
    static quint16 ntoh(quint16 nvalue);
    static qint16 ntoh(qint16 nvalue);
    static quint32 ntoh(quint32 nvalue);
    static qint32 ntoh(qint32 nvalue);
    static quint64 ntoh(quint64 nvalue);
    static qint64 ntoh(qint64 nvalue);
};

#endif
