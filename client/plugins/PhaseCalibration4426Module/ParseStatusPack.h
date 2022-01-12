//#ifndef PARSESTATUSPACK_H
//#define PARSESTATUSPACK_H

//#include "PacketCommonType.h"
//#include <QObject>

// class ParseStatusPackImpl;
// enum class SCWorkMode;
// class ParseStatusPack : public QObject
//{
//    Q_OBJECT
// public:
//    ParseStatusPack(QObject* parent = nullptr);
//    ~ParseStatusPack();
//    void stop();
//    void start();

//    /* 获取当前天线的校相模式 */
//    static int getTFSPCModle();

// private:
//    void timerEvent(QTimerEvent* event);  //定时检测参数更新的时间戳

// signals:

//    void sig_sendTFS(quint64 deviceID, int modeID, bool, const UnpackMessage& unpackMessage); /* 发送天线的数据 */
//    void sig_resetAll();

// private:
//    ParseStatusPackImpl* m_impl;
//};

//#endif  // PARSESTATUSPACK_H
