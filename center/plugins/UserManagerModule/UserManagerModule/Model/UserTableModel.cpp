#include "UserTableModel.h"

UserTableModel::UserTableModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

QVariant UserTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (role)
    {
    case Qt::DisplayRole:
    {
        if (orientation == Qt::Horizontal)
            return mHeaders.at(section);
    };
    default: break;
    }

    return QVariant();
}

int UserTableModel::rowCount(const QModelIndex& parent) const { return mUserList.size(); }

int UserTableModel::columnCount(const QModelIndex& parent) const { return mHeaders.size(); }

QVariant UserTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int row = index.row();
    int column = index.column();

    if (row < 0 || row >= mUserList.size())
        return QVariant();

    if (column < 0 || column >= mHeaders.size())
        return QVariant();

    const auto& user = mUserList.at(row);

    switch (role)
    {
    case Qt::DisplayRole:
    {
        switch (column)
        {
        case Name: return user.name;
        case Identity:
        {
            if ("0" == user.identity)
                return QString("监视员");
            if ("1" == user.identity)
                return QString("操作员");
            if ("2" == user.identity)
                return QString("管理员");
            return QString("未知身份");
        }
        case IsOnline:
        {
            if ("0" == user.isOnline || "" == user.isOnline.trimmed())
                return QString("离线");
            else if (user.isOnline == "1")
                return QString("在线");
        }
        }
    }
    }
    return QVariant();
}

User UserTableModel::getUser(int row) const { return mUserList.at(row); }

void UserTableModel::setHeaders(const QStringList& headers) { mHeaders = headers; }

void UserTableModel::setUserList(const QList<User>& userList)
{
    beginResetModel();
    mUserList = userList;
    endResetModel();
}
