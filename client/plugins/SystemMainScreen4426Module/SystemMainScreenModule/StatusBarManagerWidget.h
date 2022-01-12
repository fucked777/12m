#ifndef STATUSBARMANAGERWIDGET_H
#define STATUSBARMANAGERWIDGET_H

#include <QWidget>

#include "ProtocolXmlTypeDefine.h"
#include "StatusBarManagerThread.h"

namespace Ui
{
    class StatusBarManagerWidget;
}

class StatusBarManagerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StatusBarManagerWidget(QWidget* parent = nullptr);
    ~StatusBarManagerWidget();
    void slotRefreshUI(const StatusBarData& statusBarData);

protected:
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;

private:
    void init();

private:
    Ui::StatusBarManagerWidget* ui;

    bool mShow = false;

    int mCurrentOfflineIndex = 0;

    int mCurrentSubCtrlIndex = 0;
};

#endif  // STATUSBARMANAGERWIDGET_H
