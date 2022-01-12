#include "UserListWidget.h"
#include "UserModifyWidget.h"
#include "ui_UserListWidget.h"

#include "GlobalData.h"
#include "PlatformConfigTools.h"
#include "QssCommonHelper.h"
#include "RedisHelper.h"
#include "ServiceCheck.h"
#include "UserManagerMessageDefine.h"
#include "UserManagerMessageSerialize.h"
#include <QMessageBox>

UserListWidget::UserListWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::UserListWidget)
{
    ui->setupUi(this);
    // 初始化qss
    QssCommonHelper::setWidgetStyle(this, "Common.qss");

    initWidget();
    initSlot();
}

UserListWidget::~UserListWidget()
{
    delete ui;
    if (nullptr != mUserModifyWidget)
    {
        if (nullptr != mUserModifyWidget->parentWidget())
        {
            QWidget* parent = mUserModifyWidget->parentWidget();
            delete mUserModifyWidget;
            mUserModifyWidget = nullptr;
            delete parent;
            parent = nullptr;
        }
        else
        {
            delete mUserModifyWidget;
            mUserModifyWidget = nullptr;
        }
    }
}

void UserListWidget::showEvent(QShowEvent* event) { on_refreshBtn_clicked(); }

void UserListWidget::initWidget()
{
    //初始化qss
    QString qssPath = QApplication::applicationDirPath() + "/resources/newqss/common.qss";
    QssCommonHelper::setWidgetStyle(this, qssPath);

    QStringList headers = { "用户名", "身份" };

    mUserTableModel.setHeaders(headers);

    ui->tableView->setModel(&mUserTableModel);
    ui->tableView->verticalHeader()->setVisible(false);
    ui->tableView->setFrameShape(QFrame::NoFrame);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->setColumnWidth(UserTableModel::Name, 275);
    ui->tableView->setColumnWidth(UserTableModel::Identity, 275);
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tableView->setSelectionMode(QTableView::SingleSelection);
    ui->tableView->setSelectionBehavior(QTableView::SelectRows);

    mPopMenu = new QMenu(ui->tableView);
    mPopMenu->addAction(tr("&修改"), this, SLOT(slotModifyUser()));
    mPopMenu->addAction(tr("&删除"), this, SLOT(slotDeleteUser()));
}

void UserListWidget::initSlot()
{
    connect(ui->refreshBtn, &QPushButton::clicked, this, &UserListWidget::on_refreshBtn_clicked);
    connect(ui->tableView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotContextMenu(QPoint)));
}

void UserListWidget::initUserModifyDialog(UserModifyWidget* widget)
{
    if (nullptr != widget)
    {
        mUserModifyWidget = widget;
        auto vLayout = new QVBoxLayout;
        vLayout->addWidget(widget);
        vLayout->setMargin(0);

        QDialog* mUserModifyDialog = new QDialog;
        mUserModifyDialog->setLayout(vLayout);
        mUserModifyDialog->setWindowTitle(QString("修改用户"));
        mUserModifyDialog->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);  //只显示关闭按钮
        mUserModifyDialog->setWindowIcon(QIcon(PlatformConfigTools::configBusiness("RibbonIcon/") + QString("用户修改-1.png")));
    }
}

void UserListWidget::slotContextMenu(QPoint pos)
{
    auto index = ui->tableView->indexAt(pos);
    if (index.isValid())
    {
        mPopMenu->exec(QCursor::pos());
    }
}

void UserListWidget::slotModifyUser()
{
    int curRow = ui->tableView->currentIndex().row();
    User user = mUserTableModel.getUser(curRow);
    mUserModifyWidget->setUser(user);
    mUserModifyWidget->parentWidget()->show();
}

void UserListWidget::slotDeleteUser()
{
    int curRow = ui->tableView->currentIndex().row();
    User user = mUserTableModel.getUser(curRow);

    //权限和名字都做了判断,只有root是管理员权限
    if (user.identity == "2" || user.name == "root")
    {
        QMessageBox::critical(this, "用户删除失败", "管理员用户禁止删除", "确定");
        return;
    }

    if (QMessageBox::question(this, QString("提示"), QString("请确认是否删除用户：%1!").arg(user.name), QString("确认"), QString("取消")))
    {
        return;
    }

    JsonWriter writer;
    writer& user;

    emit signalDeleteUser(writer.GetByteArray());
}

void UserListWidget::slotDeleteUserResponse(const QByteArray& data)
{
    auto ack = Optional<User>::emptyOptional();
    data >> ack;

    if (ack)
    {
        QMessageBox::information(this, "提示", "用户删除成功", "确定");

        on_refreshBtn_clicked();
    }
    else
    {
        QMessageBox::critical(this, "用户删除失败", ack.msg(), "确定");
    }
}

void UserListWidget::on_refreshBtn_clicked() { emit signalGetUserList(); }

void UserListWidget::slotGetUserListResponse(const QByteArray& data)
{
    SERVICEONLINECHECK();  //判断服务器是否离线

    auto ack = Optional<UserList>::emptyOptional();
    data >> ack;

    if (ack)
    {
        UserList userlist = ack.value();
        GlobalData::setUserList(userlist);
        mUserTableModel.setUserList(userlist);
    }
    else
    {
        QMessageBox::critical(this, QString("提示"), QString("刷新用户列表失败"), QString("确定"));
    }
}

void UserListWidget::slotModifyUserResponse(const QByteArray& data)
{
    auto ack = Optional<User>::emptyOptional();
    data >> ack;

    if (ack)
    {
        on_refreshBtn_clicked();
    }
}
