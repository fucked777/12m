#ifndef AUTORUNPOLICYMESSAGESERIALIZE_H
#define AUTORUNPOLICYMESSAGESERIALIZE_H

#include "AutorunPolicyMessageDefine.h"
#include "Utility.h"

JsonWriter& operator&(JsonWriter& self, const AutorunPolicyData& data);
JsonReader& operator&(JsonReader& self, AutorunPolicyData& data);
QByteArray& operator>>(QByteArray& self, AutorunPolicyData& data);
QByteArray& operator<<(QByteArray& self, const AutorunPolicyData& data);
QString& operator>>(QString& self, AutorunPolicyData& data);
QString& operator<<(QString& self, const AutorunPolicyData& data);

#endif  // AUTORUNPOLICYMESSAGESERIALIZE_H
