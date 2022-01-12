#include "JsonHelper.h"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include <cassert>
#include <cstring>
#include <stack>

using namespace rapidjson;
template<typename T>
static inline QVariant valueToVariant(T& value)
{
    if (value.IsString())
    {
        return QString::fromUtf8(value.GetString());
    }
    else if (value.IsInt())
    {
        return value.GetInt();
    }
    else if (value.IsUint())
    {
        return value.GetUint();
    }
    else if (value.IsInt64())
    {
        return static_cast<qint64>(value.GetInt64());
    }
    else if (value.IsUint64())
    {
        return static_cast<quint64>(value.GetUint64());
    }
    else if (value.IsDouble())
    {
        return value.GetDouble();
    }
    else if (value.IsBool())
    {
        return value.GetBool();
    }
    else if (value.IsNumber())
    {
        return value.GetDouble();
    }

    return QVariant();
}
template<typename T>
static void writeVariantToJson(T* write, const QVariant& s)
{
    if (QVariant::Bool == s.type())
    {
        write->Bool(s.toBool());
    }
    else if (QVariant::Int == s.type())
    {
        write->Int(s.toInt());
    }
    else if (QVariant::UInt == s.type())
    {
        write->Uint(s.toUInt());
    }
    else if (QVariant::LongLong == s.type())
    {
        write->Int64(s.toLongLong());
    }
    else if (QVariant::ULongLong == s.type())
    {
        write->Uint64(s.toULongLong());
    }
    else if (QVariant::Double == s.type())
    {
        write->Double(s.toDouble());
    }
    //    else if (QVariant::Char == s.type()) {
    //         WRITER->String(s.toString());
    //    }
    else if (QVariant::String == s.type())
    {
        auto array = s.toString().toUtf8();
        write->String(array.constData(), static_cast<SizeType>(array.size()));
    }
    else if (QVariant::ByteArray == s.type())
    {
        auto base64 = s.toByteArray().toBase64();
        write->String(base64.constData(), static_cast<SizeType>(base64.size()));
    }
    else
    {
        write->Null();
    }
}

struct JsonReaderStackItem
{
    enum State
    {
        BeforeStart,  //!< An object/array is in the stack but it is not yet called
                      //!< by StartObject()/StartArray().
        Started,      //!< An object/array is called by StartObject()/StartArray().
        Closed        //!< An array is closed after read all element, but before
                      //!< EndArray().
    };

    JsonReaderStackItem(const Value* value, State state)
        : value(value)
        , state(state)
        , index()
    {
    }

    const Value* value;
    State state;
    SizeType index;  // For array iteration
};

typedef std::stack<JsonReaderStackItem> JsonReaderStack;

#define DOCUMENT reinterpret_cast<Document*>(mDocument)
#define STACK    (reinterpret_cast<JsonReaderStack*>(mStack))
#define TOP      (STACK->top())
#define CURRENT  (*TOP.value)

const bool JsonReader::IsReader = true;
const bool JsonReader::IsWriter = !JsonReader::IsReader;

JsonReader::JsonReader(const char* json)
{
    mDocument = new Document;
    DOCUMENT->Parse(json);
    mError = DOCUMENT->HasParseError();
    mStack = nullptr;
    if (!mError)
    {
        mStack = new JsonReaderStack;
        STACK->push(JsonReaderStackItem(DOCUMENT, JsonReaderStackItem::BeforeStart));
    }
}
JsonReader::JsonReader(const QString& json)
    : JsonReader(json.toUtf8())
{
}
JsonReader::JsonReader(const QByteArray& json)
    : JsonReader(json.constData())
{
}

JsonReader::~JsonReader()
{
    delete DOCUMENT;
    delete STACK;
}

// Archive concept
JsonReader& JsonReader::StartObject()
{
    if (!mError)
    {
        if (CURRENT.IsObject() && TOP.state == JsonReaderStackItem::BeforeStart)
            TOP.state = JsonReaderStackItem::Started;
        else
            mError = true;
    }
    return *this;
}

