#ifndef EXTENSIONSTATUSREPORTUNPACKER_H
#define EXTENSIONSTATUSREPORTUNPACKER_H

#include "BaseUnpacker.h"

// 分机状态上报解包器
class ExtensionStatusReportUnpacker : public BaseUnpacker
{
public:
    explicit ExtensionStatusReportUnpacker(QObject* parent = nullptr);
    ~ExtensionStatusReportUnpacker();
    DevMsgType messageType() const override;

protected:
    virtual bool unpackBody(const QByteArray& bodyBytes, UnpackMessage& message) override;

private:
    // 解析分机模式
    bool unpackExtentionModeId(const QByteArray& unitBytes, int modeIdx, int modeLength, quint64& modeId);

    // 特殊处理伪码服务器状态
    bool unpackWmBody(const QByteArray& allUnitParamBytes, const ModeCtrl& mode, ExtensionStatusReportMessage& extenStatusReportMsg);

    // 特殊处理DTE状态
    bool unpackDTEBody(const QByteArray& allUnitParamBytes, const ModeCtrl& mode, ExtensionStatusReportMessage& extenStatusReportMsg);

    // 特殊处理储存转发状态
    bool unpackSAFBody(const QByteArray& allUnitParamBytes, const ModeCtrl& mode, ExtensionStatusReportMessage& extenStatusReportMsg);
};

#endif  // EXTENSIONSTATUSREPORTUNPACKER_H
