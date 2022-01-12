#include "UserModifyWidget.h"
#include "ui_UserModifyWidget.h"

#include "GlobalData.h"
#include "JsonHelper.h"
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

UserModifyWidget::UserModifyWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::UserModifyWidget)
{
    ui->setupUi(this);
    initWidget();
    initSlot();
}

UserModifyWidget::~UserModifyWidget() { delete ui; }

void UserModifyWidget::setUser(User& user)
{
    mSelectUser = user;
    ui->userNameEdit->setText(user.name);
    ui->userNameEdit->setEnabled(false);
    ui->passWordEdit->clear();
    ui->confirmPassWordEdit->clear();
    QString identity;
    if ("0" == user.identity)
        identity = QString("监视员");
    else if ("1" == user.identity)
        identity = QString("操作员");
    else if ("2" == user.identity)
        identity = QString("管理员");
    else
        identity = QString("未知身份");
    ui->userIdentityBox->setCurrentText(identity);
}

void UserModifyWidget::initWidget()
{
    ui->userIdentityBox->clear();

    ui->userIdentityBox->addItem(QString("监视员"), 0);
    ui->userIdentityBox->addItem(QString("操作员"), 1);
    ui->userIdentityBox->addItem(QString("管理员"), 2);
    ui->userIdentityBox->setEnabled(false);

    ui->userNameEdit->setPlaceholderText("输入用户名，长度1-10");
    ui->userNameEdit->setMaxLength(10);

    ui->passWordEdit->setPlaceholderText("输入字母或者数字，长度6-20");
    ui->passWordEdit->setEchoMode(QLineEdit::Password);

    ui->confirmPassWordEdit->setPlaceholderText("再次输入密码");
    ui->confirmPassWordEdit->setEchoMode(QLineEdit::Password);
}

void UserModifyWidget::initSlot() {}

void UserModifyWidget::slotModifyUserResponse(const QByteArray& data)
{
    auto ack = Optional<User>::emptyOptional();
    data >> ack;

    if (ack)
    {
        QMessageBox::information(this, "提示", "用户修改成功", "确定");

        if (this->parent())
        {
            qobject_cast<QWidget*>(this->parent())->close();
        }
    }
    else
    {
        QMessageBox::critical(this, "用户修改失败", ack.msg(), "确定");
    }
}

void UserModifyWidget::on_confirmBtn_clicked()
{
    QString username = ui->userNameEdit->text().trimmed();
    QString password = ui->passWordEdit->text().trimmed();
    QString identity = ui->userIdentityBox->currentData().toString();

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
    user.userID = mSelectUser.userID;
    user.name = username;
    user.password = password;
    user.identity = identity;
    user.isOnline = "";

    JsonWriter writer;
    writer& user;
    emit signalModifyUser(writer.GetByteArray());
}

void UserModifyWidget::on_cancleBtn_clicked()
{
    if (this->parent())
    {
        qobject_cast<QWidget*>(this->parent())->close();
    }
}
