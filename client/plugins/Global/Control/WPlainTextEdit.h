#ifndef WPLAINTEXTEDIT_H
#define WPLAINTEXTEDIT_H

#include "ProtocolXmlTypeDefine.h"
#include "WWidget.h"
#include <QWidget>

class WStrPlainEditImpl;
class WPlainTextEdit : public WWidget
{
    Q_OBJECT

public:
    explicit WPlainTextEdit(QWidget* parent = nullptr);
    ~WPlainTextEdit() override;

    void setValue(const QVariant& value) override;
    QVariant value() override;

    void init(const QVector<QString>& attr, CmdAttribute cmdAttribute);

    void initUI();
    //处理普通参数的值
    void dealCmdValue(int cmdID, int modeID, QMap<QString, QVariant> setParamMap, DeviceID deviceID);

    //处理动态界面的值
    void dealCmdMultiValue(int cmdID, int modeID, QMap<int, QList<QPair<QString, QVariant>>> multiParamMap, DeviceID deviceID);

    int getParamType();

private:
    WStrPlainEditImpl* m_impl = nullptr;
    QVector<QString> m_initValue;
    int m_type = 0;                      // 0:普通参数类型   1：动态参数类型
    CmdAttribute m_currentCmdAttribute;  //为了上报的参数结果进行排序才添加的
};

#endif  // WPLAINTEXTEDIT_H
