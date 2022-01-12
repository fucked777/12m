#ifndef SQLGLOBALDATA_H
#define SQLGLOBALDATA_H

#include "Utility.h"
#include <QList>
#include <QObject>
#include <QSqlDatabase>
#include <QVariantMap>

class SqlGlobalData
{
public:
    static OptionalNotValue initDB();
    static Optional<QByteArray> updateItem(const QString& name, const QByteArray& value);
    static OptionalNotValue updateItemNotExist(const QString& name);
    static Optional<QByteArray> selectItem(const QString& name);
    static OptionalNotValue clearItem(const QString& name);

    static OptionalNotValue initDB(QSqlDatabase& db);
    static Optional<QByteArray> updateItem(QSqlDatabase& db, const QString& name, const QByteArray& value);
    static OptionalNotValue updateItemNotExist(QSqlDatabase& db, const QString& name);
    static Optional<QByteArray> selectItem(QSqlDatabase& db, const QString& name);
    static OptionalNotValue clearItem(QSqlDatabase& db, const QString& name);
};

#endif  // SQLGLOBALDATA_H
