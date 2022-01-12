#include "ParseDeviceWorkTaskThread.h"

#include <QApplication>
#include <QDir>
#include <QFileInfoList>
#include <QUuid>

#include "GlobalData.h"
#include "MessagePublish.h"
#include "PlatformInterface.h"
#include "RedisHelper.h"
#include "SatelliteManagementSerialize.h"
#include "TaskPlanManagerXmlReader.h"
#include "TaskPlanSql.h"

class ParseDeviceWorkTaskThreadImpl
{
public:
    static QDateTime convertTime(const QString& value)
    {
        static QString ORIGINALFORMAT = "yyyyMMddHHmmss";
        return QDateTime::fromString(value, ORIGINALFORMAT);
    }
};

ParseDeviceWorkTaskThread::ParseDeviceWorkTaskThread(QObject* parent)
    : QThread(parent)
    , mIsRunning(true)
{
    // 初始化存储设备工作计划文件夹
    initStorageDir();
}

void ParseDeviceWorkTaskThread::stop()
{
    mIsRunning = false;
    quit();
    wait();
}

void ParseDeviceWorkTaskThread::run()
{
    TaskPlanFileStorageInfo storageInfo;
    TaskPlanManagerXmlReader reader;
    reader.getTaskPlanFileDirInfo(storageInfo);

    QDir deviceFileRecvDir(PlatformConfigTools::configBusiness(storageInfo.deviceFileRecvDir));
    QDir deviceFileBackupDir(PlatformConfigTools::configBusiness(storageInfo.deviceFileBackupDir));
    QDir deviceFileErrorDir(PlatformConfigTools::configBusiness(storageInfo.deviceFileErrorDir));

    QDir tempObj;
    int resetPlanCount = 0;
    while (mIsRunning)
    {
        // 判断接收计划的目录是否存在
        if (!deviceFileRecvDir.exists())
        {
            auto absPath = deviceFileRecvDir.absolutePath();
            if(!tempObj.mkdir(absPath))
            {
                SystemLogPublish::errorMsg(QString("接收计划的目录不存在：%1").arg(absPath));
            }
            continue;
        }
        if (!deviceFileBackupDir.exists())
        {
            tempObj.mkdir(deviceFileBackupDir.absolutePath());
        }
        if (!deviceFileErrorDir.exists())
        {
            tempObj.mkdir(deviceFileErrorDir.absolutePath());
        }

        QStringList nameFilters;  // 文件筛选
        nameFilters << "*.xml";   // 只需要xml文件
        auto fileInfoList = deviceFileRecvDir.entryInfoList(nameFilters, QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);  // 获取文件列表

        // 遍历文件集
        for (auto fileInfo : fileInfoList)
        {
            if (!(fileInfo.fileName().contains("SBGZJH") || //
                 fileInfo.fileName().contains("SJCSJH") || //
                 fileInfo.fileName().contains("JHSQZL") || //
                 fileInfo.fileName().contains("XXJSHZ")))
            {
                //剪切文件到备份目录
                QString inputFullFileName = fileInfo.filePath();
                auto errorDirPath = QString("%1/%2/").arg(deviceFileErrorDir.absolutePath()).arg(GlobalData::currentDate().toString("yyyyMMdd"));
                auto ret = copyFileToDir(inputFullFileName, errorDirPath, true);
                if (ret)
                {
                    qWarning() << QString("备份FEP设备工作计划文件成功");
                }
                continue;
            }

            //设备工作计划
            if (fileInfo.fileName().toUpper().contains("SBGZJH"))
            {
                VoiceAlarmPublish::publish("收到中心计划");
                // 处理文件名信息
                DeviceWorkTaskFileInfo deviceWorkTaskFileInfo;
                QString errorMsg;
                bool isParseFileNameSuccess = parseFileName(fileInfo.filePath(), deviceWorkTaskFileInfo, errorMsg);
                if (!isParseFileNameSuccess)
                {
                    errorMsg = QString("%1：%2").arg(fileInfo.fileName()).arg(errorMsg);
                    SystemLogPublish::errorMsg(errorMsg);

                    // 备份文件名格式错误的文件
                    auto errorDirPath = QString("%1/%2").arg(deviceFileErrorDir.absolutePath()).arg(GlobalData::currentDate().toString("yyyyMMdd"));
                    auto newFileName = QString("%1_%2").arg(GlobalData::currentDateTime().toString("yyyyMMddhhmmss")).arg(fileInfo.fileName());
                    copyFileToDir(fileInfo.filePath(), errorDirPath, true, newFileName);
                    VoiceAlarmPublish::publish("中心计划解析错误");
                    continue;
                }

                // 解析任务计划文件
                QList<DeviceWorkTask> list;
                parseTaskPlanXml(fileInfo.filePath(), list);

                // 校验计划文件是否正确
                if (!checkTaskPlan(list))
                {
                    // 备份任务计划内容错误的文件
                    auto errorDirPath = QString("%1/%2").arg(deviceFileErrorDir.absolutePath()).arg(GlobalData::currentDate().toString("yyyyMMdd"));
                    auto newFileName = QString("%1_%2").arg(GlobalData::currentDateTime().toString("yyyyMMddhhmmss")).arg(fileInfo.fileName());
                    copyFileToDir(fileInfo.filePath(), errorDirPath, true, newFileName);
                    VoiceAlarmPublish::publish("中心计划解析错误");
                }
                else
                {
                    // 备份正确的任务计划文件
                    auto backupDirPath = QString("%1/%2").arg(deviceFileBackupDir.absolutePath()).arg(GlobalData::currentDate().toString("yyyyMMdd"));
                    auto newFileName = QString("%1_%2").arg(GlobalData::currentDateTime().toString("yyyyMMddhhmmss")).arg(fileInfo.fileName());
                    copyFileToDir(fileInfo.filePath(), backupDirPath, true, newFileName);
                }

                if (!list.isEmpty())
                {
                    VoiceAlarmPublish::publish("中心计划解析成功");
                    newDeviceWorkTask(list);
                    //emit signalNewDeviceWorkTask(list);
                }
            }
            //数据传输计划
            else if (fileInfo.fileName().toUpper().contains("SJCSJH"))
            {
                // VoiceAlarmPublish::publish("收到中心数据传输计划");
                // 处理文件名信息
                DeviceWorkTaskFileInfo deviceWorkTaskFileInfo;
                QString errorMsg;
                bool isParseFileNameSuccess = parseFileName(fileInfo.filePath(), deviceWorkTaskFileInfo, errorMsg);
                if (!isParseFileNameSuccess)
                {
                    errorMsg = QString("%1：%2").arg(fileInfo.fileName()).arg(errorMsg);
                    SystemLogPublish::errorMsg(errorMsg);
                    // VoiceAlarmPublish::publish("中心数据传输计划解析错误");
                }
                else
                {
                    // 解析任务计划文件
                    DataTranWorkTaskList list;
                    parseDtTaskPlanXml(fileInfo.filePath(), list);
                    if (!list.isEmpty())
                    {
                        // VoiceAlarmPublish::publish("中心数据传输计划解析成功");
                        newDataTransWorkTask(list);
                        //emit signalNewDataTransWorkTask(list);
                    }
                }

                // 备份任务计划文件
                auto backupDirPath = QString("%1/%2").arg(deviceFileBackupDir.absolutePath()).arg(GlobalData::currentDate().toString("yyyyMMdd"));
                auto newFileName = QString("%1_%2").arg(GlobalData::currentDateTime().toString("yyyyMMddhhmmss")).arg(fileInfo.fileName());
                copyFileToDir(fileInfo.filePath(), backupDirPath, true, newFileName);
            }
        }

        /* 重置计划信息 */
        if (resetPlanCount >= 10)
        {
            resetPlanCount = 0;
            TaskPlanSql::resetPlan();
        }
        ++resetPlanCount;
        QThread::msleep(1000);
    }
}

