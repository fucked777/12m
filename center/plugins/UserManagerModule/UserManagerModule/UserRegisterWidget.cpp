#include "UserRegisterWidget.h"
#include "ui_UserRegisterWidget.h"

#include "GlobalData.h"
#include "JsonHelper.h"
#include "ServiceCheck.h"
#include "UserManagerMessageDefine.h"
#include "UserManagerMessageSerialize.h"
#include "Utility.h"
#include <QMessageBox>
#include <QUuid>

// 匹配密码，字母或者数字，6-20位
static bool matchingPassword(const QString& str)
{
    QRegExp passwordReg(R"(^[0-9a-zA-Z]{6,20}$)");
    return passwordReg.exactMatch(str);
}

// 匹配用户名，字母或者数字，1-10位
static bool matchingUsername(const QString& str)
{
    QRegExp passwordReg(R"(^[0-9a-zA-Z]{1,10}$)");
    return passwordReg.exactMatch(str);
}

UserRegisterWidget::UserRegisterWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::UserRegisterWidget)
{
    ui->setupUi(this);
    initWidget();
    initSlot();

    QRegExp usernameReg(R"(^[0-9a-zA-Z]{1,10}$)");

    QRegExp passwordReg(R"(^[0-9a-zA-Z]{6,20}$)");

    {
        QValidator* validator = new QRegExpValidator(usernameReg, ui->userNameEdit);
        ui->userNameEdit->setValidator(validator);
    }
    {
        QValidator* validator = new QRegExpValidator(passwordReg, ui->passWordEdit);
        ui->passWordEdit->setValidator(validator);
    }
    {
        QValidator* validator = new QRegExpValidator(passwordReg, ui->confirmPassWordEdit);
        ui->confirmPassWordEdit->setValidator(validator);
    }
}

UserRegisterWidget::~UserRegisterWidget() { delete ui; }

void UserRegisterWidget::initWidget()
{
    ui->userIdentityBox->clear();

    ui->userIdentityBox->addItem(QString("监视员"), 0);
    ui->userIdentityBox->addItem(QString("操作员"), 1);
    //程序要求只能有一个管理员
    //    ui->userIdentityBox->addItem(QString("管理员"), 2);

    ui->userNameEdit->setPlaceholderText("输入用户名，长度1-10");
    ui->userNameEdit->setMaxLength(10);

    ui->passWordEdit->setPlaceholderText("输入字母或者数字，长度6-20");
    ui->passWordEdit->setEchoMode(QLineEdit::Password);

    ui->confirmPassWordEdit->setPlaceholderText("再次输入密码");
    ui->confirmPassWordEdit->setEchoMode(QLineEdit::Password);
}

void UserRegisterWidget::initSlot() {}

void UserRegisterWidget::slotUserRegisterResponse(const QByteArray& data)
{
    auto ack = Optional<User>::emptyOptional();
    data >> ack;

    if (ack)
    {
        QMessageBox::information(this, "提示", "用户注册成功", "确定");
        if (this->parent())
        {
            qobject_cast<QWidget*>(this->parent())->close();
        }

        ui->userNameEdit->clear();
        ui->passWordEdit->clear();
        ui->confirmPassWordEdit->clear();
    }
    else
    {
        QMessageBox::critical(this, "用户注册失败", ack.msg(), "确定");
    }
}

void UserRegisterWidget::on_confirmBtn_clicked()
{
    SERVICEONLINECHECK();  //判断服务器是否离线

    QString username = ui->userNameEdit->text().trimmed();
    QString password = ui->passWordEdit->text().trimmed();
    QString identity = ui->userIdentityBox->currentData().toString();

    if (username.isEmpty())
    {
        QMessageBox::information(this, QString("提示"), QString("用户名不能为空"), QString("确定"));
        return;
    }

    if (!matchingUsername(username))
    {
        QMessageBox::information(this, QString("提示"), QString("用户名只能为字母或者数字，长度1-10"), QString("确定"));
        return;
    }

    if (password.isEmpty())
    {
        QMessageBox::information(this, QString("提示"), QString("密码不能为空"), QString("确定"));
        return;
    }

    if (!matchingPassword(password))
    {
        QMessageBox::information(this, QString("提示"), QString("密码只能为字母或者数字，长度6-20"), QString("确定"));
        return;
    }

    if (password != ui->confirmPassWordEdit->text().trimmed())
    {
        QMessageBox::information(this, QString("提示"), QString("两次密码不一致"), QString("确定"));
        return;
    }

    User user;
    user.userID = QUuid::createUuid().toString();
    user.name = username;
    user.password = password;
    user.identity = identity;
    user.isOnline = "";

    JsonWriter writer;
    writer& user;
    emit signalUserRegister(writer.GetByteArray());
}

void UserRegisterWidget::on_cancleBtn_clicked()
{
    if (this->parent())
    {
        qobject_cast<QWidget*>(this->parent())->close();
    }
}
