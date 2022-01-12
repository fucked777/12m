#include "UserLoginWidget.h"
#include "ui_UserLoginWidget.h"

#include <QMessageBox>

#include "CppMicroServicesUtility.h"
#include "GlobalData.h"
#include "RedisHelper.h"
#include "ServiceCheck.h"
#include "UserManagerMessageDefine.h"
#include "UserManagerMessageSerialize.h"
#include <QApplication>
#include <QRegExpValidator>

UserLoginWidget::UserLoginWidget(cppmicroservices::BundleContext /*context*/, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::UserLoginWidget)
{
    ui->setupUi(this);
    initWidget();
    initSlot();

    ui->userNameLineEdit->setText("root");
    ui->pwdLineEdit->setText("123456");

    QRegExp usernameReg(R"(^[0-9a-zA-Z]{1,10}$)");

    QRegExp passwordReg(R"(^[0-9a-zA-Z]{6,20}$)");

    {
        QValidator* validator = new QRegExpValidator(usernameReg, ui->userNameLineEdit);
        ui->userNameLineEdit->setValidator(validator);
    }
    {
        QValidator* validator = new QRegExpValidator(passwordReg, ui->pwdLineEdit);
        ui->pwdLineEdit->setValidator(validator);
    }
}

UserLoginWidget::~UserLoginWidget() { delete ui; }

void UserLoginWidget::timerEvent(QTimerEvent* event)
{
    auto currentuser = GlobalData::getCurrentUser();

    if (!currentuser.name.isEmpty() && !currentuser.userID.isEmpty())
    {
        UserOnlineHeartbeat heartbeat;
        heartbeat.userID = currentuser.userID;
        heartbeat.username = currentuser.name;

        emit signalUserOnlineHeartbeat(heartbeat.toByteArray());
    }
}

void UserLoginWidget::initWidget()
{
    ui->pwdLineEdit->setEchoMode(QLineEdit::Password);
    ui->loginBtn->setFocus();
    ui->loginBtn->setDefault(true);
}

void UserLoginWidget::initSlot() {}

void UserLoginWidget::on_loginBtn_clicked()
{
    SERVICEONLINECHECK();  //判断服务器是否离线

    QString username = ui->userNameLineEdit->text().trimmed();
    QString password = ui->pwdLineEdit->text().trimmed();

    if (username.isEmpty())
    {
        QMessageBox::information(this, QString("提示"), QString("用户名不能为空"), QString("确定"));
        return;
    }

    if (password.isEmpty())
    {
        QMessageBox::information(this, QString("提示"), QString("密码不能为空"), QString("确定"));
        return;
    }

    User user;
    user.name = username;
    user.password = password;
    user.isOnline = "1";
    QDateTime current_date_time = GlobalData::currentDateTime();
    user.loginTime = current_date_time.toString("yyyy-MM-dd hh:mm:ss.zzz");

    JsonWriter writer;
    writer& user;

    emit signalUserLogin(writer.GetByteArray());
}

void UserLoginWidget::on_cancelBtn_clicked()
{
    if (this->parent())
    {
        qobject_cast<QWidget*>(this->parent())->close();
    }
}

void UserLoginWidget::slotUserLoginResponse(const QByteArray& data)
{
    auto ack = Optional<User>::emptyOptional();
    data >> ack;

    if (ack)
    {
        User currentuser = ack.value();
        GlobalData::setCurrentUser(currentuser);
        mTimerId = startTimer(1000);
        ui->userNameLabel->setText(currentuser.name);
        QString identity;
        if ("0" == currentuser.identity)
            identity = QString("监视员");
        else if ("1" == currentuser.identity)
            identity = QString("操作员");
        else if ("2" == currentuser.identity)
            identity = QString("管理员");
        else
            identity = QString("未知身份");
        ui->userIdentityLabel->setText(identity);
        ui->stackedWidget->setCurrentWidget(ui->userInfoPage);

        if (this->parent())
        {
            qobject_cast<QWidget*>(this->parent())->close();
        }
    }
    else
    {
        QMessageBox::critical(this, QString("用户登录"), QString("用户登录失败，请确认用户名密码正确"), QString("确定"));
    }
}

void UserLoginWidget::slotUserLogoutResponse(const QByteArray& data)
{
    auto ack = Optional<User>::emptyOptional();
    data >> ack;
    if (ack)
    {
        killTimer(mTimerId);
        User currentuser;
        GlobalData::setCurrentUser(currentuser);
        ui->userNameLabel->setText("");
        ui->userIdentityLabel->setText("");
        ui->userNameLineEdit->setText("root");
        ui->pwdLineEdit->setText("123456");
        ui->stackedWidget->setCurrentWidget(ui->loginPage);
    }
    else
    {
        QMessageBox::critical(this, QString("用户退出"), QString("用户退出登录失败"), QString("确定"));
    }
}

void UserLoginWidget::on_logoutBtn_clicked()
{
    auto currentuser = GlobalData::getCurrentUser();

    QString username = currentuser.name;
    QString password = currentuser.password;

    User user;
    user.name = username;
    user.password = password;
    user.isOnline = "0";

    JsonWriter writer;
    writer& user;
    emit signalUserLogout(writer.GetByteArray());
}
