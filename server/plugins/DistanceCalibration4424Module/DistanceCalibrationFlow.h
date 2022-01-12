#ifndef DISTANCECALIBRATIONFLOW_H
#define DISTANCECALIBRATIONFLOW_H
#include <QObject>
#include <QString>

struct CmdResult;
class DistanceCalibration;
struct ProtocolPack;
struct PackCommand;
struct DistanceCalibrationItem;
class DistanceCalibrationFlowImpl;
class DistanceCalibrationFlow : public QObject
{
    Q_OBJECT
public:
    DistanceCalibrationFlow(DistanceCalibration* netmsg);
    ~DistanceCalibrationFlow();
    static void pushLog(const QString&);

    void sttcStart(const DistanceCalibrationItem& item);          /* S标准TTC */
    void sktStart(const DistanceCalibrationItem& item);           /* 扩跳 */
    void skuo2Start(const DistanceCalibrationItem& item);         /* 扩2 */
    void sythsmjStart(const DistanceCalibrationItem& item);       /* 一体化上面级 */
    void sythwxStart(const DistanceCalibrationItem& item);        /* 一体化卫星 */
    void sythgmlStart(const DistanceCalibrationItem& item);       /* 一体化高码率 */
    void sythsmjk2gzbStart(const DistanceCalibrationItem& item);  /* 一体化上面级+扩二共载波 */
    void sythsmjk2bgzbStart(const DistanceCalibrationItem& item); /* 一体化上面级+扩二不共载波 */
    void sythwxsk2Start(const DistanceCalibrationItem& item);     /* 一体化卫星+扩二 */
    void sythgmlsk2Start(const DistanceCalibrationItem& item);    /* 一体化高码率+扩二 */

    void sttcEnd(const DistanceCalibrationItem& item);          /* S标准TTC */
    void sktEnd(const DistanceCalibrationItem& item);           /* 扩跳 */
    void skuo2End(const DistanceCalibrationItem& item);         /* 扩2 */
    void sythsmjEnd(const DistanceCalibrationItem& item);       /* 一体化上面级 */
    void sythwxEnd(const DistanceCalibrationItem& item);        /* 一体化卫星 */
    void sythgmlEnd(const DistanceCalibrationItem& item);       /* 一体化高码率 */
    void sythsmjk2gzbEnd(const DistanceCalibrationItem& item);  /* 一体化上面级+扩二共载波 */
    void sythsmjk2bgzbEnd(const DistanceCalibrationItem& item); /* 一体化上面级+扩二不共载波 */
    void sythwxsk2End(const DistanceCalibrationItem& item);     /* 一体化卫星+扩二 */
    void sythgmlsk2End(const DistanceCalibrationItem& item);    /* 一体化高码率+扩二 */

public slots:
    void sendByteArrayToDevice(const QByteArray& data);

private:
    bool waitExecSuccess(const PackCommand& packCommand, uint ttl = 10);
    bool waitResultSuccess(
        const QString& operatorInfo, int ttl = 40, bool (*)(const CmdResult&) = [](const CmdResult&) { return true; });

    void acceptDeviceCMDResultMessage(const ProtocolPack& pack);

    /* 等待校零结果 */
    void waitDCResult(const DistanceCalibrationItem& item);

private:
    DistanceCalibrationFlowImpl* m_impl;
};
#endif  // DISTANCECALIBRATIONFLOW_H
