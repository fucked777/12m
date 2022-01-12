#include "PlatformConfigTools.h"
#include "PlatformDefine.h"
#include <QCoreApplication>
#include <QDir>
#include <QMutex>
#include <QMutexLocker>
#include <QRegularExpression>
#include <QVariant>

#include <QDebug>

class PlatformToolsImpl
{
public:
    static QString getDataStr(const char* key) { return getDataVar(key).toString(); }
    static QVariant getDataVar(const char* key) { return qApp->property(key); }

    static void setData(const char* key, const QVariant& value) { qApp->setProperty(key, value); }

    static QString configStitching(const char* key, const QString& file)
    {
        Q_ASSERT(!file.isEmpty());
        auto tempDir = getDataStr(key);
        /* 先全部变成/  然后替换 */
        tempDir = QDir::fromNativeSeparators(QString("%1/%2").arg(tempDir, file));
        return tempDir.replace(QRegularExpression("/+"), "/");
    }
};

/***************************************************************************************/
/***************************************************************************************/
/***************************************************************************************/
/***************************************************************************************/
/***************************************************************************************/
/***************************************************************************************/

static QString plugDirImpl()
{
    auto plugDir = QString("%1/plugins").arg(QCoreApplication::applicationDirPath());
    return QDir(plugDir).absolutePath();
}
QString PlatformConfigTools::plugDir()
{
    static auto dir = plugDirImpl();
    return dir;
}
QString PlatformConfigTools::plugPath(const QString& name)
{
    static auto dir = plugDirImpl();
#if defined(Q_OS_LINUX)
    static auto createPlugFullPath = [](const QString& moduleName) {
        auto tempPath = QString("%1/lib%2.so").arg(dir, moduleName);
        return QDir::fromNativeSeparators(tempPath);
    };
#else
    static auto createPlugFullPath = [](const QString& moduleName) {
        auto tempPath = QString("%1/%2.dll").arg(dir, moduleName);
        return QDir::toNativeSeparators(tempPath);
    };
#endif
    return createPlugFullPath(name);
}
QString PlatformConfigTools::plugPath(const QString& name, const QString& prefix, const QString& suffix)
{
    static auto dir = plugDirImpl();
    if (suffix.isEmpty())
    {
        return QString("%1/%2%3").arg(dir, prefix, name);
    }
    else
    {
        return QString("%1/%2%3.%4").arg(dir, prefix, name, suffix);
    }
}

QString PlatformConfigTools::configPlatformDir() { return PlatformToolsImpl::getDataStr("PlatformConfigDir"); }
QString PlatformConfigTools::configBusinessDir() { return PlatformToolsImpl::getDataStr("BusinessConfigDir"); }
QString PlatformConfigTools::configShareDir() { return PlatformToolsImpl::getDataStr("ShareConfigDir"); }

QString PlatformConfigTools::instanceID() { return PlatformToolsImpl::getDataStr("InstanceID"); }

QString PlatformConfigTools::configPlatform(const QString& subDir) { return PlatformToolsImpl::configStitching("PlatformConfigDir", subDir); }
QString PlatformConfigTools::configBusiness(const QString& subDir) { return PlatformToolsImpl::configStitching("BusinessConfigDir", subDir); }
QString PlatformConfigTools::configShare(const QString& subDir) { return PlatformToolsImpl::configStitching("ShareConfigDir", subDir); }

QVariant PlatformConfigTools::value(const char* key) { return PlatformToolsImpl::getDataVar(key); }
QString PlatformConfigTools::valueStr(const char* key) { return PlatformToolsImpl::getDataStr(key); }
void PlatformConfigTools::setValue(const char* key, const QVariant& value) { PlatformToolsImpl::setData(key, value); }

void PlatformConfigTools::platformConfigFromMap(PlatformConfigInfo& info, const QVariantMap& tempMap)
{
    info.mainInfo.name = tempMap.value("PlatformName", info.mainInfo.name).toString();
    info.mainInfo.platformConfigDir = tempMap.value("PlatformConfigDir", info.mainInfo.platformConfigDir).toString();
    info.mainInfo.businessConfigDir = tempMap.value("BusinessConfigDir", info.mainInfo.businessConfigDir).toString();
    info.mainInfo.shareConfigDir = tempMap.value("ShareConfigDir", info.mainInfo.shareConfigDir).toString();

    for (auto iter = info.extendedInfo.begin(); iter != info.extendedInfo.end(); ++iter)
    {
        auto& value = iter.value();
        value = tempMap.value(iter.key(), value);
    }

    info.uiInfo.start = tempMap.value("Start", info.uiInfo.start).toString();
    info.uiInfo.end = tempMap.value("End", info.uiInfo.end).toString();
    info.uiInfo.mainWindow = tempMap.value("MainWindow", info.uiInfo.mainWindow).toString();
}

void PlatformConfigTools::setPlatformConfigInfo(const PlatformConfigInfo& info)
{
    /* 写入配置 */
    PlatformToolsImpl::setData("InstanceID", info.mainInfo.instanceID);
    PlatformToolsImpl::setData("PlatformConfigDir", info.mainInfo.platformConfigDir);
    PlatformToolsImpl::setData("BusinessConfigDir", info.mainInfo.businessConfigDir);
    PlatformToolsImpl::setData("ShareConfigDir", info.mainInfo.shareConfigDir);

    for (auto iter = info.extendedInfo.begin(); iter != info.extendedInfo.end(); ++iter)
    {
        auto str = iter.key().toUtf8();
        PlatformToolsImpl::setData(str.constData(), iter.value());
    }
}
