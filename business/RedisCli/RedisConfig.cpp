#include "RedisConfig.h"
#include "PlatformInterface.h"
#include <QDebug>
#include <QDomDocument>
#include <QFile>

class RedisConfigImpl
{
public:
    static Optional<QDomDocument> openXml();
    static RedisConfigInfo reload();
};

Optional<QDomDocument> RedisConfigImpl::openXml()
{
    using ResType = Optional<QDomDocument>;

    auto filePath = PlatformConfigTools::configBusiness("RedisConfig.xml");
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        auto errMsg = QString("%1:%2").arg(file.errorString(), filePath);
        return ResType(ErrorCode::OpenFileError, errMsg);
    }

    QDomDocument domDoc;
    QString errStr;
    int errLine{ 0 };
    int errcCol{ 0 };
    if (!domDoc.setContent(&file, &errStr, &errLine, &errcCol))
    {
        errStr = QString("设备配置信息解析失败:%1,错误行:%2,错误列:%2").arg(errStr).arg(errLine).arg(errcCol);
        return ResType(ErrorCode::XmlParseError, errStr);
    }
    return ResType(domDoc);
}
RedisConfigInfo RedisConfigImpl::reload()
{
    auto doc = openXml();
    if (!doc.success())
    {
        return RedisConfigInfo();
    }

    auto root = doc->documentElement();

    RedisConfigInfo info;
    auto node = root.firstChild();
    while (!node.isNull())
    {
        auto isComment = node.isComment();
        auto name = node.nodeName();
        auto element = node.toElement();
        node = node.nextSibling();

        /* 跳过注释 */
        if (isComment)
        {
            continue;
        }

        if (name == "host")
        {
            info.host = element.text().trimmed();
        }
        else if (name == "port")
        {
            info.port = static_cast<quint16>(element.text().trimmed().toUInt());
        }
        else if (name == "auth")
        {
            info.auth = element.text().trimmed();
        }
    }
    return info;
}

/************************************************************************************/
/************************************************************************************/
/************************************************************************************/
/************************************************************************************/

RedisConfigInfo RedisConfig::loadConfig() { return RedisConfigImpl::reload(); }
