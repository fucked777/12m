#include "AudioPlayServer.h"
#include "AudioConfig.h"
#include "CConverter.h"
#include "GlobalData.h"
#include "PlatformConfigTools.h"
#include "RedisHelper.h"
#include "SubscriberHelper.h"
#include <QApplication>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QDebug>
#include <QDir>
#include <QDomDocument>
#include <QFile>
#include <QMap>
#include <QMutex>
#include <QRegularExpression>
#include <QSoundEffect>
#include <QTabWidget>
#include <QtConcurrent>
#include <atomic>

class AudioPlayServerImpl
{
public:
    cpp_redis::subscriber audioSubscriber;
    QList<AudioItem> audioPlayList;
    std::atomic<bool> runFlag{ false };
    int timerID{ -1 };
    QTabWidget* tabWidget{ nullptr };
    QMap<QString, AudioItem> audioMap;    /* 音频映射 */
    QAudioOutput* audioOutput{ nullptr }; /* 播放音频 */
    int playStatusCount{ 0 };             /* 播放状态计数 */
    QFile audioFile;
};

AudioPlayServer::AudioPlayServer(QWidget* parent)
    : QObject(parent)
    , m_impl{ new AudioPlayServerImpl }
{
    qRegisterMetaType<QMap<QString, AudioItem>>("QMap<QString, AudioItem>&");
    qRegisterMetaType<QMap<QString, AudioItem>>("QMap<QString, AudioItem>");
    qRegisterMetaType<QMap<QString, AudioItem>>("const QMap<QString, AudioItem>&");

    connect(this, &AudioPlayServer::sg_playAudio, this, &AudioPlayServer::playAudio);
    connect(this, &AudioPlayServer::sg_next, this, &AudioPlayServer::next);
    connect(this, &AudioPlayServer::sg_start, this, &AudioPlayServer::startService);
    connect(this, &AudioPlayServer::sg_stop, this, &AudioPlayServer::stopService);
    connect(this, &AudioPlayServer::sg_enableItem, this, &AudioPlayServer::enableItem);
    connect(this, &AudioPlayServer::sg_changeInterval, this, &AudioPlayServer::changeInterval);
    connect(this, &AudioPlayServer::sg_setConfigData, this, &AudioPlayServer::setConfigData);
    connect(this, &AudioPlayServer::sg_clearAllAudio, this, &AudioPlayServer::clearAllAudio);

    connect(SubscriberHelper::getInstance(),&SubscriberHelper::sg_statusChange,this,&AudioPlayServer::masterStatusChange);
}

AudioPlayServer::~AudioPlayServer()
{
    stopService();
    delete m_impl;
}
QMap<QString, AudioItem> AudioPlayServer::configData() { return m_impl->audioMap; }
void AudioPlayServer::enableItem(const QString& key, bool enable)
{
    auto find = m_impl->audioMap.find(key);
    if (find == m_impl->audioMap.end())
    {
        return;
    }
    find->isEnable = enable;
}
void AudioPlayServer::changeInterval(const QString& key, qint32 interval)
{
    auto find = m_impl->audioMap.find(key);
    if (find == m_impl->audioMap.end())
    {
        return;
    }
    find->coiledPlayInterval = interval;
}
void AudioPlayServer::setConfigData(const QMap<QString, AudioItem>& configMap) { m_impl->audioMap = configMap; }
void AudioPlayServer::clearAllAudio() { m_impl->audioPlayList.clear(); }
qint32 AudioPlayServer::interval(const QString& key)
{
    auto find = m_impl->audioMap.find(key);
    if (find == m_impl->audioMap.end())
    {
        return {};
    }
    return find->coiledPlayInterval;
}
void AudioPlayServer::stopService()
{
    if (m_impl->timerID != -1)
    {
        killTimer(m_impl->timerID);
        m_impl->timerID = -1;
    }
    if (!m_impl->runFlag)
    {
        return;
    }
    m_impl->runFlag = false;
    blockSignals(!m_impl->runFlag);
    m_impl->audioOutput->stop();
    m_impl->audioOutput->reset();
    m_impl->audioPlayList.clear();
    delete m_impl->audioOutput;
    m_impl->audioOutput = nullptr;
    blockSignals(m_impl->runFlag);
}
void AudioPlayServer::waitQuit()
{
    while (m_impl->runFlag)
    {
        QThread::msleep(200);
    }
}
void AudioPlayServer::startService()
{
    if (m_impl->runFlag)
    {
        return;
    }
    m_impl->runFlag = true;
    if (m_impl->timerID == -1)
    {
        m_impl->timerID = startTimer(2000);
    }
    blockSignals(!m_impl->runFlag);

    /* 这里的wav文件全部采用统一的格式 */
    QAudioFormat audioFormat;
    //设置采样率
    audioFormat.setSampleRate(22000);
    //设置通道数
    audioFormat.setChannelCount(1);
    //设置采样大小，一般为8位或16位
    audioFormat.setSampleSize(16);
    //设置编码方式
    audioFormat.setCodec("audio/pcm");
    //设置字节序
    audioFormat.setByteOrder(QAudioFormat::LittleEndian);
    //设置样本数据类型
    audioFormat.setSampleType(QAudioFormat::UnSignedInt);
    m_impl->audioOutput = new QAudioOutput(audioFormat);
    connect(m_impl->audioOutput, &QAudioOutput::stateChanged, this, &AudioPlayServer::audioAtateChanged);
}

