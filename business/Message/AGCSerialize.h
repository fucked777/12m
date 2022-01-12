#ifndef AGCSERIALIZE_H
#define AGCSERIALIZE_H

#include "AGCDefine.h"
#include "Utility.h"

JsonWriter& operator&(JsonWriter& self, const AGCResult& data);
JsonReader& operator&(JsonReader& self, AGCResult& data);
QByteArray& operator>>(QByteArray& self, AGCResult& data);
QByteArray& operator<<(QByteArray& self, const AGCResult& data);
QString& operator>>(QString& self, AGCResult& data);
QString& operator<<(QString& self, const AGCResult& data);

JsonWriter& operator&(JsonWriter& self, const AGCResultList& data);
JsonReader& operator&(JsonReader& self, AGCResultList& data);
QByteArray& operator>>(QByteArray& self, AGCResultList& data);
QByteArray& operator<<(QByteArray& self, const AGCResultList& data);
QString& operator>>(QString& self, AGCResultList& data);
QString& operator<<(QString& self, const AGCResultList& data);

JsonWriter& operator&(JsonWriter& self, const AGCCalibrationItem& data);
JsonReader& operator&(JsonReader& self, AGCCalibrationItem& data);
QByteArray& operator>>(QByteArray& self, AGCCalibrationItem& data);
QByteArray& operator<<(QByteArray& self, const AGCCalibrationItem& data);
QString& operator>>(QString& self, AGCCalibrationItem& data);
QString& operator<<(QString& self, const AGCCalibrationItem& data);

JsonWriter& operator&(JsonWriter& self, const AGCCalibrationItemList& data);
JsonReader& operator&(JsonReader& self, AGCCalibrationItemList& data);
QByteArray& operator>>(QByteArray& self, AGCCalibrationItemList& data);
QByteArray& operator<<(QByteArray& self, const AGCCalibrationItemList& data);
QString& operator>>(QString& self, AGCCalibrationItemList& data);
QString& operator<<(QString& self, const AGCCalibrationItemList& data);

JsonWriter& operator&(JsonWriter& self, const AGCResultQuery& data);
JsonReader& operator&(JsonReader& self, AGCResultQuery& data);
QByteArray& operator>>(QByteArray& self, AGCResultQuery& data);
QByteArray& operator<<(QByteArray& self, const AGCResultQuery& data);
QString& operator>>(QString& self, AGCResultQuery& data);
QString& operator<<(QString& self, const AGCResultQuery& data);

JsonWriter& operator&(JsonWriter& self, const ClearAGCCalibrationHistry& data);
JsonReader& operator&(JsonReader& self, ClearAGCCalibrationHistry& data);
QByteArray& operator>>(QByteArray& self, ClearAGCCalibrationHistry& data);
QByteArray& operator<<(QByteArray& self, const ClearAGCCalibrationHistry& data);
QString& operator>>(QString& self, ClearAGCCalibrationHistry& data);
QString& operator<<(QString& self, const ClearAGCCalibrationHistry& data);

JsonWriter& operator&(JsonWriter& self, const AGCCalibrationProcessACK& data);
JsonReader& operator&(JsonReader& self, AGCCalibrationProcessACK& data);
QByteArray& operator>>(QByteArray& self, AGCCalibrationProcessACK& data);
QByteArray& operator<<(QByteArray& self, const AGCCalibrationProcessACK& data);
QString& operator>>(QString& self, AGCCalibrationProcessACK& data);
QString& operator<<(QString& self, const AGCCalibrationProcessACK& data);

#if 0
JsonWriter& operator&(JsonWriter& self, const AGCSendToBS& data);
JsonReader& operator&(JsonReader& self, AGCSendToBS& data);
QByteArray& operator>>(QByteArray& self, AGCSendToBS& data);
QByteArray& operator<<(QByteArray& self, const AGCSendToBS& data);
QString& operator>>(QString& self, AGCSendToBS& data);
QString& operator<<(QString& self, const AGCSendToBS& data);
#endif
#endif  // AGCSERIALIZE_H
