#ifndef COMPLEXCHECKBOX_H
#define COMPLEXCHECKBOX_H

#include "BaseWidget.h"
#include "DevProtocol.h"
#include "DeviceProcessMessageDefine.h"
#include <QCheckBox>
#include <QDomElement>
#include <QVariant>

class ComplexCheckBox : public BaseWidget
{
    Q_OBJECT
    Q_PROPERTY(QVariant value READ getValue WRITE setValue)
    Q_PROPERTY(bool isControl READ getIsControl WRITE setIsControl)

    enum Position
    {
        UnitParam_DataMap,
        UnitParam_MultiDataMap,
        ProcessControl_DataMap,
        ProcessControl_MultiDataMap
    };

    //当前控件所存储的信息
    struct ControlInfo
    {
        QString paramId;
        int checkBoxId;
        int targetId;
    };

    //单元参数信息
    struct UnitInfo
    {
        DeviceID deviceID;
        int modeId;
        int unitId;
        int targetId;
        QString paramId;
        Position position;
    };

    //目标信息
    struct TargetInfo
    {
        DeviceID deviceID;
        int modeId;
        int unitId;
        int targetId;
        QString paramId;
    };

    //命令打包信息
    struct PackCommandInfo
    {
        DevMsgType msgType;
        Position position;
        CmdRequest cmdRequest;
        UnitParamRequest unitParamRequest;

        QMap<QString, ControlInfo> controlInfos;
        QMap<QString, UnitInfo> unitInfos;
        QMap<QString, TargetInfo> targetInfos;
    };

public:
    //添加原因 由于此控件没有包含对ACU，DTE送数的控制，这里针对这种情况
    enum SpecialID
    {
        DTE_ID = 101,
        ACU_ID = 102
    };

public:
    explicit ComplexCheckBox(const QDomElement& domEle, QWidget* parent = nullptr, const int deviceId = 0, const int modeId = 0);

    void setText(const QString& text);
    QString getText() const;

    /**
     * @brief setValue
     * @param value 0表示未选中,1表示选中
     */
    void setValue(const QVariant& value);
    QVariant getValue() const;

    void setIsControl(const bool isControl);
    bool getIsControl() const;

    void setEnable(const bool isEnable);
    bool getEnable() const;

    int getId() const { return d_id; }

signals:
    void signalsCmdDeviceJson(QString json);
    void signalsUnitDeviceJson(QString json);

public slots:
    void onStateChanged(int);

private:
    void initLayout();
    void parseNode(const QDomElement& domEle);
    void parseProcessControlNode(const QDomElement& domEle);
    void parseUnitParamNode(const QDomElement& domEle);
    void parseUnitInfoNode(const QDomElement& domEle, QMap<QString, UnitInfo>& map);
    void parseControlInfoNode(const QDomElement& domEle, QMap<QString, ControlInfo>& map);
    void parseTargetInfoNode(const QDomElement& domEle, QMap<QString, TargetInfo>& map);

    void string2Map(QString string, QMap<QString, QVariant>& params);
    void string2Position(const QString& string, Position& pos);
    //没有保存枚举值，写了个转换函数
    int controlValue2CommandValue(int controlValue);

private:
    QCheckBox* mCheckBox = nullptr;
    //通用属性
    int d_id;
    DeviceID d_deviceId;
    int d_targetId;
    int d_modeId;
    QString d_text;
    bool d_isControl;
    bool d_isKP;

    //需要做刷新和控制的复选框的配置信息,对应的d_isControl为false
    PackCommandInfo d_command;

    //总管类型的复选框需要用到下面的信息,对应的d_isControl为true
    QList<int> d_childId;
    QList<ComplexCheckBox*> d_childBoxs;
};

#endif  // COMPLEXCHECKBOX_H
