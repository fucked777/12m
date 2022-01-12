#include "DMMessageSerialize.h"

JsonWriter &operator&(JsonWriter &self, const DMDataItem &value) {
  self.StartObject();
  self.Member("id") & value.id;
  self.Member("createTime") & value.createTime;
  self.Member("lastTime") & value.lastTime;
  self.Member("type") & value.type;
  self.Member("typeName") & value.typeName;
  self.Member("devID") & value.devID;

  self.Member("parameter");
  self.StartObject();
  self &value.parameterConfig;
  self.EndObject();

  self.Member("attribute");
  self.StartArray();
  for (auto &item : value.attributeConfig) {
    self.StartObject();
    self &item;
    self.EndObject();
  }
  self.EndArray();

  return self.EndObject();
}
JsonReader &operator&(JsonReader &self, DMDataItem &value) {
  self.StartObject();
  self.Member("id") & value.id;
  self.Member("createTime") & value.createTime;
  self.Member("lastTime") & value.lastTime;
  self.Member("type") & value.type;
  self.Member("typeName") & value.typeName;
  self.Member("devID") & value.devID;

  self.Member("parameter");
  self.StartObject();
  self &value.parameterConfig;
  self.EndObject();

  self.Member("attribute");
  std::size_t count = 0;
  self.StartArray(&count);
  for (std::size_t i = 0; i < count; ++i) {
    self.StartObject();
    QVariantMap tempData;
    self &tempData;
    value.attributeConfig.append(tempData);
    self.EndObject();
  }
  self.EndArray();

  return self.EndObject();
}
JsonWriter &operator&(JsonWriter &self, const DMTypeMap &value) {
  self.StartObject();
  self.Member("DeviceManagerMapData");

  self.StartObject();
  for (auto iter = value.begin(); iter != value.end(); ++iter) {
    self.Member(iter.key()) & iter.value();
  }
  self.EndObject();
  return self.EndObject();
}
JsonReader &operator&(JsonReader &self, DMTypeMap &value) {
  self.StartObject();
  self.Member("DeviceManagerMapData");

  self.StartObject();
  auto keys = self.Members();
  for (auto &key : keys) {
    DMTypeList tempList;
    self.Member(key) & tempList;
    value.insert(key, tempList);
  }
  self.EndObject();
  return self.EndObject();
}
JsonWriter &operator&(JsonWriter &self, const DMTypeList &value) {
  self.StartObject();
  self.Member("DeviceManagerListData");

  self.StartArray();
  for (auto &item : value) {
    self &item;
  }
  self.EndArray();
  return self.EndObject();
}
JsonReader &operator&(JsonReader &self, DMTypeList &value) {
  self.StartObject();
  self.Member("DeviceManagerListData");
  std::size_t count = 0;

  self.StartArray(&count);
  for (std::size_t i = 0; i < count; i++) {
    DMDataItem info;
    self &info;
    value.append(info);
  }
  self.EndArray();

  return self.EndObject();
}
JsonWriter &operator&(JsonWriter &self, const DMQuery &value) {
  self.StartObject();
  self.Member("DMType") & value.type;
  return self.EndObject();
}
JsonReader &operator&(JsonReader &self, DMQuery &value) {
  self.StartObject();
  self.Member("DMType") & value.type;
  return self.EndObject();
}

QByteArray &operator<<(QByteArray &self, const DMDataItem &value) {
  JsonWriter writer;
  writer &value;
  self.append(writer.GetByteArray());
  return self;
}
QByteArray &operator>>(QByteArray &self, DMDataItem &value) {
  JsonReader reader(self);
  reader &value;

  return self;
}
QString &operator<<(QString &self, const DMDataItem &value) {
  JsonWriter writer;
  writer &value;
  self.append(writer.GetQString());
  return self;
}
QString &operator>>(QString &self, DMDataItem &value) {
  JsonReader reader(self);
  reader &value;

  return self;
}
QByteArray &operator>>(QByteArray &self, DMTypeMap &value) {
  JsonReader reader(self);
  reader &value;

  return self;
}
QByteArray &operator<<(QByteArray &self, const DMTypeMap &value) {
  JsonWriter writer;
  writer &value;
  self.append(writer.GetByteArray());
  return self;
}
QString &operator<<(QString &self, const DMTypeMap &value) {
  JsonWriter writer;
  writer &value;
  self.append(writer.GetQString());
  return self;
}
QString &operator>>(QString &self, DMTypeMap &value) {
  JsonReader reader(self);
  reader &value;

  return self;
}

QByteArray &operator>>(QByteArray &self, DMTypeList &value) {
  JsonReader reader(self);
  reader &value;

  return self;
}
QByteArray &operator<<(QByteArray &self, const DMTypeList &value) {
  JsonWriter writer;
  writer &value;
  self.append(writer.GetByteArray());
  return self;
}
QString &operator<<(QString &self, const DMTypeList &value) {
  JsonWriter writer;
  writer &value;
  self.append(writer.GetQString());
  return self;
}
QString &operator>>(QString &self, DMTypeList &value) {
  JsonReader reader(self);
  reader &value;

  return self;
}
QByteArray &operator>>(QByteArray &self, DMQuery &value) {
  JsonReader reader(self);
  reader &value;

  return self;
}
QByteArray &operator<<(QByteArray &self, const DMQuery &value) {
  JsonWriter writer;
  writer &value;
  self.append(writer.GetByteArray());
  return self;
}
QString &operator<<(QString &self, const DMQuery &value) {
  JsonWriter writer;
  writer &value;
  self.append(writer.GetQString());
  return self;
}
QString &operator>>(QString &self, DMQuery &value) {
  JsonReader reader(self);
  reader &value;

  return self;
}

QByteArray
DMMessageSerializeHelper::joinDMDataItemParam(const DMDataItem &value) {
  JsonWriter self;
  self.StartObject();

  self.Member("parameter");
  self.StartObject();
  self &value.parameterConfig;
  self.EndObject();

  self.Member("attribute");
  self.StartArray();
  for (auto &item : value.attributeConfig) {
    self.StartObject();
    self &item;
    self.EndObject();
  }
  self.EndArray();

  self.EndObject();
  return self.GetByteArray();
}
void DMMessageSerializeHelper::splitDMDataItemParam(const QByteArray &data,
                                                    DMDataItem &value) {
  JsonReader self(data);
  self.StartObject();

  self.Member("parameter");
  self.StartObject();
  self &value.parameterConfig;
  self.EndObject();

  QVariantMap tempData;
  self.Member("attribute");
  std::size_t count = 0;
  self.StartArray(&count);
  for (std::size_t i = 0; i < count; ++i) {
    self.StartObject();
    self &tempData;
    value.attributeConfig.append(tempData);
    self.EndObject();
  }
  self.EndArray();

  self.EndObject();
}
