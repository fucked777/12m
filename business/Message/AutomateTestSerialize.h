#ifndef AUTOMATETESTSERIALIZE_H
#define AUTOMATETESTSERIALIZE_H

#include "AutomateTestDefine.h"
#include "Utility.h"

JsonWriter& operator&(JsonWriter& self, const AutomateTestItemParameterInfo& data);
JsonReader& operator&(JsonReader& self, AutomateTestItemParameterInfo& data);
QByteArray& operator>>(QByteArray& self, AutomateTestItemParameterInfo& data);
QByteArray& operator<<(QByteArray& self, const AutomateTestItemParameterInfo& data);
QString& operator>>(QString& self, AutomateTestItemParameterInfo& data);
QString& operator<<(QString& self, const AutomateTestItemParameterInfo& data);

JsonWriter& operator&(JsonWriter& self, const AutomateTestItemParameterInfoMap& data);
JsonReader& operator&(JsonReader& self, AutomateTestItemParameterInfoMap& data);
QByteArray& operator>>(QByteArray& self, AutomateTestItemParameterInfoMap& data);
QByteArray& operator<<(QByteArray& self, const AutomateTestItemParameterInfoMap& data);
QString& operator>>(QString& self, AutomateTestItemParameterInfoMap& data);
QString& operator<<(QString& self, const AutomateTestItemParameterInfoMap& data);

JsonWriter& operator&(JsonWriter& self, const AutomateTestItemParameterInfoList& data);
JsonReader& operator&(JsonReader& self, AutomateTestItemParameterInfoList& data);
QByteArray& operator>>(QByteArray& self, AutomateTestItemParameterInfoList& data);
QByteArray& operator<<(QByteArray& self, const AutomateTestItemParameterInfoList& data);
QString& operator>>(QString& self, AutomateTestItemParameterInfoList& data);
QString& operator<<(QString& self, const AutomateTestItemParameterInfoList& data);

JsonWriter& operator&(JsonWriter& self, const AutomateTestResultGroup& data);
JsonReader& operator&(JsonReader& self, AutomateTestResultGroup& data);
QByteArray& operator>>(QByteArray& self, AutomateTestResultGroup& data);
QByteArray& operator<<(QByteArray& self, const AutomateTestResultGroup& data);
QString& operator>>(QString& self, AutomateTestResultGroup& data);
QString& operator<<(QString& self, const AutomateTestResultGroup& data);

JsonWriter& operator&(JsonWriter& self, const AutomateTestResultGroupList& data);
JsonReader& operator&(JsonReader& self, AutomateTestResultGroupList& data);
QByteArray& operator>>(QByteArray& self, AutomateTestResultGroupList& data);
QByteArray& operator<<(QByteArray& self, const AutomateTestResultGroupList& data);
QString& operator>>(QString& self, AutomateTestResultGroupList& data);
QString& operator<<(QString& self, const AutomateTestResultGroupList& data);

JsonWriter& operator&(JsonWriter& self, const AutomateTestPlanItem& data);
JsonReader& operator&(JsonReader& self, AutomateTestPlanItem& data);
QByteArray& operator>>(QByteArray& self, AutomateTestPlanItem& data);
QByteArray& operator<<(QByteArray& self, const AutomateTestPlanItem& data);
QString& operator>>(QString& self, AutomateTestPlanItem& data);
QString& operator<<(QString& self, const AutomateTestPlanItem& data);

JsonWriter& operator&(JsonWriter& self, const AutomateTestPlanItemList& data);
JsonReader& operator&(JsonReader& self, AutomateTestPlanItemList& data);
QByteArray& operator>>(QByteArray& self, AutomateTestPlanItemList& data);
QByteArray& operator<<(QByteArray& self, const AutomateTestPlanItemList& data);
QString& operator>>(QString& self, AutomateTestPlanItemList& data);
QString& operator<<(QString& self, const AutomateTestPlanItemList& data);

