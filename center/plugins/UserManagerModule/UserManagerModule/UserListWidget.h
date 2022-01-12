#ifndef USERLISTWIDGET_H
#define USERLISTWIDGET_H

#include <QMenu>
#include <QWidget>

#include "UserModifyWidget.h"
#include "UserTableModel.h"

namespace Ui
{
    class UserListWidget;
}

class UserListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit UserListWidget(QWidget* parent = nullptr);
    ~UserListWidget();
    void initUserModifyDialog(UserModifyWidget *widget);

protected:
    void showEvent(QShowEvent* event);

private:
    void initWidget();
    void initSlot();

signals:
    void signalGetUserList();
    void signalDeleteUser(const QByteArray& data);

public slots:
    void slotGetUserListResponse(const QByteArray& data);
    void slotDeleteUserResponse(const QByteArray& data);
    void slotModifyUserResponse(const QByteArray& data);
    void slotContextMenu(QPoint pos);
    void slotModifyUser();
    void slotDeleteUser();

private slots:
    void on_refreshBtn_clicked();

private:
    Ui::UserListWidget* ui;
    QMenu *mPopMenu = nullptr;
    UserTableModel mUserTableModel;
    UserModifyWidget* mUserModifyWidget = nullptr;
};

#endif  // USERLISTWIDGET_H
