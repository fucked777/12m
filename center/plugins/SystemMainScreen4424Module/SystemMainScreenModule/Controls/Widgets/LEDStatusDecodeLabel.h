#ifndef LEDSTATUSDECODELABEL_H
#define LEDSTATUSDECODELABEL_H

#include "BaseWidget.h"
#include "StatusController.h"

#include "ProtocolXmlTypeDefine.h"
#include <QDomElement>
#include <QLabel>
#include <QVariant>

/**
 * @brief The LEDStatusDecodeLabel class
 * 控件说明：
 * 这个控件主要解决的问题是动态显示低速或者高速基带里面的译码状态
 * 显示逻辑
 * 1.译码这个状态通常需要判断四个状态,位于I/Q路译码单元,分别是:Viterbi译码、RS译码、LDPC译码、Turbo译码
 * 2.如果有一个译码状态显示锁定,那么状态灯就显示为绿色,并且提示显示为当前译码名字
 * 3.如果全部为红色,那么显示为未锁定,那么状态灯就显示为红色,并且提示显示为未锁定
 *
 * 刷新逻辑
 * 1.在MainScreenControlFactory类中添加类型名为LEDStatusDecodeLabel的控件
 * 2.appendDynamicParamWidget把控件添加进需要动态刷新的控件成员变量mDynamicParamWidget里面
 * 3.appendDyanamicDevids,原本的刷新都是挨个遍历QMap<int, QMap<int, QMap<int, QMap<int, QMap<QString, QWidget*>>>>> mWidgetMap这个对象的key
 * 现在我自定义控件需要向多个设备取值,因此,我这边会把所有需要用到的devids添加进成员变量mDynamicDevids中
 * 4.从redis中把所有的需要动态刷新的devid存进DeviceParamWidget.cpp文件的成员变量currentDataMap中
 * 5.把配置文件中的刷新信息存入成员变量QMap<QString, QList<LEDStatusDecodeLabel::LEDParam>> d_decodeInfos中
 * 6.按照显示逻辑刷新该控件
 *
 * 属性介绍
 * deviceId、modeId、unitId、targetId、paramId通用属性不做介绍
 * devids该控件中所有用得到的设备,使用连接符;连接,例如:devids="0x4001;0x4002"
 * green里面存储的是一个数值,如果从缓存中取出的值等于green里面的值,那么就表示灯锁定状态
 */

class LEDStatusDecodeLabel : public BaseWidget, public StatusController
{
    Q_OBJECT
    Q_PROPERTY(QVariant value READ getValue WRITE setValue)
    Q_PROPERTY(QString dealType READ getDealType WRITE setDealType)
    Q_PROPERTY(QString uniqueKey READ getUniqueKey WRITE setUniqueKey)
    Q_PROPERTY(QVariantList devids READ getDevids WRITE setDevids)

    struct LEDItem
    {
        QVariant value;
        Status status{ Unknown };
        QString toolTip{ "未知" };
    };

public:
    struct LEDParam
    {
        DeviceID deviceId;
        int modeId;
        int unitId;
        int targetId;
        QString paramId;
        QString desc;
        QString normalValue;
    };

    explicit LEDStatusDecodeLabel(const QDomElement& domEle, QWidget* parent = nullptr);

    void setValue(const QVariant& value);
    QVariant getValue() const;

    void setDealType(const QString& dealType);
    QString getDealType() const;

    void setUniqueKey(const QString& uniqueKey);
    QString getUniqueKey() const;

    void setDevids(QVariantList& list);
    QVariantList getDevids();

    QList<LEDParam> getParamInfos();

protected:
    virtual void statusChanged(Status status) override;

private:
    void initLayout();
    void parseNode(const QDomElement& domEle);

private:
    QLabel* mLabel = nullptr;

    QMap<QVariant, LEDItem> mLedItemMap;
    QList<LEDParam> d_LedParams;
    QString d_dealType;

    DeviceID d_deviceId;
    int d_modeId;
    int d_unitId;
    int d_targetId;
    QString d_uniqueKey;
    QVariantList d_deviceIds;
};

#endif  // LEDSTATUSDECODELABEL_H
