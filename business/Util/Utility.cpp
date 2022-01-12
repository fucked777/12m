#include "Utility.h"
#include "CommonSelectDelegate.h"
#include "JsonHelper.h"
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

#include "GlobalData.h"

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
void Utility::tableViewInit(QTableView* view, bool enableStretchLastSection)
{
    if (view == nullptr)
    {
        return;
    }
    view->setSelectionMode(QAbstractItemView::SingleSelection);  //设置单行选择
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setFrameShape(QFrame::NoFrame);
    view->setAlternatingRowColors(true);

    auto hHeaderView = view->horizontalHeader();
    /*第二个参数可以为：
     * QHeaderView::Interactive     ：0 用户可设置，也可被程序设置成默认大小
     * QHeaderView::Fixed           ：2 用户不可更改列宽
     * QHeaderView::Stretch         ：1 根据空间，自动改变列宽，用户与程序不能改变列宽
     * QHeaderView::ResizeToContents：3 根据内容改变列宽，用户与程序不能改变列宽
     */
    hHeaderView->setSectionResizeMode(QHeaderView::Interactive);
    hHeaderView->setHighlightSections(false);
    /* 这个个东东好像会和固定列宽冲突导致界面卡死 */
    if (enableStretchLastSection)
    {
        hHeaderView->setStretchLastSection(true);
    }

    auto vHeaderView = view->verticalHeader();
    vHeaderView->setVisible(false);

    view->setItemDelegate(new CommonSelectDelegate(view));
}
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
QTime Utility::timeFromStr(const QString& str) { return QTime::fromString(str, "HH:mm:ss"); }
QString Utility::timeToStr(const QTime& t) { return t.toString("HH:mm:ss"); }
QString Utility::timeToStr() { return GlobalData::currentTime().toString("HH:mm:ss"); }

QDateTime Utility::dateTimeFromStr(const QString& str) { return QDateTime::fromString(str, "yyyy-MM-dd HH:mm:ss"); }
QString Utility::dateTimeToStr(const QDateTime& t) { return t.toString("yyyy-MM-dd HH:mm:ss"); }
QString Utility::dateTimeToStr() { return GlobalData::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"); }

QDate Utility::dateFromStr(const QString& str) { return QDate::fromString(str, "yyyy-MM-dd"); }
QString Utility::dateToStr(const QDate& t) { return t.toString("yyyy-MM-dd"); }
QString Utility::dateToStr() { return GlobalData::currentDate().toString("yyyy-MM-dd"); }

QString Utility::createTimeUuid() { return createTimeUuid(GlobalData::currentDateTime()); }
QString Utility::createTimeUuid(const QDateTime& dateTime)
{
    return QString("%1-%2").arg(dateTime.toString("yyyyMMddHHmmss"), QUuid::createUuid().toString());
}
QString Utility::createTimeSearchPrefix(const QString& suffix) { return createTimeSearchPrefix(GlobalData::currentDateTime(), suffix); }
QString Utility::createTimeSearchPrefix(const QDateTime& dateTime, const QString& suffix)
{
    return QString("%1%2").arg(dateTime.toString("yyyyMMddHHmmss"), suffix);
}

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
