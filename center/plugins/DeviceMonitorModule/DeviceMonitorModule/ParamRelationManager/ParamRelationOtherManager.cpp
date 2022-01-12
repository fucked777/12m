#include "ParamRelationOtherManager.h"
#include "ControlFactory.h"
#include <QDate>
#include <QGroupBox>
#include <QTime>
#include <QWidget>

bool ParamRelationOtherManager::checkParamRelationCkTtcTAS(const QMap<QString, QVariant>& setParamDataMap, QString& errorMsg)
{
    int encodeStyle = setParamDataMap.value("DecodMode").toInt();  // 编码方式
    int interleav = setParamDataMap.value("InterlDepth").toInt();  // 交错深度
    int frmLen = setParamDataMap.value("FrameLength").toInt();     // 帧长
    int synLen =
        setParamDataMap.value("FrameSynCodeLength").toInt() - 1;  // 帧同步码组长  这里的-1是因为设备上代码特殊处理过，为了保持数据一致，故我们这也-1
    int comRS = setParamDataMap.value("RSCodeType").toInt();  // R-S码类型

    double fzbFrequency = setParamDataMap.value("SubModSystem").toDouble();  //副载波频率
    int fzbFrequencyUnit = int(fzbFrequency * 1000);                         //这个是单位换算后的副载波频率
    int codeRate = setParamDataMap.value("CodeRate").toInt();                //码速率

    int vFill = -1;     // 虚拟填充
    int vFillTem = -1;  // 虚拟填充的
    // 有RS编码（R-S编码、R-S和卷积级联编码）
    if (encodeStyle == 3 || encodeStyle == 4)
    {
        int tempFrm = 0;  // 存放公式的计算结果
        tempFrm = 255 * interleav + (synLen + 2);
        if (tempFrm == frmLen)
        {
            vFill = 0;
            vFillTem = 0;
        }
        else
        {
            bool temFlag = true;
            bool temFlag2 = true;
            int vFillLen = tempFrm - frmLen;
            if (vFillLen < 0)
            {
                temFlag = false;
            }
            if (vFillLen >= 255 * interleav)  // 交错深度太大
            {
                temFlag = false;
            }

            if (vFillLen % interleav != 0)
            {
                temFlag2 = false;
            }

            if (temFlag2 == false)
            {
                errorMsg = QString("帧长，交错深度，RS码类型三者不匹配，请重新输入！(总帧长 - 子帧长 = "
                                   "交错深度*N)\n当R-S码打开的时候,您应设置一个正确的值,比如交错深度为5,帧同步码组长度设置为16位,帧长设置为167字");
                return false;
            }

            int m = 0;
            m = vFillLen / interleav;
            int frmLenMin;

            if (comRS == 1)
            {
                if ((m <= 0) || (m > 222))
                {
                    frmLenMin = tempFrm - 222 * interleav;
                    errorMsg = QString("此时帧长范围应该为：%1~%2，且应满足(总帧长 - 子帧长 = "
                                       "交错深度*N)\n当R-S码打开的时候,您应设置一个正确的值,比如交错深度为5,帧同步码组长度设置为16位,帧长设置为167字")
                                   .arg(frmLenMin)
                                   .arg(tempFrm);
                    return false;
                }
            }
            else  // comRS == 2
            {
                if ((m <= 0) || (m > 238))
                {
                    frmLenMin = tempFrm - 238 * interleav;
                    errorMsg = QString("此时帧长范围应该为: %1~%2，且应满足(总帧长 - 子帧长 = "
                                       "交错深度*N)\n当R-S码打开的时候,您应设置一个正确的值,比如交错深度为5,帧同步码组长度设置为16位,帧长设置为167字")
                                   .arg(frmLenMin)
                                   .arg(tempFrm);
                    return false;
                }
            }

            if (temFlag == false)
            {
                errorMsg = QString("帧长,交错深度，RS码类型三者不匹配，请重新输入!\n当R-S码打开的时候,您应设置一个正确的值,比如交错深度为5,"
                                   "帧同步码组长度设置为16位,帧长设置为167字");
                return false;
            }

            vFill = 1;
            vFillTem = 1;
        }
    }

    int subFrameSyn = setParamDataMap.value("SubframeSynch").toInt();         // 副帧同步方式
    int idNum = setParamDataMap.value("IDCountPosit").toInt();                // ID计数位置
    int subFramePosition = setParamDataMap.value("SubfraCodeLocat").toInt();  //副帧码组位置
    int subFrameLen = setParamDataMap.value("SubCodeLength").toInt();         //副帧码组长度
    int synLen2 = setParamDataMap.value("FrameSynCodeLength").toInt();        // 帧同步码组长
    int synFramePosi = setParamDataMap.value("FrameCodeLocat").toInt();       // 帧同步码组位置

    if (subFrameSyn == 2)  // ID副帧
    {
        if (synFramePosi == 1)  //帧头
        {
            int idNumMin = -1;
            int idNumMax = frmLen;  //最大值就为帧长
            if (synLen2 == 1)       // 16位
            {
                idNumMin = 3;  // id计数位置的最小值为2 + 1 = 3个字节
            }
            else if (synLen2 == 2)  // 24位
            {
                idNumMin = 4;  // id计数位置的最小值为3 + 1 = 4个字节
            }
            else if (synLen2 == 3)  // 32位
            {
                idNumMin = 5;  // id计数位置的最小值为4 + 1 = 5个字节
            }

            if (idNum < idNumMin || idNum > idNumMax)
            {
                errorMsg = QString("当副帧同步方式为ID副帧且帧同步码组位置为帧首时，ID字位置的范围为%1-%2，请重新输入").arg(idNumMin).arg(idNumMax);
                return false;
            }
        }
        else if (synFramePosi == 2)  //帧尾
        {
            int idNumMin = 1;  //最小值就为1
            int idNumMax;
            if (synLen2 == 1)  // 16位
            {
                idNumMax = frmLen - 2;  // id计数位置的最大值
            }
            else if (synLen2 == 2)  // 24位
            {
                idNumMax = frmLen - 3;  // id计数位置的最大值
            }
            else if (synLen2 == 3)  // 32位
            {
                idNumMax = frmLen - 4;  // id计数位置的最大值
            }

            if (idNum < idNumMin || idNum > idNumMax)
            {
                errorMsg =
                    QString("当副帧同步方式为ID副帧且帧同步码组位置为帧尾时，副帧码组位置位置的范围为%1-%2，请重新输入").arg(idNumMin).arg(idNumMax);
                return false;
            }
        }
    }
    else if (subFrameSyn == 3)  //循环副帧
    {
        if (synFramePosi == 1)  //帧头
        {
            int subNumMin = -1;
            int subNumMax = -1;
            if (subFrameLen == 1)  //副帧码组长度  8位
            {
                subNumMax = frmLen;
            }
            else if (subFrameLen == 2)  //副帧码组长度  16位
            {
                subNumMax = frmLen - 1;
            }
            else if (subFrameLen == 3)  //副帧码组长度  24位
            {
                subNumMax = frmLen - 2;
            }
            else if (subFrameLen == 4)  //副帧码组长度  32位
            {
                subNumMax = frmLen - 3;
            }

            if (synLen2 == 1)  // 16位
            {
                subNumMin = 3;  // 副帧码组位置的最小值为2 + 1 = 3个字节
            }
            else if (synLen2 == 2)  // 24位
            {
                subNumMin = 4;  // 副帧码组位置的最小值为3 + 1 = 4个字节
            }
            else if (synLen2 == 3)  // 32位
            {
                subNumMin = 5;  // 副帧码组位置的最小值为4 + 1 = 5个字节
            }

            if (subFramePosition < subNumMin || subFramePosition > subNumMax)
            {
                errorMsg = QString("当副帧同步方式为循环副帧且帧同步码组位置为帧首时，副帧码组位置位置的范围为%1-%2，请重新输入")
                               .arg(subNumMin)
                               .arg(subNumMax);
                return false;
            }
        }
        else if (synFramePosi == 2)  //帧尾
        {
            int subNumMin = 1;
            int subNumMax;

            if (synLen2 == 1)  // 16位
            {
                if (subFrameLen == 1)  //副帧码组长度  8位
                {
                    subNumMax = frmLen - 2;  // 副帧码组位置的最大值
                }
                else if (subFrameLen == 2)  //副帧码组长度  16位
                {
                    subNumMax = frmLen - 3;
                }
                else if (subFrameLen == 3)  //副帧码组长度  24位
                {
                    subNumMax = frmLen - 4;
                }
                else if (subFrameLen == 4)  //副帧码组长度  32位
                {
                    subNumMax = frmLen - 5;
                }
            }
            else if (synLen2 == 2)  // 24位
            {
                if (subFrameLen == 1)  //副帧码组长度  8位
                {
                    subNumMax = frmLen - 3;  // 副帧码组位置的最大值
                }
                else if (subFrameLen == 2)  //副帧码组长度  16位
                {
                    subNumMax = frmLen - 4;
                }
                else if (subFrameLen == 3)  //副帧码组长度  24位
                {
                    subNumMax = frmLen - 5;
                }
                else if (subFrameLen == 4)  //副帧码组长度  32位
                {
                    subNumMax = frmLen - 6;
                }
            }
            else if (synLen2 == 3)  // 32位
            {
                if (subFrameLen == 1)  //副帧码组长度  8位
                {
                    subNumMax = frmLen - 4;  // 副帧码组位置的最大值
                }
                else if (subFrameLen == 2)  //副帧码组长度  16位
                {
                    subNumMax = frmLen - 5;
                }
                else if (subFrameLen == 3)  //副帧码组长度  24位
                {
                    subNumMax = frmLen - 6;
                }
                else if (subFrameLen == 4)  //副帧码组长度  32位
                {
                    subNumMax = frmLen - 7;
                }
            }

            if (subFramePosition < subNumMin || subFramePosition > subNumMax)
            {
                errorMsg =
                    QString("当副帧同步方式为循环副帧且帧同步码组位置为帧尾时，副帧码组位置的范围为%1-%2，请重新输入").arg(subNumMin).arg(subNumMax);
                return false;
            }
        }
    }

    if (frmLen < idNum)
    {
        errorMsg = QString("ID字位置不可超过帧长");
        return false;
    }

    int wordLength = setParamDataMap.value("WordLength").toInt();        // 字长
    int formatLength = setParamDataMap.value("FormatLength").toInt();    // 格式(副帧)长
    int subframeSynch = setParamDataMap.value("SubframeSynch").toInt();  // 副帧同步方式

    if (subframeSynch == 2)  // 副帧同步方式为ID副帧
    {
        int maxValue = pow(2, wordLength);

        if (formatLength > maxValue)
        {
            errorMsg = QString("ID副帧时，格式(副帧)长不能超过2^N（N为字长）");
            return false;
        }
    }

    int result = fzbFrequencyUnit % codeRate;
    if (result == 0)
    {
        int result2 = fzbFrequencyUnit / codeRate;
        //        if (result2 >= 2 && result2 <= 512)
        //        {
        //            return true;
        //        }
        //        else
        if (result2 < 2 || result2 > 512)
        {
            errorMsg =
                QString("副载波输入值%1kHz不是码率输入值%2bps的整数倍，且整除后的值必须在2到512之间，请重新输入").arg(fzbFrequency).arg(codeRate);
            return false;
        }
    }
    else if (result != 0)
    {
        errorMsg = QString("副载波输入值%1kHz不是码率输入值%2bps的整数倍，且整除后的值必须在2到512之间，请重新输入").arg(fzbFrequency).arg(codeRate);
        return false;
    }

    return true;
}

bool ParamRelationOtherManager::checkParamRelationCkTtcTU(const QMap<QString, QVariant>& setParamData, QString& errorMsg)
{
    int encodeStyle = setParamData.value("DecodMode").toInt();  // 编码方式
    int interleav = setParamData.value("InterlDepth").toInt();  // 交错深度
    int frmLen = setParamData.value("FrameLength").toInt();     // 帧长
    int synLen =
        setParamData.value("FrameSynCodeLength").toInt() - 1;  // 帧同步码组长  这里的-1是因为设备上代码特殊处理过，为了保持数据一致，故我们这也-1
    int comRS = setParamData.value("RSCodeType").toInt();  // R-S码类型

    double fzbFrequency = setParamData.value("SubModSystem").toDouble();  //副载波频率
    int fzbFrequencyUnit = int(fzbFrequency * 1000);                      //这个是单位换算后的副载波频率
    int codeRate = setParamData.value("CodeRate").toInt();                //码速率

    int vFill = -1;     // 虚拟填充
    int vFillTem = -1;  // 虚拟填充的
    // 有RS编码（R-S编码、R-S和卷积级联编码）
    if (encodeStyle == 3 || encodeStyle == 4)
    {
        int tempFrm = 0;  // 存放公式的计算结果
        tempFrm = 255 * interleav + (synLen + 2);
        if (tempFrm == frmLen)
        {
            vFill = 0;
            vFillTem = 0;
        }
        else
        {
            bool temFlag = true;
            bool temFlag2 = true;
            int vFillLen = tempFrm - frmLen;
            if (vFillLen < 0)
            {
                temFlag = false;
            }
            if (vFillLen >= 255 * interleav)  // 交错深度太大
            {
                temFlag = false;
            }

            if (vFillLen % interleav != 0)
            {
                temFlag2 = false;
            }

            if (temFlag2 == false)
            {
                errorMsg = QString("帧长，交错深度，RS码类型三者不匹配，请重新输入！(总帧长 - 子帧长 = "
                                   "交错深度*N)\n当R-S码打开的时候,您应设置一个正确的值,比如交错深度为5,帧同步码组长度设置为16位,帧长设置为167字");
                return false;
            }

            int m = 0;
            m = vFillLen / interleav;
            int frmLenMin;

            if (comRS == 1)
            {
                if ((m <= 0) || (m > 222))
                {
                    frmLenMin = tempFrm - 222 * interleav;
                    errorMsg = QString("此时帧长范围应该为：%1~%2，且应满足(总帧长 - 子帧长 = "
                                       "交错深度*N)\n当R-S码打开的时候,您应设置一个正确的值,比如交错深度为5,帧同步码组长度设置为16位,帧长设置为167字")
                                   .arg(frmLenMin)
                                   .arg(tempFrm);
                    return false;
                }
            }
            else  // comRS == 2
            {
                if ((m <= 0) || (m > 238))
                {
                    frmLenMin = tempFrm - 238 * interleav;
                    errorMsg = QString("此时帧长范围应该为: %1~%2，且应满足(总帧长 - 子帧长 = "
                                       "交错深度*N)\n当R-S码打开的时候,您应设置一个正确的值,比如交错深度为5,帧同步码组长度设置为16位,帧长设置为167字")
                                   .arg(frmLenMin)
                                   .arg(tempFrm);
                    return false;
                }
            }

            if (temFlag == false)
            {
                errorMsg = QString("帧长,交错深度，RS码类型三者不匹配，请重新输入!\n当R-S码打开的时候,您应设置一个正确的值,比如交错深度为5,"
                                   "帧同步码组长度设置为16位,帧长设置为167字");
                return false;
            }

            vFill = 1;
            vFillTem = 1;
        }
    }

    int subFrameSyn = setParamData.value("SubframeSynch").toInt();         // 副帧同步方式
    int idNum = setParamData.value("IDCountPosit").toInt();                // ID计数位置
    int subFramePosition = setParamData.value("SubfraCodeLocat").toInt();  //副帧码组位置
    int subFrameLen = setParamData.value("SubCodeLength").toInt();         //副帧码组长度
    int synLen2 = setParamData.value("FrameSynCodeLength").toInt();        // 帧同步码组长
    int synFramePosi = setParamData.value("FrameCodeLocat").toInt();       // 帧同步码组位置

    if (subFrameSyn == 2)  // ID副帧
    {
        if (synFramePosi == 1)  //帧头
        {
            int idNumMin = -1;
            int idNumMax = frmLen;  //最大值就为帧长
            if (synLen2 == 1)       // 16位
            {
                idNumMin = 3;  // id计数位置的最小值为2 + 1 = 3个字节
            }
            else if (synLen2 == 2)  // 24位
            {
                idNumMin = 4;  // id计数位置的最小值为3 + 1 = 4个字节
            }
            else if (synLen2 == 3)  // 32位
            {
                idNumMin = 5;  // id计数位置的最小值为4 + 1 = 5个字节
            }

            if (idNum < idNumMin || idNum > idNumMax)
            {
                errorMsg = QString("当副帧同步方式为ID副帧且帧同步码组位置为帧首时，ID字位置的范围为%1-%2，请重新输入").arg(idNumMin).arg(idNumMax);
                return false;
            }
        }
        else if (synFramePosi == 2)  //帧尾
        {
            int idNumMin = 1;  //最小值就为1
            int idNumMax;
            if (synLen2 == 1)  // 16位
            {
                idNumMax = frmLen - 2;  // id计数位置的最大值
            }
            else if (synLen2 == 2)  // 24位
            {
                idNumMax = frmLen - 3;  // id计数位置的最大值
            }
            else if (synLen2 == 3)  // 32位
            {
                idNumMax = frmLen - 4;  // id计数位置的最大值
            }

            if (idNum < idNumMin || idNum > idNumMax)
            {
                errorMsg =
                    QString("当副帧同步方式为ID副帧且帧同步码组位置为帧尾时，副帧码组位置位置的范围为%1-%2，请重新输入").arg(idNumMin).arg(idNumMax);
                return false;
            }
        }
    }
    else if (subFrameSyn == 3)  //循环副帧
    {
        if (synFramePosi == 1)  //帧头
        {
            int subNumMin = -1;
            int subNumMax = -1;
            if (subFrameLen == 1)  //副帧码组长度  8位
            {
                subNumMax = frmLen;
            }
            else if (subFrameLen == 2)  //副帧码组长度  16位
            {
                subNumMax = frmLen - 1;
            }
            else if (subFrameLen == 3)  //副帧码组长度  24位
            {
                subNumMax = frmLen - 2;
            }
            else if (subFrameLen == 4)  //副帧码组长度  32位
            {
                subNumMax = frmLen - 3;
            }

            if (synLen2 == 1)  // 16位
            {
                subNumMin = 3;  // 副帧码组位置的最小值为2 + 1 = 3个字节
            }
            else if (synLen2 == 2)  // 24位
            {
                subNumMin = 4;  // 副帧码组位置的最小值为3 + 1 = 4个字节
            }
            else if (synLen2 == 3)  // 32位
            {
                subNumMin = 5;  // 副帧码组位置的最小值为4 + 1 = 5个字节
            }

            if (subFramePosition < subNumMin || subFramePosition > subNumMax)
            {
                errorMsg = QString("当副帧同步方式为循环副帧且帧同步码组位置为帧首时，副帧码组位置位置的范围为%1-%2，请重新输入")
                               .arg(subNumMin)
                               .arg(subNumMax);
                return false;
            }
        }
        else if (synFramePosi == 2)  //帧尾
        {
            int subNumMin = 1;
            int subNumMax;

            if (synLen2 == 1)  // 16位
            {
                if (subFrameLen == 1)  //副帧码组长度  8位
                {
                    subNumMax = frmLen - 2;  // 副帧码组位置的最大值
                }
                else if (subFrameLen == 2)  //副帧码组长度  16位
                {
                    subNumMax = frmLen - 3;
                }
                else if (subFrameLen == 3)  //副帧码组长度  24位
                {
                    subNumMax = frmLen - 4;
                }
                else if (subFrameLen == 4)  //副帧码组长度  32位
                {
                    subNumMax = frmLen - 5;
                }
            }
            else if (synLen2 == 2)  // 24位
            {
                if (subFrameLen == 1)  //副帧码组长度  8位
                {
                    subNumMax = frmLen - 3;  // 副帧码组位置的最大值
                }
                else if (subFrameLen == 2)  //副帧码组长度  16位
                {
                    subNumMax = frmLen - 4;
                }
                else if (subFrameLen == 3)  //副帧码组长度  24位
                {
                    subNumMax = frmLen - 5;
                }
                else if (subFrameLen == 4)  //副帧码组长度  32位
                {
                    subNumMax = frmLen - 6;
                }
            }
            else if (synLen2 == 3)  // 32位
            {
                if (subFrameLen == 1)  //副帧码组长度  8位
                {
                    subNumMax = frmLen - 4;  // 副帧码组位置的最大值
                }
                else if (subFrameLen == 2)  //副帧码组长度  16位
                {
                    subNumMax = frmLen - 5;
                }
                else if (subFrameLen == 3)  //副帧码组长度  24位
                {
                    subNumMax = frmLen - 6;
                }
                else if (subFrameLen == 4)  //副帧码组长度  32位
                {
                    subNumMax = frmLen - 7;
                }
            }

            if (subFramePosition < subNumMin || subFramePosition > subNumMax)
            {
                errorMsg =
                    QString("当副帧同步方式为循环副帧且帧同步码组位置为帧尾时，副帧码组位置的范围为%1-%2，请重新输入").arg(subNumMin).arg(subNumMax);
                return false;
            }
        }
    }

    if (frmLen < idNum)
    {
        errorMsg = QString("ID字位置不可超过帧长");
        return false;
    }

    int wordLength = setParamData.value("WordLength").toInt();        // 字长
    int formatLength = setParamData.value("FormatLength").toInt();    // 格式(副帧)长
    int subframeSynch = setParamData.value("SubframeSynch").toInt();  // 副帧同步方式

    if (subframeSynch == 2)  // 副帧同步方式为ID副帧
    {
        int maxValue = pow(2, wordLength);

        if (formatLength > maxValue)
        {
            errorMsg = QString("ID副帧时，格式(副帧)长不能超过2^N（N为字长）");
            return false;
        }
    }

    int result = fzbFrequencyUnit % codeRate;
    if (result == 0)
    {
        int result2 = fzbFrequencyUnit / codeRate;
        if (result2 >= 2 && result2 <= 512)
        {
            return true;
        }
        else
        {
            errorMsg =
                QString("副载波输入值%1kHz不是码率输入值%2bps的整数倍，且整除后的值必须在2到512之间，请重新输入").arg(fzbFrequency).arg(codeRate);
            return false;
        }
    }
    else if (result != 0)
    {
        errorMsg = QString("副载波输入值%1kHz不是码率输入值%2bps的整数倍，且整除后的值必须在2到512之间，请重新输入").arg(fzbFrequency).arg(codeRate);
        return false;
    }

    return true;
}

