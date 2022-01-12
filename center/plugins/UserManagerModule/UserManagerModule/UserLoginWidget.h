#ifndef USERLOGINWIDGET_H
#define USERLOGINWIDGET_H

#include <QWidget>

namespace Ui
{
    class UserLoginWidget;
}

class IGlobalData;
class GlobalUserData;

namespace cppmicroservices
{
    class BundleContext;
}

class UserLoginWidget : public QWidget
{
    Q_OBJECT

public:
    explicit UserLoginWidget(cppmicroservices::BundleContext context, QWidget* parent = nullptr);
    ~UserLoginWidget();

protected:
    void timerEvent(QTimerEvent* event);

private:
    void initWidget();
    void initSlot();

signals:
    void signalUserLogin(const QByteArray& data);
    void signalUserLogout(const QByteArray& data);
    void signalUserOnlineHeartbeat(const QByteArray& data);

public slots:
    void slotUserLoginResponse(const QByteArray& data);
    void slotUserLogoutResponse(const QByteArray& data);

private slots:
    void on_loginBtn_clicked();
    void on_cancelBtn_clicked();

    void on_logoutBtn_clicked();

private:
    Ui::UserLoginWidget* ui;

    GlobalUserData* mGlobalUserData = nullptr;
    int mTimerId;
};

#endif  // USERLOGINWIDGET_H
