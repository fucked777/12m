#ifndef WIZARD_H
#define WIZARD_H
#include "IWizard.h"
#include <QDialog>

namespace cppmicroservices
{
    class BundleContext;
}

/*
 * 向导
 * 虽然是插件
 * 但是此插件为特殊的插件
 * 功能是更改别的插件的相关配置目录或者文件路径
 * 所以此插件内部不能使用PlatformConfigTools中任意的函数
 * 因为PlatformConfigTools中的数据有效是在此插件运行之后
 *
 * 此插件的配置文件是Wizard.xml
 * 查找路径依次是业务目录的上一级-->主程序运行目录-->程序运行目录下的Resources目录如果这两个目录都未找到则认为配置文件不存在
 * 当前界面不会弹出,会直接跳过
 *
 *
 * 暂定此界面客户端和服务器是完全一致的
 * 20210306 wp??
 */
class WizardImpl;
class Wizard : public IWizard
{
    Q_OBJECT

public:
    Wizard(cppmicroservices::BundleContext context, QWidget* parent = nullptr);
    ~Wizard() override;
    bool init(const PlatformConfigInfo& info) override;
    PlatformConfigInfo platformConfigInfo() const override;

private:
    void ok();
    void cancel();

private:
    WizardImpl* m_impl;
};
#endif  // WIZARD_H