bool ParamRelationOtherManager::checkParamRelationCkTtcRCU(const QMap<QString, QVariant>& setParamData, QString& errorMsg)
{
    int codeRate = setParamData.value("CodeRate").toInt();            //码速率
    int remoteSubFreq = setParamData.value("RemoteSubFreq").toInt();  //副载波0频
    int ChannelCoding = setParamData.value("ChannelCoding").toInt();  //信道编码
    int tztz = setParamData.value("SubModSystemR").toInt();           //调制体制
    int SettBefafter = setParamData.value("SettBefafter").toInt();    // 码速率控制
    int convCodType = setParamData.value("ConvCodType").toInt();      // 卷积编码类型 7,1/2  7,3/4

    if (SettBefafter == 1)  //编码前
    {
        int minValue = 100;
        int maxValue = 8000;

        if (codeRate < minValue || codeRate > maxValue)
        {
            errorMsg = QString("在码速率控制为卷积编码前时，码速率范围为%1-%2bps,请重新输入！").arg(minValue).arg(maxValue);
            return false;
        }
    }
    else if (SettBefafter == 2)  //编码后
    {
        if (convCodType == 1)  // 7,1/2
        {
            int minValue = 200;
            int maxValue = 16000;

            if (codeRate < minValue || codeRate > maxValue)
            {
                errorMsg =
                    QString("在码速率控制为卷积编码后，卷积编码类型为(7,1/2)时，码速率范围为%1-%2bps,请重新输入！").arg(minValue).arg(maxValue);
                return false;
            }
        }
        else if (convCodType == 2)  // 7,3/4
        {
            int minValue = 134;
            int maxValue = 10666;

            if (codeRate < minValue || codeRate > maxValue)
            {
                errorMsg =
                    QString("在码速率控制为卷积编码后，卷积编码类型为(7,3/4)时，码速率范围为%1-%2bps,请重新输入！").arg(minValue).arg(maxValue);
                return false;
            }
        }
    }

    if (tztz == 1)  //调制体制为PSK的时候才会有这个算式
    {
        int result = remoteSubFreq % codeRate;
        if (ChannelCoding == 3 || ChannelCoding == 4)
        {
            if (result == 0)
            {
                int result2 = remoteSubFreq / codeRate;
                if ((result2 >= 4 && result2 <= 1024) && SettBefafter == 2)
                {
                    return true;
                }
                else
                {
                    errorMsg = QString("在信道编码为卷积编码或者BCH+"
                                       "卷积级联编码时，码速率控制必须为编码后,且调制频率必须能够被码率整除，整除的值需在4与1024之前，请重新输入!");
                    return false;
                }
            }
            else
            {
                errorMsg = QString("调制频率必须能够被码率整除，且整除的值需在4与1024之前，请重新输入!");
                return false;
            }
        }
        else
        {
            if (result == 0)
            {
                int result2 = remoteSubFreq / codeRate;
                if (result2 >= 4 && result2 <= 1024)
                {
                    return true;
                }
                else
                {
                    errorMsg = QString("调制频率必须能够被码率整除，且调制频率除以码率后的值在4与1024之前，请重新输入!");
                    return false;
                }
            }
            else
            {
                errorMsg = QString("调制频率必须能够被码率整除，且调制频率除以码率后的值在4与1024之前，请重新输入!");
                return false;
            }
        }
    }

    return true;
}

bool ParamRelationOtherManager::checkParamRelationGSRDU(const QMap<QString, QVariant>& setParamData, QString& errorMsg)
{
    auto codeWay = setParamData.value("DecodMode").toInt();      //译码方式
    int interleav = setParamData.value("RSInterDecod").toInt();  // 交错深度
    int frmLen = setParamData.value("FrameLength").toInt();      // 帧长
    int synLen = setParamData.value("FraSynchLength").toInt();   // 帧同步码组长
    int comRS = setParamData.value("RSCodeType").toInt();        // R-S码类型

    // 有RS编码
    if (codeWay == 3)
    {
        int tempFrm = 0;  // 存放公式的计算结果
        tempFrm = 255 * interleav + (synLen + 7) / 8;
        int frmLenMin = 0;
        if (comRS == 1)
        {
            frmLenMin = tempFrm - 222 * interleav;
        }
        else
        {
            frmLenMin = tempFrm - 238 * interleav;
        }

        if ((frmLen > tempFrm) || (frmLen < frmLenMin))
        {
            errorMsg = QString("有R-S译码时帧长超出范围：%1-%2，且应满足(总帧长 - 子帧长 = 交错深度*N)，请重新输入！").arg(frmLenMin).arg(tempFrm);
            return false;
        }
        else
        {
            if (((tempFrm - frmLen) % interleav) != 0)
            {
                errorMsg = QString("有R-S译码时，帧长，交错深度，RS码类型三者不匹配，请重新输入！(总帧长 - 子帧长 = 交错深度*N)");
                return false;
            }
        }
    }

    return true;
}

bool ParamRelationOtherManager::checkParamRelationGSMNY(const QMap<QString, QVariant>& setParamData, QString& errorMsg)
{
    auto tztz = setParamData.value("ModulatSystem").toInt();  //调制体制
    int dataType = setParamData.value("FrameSynch").toInt();  // IQ路选择
    // I路
    int codeWayI = setParamData.value("I_DecodMode").toInt();        //译码方式
    int frmLenI = setParamData.value("I_FrameLength").toInt();       // I路帧长
    int interleavI = setParamData.value("I_RSInterDecod").toInt();   // 交错深度
    int frameLenI = setParamData.value("I_FraSynchLength").toInt();  //帧同步码组长
    int comRSI = setParamData.value("I_RSCodeType").toInt();         // R-S码类型
    // Q路
    int codeWayQ = setParamData.value("Q_DecodMode").toInt();        //译码方式
    int frmLenQ = setParamData.value("Q_FrameLength").toInt();       // Q路帧长
    int interleavQ = setParamData.value("Q_RSInterDecod").toInt();   // 交错深度
    int frameLenQ = setParamData.value("Q_FraSynchLength").toInt();  //帧同步码组长
    int comRSQ = setParamData.value("Q_RSCodeType").toInt();         // R-S码类型
    if (tztz == 1 || tztz == 6 || tztz == 7)                         // BPSK 16QAM 16APSK 只有I路
    {
        if (codeWayI == 3)  // R-S
        {
            int tempFrm = 0;  // 存放公式的计算结果
            tempFrm = 255 * interleavI + (frameLenI + 7) / 8;
            int frmLenMin = 0;
            if (comRSI == 1)
            {
                frmLenMin = tempFrm - 222 * interleavI;
            }
            else
            {
                frmLenMin = tempFrm - 238 * interleavI;
            }

            if ((frmLenI > tempFrm) || (frmLenI < frmLenMin))
            {
                errorMsg =
                    QString("I路：有R-S译码时帧长超出范围：%1-%2，且应满足(总帧长 - 子帧长 = 交错深度*N)，请重新输入！").arg(frmLenMin).arg(tempFrm);
                return false;
            }
            else
            {
                if (((tempFrm - frmLenI) % interleavI) != 0)
                {
                    errorMsg = QString("I路：有R-S译码时，帧长，交错深度，RS码类型三者不匹配，请重新输入！(总帧长 - 子帧长 = 交错深度*N)");
                    return false;
                }
            }
        }
    }
    else if (tztz == 2 || tztz == 4 || tztz == 5)  // QPSK SQPSK 8PSK
    {
        if (dataType == 1)  // IQ合路
        {
            if (codeWayI == 3)  // R-S
            {
                int tempFrm = 0;  // 存放公式的计算结果
                tempFrm = 255 * interleavI + (frameLenI + 7) / 8;
                int frmLenMin = 0;
                if (comRSI == 1)
                {
                    frmLenMin = tempFrm - 222 * interleavI;
                }
                else
                {
                    frmLenMin = tempFrm - 238 * interleavI;
                }

                if ((frmLenI > tempFrm) || (frmLenI < frmLenMin))
                {
                    errorMsg = QString("I路：有R-S译码时帧长超出范围：%1-%2，且应满足(总帧长 - 子帧长 = 交错深度*N)，请重新输入！")
                                   .arg(frmLenMin)
                                   .arg(tempFrm);
                    return false;
                }
                else
                {
                    if (((tempFrm - frmLenI) % interleavI) != 0)
                    {
                        errorMsg = QString("I路：有R-S译码时，帧长，交错深度，RS码类型三者不匹配，请重新输入！(总帧长 - 子帧长 = 交错深度*N)");
                        return false;
                    }
                }
            }
        }
        else if (dataType == 2)  // IQ分路
        {
            if (codeWayI == 3)  // R-S
            {
                int tempFrm = 0;  // 存放公式的计算结果
                tempFrm = 255 * interleavI + (frameLenI + 7) / 8;
                int frmLenMin = 0;
                if (comRSI == 1)
                {
                    frmLenMin = tempFrm - 222 * interleavI;
                }
                else
                {
                    frmLenMin = tempFrm - 238 * interleavI;
                }

                if ((frmLenI > tempFrm) || (frmLenI < frmLenMin))
                {
                    errorMsg = QString("I路：有R-S译码时帧长超出范围：%1-%2，且应满足(总帧长 - 子帧长 = 交错深度*N)，请重新输入！")
                                   .arg(frmLenMin)
                                   .arg(tempFrm);
                    return false;
                }
                else
                {
                    if (((tempFrm - frmLenI) % interleavI) != 0)
                    {
                        errorMsg = QString("I路：有R-S译码时，帧长，交错深度，RS码类型三者不匹配，请重新输入！(总帧长 - 子帧长 = 交错深度*N)");
                        return false;
                    }
                }
            }

            if (codeWayQ == 3)  // R-S
            {
                int tempFrm = 0;  // 存放公式的计算结果
                tempFrm = 255 * interleavQ + (frameLenQ + 7) / 8;
                int frmLenMin = 0;
                if (comRSQ == 1)
                {
                    frmLenMin = tempFrm - 222 * interleavQ;
                }
                else
                {
                    frmLenMin = tempFrm - 238 * interleavQ;
                }

                if ((frmLenQ > tempFrm) || (frmLenQ < frmLenMin))
                {
                    errorMsg = QString("Q路：有R-S译码时帧长超出范围：%1-%2，且应满足(总帧长 - 子帧长 = 交错深度*N)，请重新输入！")
                                   .arg(frmLenMin)
                                   .arg(tempFrm);
                    return false;
                }
                else
                {
                    if (((tempFrm - frmLenQ) % interleavQ) != 0)
                    {
                        errorMsg = QString("Q路：有R-S译码时，帧长，交错深度，RS码类型三者不匹配，请重新输入！(总帧长 - 子帧长 = 交错深度*N)");
                        return false;
                    }
                }
            }
        }
    }
    else if (tztz == 3)  // UQPSK时必为IQ分路
    {
        if (codeWayI == 3)  // R-S
        {
            int tempFrm = 0;  // 存放公式的计算结果
            tempFrm = 255 * interleavI + (frameLenI + 7) / 8;
            int frmLenMin = 0;
            if (comRSI == 1)
            {
                frmLenMin = tempFrm - 222 * interleavI;
            }
            else
            {
                frmLenMin = tempFrm - 238 * interleavI;
            }

            if ((frmLenI > tempFrm) || (frmLenI < frmLenMin))
            {
                errorMsg =
                    QString("I路：有R-S译码时帧长超出范围：%1-%2，且应满足(总帧长 - 子帧长 = 交错深度*N)，请重新输入！").arg(frmLenMin).arg(tempFrm);
                return false;
            }
            else
            {
                if (((tempFrm - frmLenI) % interleavI) != 0)
                {
                    errorMsg = QString("I路：有R-S译码时，帧长，交错深度，RS码类型三者不匹配，请重新输入！(总帧长 - 子帧长 = 交错深度*N)");
                    return false;
                }
            }
        }

        if (codeWayQ == 3)  // R-S
        {
            int tempFrm = 0;  // 存放公式的计算结果
            tempFrm = 255 * interleavQ + (frameLenQ + 7) / 8;
            int frmLenMin = 0;
            if (comRSQ == 1)
            {
                frmLenMin = tempFrm - 222 * interleavQ;
            }
            else
            {
                frmLenMin = tempFrm - 238 * interleavQ;
            }

            if ((frmLenQ > tempFrm) || (frmLenQ < frmLenMin))
            {
                errorMsg =
                    QString("Q路：有R-S译码时帧长超出范围：%1-%2，且应满足(总帧长 - 子帧长 = 交错深度*N)，请重新输入！").arg(frmLenMin).arg(tempFrm);
                return false;
            }
            else
            {
                if (((tempFrm - frmLenQ) % interleavQ) != 0)
                {
                    errorMsg = QString("Q路：有R-S译码时，帧长，交错深度，RS码类型三者不匹配，请重新输入！(总帧长 - 子帧长 = 交错深度*N)");
                    return false;
                }
            }
        }
    }

    return true;
}

