#ifndef HPCMD_H
#define HPCMD_H

#include <QVariantMap>
/**
 * 主要处理功放发送命令，24和26都可以使用
 * @brief The HPCmd class
 */
class HPCmd
{
public:
    enum CommandType
    {
        UnitParameter,  //单元参数
        ProcessControl  //过程控制
    };

    static void Control(const int deviceId, const int modeId, const int cmdIdOrUnitId, QVariantMap params, CommandType type, QByteArray& data,
                        QWidget* widget);
};

#endif  // HPCMD_H
