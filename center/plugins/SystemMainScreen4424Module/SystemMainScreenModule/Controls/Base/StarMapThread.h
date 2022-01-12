#ifndef STARMAPTHREAD_H
#define STARMAPTHREAD_H

#include <QObject>
#include <QPointF>
#include <QThread>
#include <QVector>
#include <atomic>

class StarMapThread : public QThread
{
    Q_OBJECT
public:
    StarMapThread();
    void stopRunning();

    void setStarMapData(int deviceID, int channel);

protected:
    void run() override;

signals:
    void signalRefreshUI(const QVector<QPointF>&);

private:
    std::atomic<bool> m_isRunning{ true };
    QVector<QPointF> mAllPoints;
    int mChannel = -1;
    int mDeviceId = -1;
};

#endif  // STARMAPTHREAD_H
