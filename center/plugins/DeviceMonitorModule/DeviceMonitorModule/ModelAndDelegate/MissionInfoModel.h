#ifndef MISSIONINFOMODEL_H
#define MISSIONINFOMODEL_H

// class QAbstractItemModel;
#include <QAbstractItemModel>

struct MissionInfo
{
    QString id;             // 任务ID
    QString uac;            // 天线
    QString center;         // 控制中心
    quint32 segment;        // 弧段
    QString starTime;       // 任务开始时间
    QString stopTime;       // 任务结束时间
    QString remainingTime;  // 剩余时间
    quint32 runStatus;      // 运行状态
    quint32 guideMode;      // 引导方式
    QString upT0;           //上升段T0(起飞时间)
};

class MissionInfoModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum Column
    {
        ID,             // 任务ID
        UAC,            // 天线
        Center,         // 控制中心
        Segment,        // 弧段
        StarTime,       // 任务开始时间
        StopTime,       // 任务结束时间
        RemainingTime,  // 剩余时间
        RunStatus,      // 运行状态
        GuideMode,      // 引导方式
        UpT0            //上升段T0(起飞时间)
    };

    MissionInfoModel(QObject* parent = nullptr);

public:
    QModelIndex index(int row, int column, const QModelIndex& parent) const;
    QModelIndex parent(const QModelIndex& child) const;

    int rowCount(const QModelIndex& parent) const;
    int columnCount(const QModelIndex& parent) const;

    void setData(const QList<MissionInfo>& missionInfoList);
    QVariant data(const QModelIndex& index, int role) const;

    void setHeaderData(const QStringList& headers);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

private:
    QStringList mHeaderList;
    QList<MissionInfo> mDataList;
};

#endif  // MISSIONINFOMODEL_H