void ParseDeviceWorkTaskThread::initStorageDir()
{
    TaskPlanFileStorageInfo storageInfo;
    TaskPlanManagerXmlReader reader;
    reader.getTaskPlanFileDirInfo(storageInfo);

    //校验参数
    reader.getTaskPlanConfigCheck(taskPlanConfigCheck);

    QStringList dirPathList;
    dirPathList << PlatformConfigTools::configBusiness(storageInfo.deviceFileRecvDir);
    dirPathList << PlatformConfigTools::configBusiness(storageInfo.deviceFileBackupDir);
    dirPathList << PlatformConfigTools::configBusiness(storageInfo.deviceFileErrorDir);

    QDir dir;
    for (auto dirPath : dirPathList)
    {
        if (!QFileInfo::exists(dirPath))
        {
            dir.mkpath(dirPath);
        }
    }
}

bool ParseDeviceWorkTaskThread::parseFileName(const QFileInfo& fileInfo, DeviceWorkTaskFileInfo& deviceWorkTaskFileInfo, QString& error)
{
    if (!fileInfo.isFile())
    {
        error = QString("%1的类型需要必须是一个文件").arg(fileInfo.absoluteFilePath());
        return false;
    }

    // 分割计划文件名称
    QString fileName = fileInfo.baseName();
    QStringList fileNameList = fileName.split("_");
    int listSize = fileNameList.size();
    if (listSize != 8)
    {
        error = QString("文件名不符合标准");
        return false;
    }
    QString m_version_number = fileNameList[0];        // 版本号
    QString m_object_identifying = fileNameList[1];    // 对象标识
    QString m_source_identifying = fileNameList[2];    // 信源标识
    QString m_mode_identifying = fileNameList[3];      // 模式标识
    QString m_msg_type_identifying = fileNameList[4];  // 信息类别标识
    QString m_date_identifying = fileNameList[5];      // 日期标识
    QString m_moment_identifying = fileNameList[6];    // 时刻标识
    QString m_number = fileNameList[7];                // 编号

    if (m_version_number.length() != 2)
    {
        error = QString("文件名中版本号不符合标准");
        return false;
    }
    if (m_version_number != "04")
    {
        error = QString("文件名中版本号只能是04");
        return false;
    }
    if (m_mode_identifying.length() != 2)
    {
        error = QString("文件名中模式不符合标准");
        return false;
    }
    if (m_date_identifying.length() != 9)
    {
        error = QString("文件名中时区日期长度不符合标准");
        return false;
    }
    if (!(m_date_identifying.startsWith("U") || m_date_identifying.startsWith("B")))
    {
        error = QString("文件名中时区不符合标准");
        return false;
    }
    if (m_moment_identifying.length() != 6)
    {
        error = QString("文件名中时刻长度不符合标准");
        return false;
    }
    if (m_number.length() != 4)
    {
        error = QString("文件名中编号长度不符合标准");
        return false;
    }

    bool isOk = false;
    auto tempUsed = m_number.toInt(&isOk);
    Q_UNUSED(tempUsed);
    if (!isOk)
    {
        error = QString("文件名中编号不符合标准");
        return false;
    }

    deviceWorkTaskFileInfo.m_version_number = m_version_number.toInt();
    deviceWorkTaskFileInfo.m_object_identifying = m_object_identifying;
    deviceWorkTaskFileInfo.m_source_identifying = m_source_identifying;
    deviceWorkTaskFileInfo.m_mode_identifying = m_mode_identifying;
    deviceWorkTaskFileInfo.m_msg_type_identifying = m_msg_type_identifying;
    deviceWorkTaskFileInfo.m_date_identifying = m_date_identifying;
    deviceWorkTaskFileInfo.m_moment_identifying = m_moment_identifying;
    deviceWorkTaskFileInfo.m_number = m_number;

    return true;
}

void ParseDeviceWorkTaskThread::parseTaskPlanXml(const QString& filePath, DeviceWorkTaskList& deviceWorkTaskList)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return;
    }

    QDomDocument doc;
    if (!doc.setContent(&file))
    {
        file.close();
        return;
    }
    file.close();

    QMap<QString, int> taskNameNumberMap;  // QMap<任务名称, 对应的序号>

    auto node = doc.documentElement().firstChild();
    // 任务实例
    DeviceWorkTask subDeviceWorkTask;
    while (!node.isNull())
    {
        auto element = node.toElement();
        if (!element.isNull())
        {
            auto tagName = element.tagName();
            auto text = element.text();
            if (QString("时间") == tagName)
            {
                subDeviceWorkTask.m_createTime = ParseDeviceWorkTaskThreadImpl::convertTime(text);
                if (!subDeviceWorkTask.m_createTime.isValid())
                {
                    SystemLogPublish::errorMsg(QString("远程计划:当前计划时间格式不正确 %1").arg(text));
                    return;
                }
            }
            else if (QString("计划流水号") == tagName)
            {
                subDeviceWorkTask.m_plan_serial_number = text;
                /* 格式检查 yyyyMMddxxxx */
                if (subDeviceWorkTask.m_plan_serial_number.size() != 12)
                {
                    SystemLogPublish::errorMsg(QString("远程计划:当前计划流水号不正确 %1").arg(subDeviceWorkTask.m_plan_serial_number));
                    return;
                }

                auto tempData = QDateTime::fromString(subDeviceWorkTask.m_plan_serial_number.left(8), "yyyyMMdd");
                if (!tempData.isValid())
                {
                    SystemLogPublish::errorMsg(QString("远程计划:当前计划流水号不正确 %1").arg(subDeviceWorkTask.m_plan_serial_number));
                    return;
                }
            }
            else if (QString("工作单位") == tagName)
            {
                subDeviceWorkTask.m_work_unit = text;
            }
            else if (QString("设备代号") == tagName)
            {
                subDeviceWorkTask.m_equipment_no = text;
            }
            else if (QString("调度模式") == tagName)
            {
                subDeviceWorkTask.m_scheduling_mode = text;
                subDeviceWorkTask.m_create_userid = "fep";
                subDeviceWorkTask.m_is_zero = TaskCalibration::CaliBrationOnce;      // 是否校零
                subDeviceWorkTask.m_calibration = TaskCalibration::CaliBrationOnce;  // 是否校相
            }
            else if (QString("任务总数") == tagName)
            {
                subDeviceWorkTask.m_task_total_num = text.toInt();
                if (subDeviceWorkTask.m_task_total_num <= 0)
                {
                    SystemLogPublish::errorMsg(QString("远程计划:当前计划任务总数不正确"));
                    return;
                }
                // 插入任务名称和对应的序号
                for (int i = 1; i <= subDeviceWorkTask.m_task_total_num; ++i)
                {
                    taskNameNumberMap.insert(QString("任务%1").arg(i), i);
                }
            }
            else if (taskNameNumberMap.contains(tagName))
            {
                DeviceWorkTask deviceWorkTask = subDeviceWorkTask;
                deviceWorkTask.m_is_zero = TaskCalibration::CaliBrationOnce;                                            // 是否校零
                deviceWorkTask.m_calibration = TaskCalibration::CaliBrationOnce;                                        // 是否校相
                deviceWorkTask.m_currentTaskName = tagName;                                                             // 任务1、任务2、任务3
                int task_number = taskNameNumberMap[tagName];                                                           // 任务编号不需要
                deviceWorkTask.m_uuid = QString("%1_%2").arg(subDeviceWorkTask.m_plan_serial_number).arg(task_number);  // 任务的唯一的UUID
                deviceWorkTask.m_source = TaskPlanSource::Remote;

                // 解析任务节点
                parseTaskNode(element, deviceWorkTask);

                deviceWorkTaskList.append(deviceWorkTask);
            }
        }
        node = node.nextSibling();
    }
}

