#ifndef JSONHELPER_H
#define JSONHELPER_H

#include <QByteArray>
#include <QDate>
#include <QDateTime>
#include <QString>
#include <QTime>
#include <QVariant>
#include <cstddef>
#include <string>
#include <type_traits>

class JsonReader
{
public:
    JsonReader(const char* json);
    JsonReader(const QString& json);
    JsonReader(const QByteArray& json);
    ~JsonReader();

    operator bool() const { return !mError; }

    JsonReader& StartObject();
    JsonReader& Member(const char* name);
    // add wjy
    JsonReader& Member(const QString& name);

    bool HasMember(const char* name) const;
    // add wjy
    bool HasMember(const QString& name) const;

    bool CurKeyValue(QString& key, QVariant& value);

    // add wjy 获取当前节点Json的数据
    QStringList Members();
    JsonReader& EndObject();

    JsonReader& StartArray(size_t* size = 0);
    JsonReader& EndArray();

    JsonReader& operator&(bool& b);
    JsonReader& operator&(quint32& u);
    JsonReader& operator&(qint32& i);
    JsonReader& operator&(double& d);
    JsonReader& operator&(std::string& s);
    JsonReader& operator&(QString& s);
    JsonReader& operator&(QDateTime& s);
    JsonReader& operator&(QDate& s);
    JsonReader& operator&(QTime& s);
    JsonReader& operator&(QByteArray& s);
    JsonReader& operator&(quint64& i);
    JsonReader& operator&(qint64& i);
    JsonReader& operator&(quint16& u);
    JsonReader& operator&(qint16& i);
    JsonReader& operator&(QVariant& s);
    JsonReader& operator&(QVariantMap& s);
    JsonReader& operator&(QVariantList& s);

    template<typename T, typename std::enable_if<std::is_enum<T>::value && (sizeof(T) == 4), int>::type = 0>
    JsonReader& operator&(T& s)
    {
        quint32 temp = 0;
        (*this) & temp;
        s = T(temp);
        return *this;
    }
    template<typename T, typename std::enable_if<std::is_enum<T>::value && (sizeof(T) == 8), int>::type = 0>
    JsonReader& operator&(T& s)
    {
        quint64 temp = 0;
        (*this) & temp;
        s = T(temp);
        return *this;
    }

    static const bool IsReader;
    static const bool IsWriter;

private:
    JsonReader(const JsonReader&) = delete;
    JsonReader& operator=(const JsonReader&) = delete;
    QVariant getVariant();
    void Next();

    // PIMPL
    void* mDocument;  ///< DOM result of parsing.
    void* mStack;     ///< Stack for iterating the DOM
    bool mError;      ///< Whether an error has occurred.
};

class JsonWriter
{
public:
    /// Constructor.
    JsonWriter();

    /// Destructor.
    ~JsonWriter();

    /// Obtains the serialized JSON string.
    const char* GetString() const;
    QByteArray GetByteArray() const;
    QString GetQString() const;

    // Archive concept

    operator bool() const { return true; }

    JsonWriter& StartObject();
    JsonWriter& Member(const char* name);
    // add wjy
    JsonWriter& Member(const QString& name);

    bool HasMember(const char* name) const;
    // add wjy
    bool HasMember(const QString& name) const;
    // add wjy 获取当前节点Json的数据
    QStringList Members();
    JsonWriter& EndObject();

    JsonWriter& StartArray();
    JsonWriter& EndArray();

    JsonWriter& operator&(bool b);
    JsonWriter& operator&(quint32 u);
    JsonWriter& operator&(qint32 i);
    JsonWriter& operator&(double d);
    JsonWriter& operator&(const std::string& s);
    JsonWriter& operator&(const QString& s);
    JsonWriter& operator&(const QDateTime& s);
    JsonWriter& operator&(const QDate& s);
    JsonWriter& operator&(const QTime& s);
    JsonWriter& operator&(const QByteArray& s);
    JsonWriter& operator&(quint64 i);
    JsonWriter& operator&(qint64 i);
    JsonWriter& operator&(quint16 u);
    JsonWriter& operator&(qint16 i);
    JsonWriter& operator&(const QVariant& s);
    JsonWriter& operator&(const QVariantMap& s);
    JsonWriter& operator&(const QVariantList& s);
    JsonWriter& SetNull();

    template<typename T, typename std::enable_if<std::is_enum<T>::value && (sizeof(T) == 4), int>::type = 0>
    JsonWriter& operator&(T s)
    {
        return (*this) & static_cast<quint32>(s);
    }
    template<typename T, typename std::enable_if<std::is_enum<T>::value && (sizeof(T) == 8), int>::type = 0>
    JsonWriter& operator&(T s)
    {
        return (*this) & static_cast<quint64>(s);
    }

    static const bool IsReader;
    static const bool IsWriter;

private:
    JsonWriter(const JsonWriter&) = delete;
    JsonWriter& operator=(const JsonWriter&) = delete;

    // PIMPL idiom
    void* mWriter;  ///< JSON writer.
    void* mStream;  ///< Stream buffer.
};

#endif  // JSONHELPER_H
