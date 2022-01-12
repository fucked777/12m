#include "AutomateTestSerialize.h"
#include "JsonHelper.h"

JsonWriter& operator&(JsonWriter& self, const AutomateTestItemParameterInfo& data)
{
    self.StartObject();
    self.Member("id") & data.id;                     /* 当前项的id */
    self.Member("rawValue") & data.rawValue;         /* 当前项的原始值 */
    self.Member("displayName") & data.displayName;   /* 当前项的显示名称 */
    self.Member("displayValue") & data.displayValue; /* 当前项的显示值 */
    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, AutomateTestItemParameterInfo& data)
{
    self.StartObject();
    self.Member("id") & data.id;                     /* 当前项的id */
    self.Member("rawValue") & data.rawValue;         /* 当前项的原始值 */
    self.Member("displayName") & data.displayName;   /* 当前项的显示名称 */
    self.Member("displayValue") & data.displayValue; /* 当前项的显示值 */
    return self.EndObject();
}
QByteArray& operator>>(QByteArray& self, AutomateTestItemParameterInfo& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QByteArray& operator<<(QByteArray& self, const AutomateTestItemParameterInfo& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator>>(QString& self, AutomateTestItemParameterInfo& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QString& operator<<(QString& self, const AutomateTestItemParameterInfo& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const AutomateTestItemParameterInfoMap& data)
{
    self.StartArray();
    for (auto& item : data)
    {
        self& item;
    }
    return self.EndArray();
}
JsonReader& operator&(JsonReader& self, AutomateTestItemParameterInfoMap& data)
{
    std::size_t count = 0;
    self.StartArray(&count);
    for (std::size_t i = 0; i < count; i++)
    {
        AutomateTestItemParameterInfo info;
        self& info;
        data.insert(info.id, info);
    }
    return self.EndArray();
}
QByteArray& operator>>(QByteArray& self, AutomateTestItemParameterInfoMap& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QByteArray& operator<<(QByteArray& self, const AutomateTestItemParameterInfoMap& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator>>(QString& self, AutomateTestItemParameterInfoMap& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QString& operator<<(QString& self, const AutomateTestItemParameterInfoMap& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const AutomateTestItemParameterInfoList& data)
{
    self.StartArray();
    for (auto& item : data)
    {
        self& item;
    }
    return self.EndArray();
}
JsonReader& operator&(JsonReader& self, AutomateTestItemParameterInfoList& data)
{
    std::size_t count = 0;
    self.StartArray(&count);
    for (std::size_t i = 0; i < count; i++)
    {
        AutomateTestItemParameterInfo info;
        self& info;
        data << info;
    }
    return self.EndArray();
}
QByteArray& operator>>(QByteArray& self, AutomateTestItemParameterInfoList& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QByteArray& operator<<(QByteArray& self, const AutomateTestItemParameterInfoList& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator>>(QString& self, AutomateTestItemParameterInfoList& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QString& operator<<(QString& self, const AutomateTestItemParameterInfoList& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const AutomateTestResultGroup& data)
{
    self.StartObject();
    self.Member("groupName") & data.groupName; /* 当前结果的组名称 */

    self.Member("ResultList");
    self.StartArray();
    for (auto& item : data.resultList)
    {
        self& item;
    }
    self.EndArray();
    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, AutomateTestResultGroup& data)
{
    self.StartObject();
    self.Member("groupName") & data.groupName; /* 当前结果的组名称 */

    self.Member("ResultList");
    std::size_t count = 0;

    self.StartArray(&count);
    for (std::size_t i = 0; i < count; i++)
    {
        AutomateTestItemParameterInfo info;
        self& info;
        data.resultList << info;
    }
    self.EndArray();
    return self.EndObject();
}
QByteArray& operator>>(QByteArray& self, AutomateTestResultGroup& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QByteArray& operator<<(QByteArray& self, const AutomateTestResultGroup& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator>>(QString& self, AutomateTestResultGroup& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QString& operator<<(QString& self, const AutomateTestResultGroup& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const AutomateTestResultGroupList& data)
{
    self.StartArray();
    for (auto& item : data)
    {
        self& item;
    }
    return self.EndArray();
}
JsonReader& operator&(JsonReader& self, AutomateTestResultGroupList& data)
{
    std::size_t count = 0;
    self.StartArray(&count);
    for (std::size_t i = 0; i < count; i++)
    {
        AutomateTestResultGroup info;
        self& info;
        data << info;
    }
    return self.EndArray();
}
QByteArray& operator>>(QByteArray& self, AutomateTestResultGroupList& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QByteArray& operator<<(QByteArray& self, const AutomateTestResultGroupList& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator>>(QString& self, AutomateTestResultGroupList& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QString& operator<<(QString& self, const AutomateTestResultGroupList& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const AutomateTestPlanItem& data)
{
    self.StartObject();

    self.Member("id") & data.id;                     /* 计划的唯一ID */
    self.Member("testName") & data.testName;         /* 当前的测试名称 */
    self.Member("testDesc") & data.testDesc;         /* 当前的测试描述 */
    self.Member("groupName") & data.groupName;       /* 测试组名称/测试分系统名称 */
    self.Member("dotDrequency") & data.dotDrequency; /* 点频 */
    self.Member("testTypeID") & data.testTypeID;     /* 测试类型ID 决定进行那种测试 */
    self.Member("testTypeName") & data.testTypeName; /* 对应测试的名称比如G/T值测试 本来是和testTypeID是一一对应的 */
    self.Member("projectCode") & data.projectCode;   /* 项目代号 */
    self.Member("taskIdent") & data.taskIdent;       /* 任务标识 */
    self.Member("taskCode") & data.taskCode;         /* 任务代号 */
    self.Member("createTime") & data.createTime;     /* 创建时间 */
    self.Member("workMode") & data.workMode;         /* 工作模式 */
    self.Member("equipComb") & data.equipComb;       /* 设备组合号 */
    self.Member("testParameterMap") & data.testParameterMap;
    self.Member("automateTestParameterBind") & data.automateTestParameterBind;

    // self.Member("upFreq") & data.upFreq;             /* 上行频率[MHz] */
    // self.Member("downFreq") & data.downFreq;         /* 下行频率[MHz] */

    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, AutomateTestPlanItem& data)
{
    self.StartObject();

    self.Member("id") & data.id;                     /* 计划的唯一ID */
    self.Member("testName") & data.testName;         /* 当前的测试名称 */
    self.Member("testDesc") & data.testDesc;         /* 当前的测试描述 */
    self.Member("groupName") & data.groupName;       /* 测试组名称/测试分系统名称 */
    self.Member("dotDrequency") & data.dotDrequency; /* 点频 */
    self.Member("testTypeID") & data.testTypeID;     /* 测试类型ID 决定进行那种测试 */
    self.Member("testTypeName") & data.testTypeName; /* 对应测试的名称比如G/T值测试 本来是和testTypeID是一一对应的 */
    self.Member("projectCode") & data.projectCode;   /* 项目代号 */
    self.Member("taskIdent") & data.taskIdent;       /* 任务标识 */
    self.Member("taskCode") & data.taskCode;         /* 任务代号 */
    self.Member("createTime") & data.createTime;     /* 创建时间 */
    self.Member("workMode") & data.workMode;         /* 工作模式 */
    self.Member("equipComb") & data.equipComb;       /* 设备组合号 */
    self.Member("testParameterMap") & data.testParameterMap;
    self.Member("automateTestParameterBind") & data.automateTestParameterBind;

    //    self.Member("upFreq") & data.upFreq;             /* 上行频率[MHz] */
    //    self.Member("downFreq") & data.downFreq;         /* 下行频率[MHz] */
    return self.EndObject();
}
QByteArray& operator>>(QByteArray& self, AutomateTestPlanItem& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QByteArray& operator<<(QByteArray& self, const AutomateTestPlanItem& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator>>(QString& self, AutomateTestPlanItem& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QString& operator<<(QString& self, const AutomateTestPlanItem& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const AutomateTestPlanItemList& data)
{
    self.StartObject();
    self.Member("AutomateTestPlanItemList");

    self.StartArray();
    for (auto& item : data)
    {
        self& item;
    }
    self.EndArray();
    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, AutomateTestPlanItemList& data)
{
    self.StartObject();
    self.Member("AutomateTestPlanItemList");

    std::size_t count = 0;
    self.StartArray(&count);
    for (std::size_t i = 0; i < count; i++)
    {
        AutomateTestPlanItem info;
        self& info;
        data << info;
    }
    self.EndArray();
    return self.EndObject();
}
QByteArray& operator>>(QByteArray& self, AutomateTestPlanItemList& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QByteArray& operator<<(QByteArray& self, const AutomateTestPlanItemList& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator>>(QString& self, AutomateTestPlanItemList& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QString& operator<<(QString& self, const AutomateTestPlanItemList& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const AutomateTestResult& data)
{
    self.StartObject();

    self.Member("id") & data.id;                 /* 唯一ID */
    self.Member("testTypeID") & data.testTypeID; /* 测试类型ID 决定进行那种测试 */
    self.Member("testTypeName") & data.testTypeName;
    self.Member("testTime") & data.testTime;
    self.Member("testPlanName") & data.testPlanName;     /* 对应创建测试项的名称 */
    self.Member("testParam") & data.testParam;           /* 当前测试项的测试参数 */
    self.Member("TestResultList") & data.testResultList; /* 测试结果列表分组 */

    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, AutomateTestResult& data)
{
    self.StartObject();

    self.Member("id") & data.id;                 /* 唯一ID */
    self.Member("testTypeID") & data.testTypeID; /* 测试类型ID 决定进行那种测试 */
    self.Member("testTypeName") & data.testTypeName;
    self.Member("testTime") & data.testTime;
    self.Member("testPlanName") & data.testPlanName;     /* 对应创建测试项的名称 */
    self.Member("testParam") & data.testParam;           /* 当前测试项的测试参数 */
    self.Member("TestResultList") & data.testResultList; /* 测试结果列表分组 */

    return self.EndObject();
}
QByteArray& operator>>(QByteArray& self, AutomateTestResult& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QByteArray& operator<<(QByteArray& self, const AutomateTestResult& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator>>(QString& self, AutomateTestResult& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QString& operator<<(QString& self, const AutomateTestResult& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const AutomateTestResultList& data)
{
    self.StartObject();
    self.Member("AutomateTestResultList");

    self.StartArray();
    for (auto& item : data)
    {
        self& item;
    }
    self.EndArray();
    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, AutomateTestResultList& data)
{
    self.StartObject();
    self.Member("AutomateTestResultList");

    std::size_t count = 0;
    self.StartArray(&count);
    for (std::size_t i = 0; i < count; i++)
    {
        AutomateTestResult automateTestResult;
        self& automateTestResult;
        data << automateTestResult;
    }
    self.EndArray();
    return self.EndObject();
}
QByteArray& operator>>(QByteArray& self, AutomateTestResultList& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QByteArray& operator<<(QByteArray& self, const AutomateTestResultList& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator>>(QString& self, AutomateTestResultList& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QString& operator<<(QString& self, const AutomateTestResultList& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const AutomateTestResultQuery& data)
{
    self.StartObject();
    self.Member("testTypeID") & data.testTypeID; /* 测试的类型ID */
    self.Member("beginTime") & data.beginTime;   /* 开始时间 */
    self.Member("endTime") & data.endTime;       /* 结束时间 */
    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, AutomateTestResultQuery& data)
{
    self.StartObject();
    self.Member("testTypeID") & data.testTypeID; /* 测试的类型ID */
    self.Member("beginTime") & data.beginTime;   /* 开始时间 */
    self.Member("endTime") & data.endTime;       /* 结束时间 */
    return self.EndObject();
}
QByteArray& operator>>(QByteArray& self, AutomateTestResultQuery& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QByteArray& operator<<(QByteArray& self, const AutomateTestResultQuery& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator>>(QString& self, AutomateTestResultQuery& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QString& operator<<(QString& self, const AutomateTestResultQuery& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const AutomateTestParameterBind& data)
{
    self.StartObject();

    self.Member("id") & data.id;                                  /* 唯一ID */
    self.Member("workMode") & data.workMode;                      /* 参数装订的工作模式 */
    self.Member("testTypeID") & data.testTypeID;                  /* 测试类型ID 决定进行那种测试 */
    self.Member("createTime") & data.createTime;                  /* 创建时间 */
    self.Member("TestBindParameter") & data.testBindParameterMap; /* 参数装订的参数Map */

    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, AutomateTestParameterBind& data)
{
    self.StartObject();

    self.Member("id") & data.id;                                  /* 唯一ID */
    self.Member("workMode") & data.workMode;                      /* 参数装订的工作模式 */
    self.Member("testTypeID") & data.testTypeID;                  /* 测试类型ID 决定进行那种测试 */
    self.Member("createTime") & data.createTime;                  /* 创建时间 */
    self.Member("TestBindParameter") & data.testBindParameterMap; /* 参数装订的参数Map */

    return self.EndObject();
}
QByteArray& operator>>(QByteArray& self, AutomateTestParameterBind& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QByteArray& operator<<(QByteArray& self, const AutomateTestParameterBind& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator>>(QString& self, AutomateTestParameterBind& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QString& operator<<(QString& self, const AutomateTestParameterBind& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const AutomateTestParameterBindGroup& data)
{
    self.StartObject();

    self.Member("testTypeID") & data.testTypeID; /* 绑定的类型ID */

    self.Member("ParameterBindGroup");
    self.StartArray();
    for (auto& item : data.bindDataList)
    {
        self& item;
    }
    self.EndArray();
    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, AutomateTestParameterBindGroup& data)
{
    self.StartObject();

    self.Member("testTypeID") & data.testTypeID; /* 绑定的类型ID */

    self.Member("ParameterBindGroup");
    std::size_t count = 0;
    self.StartArray(&count);
    for (std::size_t i = 0; i < count; i++)
    {
        AutomateTestParameterBind info;
        self& info;
        data.bindDataList << info;
    }
    self.EndArray();
    return self.EndObject();
}
QByteArray& operator>>(QByteArray& self, AutomateTestParameterBindGroup& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QByteArray& operator<<(QByteArray& self, const AutomateTestParameterBindGroup& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator>>(QString& self, AutomateTestParameterBindGroup& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QString& operator<<(QString& self, const AutomateTestParameterBindGroup& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const AutomateTestParameterBindMap& data)
{
    self.StartObject();
    self.Member("AutomateTestParameterBindMap");

    self.StartArray();
    for (auto& item : data)
    {
        self& item;
    }
    self.EndArray();
    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, AutomateTestParameterBindMap& data)
{
    self.StartObject();
    self.Member("AutomateTestParameterBindMap");

    std::size_t count = 0;
    self.StartArray(&count);
    for (std::size_t i = 0; i < count; i++)
    {
        AutomateTestParameterBindGroup info;
        self& info;
        data.insert(info.testTypeID, info);
    }
    self.EndArray();
    return self.EndObject();
}
QByteArray& operator>>(QByteArray& self, AutomateTestParameterBindMap& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QByteArray& operator<<(QByteArray& self, const AutomateTestParameterBindMap& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator>>(QString& self, AutomateTestParameterBindMap& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QString& operator<<(QString& self, const AutomateTestParameterBindMap& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const ClearAutomateTestHistry& data)
{
    self.StartObject();
    self.Member("timeInterval") & data.timeInterval; /* 默认是90天 */
    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, ClearAutomateTestHistry& data)
{
    self.StartObject();
    self.Member("timeInterval") & data.timeInterval; /* 默认是90天 */
    return self.EndObject();
}
QByteArray& operator>>(QByteArray& self, ClearAutomateTestHistry& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QByteArray& operator<<(QByteArray& self, const ClearAutomateTestHistry& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator>>(QString& self, ClearAutomateTestHistry& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QString& operator<<(QString& self, const ClearAutomateTestHistry& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}

JsonWriter& operator&(JsonWriter& self, const TestProcessACK& data)
{
    self.StartObject();
    self.Member("status") & data.status;
    self.Member("msg") & data.msg;
    return self.EndObject();
}
JsonReader& operator&(JsonReader& self, TestProcessACK& data)
{
    self.StartObject();
    self.Member("status") & data.status;
    self.Member("msg") & data.msg;
    return self.EndObject();
}
QByteArray& operator>>(QByteArray& self, TestProcessACK& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QByteArray& operator<<(QByteArray& self, const TestProcessACK& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetByteArray());
    return self;
}
QString& operator>>(QString& self, TestProcessACK& data)
{
    JsonReader reader(self);
    reader& data;
    return self;
}
QString& operator<<(QString& self, const TestProcessACK& data)
{
    JsonWriter writer;
    writer& data;
    self.append(writer.GetQString());
    return self;
}