void ParseDeviceWorkTaskThread::parseTaskNode(const QDomElement& root, DeviceWorkTask& deviceWorkTask)
{
    QMap<QString, int> targetNameNumberMap;  // QMap<目标名, 对应的序号>

    QDomNode node = root.firstChild();
    while (!node.isNull())
    {
        QDomElement element = node.toElement();
        if (!element.isNull())
        {
            auto tagName = element.tagName();
            if (QString("工作方式") == tagName)
            {
                deviceWorkTask.m_working_mode = element.text();
            }
            else if (QString("目标数量") == tagName)
            {
                deviceWorkTask.m_target_number = element.text().toInt();
                // 插入目标名称和对应的序号
                for (int i = 1; i <= deviceWorkTask.m_target_number; ++i)
                {
                    targetNameNumberMap.insert(QString("目标%1").arg(i), i);
                }
            }
            else if (QString("主跟目标") == tagName)
            {
                deviceWorkTask.m_lord_with_target = element.text();
            }
            else if (QString("跟踪顺序") == tagName)
            {
                deviceWorkTask.m_tracking_order = element.text();
                deviceWorkTask.m_task_status = TaskPlanStatus::NoStart;
            }
            else if (targetNameNumberMap.contains(tagName))
            {
                int number = targetNameNumberMap[tagName];  // 目标序号
                DeviceWorkTaskTarget target;
                parseTargetNode(element, target);  // 解析目标
                deviceWorkTask.m_totalTargetMap[number] = target;
            }
        }
        node = node.nextSibling();
    }

    if (!deviceWorkTask.m_totalTargetMap.isEmpty())
    {
        auto& firstTarget = deviceWorkTask.m_totalTargetMap.first();  // 使用第一个目标时间

        deviceWorkTask.m_preStartTime = firstTarget.m_task_ready_start_time;  // 任务准备开始时间
        deviceWorkTask.m_startTime = firstTarget.m_task_start_time;           // 任务开始时间
        deviceWorkTask.m_endTime = firstTarget.m_task_end_time;               // 任务结束时间
    }
}

void ParseDeviceWorkTaskThread::parseTargetNode(const QDomElement& root, DeviceWorkTaskTarget& target)
{
    /* 这里要先根据任务代号查询出卫星信息 */
    SatelliteManagementData sateData;
    QMap<QString, int> linkNameNumberMap;  //  QMap<链路名称, 对应的链路序号>

    QDomNode node = root.firstChild();
    while (!node.isNull())
    {
        QDomElement element = node.toElement();
        if (!element.isNull())
        {
            QString tagName = element.tagName();
            if (QString("任务代号") == tagName)
            {
                target.m_task_code = element.text();
                GlobalData::getSatelliteManagementData(target.m_task_code, sateData);
            }
            else if (QString("圈号") == tagName)
            {
                target.m_circle_no = element.text().toInt();
            }
            else if (QString("跟踪接收计划号") == tagName)
            {
                target.m_dt_transNum = element.text();
            }
            //            else if (QString("计划类型") == tagName)
            //            {
            //                target.m_planType = TaskPlanType(element.text().toInt());
            //            }
            else if (QString("时间") == tagName)  // 解析时间
            {
                parseTargetTimeNode(element, target);
            }
            else if (QString("中心") == tagName)
            {
                parseTargetCenterNode(element, target);  // 解析中心
            }
            else if (QString("链路数量") == tagName)
            {
                target.m_link_number = element.text().toInt();
                // 插入链路名称和对应的序号
                for (int i = 1; i <= target.m_link_number; ++i)
                {
                    linkNameNumberMap.insert(QString("链路%1").arg(i), i);
                }
            }
            else if (linkNameNumberMap.contains(tagName))
            {
                int number = linkNameNumberMap[tagName];  // 链路序号

                DeviceWorkTaskTargetLink link;
                bool allMode = false;
                parseLinkNode(element, link, allMode);  // 解析链路节点
                if(allMode && !sateData.m_workModeParamMap.isEmpty())
                {
                    allModeNode(target, sateData, link);
                    break;
                }
                else
                {
                    target.m_linkMap[number] = link;
                }
            }
        }
        node = node.nextSibling();
    }

    // 根据所有链路的工作模式判断目标的计划类型
    bool haveCKWorkMode = false;
    bool haveSCWorkMode = false;
    QSet<SystemWorkMode> linkWorkModeSet;
    for (auto& link : target.m_linkMap)
    {
        linkWorkModeSet << link.m_work_system;
        if (SystemWorkModeHelper::isDataTransmissionWorkMode(link.m_work_system))
        {
            haveSCWorkMode = true;
        }
        else if (SystemWorkModeHelper::isMeasureContrlWorkMode(link.m_work_system))
        {
            haveCKWorkMode = true;
        }

        if(link.m_baseband_number <= 0 && !sateData.m_workModeParamMap.isEmpty())
        {
            auto& paramMap = sateData.m_workModeParamMap;
            if(paramMap.contains(link.m_work_system))
            {
                auto tempData = paramMap.value(link.m_work_system);
                link.m_baseband_number = (tempData.m_dpInfoMap.contains(tempData.defalutNum) ? tempData.defalutNum : tempData.m_dpInfoMap.firstKey());
            }
        }
    }

    if (haveCKWorkMode && haveSCWorkMode)
    {
        target.m_planType = TaskPlanType::MeasureCtrlDataTrasn;
    }
    else if (haveCKWorkMode)
    {
        target.m_planType = TaskPlanType::MeasureControl;
    }
    else if (haveCKWorkMode)
    {
        target.m_planType = TaskPlanType::DataTrans;
    }
}

