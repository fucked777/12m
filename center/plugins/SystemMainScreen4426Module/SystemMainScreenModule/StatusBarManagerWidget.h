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

protected:
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;

private:
    void init();

private slots:
    void slotRefreshUI(const StatusBarData& statusBarData);

private:
    Ui::StatusBarManagerWidget* ui;

    bool mShow = false;

    int mCurrentOfflineIndex = 0;

    int mCurrentSubCtrlIndex = 0;

    StatusBarManagerThread* m_statusBarThread = nullptr;
};

#endif  // STATUSBARMANAGERWIDGET_H
