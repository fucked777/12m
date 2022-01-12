#ifndef AUDIOPLAYSERVER_H
#define AUDIOPLAYSERVER_H

#include <QAudio>
#include <QMap>
#include <QObject>
#include <QThread>

struct AudioItem;
class AudioPlayServerImpl;
class AudioPlayServer : public QObject
{
    Q_OBJECT
public:
    explicit AudioPlayServer(QWidget* parent = nullptr);
    ~AudioPlayServer();
    QMap<QString, AudioItem> configData(); /* 获取当前的音频配置信息 */
    qint32 interval(const QString&);       /* 获取指定音频的重复播放时间间隔 */
    void waitQuit();                       /* 等待音频服务退出 */

private:
    /* priority未使用 */
    void playAudio(qint32 /*priority*/, const QString& key);
    void timerEvent(QTimerEvent* event) override;
    void next(); /* 播放下一个 */
    void audioAtateChanged(QAudio::State state);
    void startService();                                  /* 启动服务 */
    void stopService();                                   /* 停止服务 */
    void enableItem(const QString&, bool enable);         /* 启用/禁用某个音频控制 */
    void changeInterval(const QString&, qint32 interval); /* 更改某个音频重复播放时间间隔 */
    void setConfigData(const QMap<QString, AudioItem>&);
    void clearAllAudio();

signals:
    void sg_enableItem(const QString&, bool enable);         /* 启用/禁用某个音频控制 */
    void sg_changeInterval(const QString&, qint32 interval); /* 更改某个音频重复播放时间间隔 */
    void sg_start();
    void sg_stop();
    void sg_next();
    void sg_playAudio(qint32 /*priority*/, const QString& key);
    void sg_setConfigData(const QMap<QString, AudioItem>&);
    void sg_clearAllAudio();

private:
    AudioPlayServerImpl* m_impl;
};

#endif  // AUDIOPLAYSERVER_H