void ParseDeviceWorkTaskThread::parseTargetTimeNode(const QDomElement& root, DeviceWorkTaskTarget& target)
{
    QDomNode node = root.firstChild();
    while (!node.isNull())
    {
        QDomElement element = node.toElement();
        if (!element.isNull())
        {
            QString tagName = element.tagName();
            QString value = element.text();
            if (QString("任务准备开始时间") == tagName)
            {
                target.m_task_ready_start_time = ParseDeviceWorkTaskThreadImpl::convertTime(value);
            }
            else if (QString("任务开始时间") == tagName)
            {
                target.m_task_start_time = ParseDeviceWorkTaskThreadImpl::convertTime(value);
            }
            else if (QString("跟踪起始时间") == tagName)
            {
                target.m_track_start_time = ParseDeviceWorkTaskThreadImpl::convertTime(value);
            }
            else if (QString("遥控开始时间") == tagName)
            {
                target.m_remote_start_time = value;
            }
            else if (QString("遥控结束时间") == tagName)
            {
                target.m_remote_control_end_time = value;
            }
            else if (QString("开上行信号时间") == tagName)
            {
                target.m_on_uplink_signal_time = value;
            }
            else if (QString("关上行信号时间") == tagName)
            {
                target.m_turn_off_line_signal_time = value;
            }
            else if (QString("跟踪结束时间") == tagName)
            {
                target.m_track_end_time = ParseDeviceWorkTaskThreadImpl::convertTime(value);
            }
            else if (QString("任务结束时间") == tagName)
            {
                target.m_task_end_time = ParseDeviceWorkTaskThreadImpl::convertTime(value);
            }
            else if (QString("数传开始时间") == tagName)
            {
                target.m_dt_starttime = ParseDeviceWorkTaskThreadImpl::convertTime(value);
            }
            else if (QString("数传结束时间") == tagName)
            {
                target.m_dt_endtime = ParseDeviceWorkTaskThreadImpl::convertTime(value);
            }
        }
        node = node.nextSibling();
    }
}

void ParseDeviceWorkTaskThread::parseTargetCenterNode(const QDomElement& root, DeviceWorkTaskTarget& target)
{
    QDomNode node = root.firstChild();
    while (!node.isNull())
    {
        QDomElement element = node.toElement();
        if (!element.isNull())
        {
            QString tagName = element.tagName();
            if (QString("主控中心") == tagName)
            {
                target.m_master_control_center = element.text();
            }
            else if (QString("备份中心") == tagName)
            {
                target.m_backup_center = element.text();
            }
        }
        node = node.nextSibling();
    }
}

void ParseDeviceWorkTaskThread::parseLinkNode(const QDomElement& root, DeviceWorkTaskTargetLink& link, bool& allMode)
{
    //赋值变量初始化
    //当前链路的的工作频段
    allMode = false;
    QString currentWorkFreq;
    QDomNode node = root.firstChild();
    while (!node.isNull())
    {
        QDomElement element = node.toElement();
        if (!element.isNull())
        {
            QString tagName = element.tagName();
            QString text = element.text();
            if (QString("工作点频") == tagName)
            {
                if(text.isEmpty() || text.toUpper() == "NULL")
                {
                    link.working_point_frequency = 0;
                }
                else
                {
                    // 工作频段：C、Ka、S  	xx-n:代表xx频段第n组点频  比如Ka-1:Ka频段第一组点频
                    QStringList list = text.split("-");
                    if (list.size() == 2)
                    {
                        currentWorkFreq = list[0];
                        link.working_point_frequency = list[1].toInt();
                        if(link.working_point_frequency <= 0)
                        {
                            link.working_point_frequency = 0;
                        }
                    }
                    else
                    {
                        //计划解析错误
                        link.working_point_frequency = 0;
                    }
                }
            }
            else if (QString("工作体制") == tagName)
            {
                if(text.isEmpty() || text.toUpper() == "NULL")
                {
                    allMode = true;
                }
                else
                {

                    // 标准：USB、UCB 扩1：FE1 扩2：FE2 高速：DBHR 低速：DHLR 扩调频：FHDS 一体化上面级：INT1 一体化卫星：INT2 上行高码率：INT3
                    // S频段
                    if (currentWorkFreq.compare(QString("S"), Qt::CaseInsensitive) == 0)
                    {
                        if (text == QString("USB"))
                        {
                            link.m_work_system = SystemWorkMode::STTC;
                        }
                        else if (text == QString("FE2"))
                        {
                            link.m_work_system = SystemWorkMode::Skuo2;
                        }
                        else if (text == QString("FHDS"))
                        {
                            link.m_work_system = SystemWorkMode::SKT;
                        }
                        else if (text == QString("INT1"))
                        {
                            link.m_work_system = SystemWorkMode::SYTHSMJ;
                        }
                        else if (text == QString("INT2"))
                        {
                            link.m_work_system = SystemWorkMode::SYTHWX;
                        }
                        else if (text == QString("INT3"))
                        {
                            link.m_work_system = SystemWorkMode::SYTHGML;
                        }
                    }
                    // Ka频段
                    else if (currentWorkFreq.compare(QString("Ka"), Qt::CaseInsensitive) == 0)
                    {
                        if (text == QString("FE2"))
                        {
                            link.m_work_system = SystemWorkMode::KaKuo2;
                        }
                        else if (text == QString("DHLR"))
                        {
                            link.m_work_system = SystemWorkMode::KaDS;
                        }
                        else if (text == QString("DNHR") || text == QString("DBHR")) // 这里友哥写错了两个都判断一下 20211223
                        {
                            link.m_work_system = SystemWorkMode::KaGS;
                        }
                    }
                    // X频段
                    else if (currentWorkFreq.compare(QString("X"), Qt::CaseInsensitive) == 0)
                    {
                        if (text == QString("DHLR"))
                        {
                            link.m_work_system = SystemWorkMode::XDS;
                        }
                        else if (text == QString("DNHR"))
                        {
                            link.m_work_system = SystemWorkMode::XGS;
                        }
                    }
                    else
                    {
                        allMode = true;
                    }
                }
            }
            else if (QString("任务类型") == tagName)
            {
                // 遥测:TM  测控:TC 测轨:R 数传:HR 多个用'+'隔开
                QStringList list = text.split("+");
                for (auto item : list)
                {
                    if ("TM" == item)
                    {
                        link.m_taskType_yc = true;
                    }
                    else if ("TC" == item)
                    {
                        link.m_taskType_yk = true;
                    }
                    else if ("R" == item)
                    {
                        link.m_taskType_cg = true;
                    }
                    else if ("HR" == item)
                    {
                        link.m_taskType_sc = true;
                    }
                }
            }
            // 左旋极化： LHCP 右旋极化：RHCP 水平极化：HLP  垂直极化：VLP
            else if (QString("极化方式") == tagName)
            {
                // HLP,VLP代表左右旋同时
                if ("LHCP" == text || "RHCP" == text || "HLP" == text || "VLP" == text)
                {
                    link.m_linkVal = text;
                }
            }
            // 共载波，不共载波  4424处理
            else if (QString("链路补充信息") == tagName)
            {
                if (text == "GZ")
                {
                    link.m_carrierType = CarrierType::GZB;
                }
                else if (text == "BGZ")
                {
                    link.m_carrierType = CarrierType::BGZB;
                }
            }
            else if (QString("基带编号") == tagName)
            {
                if (text.isEmpty())
                {
                    text = "NULL";
                }
                link.m_baseband_number = text;
            }
        }
        node = node.nextSibling();
    }
}

