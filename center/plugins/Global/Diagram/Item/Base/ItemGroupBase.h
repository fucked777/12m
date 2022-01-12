//#ifndef ITEMGROUPBASE_H
//#define ITEMGROUPBASE_H
//#include "ItemUtility.h"
//#include <QDebug>
//#include <QGraphicsItemGroup>
//#include <QObject>

///*
// * 对于设备状态
// * setGroupData 设置的是颜色 是QRgb
// *
// * 不在线/获取数据失败       亮灰 lightGray
// * 分控                   黄色  yellow
// * 本控制
// *      不在线/获取数据失败 亮灰  lightGray
// *      故障              红   red
// *      正常              绿   green
// *
// * 对于开关  设置的是开关当前的位置
// *
// * 这里开关只有两种状态
// * 西北/西南
// * true  为西北
// * false 为西南
// */

// class ItemGroupBase : public QObject, public QGraphicsItemGroup
//{
//    Q_OBJECT
// public:
//    ItemGroupBase(QGraphicsItem* parent = nullptr);
//    virtual ~ItemGroupBase() override;

//    virtual void setGroupData(const QVariant&);
//    virtual void setBorderGroupData(const QVariant&);
//    virtual void setEntityData(const EntityShape&);
//};

//#endif  // ITEMGROUPBASE_H