JsonReader& JsonReader::EndObject()
{
    if (!mError)
    {
        if (CURRENT.IsObject() && TOP.state == JsonReaderStackItem::Started)
            Next();
        else
            mError = true;
    }
    return *this;
}

JsonReader& JsonReader::Member(const char* name)
{
    if (!mError)
    {
        if (CURRENT.IsObject() && TOP.state == JsonReaderStackItem::Started)
        {
            Value::ConstMemberIterator memberItr = CURRENT.FindMember(name);
            if (memberItr != CURRENT.MemberEnd())
                STACK->push(JsonReaderStackItem(&memberItr->value, JsonReaderStackItem::BeforeStart));
            else
                mError = true;
        }
        else
            mError = true;
    }
    return *this;
}

JsonReader& JsonReader::Member(const QString& name)
{
    /*
     * name.toStdString().c_str()
     * 这种name.toStdString()在调用完成后对象会销毁 然后c_str()返回的char*很可能变成未定义的出现错误
     * name.toUtf8().toHex()
     * 这种逻辑是相同的但是这种级联返回的是对象即使上一级销毁了也没关系,
     */

    auto stdStr = name.toStdString();
    return Member(stdStr.c_str());
}

bool JsonReader::HasMember(const char* name) const
{
    if (!mError && CURRENT.IsObject() && TOP.state == JsonReaderStackItem::Started)
        return CURRENT.HasMember(name);
    return false;
}

bool JsonReader::HasMember(const QString& name) const
{
    auto stdStr = name.toStdString();
    return HasMember(stdStr.c_str());
}
bool JsonReader::CurKeyValue(QString& key, QVariant& value)
{
    mError = mError || (!(CURRENT.IsObject() && TOP.state == JsonReaderStackItem::Started));

    if (mError)
    {
        return false;
    }
    /* 这个没找到不应该算错误,应该是说这个值可能是可选的现在没有了而已 */
    Value::ConstMemberIterator memberItr = CURRENT.MemberBegin();
    if (memberItr == CURRENT.MemberEnd())
    {
        return false;
    }
    key = QString::fromUtf8(memberItr->name.GetString());
    value = valueToVariant(memberItr->value);
    return true;
}
QStringList JsonReader::Members()
{
    QStringList out;
    if (!mError && CURRENT.IsObject() && TOP.state == JsonReaderStackItem::Started)
    {
        for (auto ilter = CURRENT.MemberBegin(); ilter != CURRENT.MemberEnd(); ilter++)
        {
            out.push_back(QString::fromUtf8(ilter->name.GetString()));
        }
    }
    return out;
}

JsonReader& JsonReader::StartArray(size_t* size)
{
    if (!mError)
    {
        if (CURRENT.IsArray() && TOP.state == JsonReaderStackItem::BeforeStart)
        {
            TOP.state = JsonReaderStackItem::Started;
            if (size)
                *size = CURRENT.Size();

            if (!CURRENT.Empty())
            {
                const Value* value = &CURRENT[TOP.index];
                STACK->push(JsonReaderStackItem(value, JsonReaderStackItem::BeforeStart));
            }
            else
                TOP.state = JsonReaderStackItem::Closed;
        }
        else
            mError = true;
    }
    return *this;
}

JsonReader& JsonReader::EndArray()
{
    if (!mError)
    {
        if (CURRENT.IsArray() && TOP.state == JsonReaderStackItem::Closed)
            Next();
        else
            mError = true;
    }
    return *this;
}

JsonReader& JsonReader::operator&(bool& b)
{
    if (!mError)
    {
        if (CURRENT.IsBool())
        {
            b = CURRENT.GetBool();
            Next();
        }
        else
            mError = true;
    }
    return *this;
}

JsonReader& JsonReader::operator&(quint32& u)
{
    if (!mError)
    {
        if (CURRENT.IsUint())
        {
            u = CURRENT.GetUint();
            Next();
        }
        else
            mError = true;
    }
    return *this;
}

