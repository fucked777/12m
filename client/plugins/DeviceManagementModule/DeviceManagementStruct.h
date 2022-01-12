#ifndef DEVICEMANAGEMENTSTRUCT_H
#define DEVICEMANAGEMENTSTRUCT_H
#include <QList>
#include <QString>

//添加
enum struct DMDataType
{
    None,
    Antenna, /* 天线 */
    MACB,    /* 测控基带 */
    HSDB,    /* 高速数传基带 */
    LSDB,    /* 低速数传基带 */
    DSAME,   /* 数据存储与管理设备 */
    DID,     /* 数据交互设备 */
    Count,
};

struct DMDataItem
{
    DMDataItem(const std::string id,
               const quint64 createTime,
               const quint64 lastTime,
               const DMDataType type,
               const QByteArray data)
        : id(id), createTime(createTime), lastTime(lastTime), type(type), data(data)
    {
    }
    std::string id;
    quint64 createTime{ 0 };
    quint64 lastTime{ 0 };
    DMDataType type{ DMDataType::None };
    QByteArray data;
};

//删除
struct ResourceDatas
{
    ResourceDatas(const std::list<int> m_ckmain,
                  const std::list<int> m_ckstandy,
                  const std::list<int> m_dsmain,
                  const std::list<int> m_dsstandy,
                  const std::list<int> m_gsmain,
                  const std::list<int> m_gsstandy,
                  const std::list<int> m_cczfmain,
                  const std::list<int> m_cczfstandy)
        : m_ckmain(m_ckmain)
        , m_ckstandy(m_ckstandy)
        , m_dsmain(m_dsmain)
        , m_dsstandy(m_dsstandy)
        , m_gsmain(m_gsmain)
        , m_gsstandy(m_gsstandy)
        , m_cczfmain(m_cczfmain)
        , m_cczfstandy(m_cczfstandy)
    {
    }

    std::list<int> m_ckmain;
    std::list<int> m_ckstandy;
    std::list<int> m_dsmain;
    std::list<int> m_dsstandy;
    std::list<int> m_gsmain;
    std::list<int> m_gsstandy;
    std::list<int> m_cczfmain;
    std::list<int> m_cczfstandy;
};
#endif  // DEVICEMANAGEMENTSTRUCT_H
