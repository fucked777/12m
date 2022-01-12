#ifndef USERTABLEMODEL_H
#define USERTABLEMODEL_H

#include <QAbstractTableModel>
#include "UserManagerMessageDefine.h"

class UserTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Column
    {
        Name,      // 用户名称
        Identity,  // 用户身份
        IsOnline,  // 是否在线
        ColumnSize
    };

    explicit UserTableModel(QObject* parent = nullptr);

    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    int rowCount(const QModelIndex& parent) const;
    int columnCount(const QModelIndex& parent) const;
    QVariant data(const QModelIndex& index, int role) const;
    User getUser(int row) const;

    void setHeaders(const QStringList& headers);
    void setUserList(const QList<User>& userList);

private:
    QList<User> mUserList;
    QStringList mHeaders;
};

#endif  // USERTABLEMODEL_H