bool ParamRelationOtherManager::checkParamRelationDSJT(QMap<QString, QVariant>& setParamData, QString& errorMsg)
{
    int tztz = setParamData.value("tztz").toInt();  //调制体制
                                                    //    int dataType = setParamData.value("dataType").toInt();            // IQ路选择
                                                    //    int bcZhfas = setParamData.value("bcZhfas").toInt();              // 并串转换方式
                                                    //    int frmLenI = setParamData.value("frmLenI").toInt();              // I路帧长
                                                    //    int frmLenQ = setParamData.value("frmLenQ").toInt();              // Q路帧长
                                                    //    double powerRatioIQ = setParamData.value("powerIQ").toDouble();   // IQ功率比
                                                    //    double codeSpeedI = setParamData.value("codeSpeedI").toDouble();  // IQ功率比
                                                    //    double codeSpeedQ = setParamData.value("codeSpeedQ").toDouble();  // IQ功率比

    auto codeWay = setParamData.value("CodeWay").toInt();         //译码方式
    int interleav = setParamData.value("InterlaceDeep").toInt();  // 交错深度
    int frmLen = setParamData.value("FrameLenght").toInt();       // 帧长
    int synLen = setParamData.value("IWayGroupLen").toInt();      // 帧同步码组长
    int comRS = setParamData.value("RSDecoding").toInt();         // R-S码类型
    //保证帧同步码组长必须为8的倍数
    if (synLen % 8)
    {
        errorMsg = QString("帧同步码组长必须为8的整数倍");
        return false;
    }

    // 有RS编码（R-S编码、R-S和卷积级联编码）
    if (codeWay == 3 || codeWay == 4)
    {
        int tempFrm = 0;  // 存放公式的计算结果
        tempFrm = 255 * interleav + (synLen + 7) / 8;
        int frmLenMin = 0;
        if (comRS == 1)
        {
            frmLenMin = tempFrm - 222 * interleav;
        }
        else
        {
            frmLenMin = tempFrm - 238 * interleav;
        }

        if ((frmLen > tempFrm) || (frmLen < frmLenMin))
        {
            errorMsg = QString("有R-S译码时帧长超出范围：%1-%2，且应满足(总帧长 - 子帧长 = 交错深度*N)，请重新输入！").arg(frmLenMin).arg(tempFrm);
            return false;
        }
        else
        {
            if (((tempFrm - frmLen) % interleav) != 0)
            {
                errorMsg = QString("有R-S译码时，帧长，交错深度，RS码类型三者不匹配，请重新输入！(总帧长 - 子帧长 = 交错深度*N)");
                return false;
            }
        }
    }

    auto codeType = setParamData.value("CodeType").toInt();              //码型
    auto viterbiCode = setParamData.value("ViterbiDecoding").toInt();    // Viterbi译码
    auto ldpcCode = setParamData.value("LDPCCodeRate").toInt();          // Ldpc码率
    auto turboCode = setParamData.value("TurboCodeRate").toInt();        // Turbo码率
    auto codeSpeed = setParamData.value("CodeSpeed").toDouble();         //码速率
    auto codeSpeedType = setParamData.value("CodeRateSett").toDouble();  //码速率设置方式

    int initMinValue;  //这个是译码方式为关各个调制体制的初始最小值
    int initMaxValue;  //这个是译码方式为关各个调制体制的初始最大值
    double bValue;
    QString cValue;

    if (tztz == 1)  // BPSK
    {
        initMinValue = 1000;
        initMaxValue = 15000000;
    }
    else if (tztz == 2 || tztz == 5)  // QPSK OQPSK
    {
        initMinValue = 20000;
        initMaxValue = 30000000;

        //        if (dataType == 1 && bcZhfas == 1)  // IQ分路  按比特
        //        {
        //            if ((frmLen != frmLenI) || (frmLen != frmLenQ))
        //            {
        //                errorMsg = QString("当为IQ分路且并串转换方式为按比特的时候，IQ路帧长必须相等");
        //                return false;
        //            }
        //        }
    }
    else if (tztz == 4)  // UQPSK时必为IQ分路
    {
        initMinValue = 10000;
        initMaxValue = 15000000;

        //        if ((frmLen != frmLenI) || (frmLen != frmLenQ))
        //        {
        //            errorMsg = QString("当为IQ分路且并串转换方式为按比特的时候，IQ路帧长必须相等");
        //            return false;
        //        }

        //        auto value = 10 * log10(codeSpeedI / codeSpeedQ);

        //        if (powerRatioIQ != value)
        //        {
        //            errorMsg = QString("当Ka/X中频接收部分调制体制为UQPSK时，【10lg(I/Q路码率的比值)】必须与【I/Q功率分贝比】相等，请重新输入!");
        //            return false;
        //        }
    }

    if (codeType == 1 || codeType == 2 || codeType == 3)  // NRZ-L、M、S
    {
        bValue = 1;
    }
    else if (codeType == 4 || codeType == 5 || codeType == 6)  // Biφ-L、M、S
    {
        bValue = 0.5;
    }

    if (codeSpeedType == 1)  //编码前
    {
        if (codeWay == 2 || codeWay == 4)  // Viterbi或者R-S和Viterbi级联译码
        {
            if (viterbiCode == 1)  //(7,(1/2))
            {
                cValue = QString("1/2");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

                if (codeSpeed < minValue || codeSpeed > maxValue)
                {
                    errorMsg = QString("当前码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
            else if (viterbiCode == 2)
            {
                cValue = QString("2/3");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

                if (codeSpeed < minValue || codeSpeed > maxValue)
                {
                    errorMsg = QString("当前码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
            else if (viterbiCode == 3)
            {
                cValue = QString("3/4");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

                if (codeSpeed < minValue || codeSpeed > maxValue)
                {
                    errorMsg = QString("当前码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
            else if (viterbiCode == 4)
            {
                cValue = QString("5/6");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

                if (codeSpeed < minValue || codeSpeed > maxValue)
                {
                    errorMsg = QString("当前码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
            else if (viterbiCode == 5)
            {
                cValue = QString("7/8");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

                if (codeSpeed < minValue || codeSpeed > maxValue)
                {
                    errorMsg = QString("当前码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
        }
        else if (codeWay == 5)  // LDPC
        {
            if (ldpcCode == 1 || ldpcCode == 2)  // 1/2
            {
                cValue = QString("1/2");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

                if (codeSpeed < minValue || codeSpeed > maxValue)
                {
                    errorMsg = QString("当前码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
            else if (ldpcCode == 3 || ldpcCode == 4)  // 2/3
            {
                cValue = QString("2/3");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

                if (codeSpeed < minValue || codeSpeed > maxValue)
                {
                    errorMsg = QString("当前码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
            else if (ldpcCode == 5 || ldpcCode == 6)  // 4/5
            {
                cValue = QString("4/5");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

                if (codeSpeed < minValue || codeSpeed > maxValue)
                {
                    errorMsg = QString("当前码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
            if (ldpcCode == 7)  // 7/8
            {
                cValue = QString("7/8");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

                if (codeSpeed < minValue || codeSpeed > maxValue)
                {
                    errorMsg = QString("当前码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
        }
        else if (codeWay == 6)  // Turbo
        {
            if (turboCode == 1 || turboCode == 2 || turboCode == 3 || turboCode == 4)  // 1/2
            {
                cValue = QString("1/2");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

                if (codeSpeed < minValue || codeSpeed > maxValue)
                {
                    errorMsg = QString("当前码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
            else if (turboCode == 5 || turboCode == 6 || turboCode == 7 || turboCode == 8)  // 1/3
            {
                cValue = QString("1/3");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

                if (codeSpeed < minValue || codeSpeed > maxValue)
                {
                    errorMsg = QString("当前码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
            else if (turboCode == 9 || turboCode == 10 || turboCode == 11 || turboCode == 12)  // 1/4
            {
                cValue = QString("1/4");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

                if (codeSpeed < minValue || codeSpeed > maxValue)
                {
                    errorMsg = QString("当前码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
            if (turboCode == 13 || turboCode == 14 || turboCode == 15 || turboCode == 16)  // 1/6
            {
                cValue = QString("1/6");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

                if (codeSpeed < minValue || codeSpeed > maxValue)
                {
                    errorMsg = QString("当前码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
        }
        else if (codeWay == 1 || codeWay == 3)
        {
            cValue = QString("1/1");
            int minValue;
            int maxValue;

            calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

            if (codeSpeed < minValue || codeSpeed > maxValue)
            {
                errorMsg = QString("当前码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                return false;
            }
        }
    }
    else if (codeSpeedType == 2)  //编码后  码速率值不再受到译码方式参数的影响
    {
        cValue = QString("1/1");
        int minValue;
        int maxValue;

        calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

        if (codeSpeed < minValue || codeSpeed > maxValue)
        {
            errorMsg = QString("当前码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
            return false;
        }
    }

    return true;
}

bool ParamRelationOtherManager::checkParamRelationDSMNY(const QMap<QString, QVariant>& setParamData, QString& errorMsg)
{
    auto tztz = setParamData.value("CarrierModulation").toInt();  //调制体制
    int dataType = setParamData.value("DataOutWay").toInt();      // IQ路选择
    //  int powerRatioIQ = setParamData.value("IOPowerDB").toInt();   // IQ路选择
    // I路
    int codeWayI = setParamData.value("ICodeWay").toInt();              //译码方式
    int codeTypeI = setParamData.value("ICodeType").toInt();            //码型
    int viterbiCodeI = setParamData.value("IViterbiDecoding").toInt();  // Viterbi译码
    int ldpcCodeI = setParamData.value("ILDPCCodeRate").toInt();        // Ldpc码率
    int turboCodeI = setParamData.value("ITurboCodeRate").toInt();      // Turbo码率
    double codeSpeedI = setParamData.value("ICodeSpeed").toDouble();    // I路码速率
    int frmLenI = setParamData.value("IFrameLenght").toInt();           // I路帧长
    int interleavI = setParamData.value("IInterlaceDeep").toInt();      // 交错深度
    int frameLenI = setParamData.value("IIWayGroupLen").toInt();        //帧同步码组长
    int comRSI = setParamData.value("IRSDecoding").toInt();             // R-S码类型
    // Q路
    int codeWayQ = setParamData.value("QCodeWay").toInt();              //译码方式
    int codeTypeQ = setParamData.value("QCodeType").toInt();            //码型
    int viterbiCodeQ = setParamData.value("QViterbiDecoding").toInt();  // Viterbi译码
    int ldpcCodeQ = setParamData.value("QLDPCCodeRate").toInt();        // Ldpc码率
    int turboCodeQ = setParamData.value("QTurboCodeRate").toInt();      // Turbo码率
    double codeSpeedQ = setParamData.value("QCodeSpeed").toDouble();    // Q路码速率
    int frmLenQ = setParamData.value("QFrameLenght").toInt();           // Q路帧长
    int interleavQ = setParamData.value("QDoubleBaseChange").toInt();   // 交错深度
    int frameLenQ = setParamData.value("QIWayGroupLen").toInt();        //帧同步码组长
    int comRSQ = setParamData.value("QRSDecoding").toInt();             // R-S码类型

    int initMinValue;  //这个是译码方式为关各个调制体制的初始最小值
    int initMaxValue;  //这个是译码方式为关各个调制体制的初始最大值
    double bValueI;
    double bValueQ;
    QString cValueI;
    QString cValueQ;

    if (tztz == 1)  // BPSK  只有I路
    {
        initMinValue = 1000;
        initMaxValue = 15000000;

        if (codeWayI == 2 || codeWayI == 4)  // Viterbi或者R-S和Viterbi级联译码
        {
            int tempFrm = 0;  // 存放公式的计算结果
            tempFrm = 255 * interleavI + (frameLenI + 7) / 8;
            int frmLenMin = 0;
            if (comRSI == 1)
            {
                frmLenMin = tempFrm - 222 * interleavI;
            }
            else
            {
                frmLenMin = tempFrm - 238 * interleavI;
            }

            if ((frmLenI > tempFrm) || (frmLenI < frmLenMin))
            {
                errorMsg =
                    QString("I路：有R-S译码时帧长超出范围：%1-%2，且应满足(总帧长 - 子帧长 = 交错深度*N)，请重新输入！").arg(frmLenMin).arg(tempFrm);
                return false;
            }
            else
            {
                if (((tempFrm - frmLenI) % interleavI) != 0)
                {
                    errorMsg = QString("I路：有R-S译码时，帧长，交错深度，RS码类型三者不匹配，请重新输入！(总帧长 - 子帧长 = 交错深度*N)");
                    return false;
                }
            }
        }
    }
    else if (tztz == 2 || tztz == 5)  // QPSK OQPSK
    {
        initMinValue = 20000;
        initMaxValue = 30000000;

        if (dataType == 1 || dataType == 3)  // IQ合路或者I独立
        {
            if (codeWayI == 2 || codeWayI == 4)  // Viterbi或者R-S和Viterbi级联译码
            {
                int tempFrm = 0;  // 存放公式的计算结果
                tempFrm = 255 * interleavI + (frameLenI + 7) / 8;
                int frmLenMin = 0;
                if (comRSI == 1)
                {
                    frmLenMin = tempFrm - 222 * interleavI;
                }
                else
                {
                    frmLenMin = tempFrm - 238 * interleavI;
                }

                if ((frmLenI > tempFrm) || (frmLenI < frmLenMin))
                {
                    errorMsg = QString("I路：有R-S译码时帧长超出范围：%1-%2，且应满足(总帧长 - 子帧长 = 交错深度*N)，请重新输入！")
                                   .arg(frmLenMin)
                                   .arg(tempFrm);
                    return false;
                }
                else
                {
                    if (((tempFrm - frmLenI) % interleavI) != 0)
                    {
                        errorMsg = QString("I路：有R-S译码时，帧长，交错深度，RS码类型三者不匹配，请重新输入！(总帧长 - 子帧长 = 交错深度*N)");
                        return false;
                    }
                }
            }
        }
        else if (dataType == 2)  // IQ分路
        {
            if (codeWayI == 2 || codeWayI == 4)  // Viterbi或者R-S和Viterbi级联译码
            {
                int tempFrm = 0;  // 存放公式的计算结果
                tempFrm = 255 * interleavI + (frameLenI + 7) / 8;
                int frmLenMin = 0;
                if (comRSI == 1)
                {
                    frmLenMin = tempFrm - 222 * interleavI;
                }
                else
                {
                    frmLenMin = tempFrm - 238 * interleavI;
                }

                if ((frmLenI > tempFrm) || (frmLenI < frmLenMin))
                {
                    errorMsg = QString("I路：有R-S译码时帧长超出范围：%1-%2，且应满足(总帧长 - 子帧长 = 交错深度*N)，请重新输入！")
                                   .arg(frmLenMin)
                                   .arg(tempFrm);
                    return false;
                }
                else
                {
                    if (((tempFrm - frmLenI) % interleavI) != 0)
                    {
                        errorMsg = QString("I路：有R-S译码时，帧长，交错深度，RS码类型三者不匹配，请重新输入！(总帧长 - 子帧长 = 交错深度*N)");
                        return false;
                    }
                }
            }

            if (codeWayQ == 2 || codeWayQ == 4)  // Viterbi或者R-S和Viterbi级联译码
            {
                int tempFrm = 0;  // 存放公式的计算结果
                tempFrm = 255 * interleavQ + (frameLenQ + 7) / 8;
                int frmLenMin = 0;
                if (comRSQ == 1)
                {
                    frmLenMin = tempFrm - 222 * interleavQ;
                }
                else
                {
                    frmLenMin = tempFrm - 238 * interleavQ;
                }

                if ((frmLenQ > tempFrm) || (frmLenQ < frmLenMin))
                {
                    errorMsg = QString("Q路：有R-S译码时帧长超出范围：%1-%2，且应满足(总帧长 - 子帧长 = 交错深度*N)，请重新输入！")
                                   .arg(frmLenMin)
                                   .arg(tempFrm);
                    return false;
                }
                else
                {
                    if (((tempFrm - frmLenQ) % interleavQ) != 0)
                    {
                        errorMsg = QString("Q路：有R-S译码时，帧长，交错深度，RS码类型三者不匹配，请重新输入！(总帧长 - 子帧长 = 交错深度*N)");
                        return false;
                    }
                }
            }
        }
    }
    else if (tztz == 4)  // UQPSK时必为IQ分路
    {
        initMinValue = 10000;
        initMaxValue = 15000000;

        if (codeWayI == 3 || codeWayI == 4)  // R-S或者R-S和Viterbi级联译码
        {
            int tempFrm = 0;  // 存放公式的计算结果
            tempFrm = 255 * interleavI + (frameLenI + 7) / 8;
            int frmLenMin = 0;
            if (comRSI == 1)
            {
                frmLenMin = tempFrm - 222 * interleavI;
            }
            else
            {
                frmLenMin = tempFrm - 238 * interleavI;
            }

            if ((frmLenI > tempFrm) || (frmLenI < frmLenMin))
            {
                errorMsg =
                    QString("I路：有R-S译码时帧长超出范围：%1-%2，且应满足(总帧长 - 子帧长 = 交错深度*N)，请重新输入！").arg(frmLenMin).arg(tempFrm);
                return false;
            }
            else
            {
                if (((tempFrm - frmLenI) % interleavI) != 0)
                {
                    errorMsg = QString("I路：有R-S译码时，帧长，交错深度，RS码类型三者不匹配，请重新输入！(总帧长 - 子帧长 = 交错深度*N)");
                    return false;
                }
            }
        }

        if (codeWayQ == 2 || codeWayQ == 4)  // Viterbi或者R-S和Viterbi级联译码
        {
            int tempFrm = 0;  // 存放公式的计算结果
            tempFrm = 255 * interleavQ + (frameLenQ + 7) / 8;
            int frmLenMin = 0;
            if (comRSQ == 1)
            {
                frmLenMin = tempFrm - 222 * interleavQ;
            }
            else
            {
                frmLenMin = tempFrm - 238 * interleavQ;
            }

            if ((frmLenQ > tempFrm) || (frmLenQ < frmLenMin))
            {
                errorMsg =
                    QString("Q路：有R-S译码时帧长超出范围：%1-%2，且应满足(总帧长 - 子帧长 = 交错深度*N)，请重新输入！").arg(frmLenMin).arg(tempFrm);
                return false;
            }
            else
            {
                if (((tempFrm - frmLenQ) % interleavQ) != 0)
                {
                    errorMsg = QString("Q路：有R-S译码时，帧长，交错深度，RS码类型三者不匹配，请重新输入！(总帧长 - 子帧长 = 交错深度*N)");
                    return false;
                }
            }
        }

        //        auto value = 10 * log10(codeSpeedI / codeSpeedQ);

        //        if (powerRatioIQ != value)
        //        {
        //            errorMsg = QString("当调制体制为UQPSK时，【10lg(I/Q路码率的比值)】必须与【I/Q功率分贝比】相等，请重新输入!");
        //            return false;
        //        }
    }

    if (tztz == 1 || tztz == 2 || tztz == 5)  // BPSK  QPSK OQPSK
    {
        if (frameLenI % 8)
        {
            errorMsg = QString("I路帧同步码组长必须为8的整数倍");
            return false;
        }
        if (codeTypeI == 1 || codeTypeI == 2 || codeTypeI == 3)  // NRZ-L、M、S
        {
            bValueI = 1;
        }
        else if (codeTypeI == 4 || codeTypeI == 5 || codeTypeI == 6)  // Biφ-L、M、S
        {
            bValueI = 0.5;
        }

        if (codeWayI == 2 || codeWayI == 4)  // Viterbi或者R-S和Viterbi级联译码
        {
            if (viterbiCodeI == 1)  //(7,(1/2))
            {
                cValueI = QString("1/2");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                if (codeSpeedI < minValue || codeSpeedI > maxValue)
                {
                    errorMsg = QString("当前I路码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
            else if (viterbiCodeI == 2)
            {
                cValueI = QString("2/3");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                if (codeSpeedI < minValue || codeSpeedI > maxValue)
                {
                    errorMsg = QString("当前I路码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
            else if (viterbiCodeI == 3)
            {
                cValueI = QString("3/4");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                if (codeSpeedI < minValue || codeSpeedI > maxValue)
                {
                    errorMsg = QString("当前I路码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
            else if (viterbiCodeI == 4)
            {
                cValueI = QString("5/6");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                if (codeSpeedI < minValue || codeSpeedI > maxValue)
                {
                    errorMsg = QString("当前I路码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
            else if (viterbiCodeI == 5)
            {
                cValueI = QString("7/8");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                if (codeSpeedI < minValue || codeSpeedI > maxValue)
                {
                    errorMsg = QString("当前I路码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
        }
        else if (codeWayI == 5)  // Turbo
        {
            if (turboCodeI == 1 || turboCodeI == 2 || turboCodeI == 3 || turboCodeI == 4)  // 1/2
            {
                cValueI = QString("1/2");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                if (codeSpeedI < minValue || codeSpeedI > maxValue)
                {
                    errorMsg = QString("当前I路码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
            else if (turboCodeI == 5 || turboCodeI == 6 || turboCodeI == 7 || turboCodeI == 8)  // 1/3
            {
                cValueI = QString("1/3");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                if (codeSpeedI < minValue || codeSpeedI > maxValue)
                {
                    errorMsg = QString("当前I路码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
            else if (turboCodeI == 9 || turboCodeI == 10 || turboCodeI == 11 || turboCodeI == 12)  // 1/4
            {
                cValueI = QString("1/4");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                if (codeSpeedI < minValue || codeSpeedI > maxValue)
                {
                    errorMsg = QString("当前I路码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
            if (turboCodeI == 13 || turboCodeI == 14 || turboCodeI == 15 || turboCodeI == 16)  // 1/6
            {
                cValueI = QString("1/6");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                if (codeSpeedI < minValue || codeSpeedI > maxValue)
                {
                    errorMsg = QString("当前I路码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
        }
        else if (codeWayI == 6)  // LDPC
        {
            if (ldpcCodeI == 1 || ldpcCodeI == 2)  // 1/2
            {
                cValueI = QString("1/2");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                if (codeSpeedI < minValue || codeSpeedI > maxValue)
                {
                    errorMsg = QString("当前I路码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
            else if (ldpcCodeI == 3 || ldpcCodeI == 4)  // 2/3
            {
                cValueI = QString("2/3");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                if (codeSpeedI < minValue || codeSpeedI > maxValue)
                {
                    errorMsg = QString("当前I路码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
            else if (ldpcCodeI == 5 || ldpcCodeI == 6)  // 4/5
            {
                cValueI = QString("4/5");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                if (codeSpeedI < minValue || codeSpeedI > maxValue)
                {
                    errorMsg = QString("当前I路码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
            if (ldpcCodeI == 7)  // 7/8
            {
                cValueI = QString("7/8");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                if (codeSpeedI < minValue || codeSpeedI > maxValue)
                {
                    errorMsg = QString("当前I路码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
        }
        else if (codeWayI == 1 || codeWayI == 3)
        {
            cValueI = QString("1/1");
            int minValue;
            int maxValue;

            calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

            if (codeSpeedI < minValue || codeSpeedI > maxValue)
            {
                errorMsg = QString("当前I路码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                return false;
            }
        }
    }
    else if (tztz == 4)  // UQPSK
    {
        if (frameLenI % 8)
        {
            errorMsg = QString("I路帧同步码组长必须为8的整数倍");
            return false;
        }
        if (frameLenQ % 8)
        {
            errorMsg = QString("Q路帧同步码组长必须为8的整数倍");
            return false;
        }
        // I路
        if (codeTypeI == 1 || codeTypeI == 2 || codeTypeI == 3)  // NRZ-L、M、S
        {
            bValueI = 1;
        }
        else if (codeTypeI == 4 || codeTypeI == 5 || codeTypeI == 6)  // Biφ-L、M、S
        {
            bValueI = 0.5;
        }

        if (codeWayI == 2 || codeWayI == 4)  // Viterbi或者R-S和Viterbi级联译码
        {
            if (viterbiCodeI == 1)  //(7,(1/2))
            {
                cValueI = QString("1/2");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                if (codeSpeedI < minValue || codeSpeedI > maxValue)
                {
                    errorMsg = QString("当前I路码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
            else if (viterbiCodeI == 2)
            {
                cValueI = QString("2/3");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                if (codeSpeedI < minValue || codeSpeedI > maxValue)
                {
                    errorMsg = QString("当前I路码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
            else if (viterbiCodeI == 3)
            {
                cValueI = QString("3/4");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                if (codeSpeedI < minValue || codeSpeedI > maxValue)
                {
                    errorMsg = QString("当前I路码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
            else if (viterbiCodeI == 4)
            {
                cValueI = QString("5/6");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                if (codeSpeedI < minValue || codeSpeedI > maxValue)
                {
                    errorMsg = QString("当前I路码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
            else if (viterbiCodeI == 5)
            {
                cValueI = QString("7/8");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                if (codeSpeedI < minValue || codeSpeedI > maxValue)
                {
                    errorMsg = QString("当前I路码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
        }
        else if (codeWayI == 5)  // Turbo
        {
            if (turboCodeI == 1 || turboCodeI == 2 || turboCodeI == 3 || turboCodeI == 4)  // 1/2
            {
                cValueI = QString("1/2");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                if (codeSpeedI < minValue || codeSpeedI > maxValue)
                {
                    errorMsg = QString("当前I路码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
            else if (turboCodeI == 5 || turboCodeI == 6 || turboCodeI == 7 || turboCodeI == 8)  // 1/3
            {
                cValueI = QString("1/3");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                if (codeSpeedI < minValue || codeSpeedI > maxValue)
                {
                    errorMsg = QString("当前I路码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
            else if (turboCodeI == 9 || turboCodeI == 10 || turboCodeI == 11 || turboCodeI == 12)  // 1/4
            {
                cValueI = QString("1/4");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                if (codeSpeedI < minValue || codeSpeedI > maxValue)
                {
                    errorMsg = QString("当前I路码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
            if (turboCodeI == 13 || turboCodeI == 14 || turboCodeI == 15 || turboCodeI == 16)  // 1/6
            {
                cValueI = QString("1/6");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                if (codeSpeedI < minValue || codeSpeedI > maxValue)
                {
                    errorMsg = QString("当前I路码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
        }
        else if (codeWayI == 6)  // LDPC
        {
            if (ldpcCodeI == 1 || ldpcCodeI == 2)  // 1/2
            {
                cValueI = QString("1/2");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                if (codeSpeedI < minValue || codeSpeedI > maxValue)
                {
                    errorMsg = QString("当前I路码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
            else if (ldpcCodeI == 3 || ldpcCodeI == 4)  // 2/3
            {
                cValueI = QString("2/3");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                if (codeSpeedI < minValue || codeSpeedI > maxValue)
                {
                    errorMsg = QString("当前I路码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
            else if (ldpcCodeI == 5 || ldpcCodeI == 6)  // 4/5
            {
                cValueI = QString("4/5");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                if (codeSpeedI < minValue || codeSpeedI > maxValue)
                {
                    errorMsg = QString("当前I路码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
            if (ldpcCodeI == 7)  // 7/8
            {
                cValueI = QString("7/8");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

                if (codeSpeedI < minValue || codeSpeedI > maxValue)
                {
                    errorMsg = QString("当前I路码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
        }
        else if (codeWayI == 1 || codeWayI == 3)
        {
            cValueI = QString("1/1");
            int minValue;
            int maxValue;

            calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueI, cValueI, minValue, maxValue);

            if (codeSpeedI < minValue || codeSpeedI > maxValue)
            {
                errorMsg = QString("当前I路码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                return false;
            }
        }
        // Q路
        if (codeTypeQ == 1 || codeTypeQ == 2 || codeTypeQ == 3)  // NRZ-L、M、S
        {
            bValueQ = 1;
        }
        else if (codeTypeQ == 4 || codeTypeQ == 5 || codeTypeQ == 6)  // Biφ-L、M、S
        {
            bValueQ = 0.5;
        }

        if (codeWayQ == 2 || codeWayQ == 4)  // Viterbi或者R-S和Viterbi级联译码
        {
            if (viterbiCodeQ == 1)  //(7,(1/2))
            {
                cValueQ = QString("1/2");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueQ, cValueQ, minValue, maxValue);

                if (codeSpeedQ < minValue || codeSpeedQ > maxValue)
                {
                    errorMsg = QString("当前Q路码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
            else if (viterbiCodeQ == 2)
            {
                cValueQ = QString("2/3");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueQ, cValueQ, minValue, maxValue);

                if (codeSpeedQ < minValue || codeSpeedQ > maxValue)
                {
                    errorMsg = QString("当前Q路码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
            else if (viterbiCodeQ == 3)
            {
                cValueQ = QString("3/4");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueQ, cValueQ, minValue, maxValue);

                if (codeSpeedQ < minValue || codeSpeedQ > maxValue)
                {
                    errorMsg = QString("当前Q路码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
            else if (viterbiCodeQ == 4)
            {
                cValueQ = QString("5/6");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueQ, cValueQ, minValue, maxValue);

                if (codeSpeedQ < minValue || codeSpeedQ > maxValue)
                {
                    errorMsg = QString("当前Q路码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
            else if (viterbiCodeQ == 5)
            {
                cValueQ = QString("7/8");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueQ, cValueQ, minValue, maxValue);

                if (codeSpeedQ < minValue || codeSpeedQ > maxValue)
                {
                    errorMsg = QString("当前Q路码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
        }
        else if (codeWayQ == 5)  // Turbo
        {
            if (turboCodeQ == 1 || turboCodeQ == 2 || turboCodeQ == 3 || turboCodeQ == 4)  // 1/2
            {
                cValueQ = QString("1/2");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueQ, cValueQ, minValue, maxValue);

                if (codeSpeedQ < minValue || codeSpeedQ > maxValue)
                {
                    errorMsg = QString("当前Q路码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
            else if (turboCodeQ == 5 || turboCodeQ == 6 || turboCodeQ == 7 || turboCodeQ == 8)  // 1/3
            {
                cValueQ = QString("1/3");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueQ, cValueQ, minValue, maxValue);

                if (codeSpeedQ < minValue || codeSpeedQ > maxValue)
                {
                    errorMsg = QString("当前Q路码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
            else if (turboCodeQ == 9 || turboCodeQ == 10 || turboCodeQ == 11 || turboCodeQ == 12)  // 1/4
            {
                cValueQ = QString("1/4");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueQ, cValueQ, minValue, maxValue);

                if (codeSpeedQ < minValue || codeSpeedQ > maxValue)
                {
                    errorMsg = QString("当前Q路码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
            if (turboCodeQ == 13 || turboCodeQ == 14 || turboCodeQ == 15 || turboCodeQ == 16)  // 1/6
            {
                cValueQ = QString("1/6");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueQ, cValueQ, minValue, maxValue);

                if (codeSpeedQ < minValue || codeSpeedQ > maxValue)
                {
                    errorMsg = QString("当前Q路码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
        }
        else if (codeWayQ == 6)  // LDPC
        {
            if (ldpcCodeQ == 1 || ldpcCodeQ == 2)  // 1/2
            {
                cValueQ = QString("1/2");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueQ, cValueQ, minValue, maxValue);

                if (codeSpeedQ < minValue || codeSpeedQ > maxValue)
                {
                    errorMsg = QString("当前Q路码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
            else if (ldpcCodeQ == 3 || ldpcCodeQ == 4)  // 2/3
            {
                cValueQ = QString("2/3");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueQ, cValueQ, minValue, maxValue);

                if (codeSpeedQ < minValue || codeSpeedQ > maxValue)
                {
                    errorMsg = QString("当前Q路码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
            else if (ldpcCodeQ == 5 || ldpcCodeQ == 6)  // 4/5
            {
                cValueQ = QString("4/5");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueQ, cValueQ, minValue, maxValue);

                if (codeSpeedQ < minValue || codeSpeedQ > maxValue)
                {
                    errorMsg = QString("当前Q路码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
            if (ldpcCodeQ == 7)  // 7/8
            {
                cValueQ = QString("7/8");
                int minValue;
                int maxValue;

                calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueQ, cValueQ, minValue, maxValue);

                if (codeSpeedQ < minValue || codeSpeedQ > maxValue)
                {
                    errorMsg = QString("当前Q路码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                    return false;
                }
            }
        }
        else if (codeWayQ == 1 || codeWayQ == 3)
        {
            cValueQ = QString("1/1");
            int minValue;
            int maxValue;

            calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValueQ, cValueQ, minValue, maxValue);

            if (codeSpeedQ < minValue || codeSpeedQ > maxValue)
            {
                errorMsg = QString("当前Q路码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                return false;
            }
        }
    }

    return true;
}

bool ParamRelationOtherManager::checkParamRelationGZJT(const QMap<QString, QVariant>& setParamData, QString& errorMsg)
{
    int tztz = setParamData.value("tztz").toInt();                     //调制体制
    auto codeType = setParamData.value("CodeType").toInt();            //码型
    auto viterbiCode = setParamData.value("ViterbiDecoding").toInt();  // Viterbi译码

    auto ldpcType = setParamData.value("LDPCDecoding").toInt();  // Ldpc译码
    auto ldpcCode = setParamData.value("LDPCCodeRate").toInt();  // Ldpc码率

    auto turboType = setParamData.value("TurboDecoding").toInt();  // Turbo译码
    auto turboCode = setParamData.value("TurboCodeRate").toInt();  // Turbo码率
    auto codeSpeed = setParamData.value("CodeSpeed").toDouble();   //码速率

    int initMinValue;  //这个是译码方式为关各个调制体制的初始最小值
    int initMaxValue;  //这个是译码方式为关各个调制体制的初始最大值
    double bValue;
    QString cValue;

    if (tztz == 2)  // BPSK
    {
        initMinValue = 1000;
        initMaxValue = 15000000;
    }
    else if (tztz == 3 || tztz == 4)  // QPSK OQPSK
    {
        initMinValue = 20000;
        initMaxValue = 30000000;
    }
    else if (tztz == 5)  // UQPSK
    {
        initMinValue = 10000;
        initMaxValue = 15000000;
    }

    if (codeType == 0 || codeType == 1 || codeType == 2)  // NRZ-L、M、S
    {
        bValue = 1;
    }
    else if (codeType == 3 || codeType == 4 || codeType == 5)  // Biφ-L、M、S
    {
        bValue = 0.5;
    }

    if (viterbiCode != 0)  // Viterbi不为否的时候
    {
        if (viterbiCode == 1)  //(7,(1/2))
        {
            cValue = QString("1/2");
            int minValue;
            int maxValue;

            calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

            if (codeSpeed < minValue || codeSpeed > maxValue)
            {
                errorMsg = QString("当前码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                return false;
            }
        }
        else if (viterbiCode == 2)
        {
            cValue = QString("2/3");
            int minValue;
            int maxValue;

            calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

            if (codeSpeed < minValue || codeSpeed > maxValue)
            {
                errorMsg = QString("当前码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                return false;
            }
        }
        else if (viterbiCode == 3)
        {
            cValue = QString("3/4");
            int minValue;
            int maxValue;

            calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

            if (codeSpeed < minValue || codeSpeed > maxValue)
            {
                errorMsg = QString("当前码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                return false;
            }
        }
        else if (viterbiCode == 4)
        {
            cValue = QString("5/6");
            int minValue;
            int maxValue;

            calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

            if (codeSpeed < minValue || codeSpeed > maxValue)
            {
                errorMsg = QString("当前码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                return false;
            }
        }
        else if (viterbiCode == 5)
        {
            cValue = QString("7/8");
            int minValue;
            int maxValue;

            calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

            if (codeSpeed < minValue || codeSpeed > maxValue)
            {
                errorMsg = QString("当前码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                return false;
            }
        }
        else if (viterbiCode == 6)
        {
            cValue = QString("6/7");
            int minValue;
            int maxValue;

            calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

            if (codeSpeed < minValue || codeSpeed > maxValue)
            {
                errorMsg = QString("当前码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                return false;
            }
        }
    }
    else if (turboType == 1)  // Turbo译码为有
    {
        if (turboCode == 0)  // 1/2
        {
            cValue = QString("1/2");
            int minValue;
            int maxValue;

            calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

            if (codeSpeed < minValue || codeSpeed > maxValue)
            {
                errorMsg = QString("当前码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                return false;
            }
        }
        else if (turboCode == 1)  // 1/3
        {
            cValue = QString("1/3");
            int minValue;
            int maxValue;

            calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

            if (codeSpeed < minValue || codeSpeed > maxValue)
            {
                errorMsg = QString("当前码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                return false;
            }
        }
        else if (turboCode == 2)  // 1/4
        {
            cValue = QString("1/4");
            int minValue;
            int maxValue;

            calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

            if (codeSpeed < minValue || codeSpeed > maxValue)
            {
                errorMsg = QString("当前码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                return false;
            }
        }
        if (turboCode == 3)  // 1/6
        {
            cValue = QString("1/6");
            int minValue;
            int maxValue;

            calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

            if (codeSpeed < minValue || codeSpeed > maxValue)
            {
                errorMsg = QString("当前码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                return false;
            }
        }
    }
    else if (ldpcType == 1)  // LDPC译码为有
    {
        if (ldpcCode == 0)  // 1/2
        {
            cValue = QString("1/2");
            int minValue;
            int maxValue;

            calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

            if (codeSpeed < minValue || codeSpeed > maxValue)
            {
                errorMsg = QString("当前码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                return false;
            }
        }
        else if (ldpcCode == 1)  // 2/3
        {
            cValue = QString("2/3");
            int minValue;
            int maxValue;

            calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

            if (codeSpeed < minValue || codeSpeed > maxValue)
            {
                errorMsg = QString("当前码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                return false;
            }
        }
        else if (ldpcCode == 2)  // 4/5
        {
            cValue = QString("4/5");
            int minValue;
            int maxValue;

            calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

            if (codeSpeed < minValue || codeSpeed > maxValue)
            {
                errorMsg = QString("当前码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                return false;
            }
        }
        if (ldpcCode == 3)  // 7/8
        {
            cValue = QString("7/8");
            int minValue;
            int maxValue;

            calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

            if (codeSpeed < minValue || codeSpeed > maxValue)
            {
                errorMsg = QString("当前码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
                return false;
            }
        }
    }
    else  // viterbi ldpc turbo都不选的时候进里面
    {
        cValue = QString("1/1");
        int minValue;
        int maxValue;

        calcuCodeRateMinMaxValue(initMinValue, initMaxValue, bValue, cValue, minValue, maxValue);

        if (codeSpeed < minValue || codeSpeed > maxValue)
        {
            errorMsg = QString("当前码速率的范围为%1-%2bps，请重新输入").arg(minValue).arg(maxValue);
            return false;
        }
    }
    return true;
}

void ParamRelationOtherManager::calcuCodeRateMinMaxValue(int initMinValue, int initMaxValue, double bValue, QString mark, int& minValue,
                                                         int& maxValue)
{
    QString frontStep = mark.section('/', 0, 0);
    QString endStep = mark.section('/', 1, 1);
    if (endStep.toInt() >= 1 && frontStep.toInt() >= 1)
    {
        minValue = (int)(((initMinValue * bValue) * frontStep.toInt()) / endStep.toInt());
        maxValue = (int)(((initMaxValue * bValue) * frontStep.toInt()) / endStep.toInt());
    }
}

bool ParamRelationOtherManager::checkParamRelationCkSKpTAS(const QList<QMap<QString, QVariant>>& paraTargerMap, QString& errorMsg)
{
    int index = 0;
    for (auto& paraTargetData : paraTargerMap)
    {
        index++;

        int encodeStyle = paraTargetData.value("K2_DecodMode").toInt();  // 编码方式
        int interleav = paraTargetData.value("K2_InterlDepth").toInt();  // 交错深度
        int frmLen = paraTargetData.value("K2_FrameLength").toInt();     // 帧长
        int synLen = paraTargetData.value("K2_FrameSynCodeLength").toInt() -
                     1;  // 帧同步码组长  这里的-1是因为设备上代码特殊处理过，为了保持数据一致，故我们这也-1
        int comRS = paraTargetData.value("K2_RSCodeType").toInt();  // R-S码类型
        int vFill = -1;                                             // 虚拟填充
        int vFillTem = -1;                                          // 虚拟填充的

        int wordLength = paraTargetData.value("K2_WordLength").toInt();        // 字长
        int formatLength = paraTargetData.value("K2_FormatLength").toInt();    // 格式(副帧)长
        int subframeSynch = paraTargetData.value("K2_SubframeSynch").toInt();  // 副帧同步方式

        if (subframeSynch == 2)  // 副帧同步方式为ID副帧
        {
            int maxValue = pow(2, wordLength);

            if (formatLength > maxValue)
            {
                errorMsg = QString("目标%1:ID副帧时，格式(副帧)长不能超过2^N（N为字长）").arg(index);
                return false;
            }
        }

        // 有RS编码（R-S编码、R-S和卷积级联编码）
        if (encodeStyle == 3 || encodeStyle == 4)
        {
            int tempFrm = 0;  // 存放公式的计算结果
            tempFrm = 255 * interleav + (synLen + 2);
            if (tempFrm == frmLen)
            {
                vFill = 0;
                vFillTem = 0;
            }
            else
            {
                bool temFlag = true;
                bool temFlag2 = true;
                int vFillLen = tempFrm - frmLen;
                if (vFillLen < 0)
                {
                    temFlag = false;
                }
                if (vFillLen >= 255 * interleav)  // 交错深度太大
                {
                    temFlag = false;
                }

                if (vFillLen % interleav != 0)
                {
                    temFlag2 = false;
                }

                if (temFlag2 == false)
                {
                    errorMsg = QString("目标%1:帧长，交错深度，RS码类型三者不匹配，请重新输入！(总帧长 - 子帧长 = "
                                       "交错深度*N)\n当R-S码打开的时候,您应设置一个正确的值,比如交错深度为5,帧同步码组长度设置为16位,帧长设置为167字")
                                   .arg(index);
                    return false;
                }

                int m = 0;
                m = vFillLen / interleav;
                int frmLenMin;

                if (comRS == 1)
                {
                    if ((m <= 0) || (m > 222))
                    {
                        frmLenMin = tempFrm - 222 * interleav;
                        errorMsg =
                            QString("目标%1:此时帧长范围应该为：%2~%3，且应满足(总帧长 - 子帧长 = "
                                    "交错深度*N)\n当R-S码打开的时候,您应设置一个正确的值,比如交错深度为5,帧同步码组长度设置为16位,帧长设置为167字")
                                .arg(index)
                                .arg(frmLenMin)
                                .arg(tempFrm);
                        return false;
                    }
                }
                else  // comRS == 2
                {
                    if ((m <= 0) || (m > 238))
                    {
                        frmLenMin = tempFrm - 238 * interleav;
                        errorMsg =
                            QString("目标%1:此时帧长范围应该为: %2~%3，且应满足(总帧长 - 子帧长 = "
                                    "交错深度*N)\n当R-S码打开的时候,您应设置一个正确的值,比如交错深度为5,帧同步码组长度设置为16位,帧长设置为167字")
                                .arg(index)
                                .arg(frmLenMin)
                                .arg(tempFrm);
                        return false;
                    }
                }

                if (temFlag == false)
                {
                    errorMsg = QString("目标%1:帧长,交错深度，RS码类型三者不匹配，请重新输入!\n当R-S码打开的时候,您应设置一个正确的值,"
                                       "比如交错深度为5,帧同步码组长度设置为16位,帧长设置为167字")
                                   .arg(index);
                    return false;
                }

                vFill = 1;
                vFillTem = 1;
            }
        }
        int subFrameSyn = paraTargetData.value("K2_SubframeSynch").toInt();         // 副帧同步方式
        int idNum = paraTargetData.value("K2_IDCountPosit").toInt();                // ID计数位置
        int subFramePosition = paraTargetData.value("K2_SubfraCodeLocat").toInt();  //副帧码组位置
        int subFrameLen = paraTargetData.value("K2_SubCodeLength").toInt();         //副帧码组长度
        int synLen2 = paraTargetData.value("K2_FrameSynCodeLength").toInt();        // 帧同步码组长
        int synFramePosi = paraTargetData.value("K2_FrameCodeLocat").toInt();       // 帧同步码组位置

        if (subFrameSyn == 2)  // ID副帧
        {
            if (synFramePosi == 1)  //帧头
            {
                int idNumMin = -1;
                int idNumMax = frmLen;  //最大值就为帧长
                if (synLen2 == 1)       // 16位
                {
                    idNumMin = 3;  // id计数位置的最小值为2 + 1 = 3个字节
                }
                else if (synLen2 == 2)  // 24位
                {
                    idNumMin = 4;  // id计数位置的最小值为3 + 1 = 4个字节
                }
                else if (synLen2 == 3)  // 32位
                {
                    idNumMin = 5;  // id计数位置的最小值为4 + 1 = 5个字节
                }

                if (idNum < idNumMin || idNum > idNumMax)
                {
                    errorMsg = QString("目标%1:当副帧同步方式为ID副帧且帧同步码组位置为帧首时，ID字位置的范围为%2-%3，请重新输入")
                                   .arg(index)
                                   .arg(idNumMin)
                                   .arg(idNumMax);
                    return false;
                }
            }
            else if (synFramePosi == 2)  //帧尾
            {
                int idNumMin = 1;  //最小值就为1
                int idNumMax;
                if (synLen2 == 1)  // 16位
                {
                    idNumMax = frmLen - 2;  // id计数位置的最大值
                }
                else if (synLen2 == 2)  // 24位
                {
                    idNumMax = frmLen - 3;  // id计数位置的最大值
                }
                else if (synLen2 == 3)  // 32位
                {
                    idNumMax = frmLen - 4;  // id计数位置的最大值
                }

                if (idNum < idNumMin || idNum > idNumMax)
                {
                    errorMsg = QString("目标%1:当副帧同步方式为ID副帧且帧同步码组位置为帧尾时，副帧码组位置位置的范围为%2-%3，请重新输入")
                                   .arg(index)
                                   .arg(idNumMin)
                                   .arg(idNumMax);
                    return false;
                }
            }
        }
        else if (subFrameSyn == 3)  //循环副帧
        {
            if (synFramePosi == 1)  //帧头
            {
                int subNumMin = -1;
                int subNumMax = -1;
                if (subFrameLen == 1)  //副帧码组长度  8位
                {
                    subNumMax = frmLen;
                }
                else if (subFrameLen == 2)  //副帧码组长度  16位
                {
                    subNumMax = frmLen - 1;
                }
                else if (subFrameLen == 3)  //副帧码组长度  24位
                {
                    subNumMax = frmLen - 2;
                }
                else if (subFrameLen == 4)  //副帧码组长度  32位
                {
                    subNumMax = frmLen - 3;
                }

                if (synLen2 == 1)  // 16位
                {
                    subNumMin = 3;  // 副帧码组位置的最小值为2 + 1 = 3个字节
                }
                else if (synLen2 == 2)  // 24位
                {
                    subNumMin = 4;  // 副帧码组位置的最小值为3 + 1 = 4个字节
                }
                else if (synLen2 == 3)  // 32位
                {
                    subNumMin = 5;  // 副帧码组位置的最小值为4 + 1 = 5个字节
                }

                if (subFramePosition < subNumMin || subFramePosition > subNumMax)
                {
                    errorMsg = QString("目标%1:当副帧同步方式为循环副帧且帧同步码组位置为帧首时，副帧码组位置位置的范围为%2-%3，请重新输入")
                                   .arg(index)
                                   .arg(subNumMin)
                                   .arg(subNumMax);
                    return false;
                }
            }
            else if (synFramePosi == 2)  //帧尾
            {
                int subNumMin = 1;
                int subNumMax;

                if (synLen2 == 1)  // 16位
                {
                    if (subFrameLen == 1)  //副帧码组长度  8位
                    {
                        subNumMax = frmLen - 2;  // 副帧码组位置的最大值
                    }
                    else if (subFrameLen == 2)  //副帧码组长度  16位
                    {
                        subNumMax = frmLen - 3;
                    }
                    else if (subFrameLen == 3)  //副帧码组长度  24位
                    {
                        subNumMax = frmLen - 4;
                    }
                    else if (subFrameLen == 4)  //副帧码组长度  32位
                    {
                        subNumMax = frmLen - 5;
                    }
                }
                else if (synLen2 == 2)  // 24位
                {
                    if (subFrameLen == 1)  //副帧码组长度  8位
                    {
                        subNumMax = frmLen - 3;  // 副帧码组位置的最大值
                    }
                    else if (subFrameLen == 2)  //副帧码组长度  16位
                    {
                        subNumMax = frmLen - 4;
                    }
                    else if (subFrameLen == 3)  //副帧码组长度  24位
                    {
                        subNumMax = frmLen - 5;
                    }
                    else if (subFrameLen == 4)  //副帧码组长度  32位
                    {
                        subNumMax = frmLen - 6;
                    }
                }
                else if (synLen2 == 3)  // 32位
                {
                    if (subFrameLen == 1)  //副帧码组长度  8位
                    {
                        subNumMax = frmLen - 4;  // 副帧码组位置的最大值
                    }
                    else if (subFrameLen == 2)  //副帧码组长度  16位
                    {
                        subNumMax = frmLen - 5;
                    }
                    else if (subFrameLen == 3)  //副帧码组长度  24位
                    {
                        subNumMax = frmLen - 6;
                    }
                    else if (subFrameLen == 4)  //副帧码组长度  32位
                    {
                        subNumMax = frmLen - 7;
                    }
                }

                if (subFramePosition < subNumMin || subFramePosition > subNumMax)
                {
                    errorMsg = QString("目标%1:当副帧同步方式为循环副帧且帧同步码组位置为帧尾时，副帧码组位置的范围为%2-%3，请重新输入")
                                   .arg(index)
                                   .arg(subNumMin)
                                   .arg(subNumMax);
                    return false;
                }
            }
        }

        if (frmLen < idNum)
        {
            errorMsg = QString("目标%1:ID字位置不可超过帧长").arg(index);
            return false;
        }
    }
    return true;
}

bool ParamRelationOtherManager::checkParamRelationCkSKpTU(const QList<QMap<QString, QVariant>>& paraTargerMap, QString& errorMsg)
{
    int index = 0;
    for (auto& paraTargetData : paraTargerMap)
    {
        index++;
        int encodeStyle = paraTargetData.value("K2_DecodMode").toInt();  // 编码方式
        int interleav = paraTargetData.value("K2_InterlDepth").toInt();  // 交错深度
        int frmLen = paraTargetData.value("K2_FrameLength").toInt();     // 帧长
        int synLen = paraTargetData.value("K2_FrameSynCodeLength").toInt() -
                     1;  // 帧同步码组长  这里的-1是因为设备上代码特殊处理过，为了保持数据一致，故我们这也-1
        int comRS = paraTargetData.value("RSCodeType").toInt();  // R-S码类型
        int vFill = -1;                                          // 虚拟填充
        int vFillTem = -1;                                       // 虚拟填充的

        int wordLength = paraTargetData.value("K2_WordLength").toInt();        // 字长
        int formatLength = paraTargetData.value("K2_FormatLength").toInt();    // 格式(副帧)长
        int subframeSynch = paraTargetData.value("K2_SubframeSynch").toInt();  // 副帧同步方式

        if (subframeSynch == 2)  // 副帧同步方式为ID副帧
        {
            int maxValue = pow(2, wordLength);

            if (formatLength > maxValue)
            {
                errorMsg = QString("目标%1:ID副帧时，格式(副帧)长不能超过2^N（N为字长）").arg(index);
                return false;
            }
        }

        // 有RS编码（R-S编码、R-S和卷积级联编码）
        if (encodeStyle == 3 || encodeStyle == 4)
        {
            int tempFrm = 0;  // 存放公式的计算结果
            tempFrm = 255 * interleav + (synLen + 2);
            if (tempFrm == frmLen)
            {
                vFill = 0;
                vFillTem = 0;
            }
            else
            {
                bool temFlag = true;
                bool temFlag2 = true;
                int vFillLen = tempFrm - frmLen;
                if (vFillLen < 0)
                {
                    temFlag = false;
                }
                if (vFillLen >= 255 * interleav)  // 交错深度太大
                {
                    temFlag = false;
                }

                if (vFillLen % interleav != 0)
                {
                    temFlag2 = false;
                }

                if (temFlag2 == false)
                {
                    errorMsg = QString("目标%1:帧长，交错深度，RS码类型三者不匹配，请重新输入！(总帧长 - 子帧长 = "
                                       "交错深度*N)\n当R-S码打开的时候,您应设置一个正确的值,比如交错深度为5,帧同步码组长度设置为16位,帧长设置为167字")
                                   .arg(index);
                    return false;
                }

                int m = 0;
                m = vFillLen / interleav;
                int frmLenMin;

                if (comRS == 1)
                {
                    if ((m <= 0) || (m > 222))
                    {
                        frmLenMin = tempFrm - 222 * interleav;
                        errorMsg =
                            QString("目标%1:此时帧长范围应该为：%2~%3，且应满足(总帧长 - 子帧长 = "
                                    "交错深度*N)\n当R-S码打开的时候,您应设置一个正确的值,比如交错深度为5,帧同步码组长度设置为16位,帧长设置为167字")
                                .arg(index)
                                .arg(frmLenMin)
                                .arg(tempFrm);
                        return false;
                    }
                }
                else  // comRS == 2
                {
                    if ((m <= 0) || (m > 238))
                    {
                        frmLenMin = tempFrm - 238 * interleav;
                        errorMsg =
                            QString("目标%1:此时帧长范围应该为: %2~%3，且应满足(总帧长 - 子帧长 = "
                                    "交错深度*N)\n当R-S码打开的时候,您应设置一个正确的值,比如交错深度为5,帧同步码组长度设置为16位,帧长设置为167字")
                                .arg(index)
                                .arg(frmLenMin)
                                .arg(tempFrm);
                        return false;
                    }
                }

                if (temFlag == false)
                {
                    errorMsg = QString("目标%1:帧长,交错深度，RS码类型三者不匹配，请重新输入!\n当R-S码打开的时候,您应设置一个正确的值,"
                                       "比如交错深度为5,帧同步码组长度设置为16位,帧长设置为167字")
                                   .arg(index);
                    return false;
                }

                vFill = 1;
                vFillTem = 1;
            }
        }

        int subFrameSyn = paraTargetData.value("K2_SubframeSynch").toInt();         // 副帧同步方式
        int idNum = paraTargetData.value("K2_IDCountPosit").toInt();                // ID计数位置
        int subFramePosition = paraTargetData.value("K2_SubfraCodeLocat").toInt();  //副帧码组位置
        int subFrameLen = paraTargetData.value("K2_SubCodeLength").toInt();         //副帧码组长度
        int synLen2 = paraTargetData.value("K2_FrameSynCodeLength").toInt();        // 帧同步码组长
        int synFramePosi = paraTargetData.value("K2_FrameCodeLocat").toInt();       // 帧同步码组位置

        if (subFrameSyn == 2)  // ID副帧
        {
            if (synFramePosi == 1)  //帧头
            {
                int idNumMin = -1;
                int idNumMax = frmLen;  //最大值就为帧长
                if (synLen2 == 1)       // 16位
                {
                    idNumMin = 3;  // id计数位置的最小值为2 + 1 = 3个字节
                }
                else if (synLen2 == 2)  // 24位
                {
                    idNumMin = 4;  // id计数位置的最小值为3 + 1 = 4个字节
                }
                else if (synLen2 == 3)  // 32位
                {
                    idNumMin = 5;  // id计数位置的最小值为4 + 1 = 5个字节
                }

                if (idNum < idNumMin || idNum > idNumMax)
                {
                    errorMsg = QString("目标%1:当副帧同步方式为ID副帧且帧同步码组位置为帧首时，ID字位置的范围为%2-%3，请重新输入")
                                   .arg(index)
                                   .arg(idNumMin)
                                   .arg(idNumMax);
                    return false;
                }
            }
            else if (synFramePosi == 2)  //帧尾
            {
                int idNumMin = 1;  //最小值就为1
                int idNumMax;
                if (synLen2 == 1)  // 16位
                {
                    idNumMax = frmLen - 2;  // id计数位置的最大值
                }
                else if (synLen2 == 2)  // 24位
                {
                    idNumMax = frmLen - 3;  // id计数位置的最大值
                }
                else if (synLen2 == 3)  // 32位
                {
                    idNumMax = frmLen - 4;  // id计数位置的最大值
                }

                if (idNum < idNumMin || idNum > idNumMax)
                {
                    errorMsg = QString("目标%1:当副帧同步方式为ID副帧且帧同步码组位置为帧尾时，副帧码组位置位置的范围为%2-%3，请重新输入")
                                   .arg(index)
                                   .arg(idNumMin)
                                   .arg(idNumMax);
                    return false;
                }
            }
        }
        else if (subFrameSyn == 3)  //循环副帧
        {
            if (synFramePosi == 1)  //帧头
            {
                int subNumMin = -1;
                int subNumMax = -1;
                if (subFrameLen == 1)  //副帧码组长度  8位
                {
                    subNumMax = frmLen;
                }
                else if (subFrameLen == 2)  //副帧码组长度  16位
                {
                    subNumMax = frmLen - 1;
                }
                else if (subFrameLen == 3)  //副帧码组长度  24位
                {
                    subNumMax = frmLen - 2;
                }
                else if (subFrameLen == 4)  //副帧码组长度  32位
                {
                    subNumMax = frmLen - 3;
                }

                if (synLen2 == 1)  // 16位
                {
                    subNumMin = 3;  // 副帧码组位置的最小值为2 + 1 = 3个字节
                }
                else if (synLen2 == 2)  // 24位
                {
                    subNumMin = 4;  // 副帧码组位置的最小值为3 + 1 = 4个字节
                }
                else if (synLen2 == 3)  // 32位
                {
                    subNumMin = 5;  // 副帧码组位置的最小值为4 + 1 = 5个字节
                }

                if (subFramePosition < subNumMin || subFramePosition > subNumMax)
                {
                    errorMsg = QString("目标%1:当副帧同步方式为循环副帧且帧同步码组位置为帧首时，副帧码组位置位置的范围为%2-%3，请重新输入")
                                   .arg(index)
                                   .arg(subNumMin)
                                   .arg(subNumMax);
                    return false;
                }
            }
            else if (synFramePosi == 2)  //帧尾
            {
                int subNumMin = 1;
                int subNumMax;

                if (synLen2 == 1)  // 16位
                {
                    if (subFrameLen == 1)  //副帧码组长度  8位
                    {
                        subNumMax = frmLen - 2;  // 副帧码组位置的最大值
                    }
                    else if (subFrameLen == 2)  //副帧码组长度  16位
                    {
                        subNumMax = frmLen - 3;
                    }
                    else if (subFrameLen == 3)  //副帧码组长度  24位
                    {
                        subNumMax = frmLen - 4;
                    }
                    else if (subFrameLen == 4)  //副帧码组长度  32位
                    {
                        subNumMax = frmLen - 5;
                    }
                }
                else if (synLen2 == 2)  // 24位
                {
                    if (subFrameLen == 1)  //副帧码组长度  8位
                    {
                        subNumMax = frmLen - 3;  // 副帧码组位置的最大值
                    }
                    else if (subFrameLen == 2)  //副帧码组长度  16位
                    {
                        subNumMax = frmLen - 4;
                    }
                    else if (subFrameLen == 3)  //副帧码组长度  24位
                    {
                        subNumMax = frmLen - 5;
                    }
                    else if (subFrameLen == 4)  //副帧码组长度  32位
                    {
                        subNumMax = frmLen - 6;
                    }
                }
                else if (synLen2 == 3)  // 32位
                {
                    if (subFrameLen == 1)  //副帧码组长度  8位
                    {
                        subNumMax = frmLen - 4;  // 副帧码组位置的最大值
                    }
                    else if (subFrameLen == 2)  //副帧码组长度  16位
                    {
                        subNumMax = frmLen - 5;
                    }
                    else if (subFrameLen == 3)  //副帧码组长度  24位
                    {
                        subNumMax = frmLen - 6;
                    }
                    else if (subFrameLen == 4)  //副帧码组长度  32位
                    {
                        subNumMax = frmLen - 7;
                    }
                }

                if (subFramePosition < subNumMin || subFramePosition > subNumMax)
                {
                    errorMsg = QString("目标%1:当副帧同步方式为循环副帧且帧同步码组位置为帧尾时，副帧码组位置的范围为%2-%3，请重新输入")
                                   .arg(index)
                                   .arg(subNumMin)
                                   .arg(subNumMax);
                    return false;
                }
            }
        }

        if (frmLen < idNum)
        {
            errorMsg = QString("目标%1:ID字位置不可超过帧长").arg(index);
            return false;
        }
    }
    return true;
}

bool ParamRelationOtherManager::checkParamRelationCkSKpRCU(const QList<QMap<QString, QVariant>>& paraTargerMap, QString& errorMsg)
{
    int index = 0;
    for (auto& paraTargetData : paraTargerMap)
    {
        index++;
        int codeSelect = paraTargetData.value("K2_CodeSelect").toInt();    // 编码选择
        int codeSpeed = paraTargetData.value("K2_CodeRate").toInt();       // 码速率
        int convCodType = paraTargetData.value("K2_ConvCodType").toInt();  // 卷积编码类型  7,1/2  7,3/4

        if (codeSelect == 1)  //编码前
        {
            int minValue = 100;
            int maxValue = 8000;

            if (codeSpeed < minValue || codeSpeed > maxValue)
            {
                errorMsg = QString("目标%1:在编码选择为卷积编码前时，码速率范围为%2-%3bps,请重新输入！").arg(index).arg(minValue).arg(maxValue);
                return false;
            }
        }
        else if (codeSelect == 2)  //编码后
        {
            if (convCodType == 1)  // 7,1/2
            {
                int minValue = 200;
                int maxValue = 16000;

                if (codeSpeed < minValue || codeSpeed > maxValue)
                {
                    errorMsg = QString("目标%1:在编码选择为卷积编码后，卷积编码类型为(7,1/2)时，码速率范围为%2-%3bps,请重新输入！")
                                   .arg(index)
                                   .arg(minValue)
                                   .arg(maxValue);
                    return false;
                }
            }
            else if (convCodType == 2)  // 7,3/4
            {
                int minValue = 134;
                int maxValue = 10666;

                if (codeSpeed < minValue || codeSpeed > maxValue)
                {
                    errorMsg = QString("目标%1:在编码选择为卷积编码后，卷积编码类型为(7,3/4)时，码速率范围为%2-%3bps,请重新输入！")
                                   .arg(index)
                                   .arg(minValue)
                                   .arg(maxValue);
                    return false;
                }
            }
        }
    }

    return true;
}

bool ParamRelationOtherManager::checkParamRelationKTJSAndTZ(const QMap<QString, QVariant>& setParamData, QString& errorMsg)
{
    auto shortCode = setParamData.value("ShortCode").toString();
    auto longCode = setParamData.value("LongCode").toString();

    auto shortSize = shortCode.size();
    auto longSize = longCode.size();

    if (shortSize != 8)
    {
        errorMsg = QString("短码码号长度必须为8");
        return false;
    }

    if (longSize != 8)
    {
        errorMsg = QString("长码码号长度必须为8");
        return false;
    }

    if (shortCode.at(7) != '0' || shortCode.at(6) != '0' || shortCode.at(5) != 'G' || (shortCode.at(4) != 'L' && shortCode.at(4) != 'K') ||
        shortCode.at(3) < '0' || shortCode.at(2) < '0' || shortCode.at(1) < '0' || shortCode.at(0) < '0' || shortCode.at(3) > '9' ||
        shortCode.at(2) > '9' || shortCode.at(1) > '9' || shortCode.at(0) > '9')
    {
        errorMsg = QString("短码码号格式错误,此参数前四位必须是00GL或者00GK，后四位为0-9之间的数字");
        return false;
    }

    if (longCode.at(7) != '0' || longCode.at(6) != '0' || longCode.at(5) != 'T' || (longCode.at(4) != 'L' && longCode.at(4) != 'K') ||
        longCode.at(3) < '0' || longCode.at(2) < '0' || longCode.at(1) < '0' || longCode.at(0) < '0' || longCode.at(3) > '9' ||
        longCode.at(2) > '9' || longCode.at(1) > '9' || longCode.at(0) > '9')
    {
        errorMsg = QString("长码码号格式错误,此参数前四位必须是00TL或者00TK，后四位为0-9之间的数字");
        return false;
    }

    return true;
}

bool ParamRelationOtherManager::checkParamRelationYTHSKpTUAndTAS(const QList<QMap<QString, QVariant>>& paraTargerMap, QString& errorMsg)
{
    int index = 0;
    for (auto& paraTargetData : paraTargerMap)
    {
        index++;
        int encodeStyle = paraTargetData.value("K2_DecodMode").toInt();  // 编码方式
        int interleav = paraTargetData.value("K2_InterlDepth").toInt();  // 交错深度
        int frmLen = paraTargetData.value("K2_FrameLength").toInt();     // 帧长
        int synLen = paraTargetData.value("K2_FrameSynCodeLength").toInt() -
                     1;  // 帧同步码组长  这里的-1是因为设备上代码特殊处理过，为了保持数据一致，故我们这也-1
        int comRS = paraTargetData.value("K2_RSCodeType").toInt();  // R-S码类型
        int vFill = -1;                                             // 虚拟填充
        int vFillTem = -1;                                          // 虚拟填充的

        int wordLength = paraTargetData.value("K2_WordLength").toInt();        // 字长
        int formatLength = paraTargetData.value("K2_FormatLength").toInt();    // 格式(副帧)长
        int subframeSynch = paraTargetData.value("K2_SubframeSynch").toInt();  // 副帧同步方式

        if (subframeSynch == 2)  // 副帧同步方式为ID副帧
        {
            int maxValue = pow(2, wordLength);

            if (formatLength > maxValue)
            {
                errorMsg = QString("ID副帧时，格式(副帧)长不能超过2^N（N为字长）");
                return false;
            }
        }

        // 有RS编码（R-S编码、R-S和卷积级联编码）
        if (encodeStyle == 3 || encodeStyle == 4)
        {
            int tempFrm = 0;  // 存放公式的计算结果
            tempFrm = 255 * interleav + (synLen + 2);
            if (tempFrm == frmLen)
            {
                vFill = 0;
                vFillTem = 0;
            }
            else
            {
                bool temFlag = true;
                bool temFlag2 = true;
                int vFillLen = tempFrm - frmLen;
                if (vFillLen < 0)
                {
                    temFlag = false;
                }
                if (vFillLen >= 255 * interleav)  // 交错深度太大
                {
                    temFlag = false;
                }

                if (vFillLen % interleav != 0)
                {
                    temFlag2 = false;
                }

                if (temFlag2 == false)
                {
                    errorMsg =
                        QString("帧长，交错深度，RS码类型三者不匹配，请重新输入！(总帧长 - 子帧长 = "
                                "交错深度*N)\n当R-S码打开的时候,您应设置一个正确的值,比如交错深度为5,帧同步码组长度设置为16位,帧长设置为167字");
                    return false;
                }

                int m = 0;
                m = vFillLen / interleav;
                int frmLenMin;

                if (comRS == 1)
                {
                    if ((m <= 0) || (m > 222))
                    {
                        frmLenMin = tempFrm - 222 * interleav;
                        errorMsg =
                            QString("此时帧长范围应该为：%1~%2，且应满足(总帧长 - 子帧长 = "
                                    "交错深度*N)\n当R-S码打开的时候,您应设置一个正确的值,比如交错深度为5,帧同步码组长度设置为16位,帧长设置为167字")
                                .arg(frmLenMin)
                                .arg(tempFrm);
                        return false;
                    }
                }
                else  // comRS == 2
                {
                    if ((m <= 0) || (m > 238))
                    {
                        frmLenMin = tempFrm - 238 * interleav;
                        errorMsg =
                            QString("此时帧长范围应该为: %1~%2，且应满足(总帧长 - 子帧长 = "
                                    "交错深度*N)\n当R-S码打开的时候,您应设置一个正确的值,比如交错深度为5,帧同步码组长度设置为16位,帧长设置为167字")
                                .arg(frmLenMin)
                                .arg(tempFrm);
                        return false;
                    }
                }

                if (temFlag == false)
                {
                    errorMsg = QString("帧长,交错深度，RS码类型三者不匹配，请重新输入!\n当R-S码打开的时候,您应设置一个正确的值,"
                                       "比如交错深度为5,帧同步码组长度设置为16位,帧长设置为167字");
                    return false;
                }

                vFill = 1;
                vFillTem = 1;
            }
        }

        int subFrameSyn = paraTargetData.value("K2_SubframeSynch").toInt();         // 副帧同步方式
        int idNum = paraTargetData.value("K2_IDCountPosit").toInt();                // ID计数位置
        int subFramePosition = paraTargetData.value("K2_SubfraCodeLocat").toInt();  //副帧码组位置
        int subFrameLen = paraTargetData.value("K2_SubCodeLength").toInt();         //副帧码组长度
        int synLen2 = paraTargetData.value("K2_FrameSynCodeLength").toInt();        // 帧同步码组长
        int synFramePosi = paraTargetData.value("K2_FrameCodeLocat").toInt();       // 帧同步码组位置

        if (subFrameSyn == 2)  // ID副帧
        {
            if (synFramePosi == 1)  //帧头
            {
                int idNumMin = -1;
                int idNumMax = frmLen;  //最大值就为帧长
                if (synLen2 == 1)       // 16位
                {
                    idNumMin = 3;  // id计数位置的最小值为2 + 1 = 3个字节
                }
                else if (synLen2 == 2)  // 24位
                {
                    idNumMin = 4;  // id计数位置的最小值为3 + 1 = 4个字节
                }
                else if (synLen2 == 3)  // 32位
                {
                    idNumMin = 5;  // id计数位置的最小值为4 + 1 = 5个字节
                }

                if (idNum < idNumMin || idNum > idNumMax)
                {
                    errorMsg =
                        QString("当副帧同步方式为ID副帧且帧同步码组位置为帧首时，ID字位置的范围为%1-%2，请重新输入").arg(idNumMin).arg(idNumMax);
                    return false;
                }
            }
            else if (synFramePosi == 2)  //帧尾
            {
                int idNumMin = 1;  //最小值就为1
                int idNumMax;
                if (synLen2 == 1)  // 16位
                {
                    idNumMax = frmLen - 2;  // id计数位置的最大值
                }
                else if (synLen2 == 2)  // 24位
                {
                    idNumMax = frmLen - 3;  // id计数位置的最大值
                }
                else if (synLen2 == 3)  // 32位
                {
                    idNumMax = frmLen - 4;  // id计数位置的最大值
                }

                if (idNum < idNumMin || idNum > idNumMax)
                {
                    errorMsg = QString("当副帧同步方式为ID副帧且帧同步码组位置为帧尾时，副帧码组位置位置的范围为%1-%2，请重新输入")
                                   .arg(idNumMin)
                                   .arg(idNumMax);
                    return false;
                }
            }
        }
        else if (subFrameSyn == 3)  //循环副帧
        {
            if (synFramePosi == 1)  //帧头
            {
                int subNumMin = -1;
                int subNumMax = -1;
                if (subFrameLen == 1)  //副帧码组长度  8位
                {
                    subNumMax = frmLen;
                }
                else if (subFrameLen == 2)  //副帧码组长度  16位
                {
                    subNumMax = frmLen - 1;
                }
                else if (subFrameLen == 3)  //副帧码组长度  24位
                {
                    subNumMax = frmLen - 2;
                }
                else if (subFrameLen == 4)  //副帧码组长度  32位
                {
                    subNumMax = frmLen - 3;
                }

                if (synLen2 == 1)  // 16位
                {
                    subNumMin = 3;  // 副帧码组位置的最小值为2 + 1 = 3个字节
                }
                else if (synLen2 == 2)  // 24位
                {
                    subNumMin = 4;  // 副帧码组位置的最小值为3 + 1 = 4个字节
                }
                else if (synLen2 == 3)  // 32位
                {
                    subNumMin = 5;  // 副帧码组位置的最小值为4 + 1 = 5个字节
                }

                if (subFramePosition < subNumMin || subFramePosition > subNumMax)
                {
                    errorMsg = QString("当副帧同步方式为循环副帧且帧同步码组位置为帧首时，副帧码组位置位置的范围为%1-%2，请重新输入")
                                   .arg(subNumMin)
                                   .arg(subNumMax);
                    return false;
                }
            }
            else if (synFramePosi == 2)  //帧尾
            {
                int subNumMin = 1;
                int subNumMax;

                if (synLen2 == 1)  // 16位
                {
                    if (subFrameLen == 1)  //副帧码组长度  8位
                    {
                        subNumMax = frmLen - 2;  // 副帧码组位置的最大值
                    }
                    else if (subFrameLen == 2)  //副帧码组长度  16位
                    {
                        subNumMax = frmLen - 3;
                    }
                    else if (subFrameLen == 3)  //副帧码组长度  24位
                    {
                        subNumMax = frmLen - 4;
                    }
                    else if (subFrameLen == 4)  //副帧码组长度  32位
                    {
                        subNumMax = frmLen - 5;
                    }
                }
                else if (synLen2 == 2)  // 24位
                {
                    if (subFrameLen == 1)  //副帧码组长度  8位
                    {
                        subNumMax = frmLen - 3;  // 副帧码组位置的最大值
                    }
                    else if (subFrameLen == 2)  //副帧码组长度  16位
                    {
                        subNumMax = frmLen - 4;
                    }
                    else if (subFrameLen == 3)  //副帧码组长度  24位
                    {
                        subNumMax = frmLen - 5;
                    }
                    else if (subFrameLen == 4)  //副帧码组长度  32位
                    {
                        subNumMax = frmLen - 6;
                    }
                }
                else if (synLen2 == 3)  // 32位
                {
                    if (subFrameLen == 1)  //副帧码组长度  8位
                    {
                        subNumMax = frmLen - 4;  // 副帧码组位置的最大值
                    }
                    else if (subFrameLen == 2)  //副帧码组长度  16位
                    {
                        subNumMax = frmLen - 5;
                    }
                    else if (subFrameLen == 3)  //副帧码组长度  24位
                    {
                        subNumMax = frmLen - 6;
                    }
                    else if (subFrameLen == 4)  //副帧码组长度  32位
                    {
                        subNumMax = frmLen - 7;
                    }
                }

                if (subFramePosition < subNumMin || subFramePosition > subNumMax)
                {
                    errorMsg = QString("当副帧同步方式为循环副帧且帧同步码组位置为帧尾时，副帧码组位置的范围为%1-%2，请重新输入")
                                   .arg(subNumMin)
                                   .arg(subNumMax);
                    return false;
                }
            }
        }

        if (frmLen < idNum)
        {
            errorMsg = QString("目标%1:ID字位置不可超过帧长").arg(index);
            return false;
        }
    }

    return true;
}

bool ParamRelationOtherManager::checkParamRelationYTHWXRCU(const QList<QMap<QString, QVariant>>& paraTargerMap, QString& errorMsg)
{
    int index = 0;
    for (auto& paraTargetData : paraTargerMap)
    {
        index++;
        int codeSelect = paraTargetData.value("WX_CodeSelect").toInt();    // 编码选择
        int codeSpeed = paraTargetData.value("WX_CodeRate").toInt();       // 码速率
        int convCodType = paraTargetData.value("WX_ConvCodType").toInt();  // 卷积编码类型  7,1/2  7,3/4

        if (codeSelect == 1)  //编码前
        {
            int minValue = 100;
            int maxValue = 8000;

            if (codeSpeed < minValue || codeSpeed > maxValue)
            {
                errorMsg = QString("在编码选择为卷积编码前时，码速率范围为%1-%2bps,请重新输入！").arg(minValue).arg(maxValue);
                return false;
            }
        }
        else if (codeSelect == 2)  //编码后
        {
            if (convCodType == 1)  // 7,1/2
            {
                int minValue = 200;
                int maxValue = 16000;

                if (codeSpeed < minValue || codeSpeed > maxValue)
                {
                    errorMsg =
                        QString("在编码选择为卷积编码后，卷积编码类型为(7,1/2)时，码速率范围为%1-%2bps,请重新输入！").arg(minValue).arg(maxValue);
                    return false;
                }
            }
            else if (convCodType == 2)  // 7,3/4
            {
                int minValue = 134;
                int maxValue = 10666;

                if (codeSpeed < minValue || codeSpeed > maxValue)
                {
                    errorMsg =
                        QString("在编码选择为卷积编码后，卷积编码类型为(7,3/4)时，码速率范围为%1-%2bps,请重新输入！").arg(minValue).arg(maxValue);
                    return false;
                }
            }
        }
    }

    return true;
}

bool ParamRelationOtherManager::checkParamRelationYTHRCU(const QList<QMap<QString, QVariant>>& paraTargerMap, QString& errorMsg)
{
    int index = 0;
    for (auto& paraTargetData : paraTargerMap)
    {
        index++;
        int codeSelect = paraTargetData.value("K2_CodeSelect").toInt();    // 编码选择
        int codeSpeed = paraTargetData.value("K2_CodeRate").toInt();       // 码速率
        int convCodType = paraTargetData.value("K2_ConvCodType").toInt();  // 卷积编码类型  7,1/2  7,3/4

        if (codeSelect == 1)  //编码前
        {
            int minValue = 100;
            int maxValue = 8000;

            if (codeSpeed < minValue || codeSpeed > maxValue)
            {
                errorMsg = QString("在编码选择为卷积编码前时，码速率范围为%1-%2bps,请重新输入！").arg(minValue).arg(maxValue);
                return false;
            }
        }
        else if (codeSelect == 2)  //编码后
        {
            if (convCodType == 1)  // 7,1/2
            {
                int minValue = 200;
                int maxValue = 16000;

                if (codeSpeed < minValue || codeSpeed > maxValue)
                {
                    errorMsg =
                        QString("在编码选择为卷积编码后，卷积编码类型为(7,1/2)时，码速率范围为%1-%2bps,请重新输入！").arg(minValue).arg(maxValue);
                    return false;
                }
            }
            else if (convCodType == 2)  // 7,3/4
            {
                int minValue = 134;
                int maxValue = 10666;

                if (codeSpeed < minValue || codeSpeed > maxValue)
                {
                    errorMsg =
                        QString("在编码选择为卷积编码后，卷积编码类型为(7,3/4)时，码速率范围为%1-%2bps,请重新输入！").arg(minValue).arg(maxValue);
                    return false;
                }
            }
        }
    }

    return true;
}

bool ParamRelationOtherManager::checkParamRelationLSTtcTAS(const QMap<QString, QVariant>& setParamData, QString& errorMsg)
{
    int encodeStyle = setParamData.value("DecodMode").toInt();  // 编码方式
    int interleav = setParamData.value("InterlDepth").toInt();  // 交错深度
    int frmLen = setParamData.value("FrameLength").toInt();     // 帧长
    int synLen =
        setParamData.value("FrameSynCodeLength").toInt() - 1;  // 帧同步码组长  这里的-1是因为设备上代码特殊处理过，为了保持数据一致，故我们这也-1
    int comRS = setParamData.value("RSCodeType").toInt();  // R-S码类型

    double fzbFrequency = setParamData.value("SubModSystem").toDouble();  //副载波频率
    int fzbFrequencyUnit = int(fzbFrequency * 1000);                      //这个是单位换算后的副载波频率
    int codeRate = setParamData.value("CodeRate").toInt();                //码速率

    int vFill = -1;     // 虚拟填充
    int vFillTem = -1;  // 虚拟填充的
    // 有RS编码（R-S编码、R-S和卷积级联编码）
    if (encodeStyle == 3 || encodeStyle == 4)
    {
        int tempFrm = 0;  // 存放公式的计算结果
        tempFrm = 255 * interleav + (synLen + 2);
        if (tempFrm == frmLen)
        {
            vFill = 0;
            vFillTem = 0;
        }
        else
        {
            bool temFlag = true;
            bool temFlag2 = true;
            int vFillLen = tempFrm - frmLen;
            if (vFillLen < 0)
            {
                temFlag = false;
            }
            if (vFillLen >= 255 * interleav)  // 交错深度太大
            {
                temFlag = false;
            }

            if (vFillLen % interleav != 0)
            {
                temFlag2 = false;
            }

            if (temFlag2 == false)
            {
                errorMsg = QString("帧长，交错深度，RS码类型三者不匹配，请重新输入！(总帧长 - 子帧长 = "
                                   "交错深度*N)\n当R-S码打开的时候,您应设置一个正确的值,比如交错深度为5,帧同步码组长度设置为16位,帧长设置为167字");
                return false;
            }

            int m = 0;
            m = vFillLen / interleav;
            int frmLenMin;

            if (comRS == 1)
            {
                if ((m <= 0) || (m > 222))
                {
                    frmLenMin = tempFrm - 222 * interleav;
                    errorMsg = QString("此时帧长范围应该为：%1~%2，且应满足(总帧长 - 子帧长 = "
                                       "交错深度*N)\n当R-S码打开的时候,您应设置一个正确的值,比如交错深度为5,帧同步码组长度设置为16位,帧长设置为167字")
                                   .arg(frmLenMin)
                                   .arg(tempFrm);
                    return false;
                }
            }
            else  // comRS == 2
            {
                if ((m <= 0) || (m > 238))
                {
                    frmLenMin = tempFrm - 238 * interleav;
                    errorMsg = QString("此时帧长范围应该为: %1~%2，且应满足(总帧长 - 子帧长 = "
                                       "交错深度*N)\n当R-S码打开的时候,您应设置一个正确的值,比如交错深度为5,帧同步码组长度设置为16位,帧长设置为167字")
                                   .arg(frmLenMin)
                                   .arg(tempFrm);
                    return false;
                }
            }

            if (temFlag == false)
            {
                errorMsg = QString("帧长,交错深度，RS码类型三者不匹配，请重新输入!\n当R-S码打开的时候,您应设置一个正确的值,比如交错深度为5,"
                                   "帧同步码组长度设置为16位,帧长设置为167字");
                return false;
            }

            vFill = 1;
            vFillTem = 1;
        }
    }

    int subFrameSyn = setParamData.value("SubframeSynch").toInt();         // 副帧同步方式
    int idNum = setParamData.value("IDCountPosit").toInt();                // ID计数位置
    int subFramePosition = setParamData.value("SubfraCodeLocat").toInt();  //副帧码组位置
    int subFrameLen = setParamData.value("SubCodeLength").toInt();         //副帧码组长度
    int synLen2 = setParamData.value("FrameSynCodeLength").toInt();        // 帧同步码组长
    int synFramePosi = setParamData.value("FrameCodeLocat").toInt();       // 帧同步码组位置

    if (subFrameSyn == 2)  // ID副帧
    {
        if (synFramePosi == 1)  //帧头
        {
            int idNumMin = -1;
            int idNumMax = frmLen;  //最大值就为帧长
            if (synLen2 == 1)       // 16位
            {
                idNumMin = 3;  // id计数位置的最小值为2 + 1 = 3个字节
            }
            else if (synLen2 == 2)  // 24位
            {
                idNumMin = 4;  // id计数位置的最小值为3 + 1 = 4个字节
            }
            else if (synLen2 == 3)  // 32位
            {
                idNumMin = 5;  // id计数位置的最小值为4 + 1 = 5个字节
            }

            if (idNum < idNumMin || idNum > idNumMax)
            {
                errorMsg = QString("当副帧同步方式为ID副帧且帧同步码组位置为帧首时，ID字位置的范围为%1-%2，请重新输入").arg(idNumMin).arg(idNumMax);
                return false;
            }
        }
        else if (synFramePosi == 2)  //帧尾
        {
            int idNumMin = 1;  //最小值就为1
            int idNumMax;
            if (synLen2 == 1)  // 16位
            {
                idNumMax = frmLen - 2;  // id计数位置的最大值
            }
            else if (synLen2 == 2)  // 24位
            {
                idNumMax = frmLen - 3;  // id计数位置的最大值
            }
            else if (synLen2 == 3)  // 32位
            {
                idNumMax = frmLen - 4;  // id计数位置的最大值
            }

            if (idNum < idNumMin || idNum > idNumMax)
            {
                errorMsg =
                    QString("当副帧同步方式为ID副帧且帧同步码组位置为帧尾时，副帧码组位置位置的范围为%1-%2，请重新输入").arg(idNumMin).arg(idNumMax);
                return false;
            }
        }
    }
    else if (subFrameSyn == 3)  //循环副帧
    {
        if (synFramePosi == 1)  //帧头
        {
            int subNumMin = -1;
            int subNumMax = -1;
            if (subFrameLen == 1)  //副帧码组长度  8位
            {
                subNumMax = frmLen;
            }
            else if (subFrameLen == 2)  //副帧码组长度  16位
            {
                subNumMax = frmLen - 1;
            }
            else if (subFrameLen == 3)  //副帧码组长度  24位
            {
                subNumMax = frmLen - 2;
            }
            else if (subFrameLen == 4)  //副帧码组长度  32位
            {
                subNumMax = frmLen - 3;
            }

            if (synLen2 == 1)  // 16位
            {
                subNumMin = 3;  // 副帧码组位置的最小值为2 + 1 = 3个字节
            }
            else if (synLen2 == 2)  // 24位
            {
                subNumMin = 4;  // 副帧码组位置的最小值为3 + 1 = 4个字节
            }
            else if (synLen2 == 3)  // 32位
            {
                subNumMin = 5;  // 副帧码组位置的最小值为4 + 1 = 5个字节
            }

            if (subFramePosition < subNumMin || subFramePosition > subNumMax)
            {
                errorMsg = QString("当副帧同步方式为循环副帧且帧同步码组位置为帧首时，副帧码组位置位置的范围为%1-%2，请重新输入")
                               .arg(subNumMin)
                               .arg(subNumMax);
                return false;
            }
        }
        else if (synFramePosi == 2)  //帧尾
        {
            int subNumMin = 1;
            int subNumMax;

            if (synLen2 == 1)  // 16位
            {
                if (subFrameLen == 1)  //副帧码组长度  8位
                {
                    subNumMax = frmLen - 2;  // 副帧码组位置的最大值
                }
                else if (subFrameLen == 2)  //副帧码组长度  16位
                {
                    subNumMax = frmLen - 3;
                }
                else if (subFrameLen == 3)  //副帧码组长度  24位
                {
                    subNumMax = frmLen - 4;
                }
                else if (subFrameLen == 4)  //副帧码组长度  32位
                {
                    subNumMax = frmLen - 5;
                }
            }
            else if (synLen2 == 2)  // 24位
            {
                if (subFrameLen == 1)  //副帧码组长度  8位
                {
                    subNumMax = frmLen - 3;  // 副帧码组位置的最大值
                }
                else if (subFrameLen == 2)  //副帧码组长度  16位
                {
                    subNumMax = frmLen - 4;
                }
                else if (subFrameLen == 3)  //副帧码组长度  24位
                {
                    subNumMax = frmLen - 5;
                }
                else if (subFrameLen == 4)  //副帧码组长度  32位
                {
                    subNumMax = frmLen - 6;
                }
            }
            else if (synLen2 == 3)  // 32位
            {
                if (subFrameLen == 1)  //副帧码组长度  8位
                {
                    subNumMax = frmLen - 4;  // 副帧码组位置的最大值
                }
                else if (subFrameLen == 2)  //副帧码组长度  16位
                {
                    subNumMax = frmLen - 5;
                }
                else if (subFrameLen == 3)  //副帧码组长度  24位
                {
                    subNumMax = frmLen - 6;
                }
                else if (subFrameLen == 4)  //副帧码组长度  32位
                {
                    subNumMax = frmLen - 7;
                }
            }

            if (subFramePosition < subNumMin || subFramePosition > subNumMax)
            {
                errorMsg =
                    QString("当副帧同步方式为循环副帧且帧同步码组位置为帧尾时，副帧码组位置的范围为%1-%2，请重新输入").arg(subNumMin).arg(subNumMax);
                return false;
            }
        }
    }

    if (frmLen < idNum)
    {
        errorMsg = QString("ID字位置不可超过帧长");
        return false;
    }

    int result = fzbFrequencyUnit % codeRate;
    if (result == 0)
    {
        int result2 = fzbFrequencyUnit / codeRate;
        if (result2 >= 2 && result2 <= 512)
        {
            return true;
        }
        else
        {
            errorMsg =
                QString("副载波输入值%1kHz不是码率输入值%2bps的整数倍，且整除后的值必须在2到512之间，请重新输入").arg(fzbFrequency).arg(codeRate);
            return false;
        }
    }
    else if (result != 0)
    {
        errorMsg = QString("副载波输入值%1kHz不是码率输入值%2bps的整数倍，且整除后的值必须在2到512之间，请重新输入").arg(fzbFrequency).arg(codeRate);
        return false;
    }

    return true;
}

bool ParamRelationOtherManager::checkParamRelationYTHSMJK2GTtcTAS(const QMap<QString, QVariant>& setParamData, QString& errorMsg)
{
    int encodeStyle = setParamData.value("DecodMode").toInt();  // 编码方式
    int interleav = setParamData.value("InterlDepth").toInt();  // 交错深度
    int frmLen = setParamData.value("FrameLength").toInt();     // 帧长
    int synLen =
        setParamData.value("FrameSynCodeLength").toInt() - 1;  // 帧同步码组长  这里的-1是因为设备上代码特殊处理过，为了保持数据一致，故我们这也-1
    int comRS = setParamData.value("RSCodeType").toInt();  // R-S码类型

    int vFill = -1;     // 虚拟填充
    int vFillTem = -1;  // 虚拟填充的
    // 有RS编码（R-S编码、R-S和卷积级联编码）
    if (encodeStyle == 3 || encodeStyle == 4)
    {
        int tempFrm = 0;  // 存放公式的计算结果
        tempFrm = 255 * interleav + (synLen + 2);
        if (tempFrm == frmLen)
        {
            vFill = 0;
            vFillTem = 0;
        }
        else
        {
            bool temFlag = true;
            bool temFlag2 = true;
            int vFillLen = tempFrm - frmLen;
            if (vFillLen < 0)
            {
                temFlag = false;
            }
            if (vFillLen >= 255 * interleav)  // 交错深度太大
            {
                temFlag = false;
            }

            if (vFillLen % interleav != 0)
            {
                temFlag2 = false;
            }

            if (temFlag2 == false)
            {
                errorMsg = QString("帧长，交错深度，RS码类型三者不匹配，请重新输入！(总帧长 - 子帧长 = "
                                   "交错深度*N)\n当R-S码打开的时候,您应设置一个正确的值,比如交错深度为5,帧同步码组长度设置为16位,帧长设置为167字");
                return false;
            }

            int m = 0;
            m = vFillLen / interleav;
            int frmLenMin;

            if (comRS == 1)
            {
                if ((m <= 0) || (m > 222))
                {
                    frmLenMin = tempFrm - 222 * interleav;
                    errorMsg = QString("此时帧长范围应该为：%1~%2，且应满足(总帧长 - 子帧长 = "
                                       "交错深度*N)\n当R-S码打开的时候,您应设置一个正确的值,比如交错深度为5,帧同步码组长度设置为16位,帧长设置为167字")
                                   .arg(frmLenMin)
                                   .arg(tempFrm);
                    return false;
                }
            }
            else  // comRS == 2
            {
                if ((m <= 0) || (m > 238))
                {
                    frmLenMin = tempFrm - 238 * interleav;
                    errorMsg = QString("此时帧长范围应该为: %1~%2，且应满足(总帧长 - 子帧长 = "
                                       "交错深度*N)\n当R-S码打开的时候,您应设置一个正确的值,比如交错深度为5,帧同步码组长度设置为16位,帧长设置为167字")
                                   .arg(frmLenMin)
                                   .arg(tempFrm);
                    return false;
                }
            }

            if (temFlag == false)
            {
                errorMsg = QString("帧长,交错深度，RS码类型三者不匹配，请重新输入!\n当R-S码打开的时候,您应设置一个正确的值,比如交错深度为5,"
                                   "帧同步码组长度设置为16位,帧长设置为167字");
                return false;
            }

            vFill = 1;
            vFillTem = 1;
        }
    }

    int subFrameSyn = setParamData.value("SubframeSynch").toInt();         // 副帧同步方式
    int idNum = setParamData.value("IDCountPosit").toInt();                // ID计数位置
    int subFramePosition = setParamData.value("SubfraCodeLocat").toInt();  //副帧码组位置
    int subFrameLen = setParamData.value("SubCodeLength").toInt();         //副帧码组长度
    int synLen2 = setParamData.value("FrameSynCodeLength").toInt();        // 帧同步码组长
    int synFramePosi = setParamData.value("FrameCodeLocat").toInt();       // 帧同步码组位置

    if (subFrameSyn == 2)  // ID副帧
    {
        if (synFramePosi == 1)  //帧头
        {
            int idNumMin = -1;
            int idNumMax = frmLen;  //最大值就为帧长
            if (synLen2 == 1)       // 16位
            {
                idNumMin = 3;  // id计数位置的最小值为2 + 1 = 3个字节
            }
            else if (synLen2 == 2)  // 24位
            {
                idNumMin = 4;  // id计数位置的最小值为3 + 1 = 4个字节
            }
            else if (synLen2 == 3)  // 32位
            {
                idNumMin = 5;  // id计数位置的最小值为4 + 1 = 5个字节
            }

            if (idNum < idNumMin || idNum > idNumMax)
            {
                errorMsg = QString("当副帧同步方式为ID副帧且帧同步码组位置为帧首时，ID字位置的范围为%1-%2，请重新输入").arg(idNumMin).arg(idNumMax);
                return false;
            }
        }
        else if (synFramePosi == 2)  //帧尾
        {
            int idNumMin = 1;  //最小值就为1
            int idNumMax;
            if (synLen2 == 1)  // 16位
            {
                idNumMax = frmLen - 2;  // id计数位置的最大值
            }
            else if (synLen2 == 2)  // 24位
            {
                idNumMax = frmLen - 3;  // id计数位置的最大值
            }
            else if (synLen2 == 3)  // 32位
            {
                idNumMax = frmLen - 4;  // id计数位置的最大值
            }

            if (idNum < idNumMin || idNum > idNumMax)
            {
                errorMsg =
                    QString("当副帧同步方式为ID副帧且帧同步码组位置为帧尾时，副帧码组位置位置的范围为%1-%2，请重新输入").arg(idNumMin).arg(idNumMax);
                return false;
            }
        }
    }
    else if (subFrameSyn == 3)  //循环副帧
    {
        if (synFramePosi == 1)  //帧头
        {
            int subNumMin = -1;
            int subNumMax = -1;
            if (subFrameLen == 1)  //副帧码组长度  8位
            {
                subNumMax = frmLen;
            }
            else if (subFrameLen == 2)  //副帧码组长度  16位
            {
                subNumMax = frmLen - 1;
            }
            else if (subFrameLen == 3)  //副帧码组长度  24位
            {
                subNumMax = frmLen - 2;
            }
            else if (subFrameLen == 4)  //副帧码组长度  32位
            {
                subNumMax = frmLen - 3;
            }

            if (synLen2 == 1)  // 16位
            {
                subNumMin = 3;  // 副帧码组位置的最小值为2 + 1 = 3个字节
            }
            else if (synLen2 == 2)  // 24位
            {
                subNumMin = 4;  // 副帧码组位置的最小值为3 + 1 = 4个字节
            }
            else if (synLen2 == 3)  // 32位
            {
                subNumMin = 5;  // 副帧码组位置的最小值为4 + 1 = 5个字节
            }

            if (subFramePosition < subNumMin || subFramePosition > subNumMax)
            {
                errorMsg = QString("当副帧同步方式为循环副帧且帧同步码组位置为帧首时，副帧码组位置位置的范围为%1-%2，请重新输入")
                               .arg(subNumMin)
                               .arg(subNumMax);
                return false;
            }
        }
        else if (synFramePosi == 2)  //帧尾
        {
            int subNumMin = 1;
            int subNumMax;

            if (synLen2 == 1)  // 16位
            {
                if (subFrameLen == 1)  //副帧码组长度  8位
                {
                    subNumMax = frmLen - 2;  // 副帧码组位置的最大值
                }
                else if (subFrameLen == 2)  //副帧码组长度  16位
                {
                    subNumMax = frmLen - 3;
                }
                else if (subFrameLen == 3)  //副帧码组长度  24位
                {
                    subNumMax = frmLen - 4;
                }
                else if (subFrameLen == 4)  //副帧码组长度  32位
                {
                    subNumMax = frmLen - 5;
                }
            }
            else if (synLen2 == 2)  // 24位
            {
                if (subFrameLen == 1)  //副帧码组长度  8位
                {
                    subNumMax = frmLen - 3;  // 副帧码组位置的最大值
                }
                else if (subFrameLen == 2)  //副帧码组长度  16位
                {
                    subNumMax = frmLen - 4;
                }
                else if (subFrameLen == 3)  //副帧码组长度  24位
                {
                    subNumMax = frmLen - 5;
                }
                else if (subFrameLen == 4)  //副帧码组长度  32位
                {
                    subNumMax = frmLen - 6;
                }
            }
            else if (synLen2 == 3)  // 32位
            {
                if (subFrameLen == 1)  //副帧码组长度  8位
                {
                    subNumMax = frmLen - 4;  // 副帧码组位置的最大值
                }
                else if (subFrameLen == 2)  //副帧码组长度  16位
                {
                    subNumMax = frmLen - 5;
                }
                else if (subFrameLen == 3)  //副帧码组长度  24位
                {
                    subNumMax = frmLen - 6;
                }
                else if (subFrameLen == 4)  //副帧码组长度  32位
                {
                    subNumMax = frmLen - 7;
                }
            }

            if (subFramePosition < subNumMin || subFramePosition > subNumMax)
            {
                errorMsg =
                    QString("当副帧同步方式为循环副帧且帧同步码组位置为帧尾时，副帧码组位置的范围为%1-%2，请重新输入").arg(subNumMin).arg(subNumMax);
                return false;
            }
        }
    }

    if (frmLen < idNum)
    {
        errorMsg = QString("ID字位置不可超过帧长");
        return false;
    }

    return true;
}

bool ParamRelationOtherManager::checkParamRelationLSKuoTAS(const QList<QMap<QString, QVariant>>& paraTargerMap, QString& errorMsg)
{
    int index = 0;
    for (auto& paraTargetData : paraTargerMap)
    {
        index++;

        int encodeStyle = paraTargetData.value("DecodMode").toInt();  // 编码方式
        int interleav = paraTargetData.value("InterlDepth").toInt();  // 交错深度
        int frmLen = paraTargetData.value("FrameLength").toInt();     // 帧长
        int synLen = paraTargetData.value("FrameSynCodeLength").toInt() -
                     1;  // 帧同步码组长  这里的-1是因为设备上代码特殊处理过，为了保持数据一致，故我们这也-1
        int comRS = paraTargetData.value("RSCodeType").toInt();  // R-S码类型
        int vFill = -1;                                          // 虚拟填充
        int vFillTem = -1;                                       // 虚拟填充的

        for (auto& targetData : paraTargetData.keys())
        {
            if (targetData == "WordLength")
            {
                int wordLength = paraTargetData.value(targetData).toInt();
                if (wordLength % 8 != 0)
                {
                    errorMsg = QString("目标%1:字长不是8的倍数，请重新输入！").arg(index);
                    return false;
                }
            }
        }

        // 有RS编码（R-S编码、R-S和卷积级联编码）
        if (encodeStyle == 3 || encodeStyle == 4)
        {
            int tempFrm = 0;  // 存放公式的计算结果
            tempFrm = 255 * interleav + (synLen + 2);
            if (tempFrm == frmLen)
            {
                vFill = 0;
                vFillTem = 0;
            }
            else
            {
                bool temFlag = true;
                bool temFlag2 = true;
                int vFillLen = tempFrm - frmLen;
                if (vFillLen < 0)
                {
                    temFlag = false;
                }
                if (vFillLen >= 255 * interleav)  // 交错深度太大
                {
                    temFlag = false;
                }

                if (vFillLen % interleav != 0)
                {
                    temFlag2 = false;
                }

                if (temFlag2 == false)
                {
                    errorMsg = QString("目标%1:帧长，交错深度，RS码类型三者不匹配，请重新输入！(总帧长 - 子帧长 = "
                                       "交错深度*N)\n当R-S码打开的时候,您应设置一个正确的值,比如交错深度为5,帧同步码组长度设置为16位,帧长设置为167字")
                                   .arg(index);
                    return false;
                }

                int m = 0;
                m = vFillLen / interleav;
                int frmLenMin;

                if (comRS == 1)
                {
                    if ((m <= 0) || (m > 222))
                    {
                        frmLenMin = tempFrm - 222 * interleav;
                        errorMsg =
                            QString("目标%1:此时帧长范围应该为：%2~%3，且应满足(总帧长 - 子帧长 = "
                                    "交错深度*N)\n当R-S码打开的时候,您应设置一个正确的值,比如交错深度为5,帧同步码组长度设置为16位,帧长设置为167字")
                                .arg(index)
                                .arg(frmLenMin)
                                .arg(tempFrm);
                        return false;
                    }
                }
                else  // comRS == 2
                {
                    if ((m <= 0) || (m > 238))
                    {
                        frmLenMin = tempFrm - 238 * interleav;
                        errorMsg =
                            QString("目标%1:此时帧长范围应该为: %2~%3，且应满足(总帧长 - 子帧长 = "
                                    "交错深度*N)\n当R-S码打开的时候,您应设置一个正确的值,比如交错深度为5,帧同步码组长度设置为16位,帧长设置为167字")
                                .arg(index)
                                .arg(frmLenMin)
                                .arg(tempFrm);
                        return false;
                    }
                }

                if (temFlag == false)
                {
                    errorMsg = QString("目标%1:帧长,交错深度，RS码类型三者不匹配，请重新输入!\n当R-S码打开的时候,您应设置一个正确的值,"
                                       "比如交错深度为5,帧同步码组长度设置为16位,帧长设置为167字")
                                   .arg(index);
                    return false;
                }

                vFill = 1;
                vFillTem = 1;
            }
        }

        int subFrameSyn = paraTargetData.value("SubframeSynch").toInt();         // 副帧同步方式
        int idNum = paraTargetData.value("IDCountPosit").toInt();                // ID计数位置
        int subFramePosition = paraTargetData.value("SubfraCodeLocat").toInt();  //副帧码组位置
        int subFrameLen = paraTargetData.value("SubCodeLength").toInt();         //副帧码组长度
        int synLen2 = paraTargetData.value("FrameSynCodeLength").toInt();        // 帧同步码组长
        int synFramePosi = paraTargetData.value("FrameCodeLocat").toInt();       // 帧同步码组位置

        if (subFrameSyn == 2)  // ID副帧
        {
            if (synFramePosi == 1)  //帧头
            {
                int idNumMin = -1;
                int idNumMax = frmLen;  //最大值就为帧长
                if (synLen2 == 1)       // 16位
                {
                    idNumMin = 3;  // id计数位置的最小值为2 + 1 = 3个字节
                }
                else if (synLen2 == 2)  // 24位
                {
                    idNumMin = 4;  // id计数位置的最小值为3 + 1 = 4个字节
                }
                else if (synLen2 == 3)  // 32位
                {
                    idNumMin = 5;  // id计数位置的最小值为4 + 1 = 5个字节
                }

                if (idNum < idNumMin || idNum > idNumMax)
                {
                    errorMsg = QString("目标%1:当副帧同步方式为ID副帧且帧同步码组位置为帧首时，ID字位置的范围为%2-%3，请重新输入")
                                   .arg(index)
                                   .arg(idNumMin)
                                   .arg(idNumMax);
                    return false;
                }
            }
            else if (synFramePosi == 2)  //帧尾
            {
                int idNumMin = 1;  //最小值就为1
                int idNumMax;
                if (synLen2 == 1)  // 16位
                {
                    idNumMax = frmLen - 2;  // id计数位置的最大值
                }
                else if (synLen2 == 2)  // 24位
                {
                    idNumMax = frmLen - 3;  // id计数位置的最大值
                }
                else if (synLen2 == 3)  // 32位
                {
                    idNumMax = frmLen - 4;  // id计数位置的最大值
                }

                if (idNum < idNumMin || idNum > idNumMax)
                {
                    errorMsg = QString("目标%1:当副帧同步方式为ID副帧且帧同步码组位置为帧尾时，副帧码组位置位置的范围为%2-%3，请重新输入")
                                   .arg(index)
                                   .arg(idNumMin)
                                   .arg(idNumMax);
                    return false;
                }
            }
        }
        else if (subFrameSyn == 3)  //循环副帧
        {
            if (synFramePosi == 1)  //帧头
            {
                int subNumMin = -1;
                int subNumMax = -1;
                if (subFrameLen == 1)  //副帧码组长度  8位
                {
                    subNumMax = frmLen;
                }
                else if (subFrameLen == 2)  //副帧码组长度  16位
                {
                    subNumMax = frmLen - 1;
                }
                else if (subFrameLen == 3)  //副帧码组长度  24位
                {
                    subNumMax = frmLen - 2;
                }
                else if (subFrameLen == 4)  //副帧码组长度  32位
                {
                    subNumMax = frmLen - 3;
                }

                if (synLen2 == 1)  // 16位
                {
                    subNumMin = 3;  // 副帧码组位置的最小值为2 + 1 = 3个字节
                }
                else if (synLen2 == 2)  // 24位
                {
                    subNumMin = 4;  // 副帧码组位置的最小值为3 + 1 = 4个字节
                }
                else if (synLen2 == 3)  // 32位
                {
                    subNumMin = 5;  // 副帧码组位置的最小值为4 + 1 = 5个字节
                }

                if (subFramePosition < subNumMin || subFramePosition > subNumMax)
                {
                    errorMsg = QString("目标%1:当副帧同步方式为循环副帧且帧同步码组位置为帧首时，副帧码组位置位置的范围为%2-%3，请重新输入")
                                   .arg(index)
                                   .arg(subNumMin)
                                   .arg(subNumMax);
                    return false;
                }
            }
            else if (synFramePosi == 2)  //帧尾
            {
                int subNumMin = 1;
                int subNumMax;

                if (synLen2 == 1)  // 16位
                {
                    if (subFrameLen == 1)  //副帧码组长度  8位
                    {
                        subNumMax = frmLen - 2;  // 副帧码组位置的最大值
                    }
                    else if (subFrameLen == 2)  //副帧码组长度  16位
                    {
                        subNumMax = frmLen - 3;
                    }
                    else if (subFrameLen == 3)  //副帧码组长度  24位
                    {
                        subNumMax = frmLen - 4;
                    }
                    else if (subFrameLen == 4)  //副帧码组长度  32位
                    {
                        subNumMax = frmLen - 5;
                    }
                }
                else if (synLen2 == 2)  // 24位
                {
                    if (subFrameLen == 1)  //副帧码组长度  8位
                    {
                        subNumMax = frmLen - 3;  // 副帧码组位置的最大值
                    }
                    else if (subFrameLen == 2)  //副帧码组长度  16位
                    {
                        subNumMax = frmLen - 4;
                    }
                    else if (subFrameLen == 3)  //副帧码组长度  24位
                    {
                        subNumMax = frmLen - 5;
                    }
                    else if (subFrameLen == 4)  //副帧码组长度  32位
                    {
                        subNumMax = frmLen - 6;
                    }
                }
                else if (synLen2 == 3)  // 32位
                {
                    if (subFrameLen == 1)  //副帧码组长度  8位
                    {
                        subNumMax = frmLen - 4;  // 副帧码组位置的最大值
                    }
                    else if (subFrameLen == 2)  //副帧码组长度  16位
                    {
                        subNumMax = frmLen - 5;
                    }
                    else if (subFrameLen == 3)  //副帧码组长度  24位
                    {
                        subNumMax = frmLen - 6;
                    }
                    else if (subFrameLen == 4)  //副帧码组长度  32位
                    {
                        subNumMax = frmLen - 7;
                    }
                }

                if (subFramePosition < subNumMin || subFramePosition > subNumMax)
                {
                    errorMsg = QString("目标%1:当副帧同步方式为循环副帧且帧同步码组位置为帧尾时，副帧码组位置的范围为%2-%3，请重新输入")
                                   .arg(index)
                                   .arg(subNumMin)
                                   .arg(subNumMax);
                    return false;
                }
            }
        }

        if (frmLen < idNum)
        {
            errorMsg = QString("目标%1:ID字位置不可超过帧长").arg(index);
            return false;
        }
    }
    return true;
}

bool ParamRelationOtherManager::checkParamRelationBeforeTest(const QMap<QString, QVariant>& setParamData, QString& errorMsg)
{
    int sInput = setParamData.value("SFreqSwitch").toInt();
    int xInput = setParamData.value("XFreqSwitch").toInt();
    int kaInput = setParamData.value("KaFreqSwitch").toInt();

    if ((sInput == 2 && xInput == 2 && kaInput == 4) || (sInput == 2 && xInput == 2) || (sInput == 2 && kaInput == 4) ||
        (xInput == 2 && kaInput == 4))
    {
        errorMsg = QString("S、X、Ka三个频段输入不允许同时设置前端信号源,只能有一个频段可设置!");
        return false;
    }

    return true;
}

bool ParamRelationOtherManager::checkParamCCZFRelationSU(const QMap<QString, QVariant>& setParamData, QString& errorMsg)
{
    auto date = setParamData.value("Date").toString();
    if (date.isEmpty())
    {
        errorMsg = QString("日期不能为空");
        return false;
    }

    if (date.at(0) != 'U' && date.at(0) != 'B')
    {
        errorMsg = QString("日期设置首字符必须为U(世界时)或者B(北京时),请重新输入！");
        return false;
    }

    return true;
}

bool ParamRelationOtherManager::checkCmdParamGSRelation(const QMap<QString, QVariant>& setParamData, QString& errorMsg)
{
    //获取I路时间和日期值
    auto accumBaseDate = QDate(2000, 1, 1);
    auto iStartDate = setParamData.value("IBackStartDate").toString();
    QVariant iStartDateInt;
    auto iStarTime = setParamData.value("IBackStartTime").toString();
    QVariant iStartTimeInt;
    auto iEndDate = setParamData.value("IBackEndDate").toString();
    QVariant iEndDateInt;
    auto iEndTime = setParamData.value("IBackEndTime").toString();
    QVariant iEndTimeInt;
    if (!iStartDate.isEmpty())
    {
        auto paramDate = QDate::fromString(iStartDate, "yyyy年MM月dd日");
        iStartDateInt = accumBaseDate.daysTo(paramDate) + 1;
    }

    if (!iStarTime.isEmpty())
    {
        auto paramTime = QTime::fromString(iStarTime, "hh时mm分ss秒zzz毫秒");
        iStartTimeInt = paramTime.msecsSinceStartOfDay();
    }

    if (!iEndDate.isEmpty())
    {
        auto paramDate = QDate::fromString(iEndDate, "yyyy年MM月dd日");
        iEndDateInt = accumBaseDate.daysTo(paramDate) + 1;
    }

    if (!iEndTime.isEmpty())
    {
        auto paramTime = QTime::fromString(iEndTime, "hh时mm分ss秒zzz毫秒");
        iEndTimeInt = paramTime.msecsSinceStartOfDay();
    }
    //获取Q路时间和日期值
    auto qStartDate = setParamData.value("QBackStartDate").toString();
    QVariant qStartDateInt;
    auto qStarTime = setParamData.value("QBackStartTime").toString();
    QVariant qStartTimeInt;
    auto qEndDate = setParamData.value("QBackEndDate").toString();
    QVariant qEndDateInt;
    auto qEndTime = setParamData.value("QBackEndTime").toString();
    QVariant qEndTimeInt;
    if (!qStartDate.isEmpty())
    {
        auto paramDate = QDate::fromString(qStartDate, "yyyy年MM月dd日");
        qStartDateInt = accumBaseDate.daysTo(paramDate) + 1;
    }

    if (!qStarTime.isEmpty())
    {
        auto paramTime = QTime::fromString(qStarTime, "hh时mm分ss秒zzz毫秒");
        qStartTimeInt = paramTime.msecsSinceStartOfDay();
    }

    if (!qEndDate.isEmpty())
    {
        auto paramDate = QDate::fromString(qEndDate, "yyyy年MM月dd日");
        qEndDateInt = accumBaseDate.daysTo(paramDate) + 1;
    }

    if (!qEndTime.isEmpty())
    {
        auto paramTime = QTime::fromString(qEndTime, "hh时mm分ss秒zzz毫秒");
        qEndTimeInt = paramTime.msecsSinceStartOfDay();
    }

    if (iStartDateInt > iEndDateInt)
    {
        errorMsg = QString("I路开始日期不能晚于I路结束日期");
        return false;
    }
    else if (iStartDateInt == iEndDateInt)
    {
        if (iStartTimeInt > iEndTimeInt)
        {
            errorMsg = QString("I路开始时间不能晚于I路结束时间");
            return false;
        }
    }

    if (qStartDateInt > qEndDateInt)
    {
        errorMsg = QString("Q路开始日期不能晚于Q路结束日期");
        return false;
    }
    else if (qStartDateInt == qEndDateInt)
    {
        if (qStartTimeInt > qEndTimeInt)
        {
            errorMsg = QString("Q路开始时间不能晚于Q路结束时间");
            return false;
        }
    }

    return true;
}

bool ParamRelationOtherManager::checkCmdParamCCZFRelation(const QMap<QString, QVariant>& setParamData, QString& errorMsg, int cmdID,
                                                          DeviceID& deviceID)
{
    if ((deviceID.sysID == 8 && deviceID.devID == 1 && cmdID == 1) || (deviceID.sysID == 8 && deviceID.devID == 1 && cmdID == 3))
    {
        auto startDatetimeStr = setParamData.value("StartTime").toString();
        auto startDatetime = QDateTime::fromString(startDatetimeStr, DATETIME_DISPLAY_FORMAT3);

        auto endDatetimeStr = setParamData.value("TEndTime").toString();
        auto endDatetime = QDateTime::fromString(endDatetimeStr, DATETIME_DISPLAY_FORMAT3);

        int relationTime = startDatetime.toTime_t() - endDatetime.toTime_t();

        if (relationTime > 0)
        {
            errorMsg = QString("任务开始时间不能晚于任务结束时间");
            return false;
        }
    }
    else if (deviceID.sysID == 8 && deviceID.devID == 1 && cmdID == 9)
    {
        auto startDatetimeStr = setParamData.value("StartTime").toString();
        auto startDatetime = QDateTime::fromString(startDatetimeStr, DATETIME_DISPLAY_FORMAT3);

        auto endDatetimeStr = setParamData.value("EndTime").toString();
        auto endDatetime = QDateTime::fromString(endDatetimeStr, DATETIME_DISPLAY_FORMAT3);

        int relationTime = startDatetime.toTime_t() - endDatetime.toTime_t();

        if (relationTime > 0)
        {
            errorMsg = QString("接收开始时间不能晚于接收结束时间");
            return false;
        }
    }

    return true;
}

bool ParamRelationOtherManager::checkParamSPZDRelationTFTU(const QMap<QString, QVariant>& setParamData, QString& errorMsg)
{
    int YearR = setParamData.value("YearR").toInt();    //年
    int MonthR = setParamData.value("MonthR").toInt();  //月
    int DayR = setParamData.value("DayR").toInt();      //日

    //闰年
    if (!((YearR % 4 == 0 && MonthR % 100 != 0) || YearR % 400 == 0) && MonthR == 2 && DayR == 29)
    {
        errorMsg = QString("%1是平年,2月只有28天!").arg(YearR);
        return false;
    }

    return true;
}