JsonReader& JsonReader::operator&(qint32& i)
{
    if (!mError)
    {
        if (CURRENT.IsInt())
        {
            i = CURRENT.GetInt();
            Next();
        }
        else
            mError = true;
    }
    return *this;
}

JsonReader& JsonReader::operator&(quint64& u)
{
    if (!mError)
    {
        if (CURRENT.IsUint64())
        {
            u = CURRENT.GetUint64();
            Next();
        }
        else
            mError = true;
    }
    return *this;
}
JsonReader& JsonReader::operator&(qint64& i)
{
    if (!mError)
    {
        if (CURRENT.IsInt64())
        {
            i = CURRENT.GetInt64();
            Next();
        }
        else
            mError = true;
    }
    return *this;
}
JsonReader& JsonReader::operator&(quint16& u)
{
    if (!mError)
    {
        if (CURRENT.IsUint())
        {
            u = static_cast<quint16>(CURRENT.GetUint());
            Next();
        }
        else
            mError = true;
    }
    return *this;
}
JsonReader& JsonReader::operator&(qint16& i)
{
    if (!mError)
    {
        if (CURRENT.IsInt())
        {
            i = static_cast<qint16>(CURRENT.GetInt());
            Next();
        }
        else
            mError = true;
    }
    return *this;
}

JsonReader& JsonReader::operator&(QVariant& s)
{
    if (!mError)
    {
        if (CURRENT.IsNull())
        {
            Next();
        }
        else if (CURRENT.IsString())
        {
            // s = QString(CURRENT.GetString()).toLatin1();
            s = QString::fromUtf8(CURRENT.GetString());
            Next();
        }
        else if (CURRENT.IsInt())
        {
            s = CURRENT.GetInt();
            Next();
        }
        else if (CURRENT.IsUint())
        {
            s = CURRENT.GetUint();
            Next();
        }
        else if (CURRENT.IsInt64())
        {
            s = static_cast<qint64>(CURRENT.GetInt64());
            Next();
        }
        else if (CURRENT.IsUint64())
        {
            s = static_cast<quint64>(CURRENT.GetUint64());
            Next();
        }
        else if (CURRENT.IsDouble())
        {
            s = CURRENT.GetDouble();
            Next();
        }
        else if (CURRENT.IsNumber())
        {
            s = CURRENT.GetDouble();
            Next();
        }
        else
            mError = true;
    }
    return *this;
}

JsonReader& JsonReader::operator&(QVariantMap& s)
{
    if (!mError && CURRENT.IsObject() && TOP.state == JsonReaderStackItem::Started)
    {
        for (auto ilter = CURRENT.MemberBegin(); ilter != CURRENT.MemberEnd(); ilter++)
        {
            s.insert(QString::fromUtf8(ilter->name.GetString()), valueToVariant(ilter->value));
        }
    }
    return *this;
}
JsonReader& JsonReader::operator&(QVariantList& s)
{
    if (!mError && CURRENT.IsObject() && TOP.state == JsonReaderStackItem::Started)
    {
        for (auto ilter = CURRENT.MemberBegin(); ilter != CURRENT.MemberEnd(); ilter++)
        {
            s.push_back(valueToVariant(ilter->value));
        }
    }
    return *this;
}

JsonReader& JsonReader::operator&(double& d)
{
    if (!mError)
    {
        if (CURRENT.IsNumber())
        {
            d = CURRENT.GetDouble();
            Next();
        }
        else
            mError = true;
    }
    return *this;
}
JsonReader& JsonReader::operator&(std::string& s)
{
    if (!mError)
    {
        if (CURRENT.IsString())
        {
            s = std::string(CURRENT.GetString());
            Next();
        }
        else
            mError = true;
    }
    return *this;
}

