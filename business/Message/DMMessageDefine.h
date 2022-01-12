#ifndef DMMESSAGEDEFINE_H
#define DMMESSAGEDEFINE_H
#include "JsonHelper.h"
#include <QByteArray>
#include <QDateTime>
#include <QObject>
#include <QString>

struct DMDataItem {
  QString id;
  QDateTime createTime;
  QDateTime lastTime;
  QString type;
  QString typeName;
  QString devID; /* 设备ID */
  QList<QVariantMap> attributeConfig;
  QVariantMap parameterConfig;

  // QString jsonData; /* json */
};

using DMTypeMap = QMap<QString, QList<DMDataItem>>;
using DMTypeList = QList<DMDataItem>;

struct DMQuery {
  QString type;
};

Q_DECLARE_METATYPE(DMDataItem);
Q_DECLARE_METATYPE(DMQuery);
#endif // DMMESSAGEDEFINE_H
