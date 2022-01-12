#include "Utility.h"
#include <QCoreApplication>
#include <QDate>
#include <QDir>
#include <QGlobalStatic>
#include <QHash>
#include <QHeaderView>
#include <QRegularExpression>
#include <QTableView>
#include <QUuid>
#include <QVariant>

void Utility::killTimer(QObject* object, int& timerID)
{
    if (timerID != -1)
    {
        object->killTimer(timerID);
        timerID = -1;
    }
}
void Utility::startTimer(QObject* object, int& timerID, int interval)
{
    if (timerID == -1)
    {
        timerID = object->startTimer(interval);
    }
}
class ErrorCodeStrHelper
{
    QHash<ErrorCode, QString> m_errorCodeStr;

public:
    ErrorCodeStrHelper()
    {
#define X(a, b, c) c,
        const char* gErrorCodeCStr[] = { ERROR_CODE(X) };
#undef X
#define X(a, b, c) ErrorCode::a,
        ErrorCode gErrorCodeEnum[] = { ERROR_CODE(X) };
#undef X
        static_assert((sizeof(gErrorCodeCStr) / sizeof(gErrorCodeCStr[0])) == (sizeof(gErrorCodeEnum) / sizeof(gErrorCodeEnum[0])), "枚举参数不相同");
        for (std::size_t i = 0; i < (sizeof(gErrorCodeCStr) / sizeof(gErrorCodeCStr[0])); ++i)
        {
            m_errorCodeStr.insert(gErrorCodeEnum[i], gErrorCodeCStr[i]);
        }
    }
    ~ErrorCodeStrHelper() {}

    inline QString getErrorStr(ErrorCode code) { return m_errorCodeStr.value(code, "未知错误"); }
};

inline uint qHash(const ErrorCode& key, uint seed) { return qHash(static_cast<typename EnumSizeType<sizeof(ErrorCode)>::type>(key), seed); }

Q_GLOBAL_STATIC(ErrorCodeStrHelper, gErrorCodeStr);
QString Utility::errorCodeStr(ErrorCode code) { return gErrorCodeStr->getErrorStr(code); }
QString Utility::dirStitchingQt(const QString& dir1, const QString& dir2)
{
    /* 先全部变成/  然后替换 */
    auto tempDir = QDir::fromNativeSeparators(QString("%1/%2").arg(dir1, dir2));
    tempDir.replace(QRegularExpression("/+"), "/");
    QDir dir(tempDir);
    return dir.absolutePath();
}
QString Utility::dirStitchingWin(const QString& dir1, const QString& dir2)
{
    /* 先全部变成\  然后替换 */
    auto tempDir = QDir::fromNativeSeparators(QString("%1\\%2").arg(dir1, dir2));
    tempDir.replace(QRegularExpression("\\+"), "\\");
    QDir dir(tempDir);
    return dir.absolutePath();
}
QString Utility::dirStitchingLinux(const QString& dir1, const QString& dir2) { return dirStitchingQt(dir1, dir2); }

QString Regexp::ip() { return R"(^((25[0-5]|2[0-4]\d|[01]?\d\d?)\.){3}(25[0-5]|2[0-4]\d|[01]?\d\d?)$)"; }
QString Regexp::CodeString() { return R"([0-9a-zA-Z_]+)"; }
QString Regexp::hex(int maxLen) { return maxLen <= 0 ? "^[a-fA-F0-9]+$" : QString("[a-fA-F0-9]{1,%1}").arg(maxLen); }
QString Regexp::hexFWM(int fixLen) { return fixLen <= 0 ? "^[a-fA-F0-9]+$" : QString("[a-fA-F0-9]{%1}").arg(fixLen); }
QString Regexp::englishChar(int len) { return len <= 0 ? "^[A-Za-z0-9_]+$" : QString("[A-Za-z0-9_]{1,%1}").arg(len); }
QString Regexp::englishCharFWM(int fixLen) { return fixLen <= 0 ? "^[A-Za-z0-9_]+$" : QString("[A-Za-z0-9_]{%1}").arg(fixLen); }
QString Regexp::english(int len) { return len <= 0 ? "^[A-Za-z]+$" : QString("[A-Za-z]{%1}").arg(len); }

bool Regexp::matching(const QString& regStr, const QString& str)
{
    QRegularExpression reg(regStr);
    auto match = reg.match(str);
    return match.hasMatch();
}