void ParseDeviceWorkTaskThread::allModeNode(DeviceWorkTaskTarget& target, const SatelliteManagementData& sateData, DeviceWorkTaskTargetLink link)
{
    int number = 1;
    auto& paramMap = sateData.m_workModeParamMap;
    /* 只要一个链路是全模式则整个目标使用全模式
     * 工作模式为空 则选取当前卫星所有不冲突的模式
     * 数传都不冲突
     * 测控模式除了Skuo2和ka扩2不冲突都是冲突的
     * 优先级
     * skuo2 sttc kakuo2 skt
    */
    target.m_linkMap.clear();
    link.m_linkVal.clear();

    // 测控
    if(paramMap.contains(Skuo2) && paramMap.contains(KaKuo2))
    {
        auto tempData = paramMap.value(Skuo2);
        link.m_work_system = Skuo2;
        link.working_point_frequency = (tempData.m_dpInfoMap.contains(tempData.defalutNum) ? tempData.defalutNum : tempData.m_dpInfoMap.firstKey());
        target.m_linkMap[number++] = link;

        tempData = paramMap.value(KaKuo2);
        link.m_work_system = KaKuo2;
        link.working_point_frequency = (tempData.m_dpInfoMap.contains(tempData.defalutNum) ? tempData.defalutNum : tempData.m_dpInfoMap.firstKey());
        target.m_linkMap[number++] = link;
    }
    else if(paramMap.contains(Skuo2))
    {
        auto tempData = paramMap.value(Skuo2);
        link.m_work_system = Skuo2;
        link.working_point_frequency = (tempData.m_dpInfoMap.contains(tempData.defalutNum) ? tempData.defalutNum : tempData.m_dpInfoMap.firstKey());
        target.m_linkMap[number++] = link;
    }
    else if(paramMap.contains(STTC))
    {
        auto tempData = paramMap.value(STTC);
        link.m_work_system = STTC;
        link.working_point_frequency = (tempData.m_dpInfoMap.contains(tempData.defalutNum) ? tempData.defalutNum : tempData.m_dpInfoMap.firstKey());
        target.m_linkMap[number++] = link;
    }
    else if(paramMap.contains(KaKuo2))
    {
        auto tempData = paramMap.value(KaKuo2);
        link.m_work_system = KaKuo2;
        link.working_point_frequency = (tempData.m_dpInfoMap.contains(tempData.defalutNum) ? tempData.defalutNum : tempData.m_dpInfoMap.firstKey());
        target.m_linkMap[number++] = link;
    }
    else if(paramMap.contains(SKT))
    {
        auto tempData = paramMap.value(SKT);
        link.m_work_system = SKT;
        link.working_point_frequency = (tempData.m_dpInfoMap.contains(tempData.defalutNum) ? tempData.defalutNum : tempData.m_dpInfoMap.firstKey());
        target.m_linkMap[number++] = link;
    }

    // 数传
    for(auto iter = paramMap.begin(); iter != paramMap.end(); ++iter)
    {
        auto workMode = iter.key();
        if(!SystemWorkModeHelper::isDataTransmissionWorkMode(workMode))
        {
            continue;
        }
        auto tempData = iter.value();
        link.m_work_system = workMode;
        link.working_point_frequency = (tempData.m_dpInfoMap.contains(tempData.defalutNum) ? tempData.defalutNum : tempData.m_dpInfoMap.firstKey());
        target.m_linkMap[number++] = link;
    }

    target.m_link_number = target.m_linkMap.size();
}


bool ParseDeviceWorkTaskThread::checkTaskPlan(DeviceWorkTaskList& deviceWorkTaskList)
{
    if (deviceWorkTaskList.isEmpty())
    {
        SystemLogPublish::errorMsg(QString("解析中心下发计划文件内容解析错误数据为空"));
        return false;
    }

    // 使用第一个任务信息，同一个计划文件工作单位、设备代号都一样
    auto firstDeviceWorkTask = deviceWorkTaskList.first();
    // 工作单位
    if (!taskPlanConfigCheck.m_workUnitList.contains(firstDeviceWorkTask.m_work_unit))
    {
        SystemLogPublish::errorMsg(QString("解析中心下发计划文件出错，未知工作单位：%1").arg(firstDeviceWorkTask.m_work_unit));
        deviceWorkTaskList.clear();
        return false;
    }
    // 设备代号
    if (!taskPlanConfigCheck.m_m_equipmentNoList.contains(firstDeviceWorkTask.m_equipment_no.toUpper()))
    {
        SystemLogPublish::errorMsg(QString("解析中心下发计划文件出错，未知设备代号：%1").arg(firstDeviceWorkTask.m_equipment_no));
        deviceWorkTaskList.clear();
        return false;
    }
    // 调度模式
    if (!taskPlanConfigCheck.m_schedulingModeList.contains(firstDeviceWorkTask.m_scheduling_mode.toUpper()))
    {
        SystemLogPublish::errorMsg(QString("解析中心下发计划文件出错，未知调度模式：%1").arg(firstDeviceWorkTask.m_equipment_no));
        deviceWorkTaskList.clear();
        return false;
    }

    // 判断任务总数,如果任务总数和解析出来的总数不一致，认为任务错误
    if (deviceWorkTaskList.size() != firstDeviceWorkTask.m_task_total_num)
    {
        SystemLogPublish::errorMsg("解析中心下发计划文件出错，系统任务总数不一致");
        deviceWorkTaskList.clear();
        return false;
    }

    QString errorMsg;

    bool noError = true;
    // 检查任务信息
    int taskNo = 1;
    // 获取卫星数据
    SatelliteManagementDataMap satelliteDataMap;
    GlobalData::getSatelliteManagementData(satelliteDataMap);

    for (auto iter = deviceWorkTaskList.begin(); iter != deviceWorkTaskList.end(); ++taskNo)
    {
        auto deviceWorkTask = (*iter);
        if (!checkDeviceWorkTask(deviceWorkTask, satelliteDataMap, errorMsg))
        {
            noError = false;

            errorMsg = QString("任务%1%2").arg(taskNo).arg(errorMsg);
            SystemLogPublish::errorMsg(errorMsg);

            // 移除错误的任务
            iter = deviceWorkTaskList.erase(iter);
        }
        else
        {
            ++iter;
        }
    }

    // 有部分计划错误，有部分任务计划正确
    if (!noError && !deviceWorkTaskList.isEmpty())
    {
        SystemLogPublish::errorMsg("解析中心下发计划文件中存在错误，部分计划未加载");
    }

    return noError;
}