void AudioPlayServer::playAudio(qint32 /*priority*/, const QString& key)
{
    auto find = m_impl->audioMap.find(key);
    if (find == m_impl->audioMap.end())
    {
        return;
    }
    auto& infoItem = find.value();
    if (!infoItem.isEnable)
    {
        return;
    }
    /* 还没到时候 */
    auto curTimeMS = GlobalData::currentDateTime().toMSecsSinceEpoch();
    if (curTimeMS - infoItem.timeRecord <= infoItem.coiledPlayInterval * 1000)
    {
        return;
    }
    infoItem.timeRecord = curTimeMS;
    /* 阻塞的太厉害了,先清空,放不过来了 */
    if (m_impl->audioPlayList.size() > 100)
    {
        m_impl->audioPlayList.clear();
    }
    m_impl->audioPlayList.append(infoItem);
    emit sg_next();
}
void AudioPlayServer::audioAtateChanged(QAudio::State state)
{
    switch (state)
    {
        /* 正在播放 */
    case QAudio::ActiveState:
    {
        ++m_impl->playStatusCount;
        if (m_impl->playStatusCount > 50)
        {
            m_impl->audioOutput->reset();
            emit sg_next();
        }
        break;
    }
    case QAudio::SuspendedState:
    {
        /* 暂停 这里不使用 */
        break;
    }
    case QAudio::StoppedState:
    {
        /* 停止 这里不使用 */
        // emit sg_next();
        break;
    }
    case QAudio::IdleState:
    {
        /* 没有数据了 认为放完了 */
        emit sg_next();
        break;
    }
    default: break;
    }
}
void AudioPlayServer::next()
{
    m_impl->playStatusCount = 0;
    if (m_impl->audioPlayList.isEmpty() || m_impl->audioOutput->state() == QAudio::ActiveState)
    {
        return;
    }

    auto audioInfoItem = m_impl->audioPlayList.first();
    m_impl->audioPlayList.pop_front();
    m_impl->audioOutput->reset();

    audioInfoItem.audioFileDir = PlatformConfigTools::configBusinessDir();
    auto filePath = AudioConfig::dirStitchingQt(audioInfoItem.audioFileDir + "/Audio/AudioFile/", QString("%1.wav").arg(audioInfoItem.audioFileName));
    m_impl->audioFile.close();
    m_impl->audioFile.setFileName(filePath);
    if (!m_impl->audioFile.open(QIODevice::ReadOnly))
    {
        emit sg_next();
        return;
    }
    m_impl->audioOutput->start(&(m_impl->audioFile));
}

void AudioPlayServer::masterStatusChange()
{
    if (m_impl->audioSubscriber.is_connected())
    {
        m_impl->audioSubscriber.disconnect();
    }
}
void AudioPlayServer::timerEvent(QTimerEvent* /*event*/)
{
    if (m_impl->audioSubscriber.is_connected())
    {
        return;
    }
    if (!RedisHelper::getInstance().getSubscriber(m_impl->audioSubscriber))
    {
        return;
    }

    m_impl->audioSubscriber.subscribe("Music", [=](const std::string& /*chanel*/, const std::string& msg) {
        auto qmsg = QString::fromStdString(msg);
        emit sg_playAudio(-1, qmsg);
        // playAudio(-1, qmsg);
    });
    m_impl->audioSubscriber.commit();
}
