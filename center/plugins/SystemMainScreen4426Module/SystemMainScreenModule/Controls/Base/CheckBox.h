#ifndef CHECKBOX_H
#define CHECKBOX_H

#include <QCheckBox>
#include <QDomElement>
#include <QVariant>

#include "BaseWidget.h"

typedef struct ParamControl
{
    int deviceId;
    int modeId;
    int cmdid;
    int cmdType;

    QMap<QString, QVariant> controlInfo;
    QMap<QString, QVariant> addtionalCtrlInfo;

    //目前只有扩频的加调控制才需要，因为他的过程控制命令会关联其他控件
    int relateId;
    QString desc;
} Control;

typedef struct ParamUpdate
{
    int deviceId;
    int modeId;
    int unitId;
    int targetId;

    QString paramId;
} Update;

class CheckBox : public BaseWidget
{
    Q_OBJECT
    Q_PROPERTY(QVariant value READ getValue WRITE setValue)

public:
    enum CommandType
    {
        STTC_SC = 0,  //标准TTC送数控制
        STTC_JT,      //标准TTC加调控制
        Skuo2_SC,     //以此类推
        Skuo2_JT,
        SYTHSMJ_SC,
        SYTHSMJ_JT,
        SYTHWX_SC,
        SYTHWX_JT,
        SYTHGML_SC,
        SYTHGML_JT,
        SYTHSMJK2GZB_SC,
        SYTHSMJK2GZB_JT,
        SYTHSMJK2BGZB_SC,
        SYTHSMJK2BGZB_JT,
        SYTHWXSK2_SC,
        SYTHWXSK2_JT,
        SYTHGMLSK2_SC,
        SYTHGMLSK2_JT,
        SKT_SC,
        SKT_JT
    };
    explicit CheckBox(const QDomElement& domEle, QWidget* parent = nullptr, const int deviceId = 0, const int modeId = 0);

    void setText(const QString& text);
    QString getText() const;

    void setValue(const QVariant& value);
    QVariant getValue() const;

    void setCheckable(const bool enabled);
    bool getCheckable() const;

    int getId() const;
    QList<int> getRelateId() const;

    bool isControlType();

signals:
    void signalsCmdDeviceJson(QString json);
    void signalsUnitDeviceJson(QString json);

public slots:
    void onStateChanged(int);

private:
    void initLayout();
    void parseNode(const QDomElement& domEle);

    void processSTTC_SC();
    void processSTTC_JT();
    void processSkuo2_SC();
    void processSkuo2_JT();
    void processSYTHSMJ_SC();
    void processSYTHSMJ_JT();
    void processSYTHWX_SC() {}
    void processSYTHWX_JT() {}
    void processSYTHGML_SC() {}
    void processSYTHGML_JT() {}
    void processSYTHSMJK2GZB_SC() {}
    void processSYTHSMJK2GZB_JT() {}
    void processSYTHSMJK2BGZB_SC() {}
    void processSYTHSMJK2BGZB_JT() {}
    void processSYTHWXSK2_SC() {}
    void processSYTHWXSK2_JT() {}
    void processSYTHGMLSK2_SC() {}
    void processSYTHGMLSK2_JT() {}
    void processSKT_SC() {}
    void processSKT_JT() {}

private:
    QCheckBox* mCheckBox = nullptr;
    QString d_text;
    //如果所控参数的目标数大于当前目标数，那么过滤掉回复原始状态的那一次操作
    int d_targetNumOverSizeCount;

    bool d_checkable;
    bool d_isControlType;
    int d_deviceId;
    int d_modeId;
    int d_id;

    QString d_controlMode;
    QList<int> d_childId;
    QList<CheckBox*> d_childBoxs;
    Control d_control;
    Update d_update;

    CheckBox* d_relateBox;
};

#endif  // CHECKBOX_H
