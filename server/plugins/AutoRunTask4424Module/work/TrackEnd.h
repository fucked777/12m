#ifndef TRACKEND_H
#define TRACKEND_H

#include "BaseEvent.h"
struct TaskTimeList;
class TrackEnd : public BaseEvent
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit TrackEnd(QObject* parent = nullptr);
    static int typeId;  //实现反射使用的

    void doSomething();

    // 基带停止存盘 送数
    void deviceTaskEnd(TaskTimeList taskTimeList);
    // ACU 任务结束
    void acuTaskEnd(TaskTimeList taskTimeList);
    // DTE 任务结束
    void dteTaskEnd(TaskTimeList taskTimeList);
signals:

public slots:
private:
    QMap<QString, bool> m_simpleMap = { { "左旋", true }, { "LHCP", true }, { "右旋", true }, { "RHCP", true } };
    QMap<QString, bool> m_doubleMap = { { "HLP", true }, { "左右旋同时", true }, { "VLP", true } };
};
// Q_DECLARE_METATYPE(TrackEnd)
#endif  // TRACKEND_H