JsonReader& JsonReader::operator&(QString& s)
{
    if (!mError)
    {
        if (CURRENT.IsString())
        {
            s = QString::fromUtf8(CURRENT.GetString());
            Next();
        }
        else
            mError = true;
    }
    return *this;
}

JsonReader& JsonReader::operator&(QDateTime& s)
{
    // QDateTime转字符串
    if (!mError)
    {
        if (CURRENT.IsString())
        {
            auto str = QString::fromUtf8(CURRENT.GetString());
            s = QDateTime::fromString(str, "yyyy-MM-dd hh:mm:ss:zzz");
            Next();
        }
        else
            mError = true;
    }

    return *this;
}
JsonReader& JsonReader::operator&(QDate& s)
{
    if (!mError)
    {
        if (CURRENT.IsString())
        {
            auto str = QString::fromUtf8(CURRENT.GetString());
            s = QDate::fromString(str, "yyyy-MM-dd");
            Next();
        }
        else
            mError = true;
    }

    return *this;
}
JsonReader& JsonReader::operator&(QTime& s)
{
    if (!mError)
    {
        if (CURRENT.IsString())
        {
            auto str = QString::fromUtf8(CURRENT.GetString());
            s = QTime::fromString(str, "HH:mm:ss:zzz");
            Next();
        }
        else
            mError = true;
    }

    return *this;
}

JsonReader& JsonReader::operator&(QByteArray& s)
{
    if (!mError)
    {
        if (CURRENT.IsString())
        {
            auto str = QString::fromUtf8(CURRENT.GetString());
            s = QByteArray::fromBase64(str.toUtf8());
            Next();
        }
        else
            mError = true;
    }
    return *this;
}

void JsonReader::Next()
{
    if (!mError)
    {
        assert(!STACK->empty());
        STACK->pop();

        if (!STACK->empty() && CURRENT.IsArray())
        {
            if (TOP.state == JsonReaderStackItem::Started)
            {  // Otherwise means reading array item
               // pass end
                if (TOP.index < CURRENT.Size() - 1)
                {
                    const Value* value = &CURRENT[++TOP.index];
                    STACK->push(JsonReaderStackItem(value, JsonReaderStackItem::BeforeStart));
                }
                else
                    TOP.state = JsonReaderStackItem::Closed;
            }
            else
                mError = true;
        }
    }
}

#undef DOCUMENT
#undef STACK
#undef TOP
#undef CURRENT

////////////////////////////////////////////////////////////////////////////////
// JsonWriter

#define WRITER reinterpret_cast<PrettyWriter<StringBuffer>*>(mWriter)
#define STREAM reinterpret_cast<StringBuffer*>(mStream)

const bool JsonWriter::IsReader = false;
const bool JsonWriter::IsWriter = !JsonWriter::IsReader;

JsonWriter::JsonWriter()
{
    mStream = new StringBuffer;
    mWriter = new PrettyWriter<StringBuffer>(*STREAM);
}

JsonWriter::~JsonWriter()
{
    delete WRITER;
    delete STREAM;
}

const char* JsonWriter::GetString() const { return STREAM->GetString(); }
QByteArray JsonWriter::GetByteArray() const { return QByteArray(STREAM->GetString(), static_cast<int>(STREAM->GetSize())); }
QString JsonWriter::GetQString() const { return QString(GetString()); }

JsonWriter& JsonWriter::StartObject()
{
    WRITER->StartObject();
    return *this;
}

JsonWriter& JsonWriter::EndObject()
{
    WRITER->EndObject();
    return *this;
}

JsonWriter& JsonWriter::Member(const char* name)
{
    WRITER->String(name, static_cast<SizeType>(std::strlen(name)));
    return *this;
}

JsonWriter& JsonWriter::Member(const QString& name)
{
    auto stdStr = name.toStdString();
    WRITER->String(stdStr.c_str(), static_cast<SizeType>(stdStr.size()));
    return *this;
}

bool JsonWriter::HasMember(const char*) const
{
    // This function is for JsonReader only.
    assert(false);
    return false;
}

