#ifndef IWIZARD_H
#define IWIZARD_H
#include "PlatformDefine.h"
#include <QDialog>

/* 向导界面的接口 */
/*
 * 20210306
 * 研究了一下
 * 不采用向导直接更改配置文件的方式去操纵平台的加载数据
 * 直接更改配置文件的方式感觉不太好
 *
 * 最终采用的逻辑是
 * 平台启动-->加载配置文件-->无向导-->直接使用现有的配置数据启动
 *                     -->有向导-->将现在配置数据通过引用的方式传入向导-->向导通过某种方式改变传入的配置-->使用修改后的配置数据启动
 */
class IWizard : public QDialog
{
public:
    inline IWizard(QWidget* parent = nullptr)
        : QDialog(parent)
    {
    }
    virtual inline ~IWizard() {}
    virtual bool init(const PlatformConfigInfo& info) = 0;     /* 用已有配置数据初始化向导界面 */
    virtual PlatformConfigInfo platformConfigInfo() const = 0; /* 获取经过向导处理的配置数据 */
    QString errMsg() const { return m_errMsg; }

protected:
    QString m_errMsg;
};

#endif  // IWIZARD_H