JsonWriter& operator&(JsonWriter& self, const AutomateTestResult& data);
JsonReader& operator&(JsonReader& self, AutomateTestResult& data);
QByteArray& operator>>(QByteArray& self, AutomateTestResult& data);
QByteArray& operator<<(QByteArray& self, const AutomateTestResult& data);
QString& operator>>(QString& self, AutomateTestResult& data);
QString& operator<<(QString& self, const AutomateTestResult& data);

JsonWriter& operator&(JsonWriter& self, const AutomateTestResultList& data);
JsonReader& operator&(JsonReader& self, AutomateTestResultList& data);
QByteArray& operator>>(QByteArray& self, AutomateTestResultList& data);
QByteArray& operator<<(QByteArray& self, const AutomateTestResultList& data);
QString& operator>>(QString& self, AutomateTestResultList& data);
QString& operator<<(QString& self, const AutomateTestResultList& data);

JsonWriter& operator&(JsonWriter& self, const AutomateTestResultQuery& data);
JsonReader& operator&(JsonReader& self, AutomateTestResultQuery& data);
QByteArray& operator>>(QByteArray& self, AutomateTestResultQuery& data);
QByteArray& operator<<(QByteArray& self, const AutomateTestResultQuery& data);
QString& operator>>(QString& self, AutomateTestResultQuery& data);
QString& operator<<(QString& self, const AutomateTestResultQuery& data);

JsonWriter& operator&(JsonWriter& self, const AutomateTestParameterBind& data);
JsonReader& operator&(JsonReader& self, AutomateTestParameterBind& data);
QByteArray& operator>>(QByteArray& self, AutomateTestParameterBind& data);
QByteArray& operator<<(QByteArray& self, const AutomateTestParameterBind& data);
QString& operator>>(QString& self, AutomateTestParameterBind& data);
QString& operator<<(QString& self, const AutomateTestParameterBind& data);

JsonWriter& operator&(JsonWriter& self, const AutomateTestParameterBindGroup& data);
JsonReader& operator&(JsonReader& self, AutomateTestParameterBindGroup& data);
QByteArray& operator>>(QByteArray& self, AutomateTestParameterBindGroup& data);
QByteArray& operator<<(QByteArray& self, const AutomateTestParameterBindGroup& data);
QString& operator>>(QString& self, AutomateTestParameterBindGroup& data);
QString& operator<<(QString& self, const AutomateTestParameterBindGroup& data);

JsonWriter& operator&(JsonWriter& self, const AutomateTestParameterBindMap& data);
JsonReader& operator&(JsonReader& self, AutomateTestParameterBindMap& data);
QByteArray& operator>>(QByteArray& self, AutomateTestParameterBindMap& data);
QByteArray& operator<<(QByteArray& self, const AutomateTestParameterBindMap& data);
QString& operator>>(QString& self, AutomateTestParameterBindMap& data);
QString& operator<<(QString& self, const AutomateTestParameterBindMap& data);

JsonWriter& operator&(JsonWriter& self, const ClearAutomateTestHistry& data);
JsonReader& operator&(JsonReader& self, ClearAutomateTestHistry& data);
QByteArray& operator>>(QByteArray& self, ClearAutomateTestHistry& data);
QByteArray& operator<<(QByteArray& self, const ClearAutomateTestHistry& data);
QString& operator>>(QString& self, ClearAutomateTestHistry& data);
QString& operator<<(QString& self, const ClearAutomateTestHistry& data);

JsonWriter& operator&(JsonWriter& self, const TestProcessACK& data);
JsonReader& operator&(JsonReader& self, TestProcessACK& data);
QByteArray& operator>>(QByteArray& self, TestProcessACK& data);
QByteArray& operator<<(QByteArray& self, const TestProcessACK& data);
QString& operator>>(QString& self, TestProcessACK& data);
QString& operator<<(QString& self, const TestProcessACK& data);

#endif  // AUTOMATETESTSERIALIZE_H