bool JsonWriter::HasMember(const QString& /*name*/) const
{
    // This function is for JsonReader only.
    assert(false);
    return false;
}

QStringList JsonWriter::Members()
{
    // This function is for JsonReader only.
    assert(false);
    return {};
}

JsonWriter& JsonWriter::StartArray()
{
    WRITER->StartArray();
    return *this;
}

JsonWriter& JsonWriter::EndArray()
{
    WRITER->EndArray();
    return *this;
}

JsonWriter& JsonWriter::operator&(bool b)
{
    WRITER->Bool(b);
    return *this;
}

JsonWriter& JsonWriter::operator&(quint32 u)
{
    WRITER->Uint(u);
    return *this;
}

JsonWriter& JsonWriter::operator&(qint32 i)
{
    WRITER->Int(i);
    return *this;
}

JsonWriter& JsonWriter::operator&(quint64 i)
{
    WRITER->Uint64(static_cast<uint64_t>(i));
    return *this;
}
JsonWriter& JsonWriter::operator&(qint64 i)
{
    WRITER->Int64(static_cast<int64_t>(i));
    return *this;
}
JsonWriter& JsonWriter::operator&(quint16 u)
{
    WRITER->Uint(static_cast<quint32>(u));
    return *this;
}
JsonWriter& JsonWriter::operator&(qint16 i)
{
    WRITER->Int(static_cast<qint32>(i));
    return *this;
}

JsonWriter& JsonWriter::operator&(const QVariant& s)
{
    writeVariantToJson(WRITER, s);
    return *this;
}

JsonWriter& JsonWriter::operator&(const QVariantMap& s)
{
    for (auto iter = s.begin(); iter != s.end(); ++iter)
    {
        Member(iter.key());
        writeVariantToJson(WRITER, iter.value());
    }
    return *this;
}
JsonWriter& JsonWriter::operator&(const QVariantList& s)
{
    auto templateStr = "value%1";
    auto size = s.size();
    for (int i = 0; i < size; ++i)
    {
        Member(QString(templateStr).arg(i));
        writeVariantToJson(WRITER, s.at(i));
    }
    return *this;
}

JsonWriter& JsonWriter::operator&(double d)
{
    WRITER->Double(d);
    return *this;
}

JsonWriter& JsonWriter::operator&(const std::string& s)
{
    WRITER->String(s.c_str(), static_cast<SizeType>(s.size()));
    return *this;
}

JsonWriter& JsonWriter::operator&(const QString& s)
{
    /* toStdstring是一样的 还多一步拷贝 */
    auto array = s.toUtf8();
    WRITER->String(array.constData(), static_cast<SizeType>(array.size()));
    return *this;
}

JsonWriter& JsonWriter::operator&(const QDateTime& s)
{
    /* toStdstring是一样的 还多一步拷贝 */
    auto str = s.toString("yyyy-MM-dd hh:mm:ss:zzz");
    auto array = str.toUtf8();
    WRITER->String(array.constData(), static_cast<SizeType>(array.size()));
    return *this;
}
JsonWriter& JsonWriter::operator&(const QDate& s)
{
    auto str = s.toString("yyyy-MM-dd");
    auto array = str.toUtf8();
    WRITER->String(array.constData(), static_cast<SizeType>(array.size()));
    return *this;
}
JsonWriter& JsonWriter::operator&(const QTime& s)
{
    auto str = s.toString("HH:mm:ss:zzz");
    auto array = str.toUtf8();
    WRITER->String(array.constData(), static_cast<SizeType>(array.size()));
    return *this;
}

JsonWriter& JsonWriter::operator&(const QByteArray& s)
{
    /* 直接使用二进制,json无法传输的 */
    auto base64 = s.toBase64();
    WRITER->String(base64.constData(), static_cast<SizeType>(base64.size()));
    return *this;
}

JsonWriter& JsonWriter::SetNull()
{
    WRITER->Null();
    return *this;
}

#undef STREAM
#undef WRITER
