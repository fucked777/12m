#ifndef PARAMMACROMANAGERWIDGET_H
#define PARAMMACROMANAGERWIDGET_H
#include "ParamMacroMessage.h"
#include "ParamMacroMessageSerialize.h"
#include <QObject>
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class ParamMacroManagerWidget;
}
QT_END_NAMESPACE

namespace cppmicroservices
{
    class BundleContext;
}

class ParamMacroManagerImpl;
class ParamMacroManagerWidget : public QWidget
{
    Q_OBJECT

public:
    ParamMacroManagerWidget(cppmicroservices::BundleContext context, QWidget* parent = nullptr);

    ~ParamMacroManagerWidget();

private:
    /*初始化加载UI*/
    void loadUI();
    // 检查服务器状态、用户权限
    bool check();

signals:
    void signalAddParamMacro(const QByteArray& json);
    void signalEditParamMacro(const QByteArray& json);
    void signalDelParamMacro(const QByteArray& json);

    // 这是发送给弹窗的界面的
    void signalOperationDialog(bool, const QString& msg = QString());

public slots:
    void slotAddItemACK(const QByteArray& data);
    void slotDeleteItemACK(const QByteArray& data);
    void slotModifyItemACK(const QByteArray& data);

private slots:
    void slotAddBtnClicked();
    void slotShowBtnClicked();
    void slotDeleteBtnClicked();
    void slotEditBtnClicked();
    void slotRefreshBtnClicked();
    void slotCopyBtnClicked();

private:
    void showEvent(QShowEvent*) override;

private:
    Ui::ParamMacroManagerWidget* ui;
    ParamMacroManagerImpl* mImpl = nullptr;
};
#endif  // PARAMMACROMANAGERWIDGET_H