bool ParseDeviceWorkTaskThread::checkDeviceWorkTask(const DeviceWorkTask& deviceWorkTask, const SatelliteManagementDataMap& satelliteDataMap, QString& errorMsg)
{
    // 工作方式
    if (!taskPlanConfigCheck.m_workingModeList.contains(deviceWorkTask.m_working_mode))
    {
        errorMsg = QString("未知系统工作方式：%1").arg(deviceWorkTask.m_working_mode);
        return false;
    }

    // 判断卫星数据中是否包含主跟目标任务代号
    if (!satelliteDataMap.contains(deviceWorkTask.m_lord_with_target))
    {
        errorMsg = QString("主跟目标任务代号未知%1").arg(deviceWorkTask.m_lord_with_target);
        return false;
    }

    // 检查目标信息
    for (auto iter = deviceWorkTask.m_totalTargetMap.begin(); iter != deviceWorkTask.m_totalTargetMap.end(); ++iter)
    {
        auto targetNo = iter.key();
        auto &target = iter.value();
        if (!checkTarget(target, satelliteDataMap, errorMsg))
        {
            errorMsg = QString("目标%1%2").arg(targetNo).arg(errorMsg);
            return false;
        }
    }

    return true;
}

bool ParseDeviceWorkTaskThread::checkTarget(const DeviceWorkTaskTarget& target, const SatelliteManagementDataMap& satelliteDataMap, QString& errorMsg)
{
    // 判断任务代号
    if (!satelliteDataMap.contains(target.m_task_code))
    {
        errorMsg = QString("任务代号无法在系统进行匹配");
        return false;
    }
    //    if (target.m_circle_no <= 0)
    //    {
    //        errorMsg = QString("圈号需要大于0");
    //        return false;
    //    }
    //    if (target.m_planType == TaskPlanType::Unknown)
    //    {
    //        errorMsg = QString("任务计划类型无效");
    //        return false;
    //    }

    QDateTime taskReadyStartTime = target.m_task_ready_start_time;                                                  // 任务准备开始时间
    QDateTime taskStartTime = target.m_task_start_time;                                                             // 任务开始时间
    QDateTime trackStartTime = target.m_track_start_time;                                                           // 跟踪起始时间
    QDateTime remoteStartTime = ParseDeviceWorkTaskThreadImpl::convertTime(target.m_remote_start_time);             // 遥控开始时间
    QDateTime remoteControlEndTime = ParseDeviceWorkTaskThreadImpl::convertTime(target.m_remote_control_end_time);  // 遥控结束时间
    QDateTime onUplinkSignalTime = ParseDeviceWorkTaskThreadImpl::convertTime(target.m_on_uplink_signal_time);      // 开上行信号时间
    QDateTime OffUplinkSignalTime = QDateTime::fromString(target.m_turn_off_line_signal_time);                      // 关上行信号时间
    QDateTime trackEndTime = target.m_track_end_time;                                                               // 跟踪结束时间
    QDateTime taskEndTime = target.m_task_end_time;                                                                 // 任务结束时间

    // 判断时间是否有效
    if (!taskReadyStartTime.isValid())
    {
        errorMsg = QString("任务准备开始时间无效");
        return false;
    }
    if (!taskStartTime.isValid())
    {
        errorMsg = QString("任务开始时间无效");
        return false;
    }
    if (!trackStartTime.isValid())
    {
        errorMsg = QString("跟踪开始时间无效");
        return false;
    }
    if (!trackEndTime.isValid())
    {
        errorMsg = QString("跟踪结束时间无效");
        return false;
    }
    if (!taskEndTime.isValid())
    {
        errorMsg = QString("任务结束时间无效");
        return false;
    }
    if (!remoteStartTime.isValid() && (target.m_remote_start_time != TimeZero && target.m_remote_start_time != TimeF))
    {
        errorMsg = QString("遥控开始时间无效");
        return false;
    }
    if (!remoteControlEndTime.isValid() && (target.m_remote_control_end_time != TimeZero && target.m_remote_control_end_time != TimeF))
    {
        errorMsg = QString("遥控结束时间无效");
        return false;
    }
    if (!onUplinkSignalTime.isValid() && (target.m_on_uplink_signal_time != TimeZero && target.m_on_uplink_signal_time != TimeF))
    {
        errorMsg = QString("开上行信号时间无效");
        return false;
    }
    if (!OffUplinkSignalTime.isValid() && (target.m_turn_off_line_signal_time != TimeZero && target.m_turn_off_line_signal_time != TimeF))
    {
        errorMsg = QString("关上行信号时间无效");
        return false;
    }

    // 判断时间之间的关联是否有效
    if (taskStartTime < taskReadyStartTime)
    {
        errorMsg = QString("任务开始时间不能早于任务准备开始时间");
        return false;
    }
    if (taskEndTime < taskStartTime)
    {
        errorMsg = QString("任务结束时间不能早于任务开始时间");
        return false;
    }
    if (trackStartTime < taskStartTime || trackStartTime > taskEndTime)
    {
        errorMsg = QString("跟踪开始时间不在任务开始时间和任务结束时间之间");
        return false;
    }
    if (trackEndTime < taskStartTime || trackEndTime > taskEndTime)
    {
        errorMsg = QString("跟踪结束时间不在任务开始时间和任务结束时间之间");
        return false;
    }
    if (trackEndTime < trackStartTime)
    {
        errorMsg = QString("跟踪结束时间不能早于跟踪开始时间");
        return false;
    }
    // 开遥控或者关遥控时间无效，那么开遥控和关遥控要么都全0，要么都全F，需要一致
    if ((!remoteStartTime.isValid() || !remoteControlEndTime.isValid()) && (target.m_remote_start_time != target.m_remote_control_end_time))
    {
        errorMsg = QString("遥控开始时间和遥控结束时间不匹配");
        return false;
    }
    // 开上行或者关上行时间无效，那么开上行和关上行要么都全0，要么都全F，需要一致
    if ((!onUplinkSignalTime.isValid() || !OffUplinkSignalTime.isValid()) && (target.m_on_uplink_signal_time != target.m_turn_off_line_signal_time))
    {
        errorMsg = QString("开上行信号时间和关上行信号时间不匹配");
        return false;
    }
    // 开遥控或者关遥控时间有效（其中一个有效，另外一个必然有效）
    if (remoteStartTime.isValid() || remoteControlEndTime.isValid())
    {
        if (remoteControlEndTime < remoteStartTime)
        {
            errorMsg = QString("遥控结束时间不能早于遥控开始时间");
            return false;
        }
        if (remoteStartTime < trackStartTime || remoteStartTime > trackEndTime)
        {
            errorMsg = QString("遥控开始时间不在跟踪开始时间和跟踪结束时间之间");
            return false;
        }
        if (remoteControlEndTime < trackStartTime || remoteControlEndTime > trackEndTime)
        {
            errorMsg = QString("遥控结束时间不在跟踪开始时间和跟踪结束时间之间");
            return false;
        }
    }
    // 开上行或者关上行时间有效（其中一个有效，另外一个必然有效）
    if (onUplinkSignalTime.isValid() || OffUplinkSignalTime.isValid())
    {
        if (OffUplinkSignalTime < onUplinkSignalTime)
        {
            errorMsg = QString("关上行信号时间不能早于开上行信号时间");
            return false;
        }
        if (OffUplinkSignalTime < trackStartTime || OffUplinkSignalTime > trackEndTime)
        {
            errorMsg = QString("开上行信号时间不在跟踪开始时间和跟踪结束时间之间");
            return false;
        }
        if (onUplinkSignalTime < trackStartTime || onUplinkSignalTime > trackEndTime)
        {
            errorMsg = QString("关上行信号时间不在跟踪开始时间和跟踪结束时间之间");
            return false;
        }
    }

    // 检查链路信息
    if (target.m_link_number != target.m_linkMap.size())
    {
        errorMsg = QString("链路数量和链路总数不匹配");
        return false;
    }
    for (auto linkNo : target.m_linkMap.keys())
    {
        auto link = target.m_linkMap.value(linkNo);
        if (!checkLink(link, errorMsg))
        {
            errorMsg = QString("链路%1%2").arg(linkNo).arg(errorMsg);
            return false;
        }
    }

    return true;
}

