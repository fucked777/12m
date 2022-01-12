#ifndef TIMEINFOMATIONWIDGET_H
#define TIMEINFOMATIONWIDGET_H

#include <QWidget>

namespace Ui
{
    class TimeInfomationWidget;
}

struct TimeConfigInfo;
class QButtonGroup;
class TimeInfomationWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TimeInfomationWidget(QWidget* parent = nullptr);
    ~TimeInfomationWidget();

protected:
    void timerEvent(QTimerEvent* event) override;

private:
    void init();

private:
    void slotTimeTypeBtnClicked(int id);

signals:
    void signalTimeConfigInfoChanged(const TimeConfigInfo& info);

private:
    Ui::TimeInfomationWidget* ui;

    QButtonGroup* mBtnGroup = nullptr;

    bool mIsChanging = false;
    int mTimerID{-1};
};

#endif  // TIMEINFOMATIONWIDGET_H
