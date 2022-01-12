//#ifndef PhaseCalibrationFLOW_H
//#define PhaseCalibrationFLOW_H
//#include <QObject>
//#include <QString>

// struct CmdResult;
// class PhaseCalibration;
// struct ProtocolPack;
// struct PackCommand;
// struct PhaseCalibrationItem;
// class PhaseCalibrationFlowImpl;
// class PhaseCalibrationFlow : public QObject
//{
//    Q_OBJECT
// public:
//    PhaseCalibrationFlow(PhaseCalibration* netmsg);
//    ~PhaseCalibrationFlow();
//    static void pushLog(const QString&);

//    void sttcStart(const PhaseCalibrationItem& item);          /* S标准TTC */
//    void sktStart(const PhaseCalibrationItem& item);           /* 扩跳 */
//    void skuo2Start(const PhaseCalibrationItem& item);         /* 扩2 */
//    void sythsmjStart(const PhaseCalibrationItem& item);       /* 一体化上面级 */
//    void sythwxStart(const PhaseCalibrationItem& item);        /* 一体化卫星 */
//    void sythgmlStart(const PhaseCalibrationItem& item);       /* 一体化高码率 */
//    void sythsmjk2gzbStart(const PhaseCalibrationItem& item);  /* 一体化上面级+扩二共载波 */
//    void sythsmjk2bgzbStart(const PhaseCalibrationItem& item); /* 一体化上面级+扩二不共载波 */
//    void sythwxsk2Start(const PhaseCalibrationItem& item);     /* 一体化卫星+扩二 */
//    void sythgmlsk2Start(const PhaseCalibrationItem& item);    /* 一体化高码率+扩二 */

//    void sttcEnd(const PhaseCalibrationItem& item);          /* S标准TTC */
//    void sktEnd(const PhaseCalibrationItem& item);           /* 扩跳 */
//    void skuo2End(const PhaseCalibrationItem& item);         /* 扩2 */
//    void sythsmjEnd(const PhaseCalibrationItem& item);       /* 一体化上面级 */
//    void sythwxEnd(const PhaseCalibrationItem& item);        /* 一体化卫星 */
//    void sythgmlEnd(const PhaseCalibrationItem& item);       /* 一体化高码率 */
//    void sythsmjk2gzbEnd(const PhaseCalibrationItem& item);  /* 一体化上面级+扩二共载波 */
//    void sythsmjk2bgzbEnd(const PhaseCalibrationItem& item); /* 一体化上面级+扩二不共载波 */
//    void sythwxsk2End(const PhaseCalibrationItem& item);     /* 一体化卫星+扩二 */
//    void sythgmlsk2End(const PhaseCalibrationItem& item);    /* 一体化高码率+扩二 */

// private:
//    bool waitExecSuccess(const PackCommand& packCommand, uint ttl = 10);
//    bool waitResultSuccess(
//        const QString& operatorInfo, int ttl = 40, bool (*)(const CmdResult&) = [](const CmdResult&) { return true; });

//    void sendByteArrayToDevice(const QByteArray& data);
//    void acceptDeviceCMDResultMessage(const ProtocolPack& pack);

//    /* 等待校零结果 */
//    void waitDCResult(const PhaseCalibrationItem& item);

// private:
//    PhaseCalibrationFlowImpl* m_impl;
//};
//#endif  // PhaseCalibrationFLOW_H
