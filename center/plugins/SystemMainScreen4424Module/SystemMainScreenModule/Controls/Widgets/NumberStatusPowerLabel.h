#ifndef NUMBERSTATUSPOWERLABEL_H
#define NUMBERSTATUSPOWERLABEL_H

#include "ProtocolXmlTypeDefine.h"
#include "StatusLabel.h"

#include <QVariant>

/**
 * @brief The NumberStatusPowerLabel class
 * 控件说明:
 * 这个控件主要解决的问题是动态显示26测控信道的发射功率
 * 1.测控信道分别S测控、Ka测控和Ka低速
 * 2.S测控激励从S功放切换开关取出参数ID为"SwitcherSet"的参数值,判断出当前在线的功放是哪个
 * 然后再到对应的功放中取参数ID为"TransmPower"的参数值,作为激励状态
 * 3.Ka测控则是从Ka测控基带就可以取出在线状态和激励状态
 * 4.Ka低速同Ka测控
 *
 *
 * 刷新逻辑:
 * 1.在MainScreenControlFactory类中添加类型名为NumberStatusPowerLabel的控件
 * 2.appendDynamicParamWidget把控件添加进需要动态刷新的控件成员变量mDynamicParamWidget里面
 * 3.appendDyanamicDevids,原本的刷新都是挨个遍历QMap<int, QMap<int, QMap<int, QMap<int, QMap<QString, QWidget*>>>>> mWidgetMap这个对象的key
 * 现在我自定义控件需要向多个设备取值,因此,我这边会把所有需要用到的devids添加进成员变量mDynamicDevids中
 * 4.从redis中把所有的需要动态刷新的devid存进DeviceParamWidget.cpp文件的成员变量currentDataMap中
 * 5.把配置文件中的刷新信息存入成员变量QMap<QString, QList<NumberStatusPowerLabel::SumInfo>> d_powerInfos;中
 * 6.按照显示逻辑刷新该控件
 *
 * 属性介绍
 * deviceId、modeId、unitId、targetId、paramId通用属性不做介绍
 * devids该控件中所有用得到的设备,使用连接符;连接,例如:devids="0x4001;0x4002"
 * green里面存储的是一个数值,如果从缓存中取出的值等于green里面的值,那么就表示灯锁定状态
 * red同green
 * 标签名为ParamInfo表示刷值信息;标签名为MainInfo表示主备信息
 */

class NumberStatusPowerLabel : public StatusLabel
{
    Q_OBJECT
    Q_PROPERTY(QVariant value READ getValue WRITE setValue)
    Q_PROPERTY(QString dealType READ getDealType WRITE setDealType)
    Q_PROPERTY(QString uniqueKey READ getUniqueKey WRITE setUniqueKey)
    Q_PROPERTY(QVariantList devids READ getDevids WRITE setDevids)
public:
    struct ParamInfo
    {
        DeviceID deviceId;
        int modeId;
        int unitId;
        int targetId;
        QString paramId;
    };

    struct MainInfo
    {
        DeviceID deviceId;
        int modeId;
        int unitId;
        int targetId;
        QString paramId;
        QString value;
    };

    struct SumInfo
    {
        ParamInfo paramInfo;
        MainInfo mainInfo;
    };

    explicit NumberStatusPowerLabel(const QDomElement& domEle, QWidget* parent = nullptr);

    void setValue(const QVariant& value);
    QVariant getValue() const;

    void setDealType(const QString& dealType);
    QString getDealType() const;

    void setUniqueKey(const QString& uniqueKey);
    QString getUniqueKey() const;

    void setDevids(QVariantList& list);
    QVariantList getDevids();

    QList<SumInfo> getParamInfos();

private:
    void parseNode(const QDomElement& domEle);

private:
    QList<SumInfo> d_SumInfos;
    QString d_dealType;
    QString d_uniqueKey;
    QVariantList d_deviceIds;
};

#endif  // NUMBERSTATUSPOWERLABEL_H
