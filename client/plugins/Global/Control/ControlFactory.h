#ifndef CONTROLFACTORY_H
#define CONTROLFACTORY_H

#include "ProtocolXmlTypeDefine.h"
#include "WWidget.h"

#include <QString>
#include <QVector>
class ControlFactory
{
public:
    //状态控件  创出来的下拉框控件是按照协议的顺序排序的
    static WWidget* createStatusControl(const ParameterAttribute& attr, const QMap<QString, QVariant>& map);
    static WWidget* createStatusTypeControl(DisplayFormat displayFormat);
    //设置控件  创出来的下拉框控件是按照协议的顺序排序的
    static WWidget* createSetControl(const ParameterAttribute& attr, const QMap<QString, QVariant>& map);
    static WWidget* createSetTypeControl(DisplayFormat displayFormat);

    //状态/设置控件 创出来的下拉框控件是按照svalue排序，既自定义排序
    static WWidget* createStatusControlOrder(const ParameterAttribute& attr, const QMap<int, QPair<QString, QVariant>>& map);
    static WWidget* createSetControlOrder(const ParameterAttribute& attr, const QMap<int, QPair<QString, QVariant>>& map);
    // 只创建输入框的框架
    static WWidget* createSetControlOnly(const ParameterAttribute& attr, const QMap<QString, QVariant>& map);
    //创建矩阵
    static QWidget* createMatrix(const int& unitCode, const QList<QPair<ParameterAttribute, QMap<int, QString>>>& attrEnumList);
    //创建一个测控框   用于各项命令测试
    static QWidget* createPlainText(const QVector<QString>& attr, CmdAttribute cmdAttributeD);
};

#endif  // CONTROLFACTORY_H
