#ifndef CONVERTER_H
#define CONVERTER_H

#include "Endian.h"
#include <QByteArray>
#include <QDebug>
#include <QString>
#include <QVariant>
#include <qendian.h>
#include <type_traits>

/* 只考虑小端,按照协议是只有小端 */
class CConverter
{
public:
    /* 一个字节 */
    template<typename T, typename std::enable_if<sizeof(typename std::decay<T>::type) == 1, int>::type = 0>
    static typename std::decay<T>::type fromByteArray(const QByteArray& byteArr);
    template<typename T, typename std::enable_if<sizeof(typename std::decay<T>::type) == 2 || sizeof(typename std::decay<T>::type) == 4 ||
                                                     sizeof(typename std::decay<T>::type) == 8,
                                                 int>::type = 0>
    static typename std::decay<T>::type fromByteArray(const QByteArray& byteArr);

    template<typename T, typename std::enable_if<std::is_same<typename std::decay<T>::type, QString>::value, int>::type = 0>
    static typename std::decay<T>::type fromByteArray(const QByteArray& byteArr);

    /*****************************************************************************************/
    template<typename T, typename std::enable_if<sizeof(typename std::decay<T>::type) == 1, int>::type = 0>
    static QByteArray toByteArray(typename std::decay<T>::type target);
    template<typename T, typename std::enable_if<sizeof(typename std::decay<T>::type) == 2 || sizeof(typename std::decay<T>::type) == 4 ||
                                                     sizeof(typename std::decay<T>::type) == 8,
                                                 int>::type = 0>
    static QByteArray toByteArray(typename std::decay<T>::type target);
    template<typename T, typename std::enable_if<std::is_same<typename std::decay<T>::type, QString>::value, int>::type = 0>
    static QByteArray toByteArray(const typename std::decay<T>::type& target);
};

/* 一个字节 */
template<typename T, typename std::enable_if<sizeof(typename std::decay<T>::type) == 1, int>::type>
typename std::decay<T>::type CConverter::fromByteArray(const QByteArray& byteArr)
{
    //字节数组长度不为0，直接取字节数组的第一个字符返回
    return byteArr.isEmpty() ? 0 : static_cast<typename std::decay<T>::type>(byteArr.at(0));
}
template<typename T, typename std::enable_if<sizeof(typename std::decay<T>::type) == 2 || sizeof(typename std::decay<T>::type) == 4 ||
                                                 sizeof(typename std::decay<T>::type) == 8,
                                             int>::type>
typename std::decay<T>::type CConverter::fromByteArray(const QByteArray& byteArr)
{
    using DecayType = typename std::decay<T>::type;
    static constexpr int typeSize = static_cast<int>(sizeof(typename std::decay<T>::type));

    auto bak = byteArr;
    auto curArraySize = bak.size();
    //判断字节数组长度是否大于指定长度，大于直接返回0
    if (curArraySize > typeSize)
    {
        return DecayType{};
    }
    //不够长度，在后面添0 小端
    bak.append(QByteArray(typeSize - curArraySize, char(0)));

#if IS_LITTLEENDIAN
    return *(static_cast<DecayType*>(static_cast<void*>(bak.data())));

#else
    auto value = *(static_cast<DecayType*>(static_cast<void*>(bak.data())));
    return Endian::bSwap<DecayType>(value);
#endif
}
template<typename T, typename std::enable_if<std::is_same<typename std::decay<T>::type, QString>::value, int>::type>
typename std::decay<T>::type CConverter::fromByteArray(const QByteArray& byteArr)
{
    using DecayType = typename std::decay<T>::type;
    return DecayType(byteArr);
}

template<typename T, typename std::enable_if<sizeof(typename std::decay<T>::type) == 1, int>::type>
QByteArray CConverter::toByteArray(typename std::decay<T>::type target)
{
    return QByteArray(1, static_cast<char>(target));
}
template<typename T, typename std::enable_if<sizeof(typename std::decay<T>::type) == 2 || sizeof(typename std::decay<T>::type) == 4 ||
                                                 sizeof(typename std::decay<T>::type) == 8,
                                             int>::type>
QByteArray CConverter::toByteArray(typename std::decay<T>::type target)
{
    static constexpr int typeSize = static_cast<int>(sizeof(typename std::decay<T>::type));

#if IS_LITTLEENDIAN
    return QByteArray(reinterpret_cast<char*>(&target), typeSize);

#else
    using DecayType = typename std::decay<T>::type;
    auto value = Endian::bSwap<DecayType>(target);
    return QByteArray(reinterpret_cast<char*>(&value), typeSize);
#endif
}
template<typename T, typename std::enable_if<std::is_same<typename std::decay<T>::type, QString>::value, int>::type>
QByteArray CConverter::toByteArray(const typename std::decay<T>::type& target)
{
    return target.toUtf8();
}

#endif  // CONVERTER_H
