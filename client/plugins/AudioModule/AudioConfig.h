#ifndef AUDIOCONFIG_H
#define AUDIOCONFIG_H
#include <QDomElement>
#include <QMap>
#include <QString>
#include <QVariant>

struct AudioItem
{
    bool isEnable{ true }; /* 是否启用 */
    QString audioFileDir;
    QString audioFileName;               /* 音频文件名称,后缀一定是wav(这里不带后缀) */
    QString desc;                        /* 显示的名称 */
    qint32 coiledPlayInterval{ 5 * 60 }; /* 连续播放的时间间隔,单位秒 */
    qint64 timeRecord{ 0 };
};

struct AudioInfo
{
    qint32 layoutColumn{ 4 };                   /* 默认4列 */
    qint32 defaultCoiledPlayInterval{ 5 * 60 }; /* 连续播放的时间间隔,单位秒 */
    QString audioFileDir;                       /* 音频文件目录,读取时会转换成绝对路径保存时会还原成相对路径 */
    QString configFilePath;                     /* 配置文件的路径,保存时使用 */
    QString title;
    QMap<QString, AudioItem> audioMap; /* 音频映射 */
};

class AudioConfig
{
public:
    static bool parseConfig(QList<AudioInfo>& audioInfoList, QString& errMsg);
    static bool saveConfig(AudioInfo& audioInfo, QString& errMsg, qint32 defaultCoiledPlayInterval = -1);
    static QString dirStitchingQt(const QString& dir1, const QString& dir2);

private:
    static bool parseConfig(AudioInfo& audioInfo, QString& errMsg, const QString& filePath);
    static void parseItem(QDomElement& element, AudioInfo& audioInfo);
};
Q_DECLARE_METATYPE(AudioItem);
Q_DECLARE_METATYPE(AudioInfo);
#endif  // AUDIOCONFIG_H