bool ParseDeviceWorkTaskThread::checkLink(const DeviceWorkTaskTargetLink& link, QString& errorMsg)
{
    if (link.m_work_system == SystemWorkMode::NotDefine)
    {
        errorMsg = QString("系统工作模式不正确");
        return false;
    }
    if (link.working_point_frequency <= 0)
    {
        errorMsg = QString("工作点频不正确: %1").arg(link.working_point_frequency);
        return false;
    }

    return true;
}

bool ParseDeviceWorkTaskThread::copyFileToDir(const QString& filePath, const QString& dirPath, bool isDeleteSrcFile, const QString& newFileName)
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.isFile())
    {
        return false;
    }

    QString newFilePath;
    if (newFileName.isEmpty())
    {
        newFilePath = QString("%1/%2").arg(dirPath).arg(fileInfo.fileName());
    }
    else
    {
        newFilePath = QString("%1/%2").arg(dirPath).arg(newFileName);
    }

    if (!QFileInfo::exists(dirPath))
    {
        QDir dir;
        dir.mkpath(dirPath);
    }

    auto isCopySuccess = QFile::copy(filePath, newFilePath);
    if (isCopySuccess)
    {
        SystemLogPublish::infoMsg(QString("备份设备工作计划原始文件成功"));
    }
    else
    {
        SystemLogPublish::errorMsg(QString("备份设备工作计划原始文件失败"));
    }

    if (isDeleteSrcFile)
    {
        auto isDelSuccess = QFile::remove(filePath);
        if (isDelSuccess)
        {
            SystemLogPublish::infoMsg(QString("删除设备工作计划原始文件成功"));
        }
        else
        {
            SystemLogPublish::errorMsg(QString("删除设备工作计划原始文件失败"));
        }
    }

    return isCopySuccess && isDeleteSrcFile;
}

bool ParseDeviceWorkTaskThread::parseDtFileName(const QFileInfo& fileInfo, DeviceWorkTaskFileInfo& deviceWorkTaskFileInfo, QString& error)
{
    if (!fileInfo.isFile())
    {
        error = QString("%1的类型需要必须是一个文件").arg(fileInfo.absoluteFilePath());
        return false;
    }

    // 分割计划文件名称
    QString fileName = fileInfo.baseName();
    QStringList fileNameList = fileName.split("_");
    if (fileNameList.length() != 8)
    {
        error = QString("文件名不符合标准");
        return false;
    }
    QString m_version_number = fileNameList[0];        // 版本号
    QString m_object_identifying = fileNameList[1];    // 对象标识
    QString m_source_identifying = fileNameList[2];    // 信源标识
    QString m_mode_identifying = fileNameList[3];      // 模式标识
    QString m_msg_type_identifying = fileNameList[4];  // 信息类别标识
    QString m_date_identifying = fileNameList[5];      // 日期标识
    QString m_moment_identifying = fileNameList[6];    // 时刻标识
    QString m_number = fileNameList[7];                // 编号

    if (m_version_number.length() != 2)
    {
        error = QString("文件名中版本号不符合标准");
        return false;
    }
    if (m_version_number != "04")
    {
        error = QString("文件名中版本号只能是04");
        return false;
    }
    if (m_mode_identifying.length() != 2)
    {
        error = QString("文件名中模式不符合标准");
        return false;
    }
    if (m_date_identifying.length() != 9)
    {
        error = QString("文件名中时区日期长度不符合标准");
        return false;
    }
    if (!(m_date_identifying.startsWith("U") || m_date_identifying.startsWith("B")))
    {
        error = QString("文件名中时区不符合标准");
        return false;
    }
    if (m_moment_identifying.length() != 6)
    {
        error = QString("文件名中时刻长度不符合标准");
        return false;
    }
    if (m_number.length() != 4)
    {
        error = QString("文件名中编号长度不符合标准");
        return false;
    }

    bool isOk = false;
    auto tempUsed = m_number.toInt(&isOk);
    Q_UNUSED(tempUsed);
    if (!isOk)
    {
        error = QString("文件名中编号不符合标准");
        return false;
    }

    deviceWorkTaskFileInfo.m_version_number = m_version_number.toInt();
    deviceWorkTaskFileInfo.m_object_identifying = m_object_identifying;
    deviceWorkTaskFileInfo.m_source_identifying = m_source_identifying;
    deviceWorkTaskFileInfo.m_mode_identifying = m_mode_identifying;
    deviceWorkTaskFileInfo.m_msg_type_identifying = m_msg_type_identifying;
    deviceWorkTaskFileInfo.m_date_identifying = m_date_identifying;
    deviceWorkTaskFileInfo.m_moment_identifying = m_moment_identifying;
    deviceWorkTaskFileInfo.m_number = m_number;

    return true;
}

