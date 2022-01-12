#ifndef RESOURCERESTRUCTURINGSTRUCT_H
#define RESOURCERESTRUCTURINGSTRUCT_H

#include <QList>
//添加-确定
struct ResourceDatas
{
    ResourceDatas(const std::list<int> m_ckmain, const std::list<int> m_ckstandy, const std::list<int> m_dsmain, const std::list<int> m_dsstandy,
                  const std::list<int> m_gsmain, const std::list<int> m_gsstandy, const std::list<int> m_cczfmain, const std::list<int> m_cczfstandy)
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

//工作模式结构体
struct str_WorkMode
{
    int STTC;    // S标准TTC 0-不包含 1-包含
    int SKuo2;   // S扩二 0-不包含 1-包含
    int KaKuo2;  // Ka扩二 0-不包含 1-包含
    int XLS;     // X低速数传  0-不包含 1-包含
    int KaLS;    // X/Ka低速数传 0-不包含 1-包含
    int KaHS;    // Ka高速数传 0-不包含 1-包含

    str_WorkMode(int sttc, int skuo2, int kakuo2, int xls, int kals, int kahs)
        : STTC(sttc)
        , SKuo2(skuo2)
        , KaKuo2(kakuo2)
        , XLS(xls)
        , KaLS(kals)
        , KaHS(kahs)
    {
    }

    str_WorkMode()
        : STTC(0)
        , SKuo2(0)
        , KaKuo2(0)
        , XLS(0)
        , KaLS(0)
        , KaHS(0)
    {
    }
};

struct ResourceBuild
{
    ResourceBuild(const std::string m_resourceBuildId, const std::string m_name, const std::string m_desc, const str_WorkMode m_workmode,
                  const int m_ismaster, const QByteArray m_other)
        : m_resourceBuildId(m_resourceBuildId)
        , m_name(m_name)
        , m_desc(m_desc)
        , m_workmode(m_workmode)
        , m_ismaster(m_ismaster)
        , m_other(m_other)
    {
    }
    std::string m_resourceBuildId;  //资源唯一ID
    std::string m_name;             //基带配置名称
    std::string m_desc;             //基带配置描述
    // QString m_workmode;         //工作模式
    // QMap<int, bool> m_workmode;  //工作模式
    str_WorkMode m_workmode;
    int m_ismaster;      //是否主备
    QByteArray m_other;  //基带配置信息
};

//删除    //用户登录
struct ResourceRebuildDelRequest
{
    ResourceRebuildDelRequest(const std::string m_resourceBuildId)
        : m_resourceBuildId(m_resourceBuildId)
    {
    }
    std::string m_resourceBuildId;
};
#endif  // RESOURCERESTRUCTURINGSTRUCT_H
