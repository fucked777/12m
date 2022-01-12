#ifndef ORBITPRODICTIONSTRUCT_H
#define ORBITPRODICTIONSTRUCT_H
#include <QDateTime>
#include <QString>

//开始预报
struct OrbitProdictionInfo
{
    OrbitProdictionInfo(const std::string m_id,
                        const std::string m_salellitename,
                        const int m_circlenumber,
                        const QDateTime m_intime,
                        const QDateTime m_outime,
                        const double m_fy_max,
                        const double m_in_fw,
                        const double m_in_fy,
                        const double m_out_fw,
                        const double m_out_fy)
        : m_id(m_id)
        , m_salellitename(m_salellitename)
        , m_circlenumber(m_circlenumber)
        , m_intime(m_intime)
        , m_outime(m_outime)
        , m_fy_max(m_fy_max)
        , m_in_fw(m_in_fw)
        , m_in_fy(m_in_fy)
        , m_out_fw(m_out_fw)
        , m_out_fy(m_out_fy)
    {
    }
    std::string m_id;             // ID
    std::string m_salellitename;  //卫星名
    int m_circlenumber;           //圈号
    QDateTime m_intime;           //进站时间
    QDateTime m_outime;           //出站时间
    double m_fy_max;              //最高俯仰（度）
    double m_in_fw;               //进站方位（度）
    double m_in_fy;               //进站俯仰（度）
    double m_out_fw;              //出站方位（度）
    double m_out_fy;              //出站俯仰（度）
};

#endif  // ORBITPRODICTIONSTRUCT_H