void ParseDeviceWorkTaskThread::parseDtTaskPlanXml(const QString& filePath, DataTranWorkTaskList& dataTranWorkTaskList)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return;
    }

    QDomDocument doc;
    if (!doc.setContent(&file))
    {
        file.close();
        return;
    }
    file.close();

    QMap<QString, int> taskNameNumberMap;  // QMap<任务名称, 对应的序号>

    auto node = doc.documentElement().firstChild();
    DataTranWorkTask tempDTWorkTask;
    while (!node.isNull())
    {
        QDomElement element = node.toElement();
        if (!element.isNull())
        {
            auto tagName = element.tagName();
            auto text = element.text();
            if (QString("时间") == tagName)
            {
                tempDTWorkTask.m_create_userid = "fep";
                tempDTWorkTask.m_createTime = ParseDeviceWorkTaskThreadImpl::convertTime(text);
            }
            else if (QString("计划流水号") == tagName)
            {
                tempDTWorkTask.m_plan_serial_number = text;
            }
            else if (QString("工作单位") == tagName)
            {
                tempDTWorkTask.m_work_unit = text;
            }
            else if (QString("设备代号") == tagName)
            {
                tempDTWorkTask.m_equipment_no = text;
            }
            else if (QString("任务总数") == tagName)
            {
                tempDTWorkTask.m_task_total_num = text.toInt();
                // 初始化任务描述
                for (int i = 1; i <= tempDTWorkTask.m_task_total_num; ++i)
                {
                    taskNameNumberMap.insert(QString("任务%1").arg(i), i);
                }
            }
            else if (taskNameNumberMap.find(tagName) != taskNameNumberMap.end())
            {
                tempDTWorkTask.m_task_status = TaskPlanStatus::NoStart;
                tempDTWorkTask.m_uuid = QString("%1_%2").arg(tempDTWorkTask.m_plan_serial_number).arg(taskNameNumberMap[tagName]);

                DataTranWorkTask newDTWorkTask = tempDTWorkTask;

                parseDtTaskNode(element, newDTWorkTask);  // 解析任务节点
                dataTranWorkTaskList.append(newDTWorkTask);
            }
        }
        node = node.nextSibling();
    }
}

void ParseDeviceWorkTaskThread::parseDtTaskNode(const QDomElement& root, DataTranWorkTask& dataTranWorkTask)
{
    QDomNode node = root.firstChild();
    while (!node.isNull())
    {
        QDomElement element = node.toElement();
        if (!element.isNull())
        {
            auto tagName = element.tagName();
            auto text = element.text();
            if (QString("工作方式") == tagName)
            {
                dataTranWorkTask.m_working_mode = TaskPlanHelper::KeyStringToDataTransMode(text);
            }
            else if (QString("任务代号") == tagName)
            {
                dataTranWorkTask.m_task_code = text;
            }
            else if (QString("跟踪接收计划号") == tagName)
            {
                dataTranWorkTask.m_dt_transNum = text;
            }
            else if (QString("数据传输中心") == tagName)
            {
                dataTranWorkTask.m_datatrans_center = text;
            }
            else if (QString("优先级") == tagName)
            {
                dataTranWorkTask.m_priority = DataTransPriority(text.toInt());
            }
            else if (QString("传输协议") == tagName)
            {
                dataTranWorkTask.m_transport_protocols = text;
            }
            else if (QString("任务开始时间") == tagName)
            {
                dataTranWorkTask.m_task_start_time = ParseDeviceWorkTaskThreadImpl::convertTime(text);
            }
            else if (QString("任务结束时间") == tagName)
            {
                dataTranWorkTask.m_task_end_time = ParseDeviceWorkTaskThreadImpl::convertTime(text);
            }
            else if (QString("事后传输时间") == tagName)
            {
                dataTranWorkTask.m_transport_time = ParseDeviceWorkTaskThreadImpl::convertTime(text);
            }
            else if (QString("通道号") == tagName)
            {
                dataTranWorkTask.m_channelNum = text;
            }
            else if (QString("圈次内编号") == tagName)
            {
                dataTranWorkTask.m_ring_num = text.toInt();
            }
        }
        node = node.nextSibling();
    }
}

void ParseDeviceWorkTaskThread::newDataTransWorkTask(const DataTranWorkTaskList& dataTranWorkTaskList)
{
    /* 直接删除原来的数据 */
    for (auto& item : dataTranWorkTaskList)
    {
        if(TaskPlanSql::dtPlanIsRunning(item.m_plan_serial_number))
        {
            SystemLogPublish::errorMsg(QString("远程数传计划流水号相同且已经运行:%1").arg(item.m_plan_serial_number));
            return;
        }
        TaskPlanSql::deleteDTPlan(item.m_plan_serial_number);
    }

    //数据传输计划直接插入数据库表
    auto result = TaskPlanSql::insertPlan(dataTranWorkTaskList);
    if (!result)
    {
        SystemLogPublish::errorMsg(QString("远程数传计划插入错误:%1").arg(result.msg()));
    }
}

void ParseDeviceWorkTaskThread::newDeviceWorkTask(const QList<DeviceWorkTask>& deviceWorkTaskList)
{
    /* 获取当前运行的任务计划 */
    auto runningTask = AutoRunTaskStepStatusPublish::getRunningTask();
    auto curDate = GlobalData::currentDateTime();
    auto nostartWorkTaskList = Optional<DeviceWorkTaskList>::emptyOptional();

    if(!runningTask.taskPlanData.deviceWorkTask.m_totalTargetMap.isEmpty() &&
       runningTask.taskPlanData.deviceWorkTask.m_endTime > curDate)
    {
        //如果是占用就删除非占用的未执行的任务计划
        if (runningTask.taskPlanData.deviceWorkTask.m_working_mode == "ZY")
        {
            //查询出未执行的任务计划 ，删除未执行的任务计划
            nostartWorkTaskList = TaskPlanSql::queryNoStartDevPlan();
        }
        else
        {
            //查询出未执行和正在运行的任务计划 ，删除未执行的任务计划
            nostartWorkTaskList = TaskPlanSql::queryRunningOrNoStartDevPlan();
        }
    }

    int delSuccess = 0;
    if(nostartWorkTaskList)
    {
        for (auto& item : nostartWorkTaskList.value())
        {
            auto ans = TaskPlanSql::deletePlan(item.m_uuid);
            if (ans.success())
            {
                delSuccess++;
            }
        }
    }

    //插入新的任务计划
    int insertSuccess = 0;
    for (auto& item : deviceWorkTaskList)
    {
        auto ans = TaskPlanSql::insertPlan(item);
        if (ans.success())
        {
            insertSuccess++;
        }else{
            SystemLogPublish::errorMsg(ans.msg());
        }
    }
}
