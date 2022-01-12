#include "AudioConfig.h"
#include "PlatformConfigTools.h"
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QTextStream>

QString AudioConfig::dirStitchingQt(const QString& dir1, const QString& dir2)
{
    /* 先全部变成/  然后替换 */
    auto tempDir = QDir::fromNativeSeparators(QString("%1/%2").arg(dir1, dir2));
    tempDir.replace(QRegularExpression("/+"), "/");
    QDir dir(tempDir);
    return dir.absolutePath();
}
void AudioConfig::parseItem(QDomElement& element, AudioInfo& audioInfo)
{
    auto node = element.firstChild();
    while (!node.isNull())
    {
        auto name = node.nodeName();
        auto isComment = node.isComment();
        auto element = node.toElement();
        node = node.nextSibling();
        /* 跳过注释 */
        if (isComment)
        {
            continue;
        }
        if (name != "Item")
        {
            continue;
        }
        auto tempMapStr = element.attribute("map").trimmed();
        if (tempMapStr.isEmpty())
        {
            continue;
        }
        auto tempList = tempMapStr.split("|");
        if (tempList.size() != 2)
        {
            continue;
        }
        AudioItem item;
        item.audioFileName = tempList.at(1);

        if (element.hasAttribute("interval"))
        {
            item.coiledPlayInterval = element.attribute("interval").trimmed().toInt();
            item.coiledPlayInterval = item.coiledPlayInterval <= 0 ? audioInfo.defaultCoiledPlayInterval : item.coiledPlayInterval;
        }
        else
        {
            item.coiledPlayInterval = audioInfo.defaultCoiledPlayInterval;
        }

        if (element.hasAttribute("desc"))
        {
            item.desc = element.attribute("desc").trimmed();
        }
        else
        {
            item.desc = tempList.at(1);
        }

        item.audioFileDir = audioInfo.audioFileDir;
        item.isEnable = QVariant(element.attribute("enable", "true").trimmed()).toBool();
        audioInfo.audioMap.insert(tempList.at(0), item);
    }
}
bool AudioConfig::parseConfig(AudioInfo& audioInfo, QString& errMsg, const QString& filePath)
{
    /* 加载音频的配置文件
     * 20210315暂定是直接录音然后使用一个key对应一个音频然后播放
     */
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        errMsg = file.errorString();
        return false;
    }
    QDomDocument doc;
    if (!doc.setContent(&file, &errMsg))
    {
        return false;
    }
    QDomElement root = doc.documentElement();

    auto node = root.firstChild();
    while (!node.isNull())
    {
        auto name = node.nodeName();
        auto isComment = node.isComment();
        auto element = node.toElement();
        node = node.nextSibling();
        /* 跳过注释 */
        if (isComment)
        {
            continue;
        }

        if (name == "AudioPath")
        {
            audioInfo.audioFileDir = dirStitchingQt(QApplication::applicationDirPath(), element.text().trimmed());
        }
        if (name == "Title")
        {
            audioInfo.title = element.text().trimmed();
        }
        if (name == "Interval")
        {
            audioInfo.defaultCoiledPlayInterval = element.text().trimmed().toInt();
        }
        if (name == "Column")
        {
            audioInfo.layoutColumn = element.text().trimmed().toInt();
            audioInfo.layoutColumn = audioInfo.layoutColumn <= 0 ? 4 : audioInfo.layoutColumn;
        }
        else if (name == "Audio")
        {
            parseItem(element, audioInfo);
        }
    }
    return true;
}
bool AudioConfig::parseConfig(QList<AudioInfo>& audioInfoList, QString& errMsg)
{
    QList<AudioInfo> tempAudioInfoList;
    /* 加载音频的配置文件
     * 20210315暂定是直接录音然后使用一个key对应一个音频然后播放
     */

    QString configDir = PlatformConfigTools::configBusinessDir() + "/Audio/";
    QDir dir(configDir);
    if (!dir.exists())
    {
        errMsg = QString("未找到音频配置文件目录:%1").arg(configDir);
        return false;
    }

    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    auto dirInfoList = dir.entryInfoList();

    for (auto& fileInfoItem : dirInfoList)
    {
        AudioInfo audioInfo;
        audioInfo.configFilePath = fileInfoItem.absoluteFilePath();
        auto ret = parseConfig(audioInfo, errMsg, audioInfo.configFilePath);
        if (!ret)
        {
            return false;
        }
        if (audioInfo.audioMap.isEmpty())
        {
            continue;
        }
        tempAudioInfoList << audioInfo;
    }
    audioInfoList = tempAudioInfoList;
    return true;
}
bool AudioConfig::saveConfig(AudioInfo& audioInfo, QString& errMsg, qint32 defaultCoiledPlayInterval)
{
    QDomDocument doc;
    //写入xml头部
    QDomProcessingInstruction instruction;  //添加处理命令
    instruction = doc.createProcessingInstruction("xml", R"(version="1.0" encoding="UTF-8")");
    doc.appendChild(instruction);
    //添加根节点
    auto root = doc.createElement("Configure");
    doc.appendChild(root);

    auto tempPath = QApplication::applicationDirPath() + "/";
    auto element = doc.createElement("AudioPath");
    element.appendChild(doc.createTextNode(audioInfo.audioFileDir.replace(tempPath, "")));
    root.appendChild(element);

    element = doc.createElement("Title");
    element.appendChild(doc.createTextNode(audioInfo.title));
    root.appendChild(element);

    element = doc.createElement("Interval");
    element.appendChild(
        doc.createTextNode(QString::number(defaultCoiledPlayInterval <= 0 ? audioInfo.defaultCoiledPlayInterval : defaultCoiledPlayInterval)));
    root.appendChild(element);

    element = doc.createElement("Column");
    element.appendChild(doc.createTextNode(QString::number(audioInfo.layoutColumn)));
    root.appendChild(element);

    element = doc.createElement("Audio");

    for (auto iter = audioInfo.audioMap.begin(); iter != audioInfo.audioMap.end(); ++iter)
    {
        auto tempElement = doc.createElement("Item");
        tempElement.setAttribute("map", QString("%1|%2").arg(iter.key(), iter->audioFileName));
        tempElement.setAttribute("enable", iter->isEnable ? "true" : "false");
        tempElement.setAttribute("desc", iter->desc);
        if (iter->coiledPlayInterval != audioInfo.defaultCoiledPlayInterval)
        {
            tempElement.setAttribute("interval", QString::number(iter->coiledPlayInterval));
        }
        element.appendChild(tempElement);
    }

    root.appendChild(element);

    //写xml
    QFile file(audioInfo.configFilePath);                //相对路径、绝对路径、资源路径都可以
    if (!file.open(QFile::WriteOnly | QFile::Truncate))  //可以用QIODevice，Truncate表示清空原来的内容
    {
        errMsg = file.errorString();
        return false;
    }

    //输出到文件
    QTextStream outStream(&file);
    doc.save(outStream, 4);  //缩进4格
    return true;
}
