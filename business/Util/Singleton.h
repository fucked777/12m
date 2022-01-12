#ifndef SINGLETON_H
#define SINGLETON_H
#include <iostream>
/**
 * @className ISingleton
 * @brief brief
 * @author Wangjy
 * @createTime 2019-09-27
 */
template <typename T>
class ISingleton
{
public:
    static T& getInstance() noexcept(std::is_nothrow_constructible<T>::value)
    {
        static T instance;
        return instance;
    }

    virtual ~ISingleton() noexcept {}

    ISingleton(const ISingleton&) = delete;
    ISingleton& operator=(const ISingleton&) = delete;

protected:
    ISingleton() {}
};

/********************************
 *

class Pack1 : public ISingleton<Pack1>,public IPackInterface
{
    friend class ISingleton<Pack1>;
public:
    Pack1(const Pack1&) = delete;
    Pack1& operator = (const Pack1&) = delete;
    void packSomeThing();
    void packSomeThing2();

private:
    Pack1() = default;
};
*/

#endif // SINGLETON_H
