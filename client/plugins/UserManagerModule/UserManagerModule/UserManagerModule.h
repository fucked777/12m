#ifndef UserManagerModule_H
#define UserManagerModule_H

#include "LxTsspCommonDefinition.h"
#include "UserManagerModule_global.h"

class UserLoginWidget;
class UserRegisterWidget;
class UserListWidget;
class UserManagerModule : public ILxTsspObject
{
public:
    UserManagerModule(TLxTsspObjectInfo ObjectInfo);

public:
    /*--------------对象信息接口部分-------------------------------*/
    /**
     * 获取对象所属的模块信息：信息包括模块的公司名称、模块名称、配置标识、版本、开发人员等
     * ，用于实时运行时查看。
     * @param[out] ModuleInfo：模块信息。
     * @return 1：成功；0：未定义；-1：失败。
     */
    virtual int GetModuleInfo(TLxTsspModuleInfo& ModuleInfo) const;

    /**
     * 获取对象相关信息。对象相关信息包括对象名称，对象的分系统号、设备号、流号，
     * 本地对象还是远程对象，主用还是备用等。
     * @param[out] ObjInfo：对象信息。
     * @return 1：成功；0：未定义；-1：失败。
     */
    virtual int GetObjectInfo(TLxTsspObjectInfo& ObjectInfo) const;

    /*--------------对象控制接口部分-------------------------------*/
    /**
     * 预初始化：模块在此函数中执行必要的预初始化工作，如查找需要的对象，
     *  向对象管理器添加私有对象等。主用工作是准备对象运行的外界资源条件。
     * @return 1：成功；0：未定义；-1：失败。
     */
    virtual int PreInitialize();

    /**
     * 初始化 ：模块在此函数中执行必要的初始化工作。
     * @return 1：成功；0：未定义；-1：失败。
     */
    virtual int Initialize();
    /**
     * 启动：模块在此函数中启动功能的执行，尤其是包含线程的模块，应该在此启动线程；
     * @return 1：成功；0：未定义；-1：失败。
     */
    virtual int Start();

    /**
     * 停止 ：模块在此函数中停止功能的执行，尤其是包含线程的模块，应该在此结束线程。
     * @return 1：成功；0：未定义；-1：失败。
     */
    virtual int Stop();

    /**
     * 清理：模块在此函数中清理申请的资源，为进程退出做准备。
     * @return 1：成功；0：未定义；-1：失败。
     */
    virtual bool ClearUp();

    /*--------------对象故障诊断接口部分----------------------------*/
    /**
     * 取错误码： 取对象的错误码。
     * @return： 返回模块的错误码，用于判断模块不能执行的原因:
     *           0表示运行正常；-1表示没有配置信息；-2表示初始化失败；
     *           -3表示启动操作失败；-4表示停止操作失败；-5表示清理资源失败；
     *           其它错误依情况增加；
     */
    virtual int GetErrorNumber();

    /**
     * 取错误信息
     * @param iErrorNum：错误码。
     * @param[out] szErrorMsg：错误信息，描述性的错误信息，用于判断对象不能执行的原因。
     * @return 1：成功；0：未定义；-1：表示没有对应的错误信息；
     */
    virtual int GetErrorMsg(int iErrorNo, QString& strErrorMsg);

    /**
     * 是否在线 ：对于远程对象可能存在不在线情况，普通模块直接返回true即可。
     *          对于代理模块需要判断远程实际对象响应情况、通道通断情况来判断。
     * @return true：在线；false：不在线。
     */
    virtual bool IsOnline();

    /**
     * 发送消息 ：通过此函数可实现信息的直接发送，对于对象管理器，当此函数被调用时，
     *          向管理的所有对象发送此消息。
     * @param msg：消息。对象收到消息后，可以拷贝消息内容，但不应更改。
     */
    virtual void SendMessage(TLxTsspMessage msg);

    /**
     * 设置组参数 ：通过此函数把参数设置给对象，设备对象加载到具体设备；
     * @param parameterList：参数块列表,
     * quint64参数块标识，格式为：
     * 站号(2)/分系统号(1)/单机号(2)/数据块标识(2)/序号(1)。
     * 站号(2)/分系统号(1)/单机号(2)不用时填0。
     * @return 1：成功；-1：失败。
     */
    virtual int SetParameter(QMap<quint64, QByteArray> parameterList);

    /**
     * 设置参数 ：设置单个参数。
     * @param ui64ParamID：参数ID,格式为：
     * 站号(2)/分系统号(1)/单机号(2)/数据块标识(2)/序号(1)。
     * 站号(2)/分系统号(1)/单机号(2)不用时填0。
     * @param parameter：参数块数据
     * @return 1：成功；0：非本对象参数；-1：失败。
     */
    virtual int SetParameter(quint64 ui64ParamID, QByteArray parameter);

    /**
     * 取对象的参数ID ：取本对象需要的参数块的ID。在参数宏加载时，
     *      调用此函数确定对象需要的参数块，然后从参数宏中取出相应的参数，设置给该对象。
     * @param[out] pParameteIDList：参数块ID的列表。参数ID,格式为：
     * 站号(2)/分系统号(1)/单机号(2)/数据块标识(2)/序号(1)。
     * 站号(2)/分系统号(1)/单机号(2)不用时填0。
     * @return 大于等于0：参数块个数。-1：失败。
     */
    virtual int GetParameterID(QList<quint64>& pParameterIDList) const;

    /**
     * 取参数 ：获取本对象的参数，可以一次读取所有参数。
     * @param[out] parameterList：参数块ID和参数值的键值对的列表。参数ID,格式为：
     * 站号(2)/分系统号(1)/单机号(2)/数据块标识(2)/序号(1)。
     * 站号(2)/分系统号(1)/单机号(2)不用时填0。
     * @return 大于等于0：参数块个数。-1：失败。
     */
    virtual int GetParameter(QMap<quint64, QByteArray>& parameterList) const;

    /**
     * 取参数 ：获取本对象的参数，按照指定的ID读取。
     * @param ui64ParamID：参数块ID。格式为：
     * 站号(2)/分系统号(1)/单机号(2)/数据块标识(2)/序号(1)。
     * 站号(2)/分系统号(1)/单机号(2)不用时填0。
     * @param[out] parameter：参数块的值；
     * @return 1：成功；-1：失败。
     */
    virtual int GetParameter(quint64 ui64ParamID, QByteArray& parameter) const;

    /**
     * 获取状态 ：获取本对象的状态，可以一次读取所有状态。
     * @param[out] stateList：状态块ID和状态值的键值对的列表。quint64为状态号，格式为：
     * 站号(2)/分系统号(1)/单机号(2)/数据块标识(2)/序号(1)。
     * 站号(2)/分系统号(1)/单机号(2)不用时填0。
     * 状态块标识(2)/序号(1)为必填项，其它根据需要填写。
     * @return 大于等于0：状态块个数。-1：失败。
     */
    virtual int GetState(QMap<quint64, TLxTsspDeviceState>& stateList) const;

    /**
     * 取单个状态 ：根据状态ID取相应的状态，注意如果参数不加载到硬件，也要作为状态返回。
     * @param ui64StatusID:状态块ID。格式为：
     * 站号(2)/分系统号(1)/单机号(2)/数据块标识(2)/序号(1)。
     * 站号(2)/分系统号(1)/单机号(2)不用时填0。
     * @param[out] status：状态值。
     * @return 1：成功；-1：失败。
     */
    virtual int GetState(quint64 ui64StatusID, TLxTsspDeviceState& status) const;

    /**
     * 直接取状态 ：直接从硬件读取状态，不取当前缓存的状态，强调实时性时使用。
     * @param ui64StatusID:状态块ID。格式为：
     * 站号(2)/分系统号(1)/单机号(2)/数据块标识(2)/序号(1)。
     * 站号(2)/分系统号(1)/单机号(2)不用时填0。
     * @param[out] status：状态值。
     * @return 1：成功；-1：失败。
     */
    virtual int GetStateDirect(quint64 ui64StatusID, TLxTsspDeviceState& status);

    /**
     * 向对象写数据 ：向对象写数据
     * @param ui64DataID：数据ID；格式如下：
     * 站号(2)/分系统号(1)/单机号(2)/数据块标识(2)/序号(1)。
     * 站号(2)/分系统号(1)/单机号(2)不用时填0。
     * @param uiDataSn：数据流号；
     * @param aData：数据缓冲区；
     * @param uiDataLen：数据字节长度。
     * @return 大于等于0：实际写入的数据长度；-1：失败。
     */
    virtual int WriteData(quint64 ui64DataID, uchar* aData, uint uiDataLen);

    /**
     * @brief ReadData从模块读取数据 ：从模块读取数据
     * @param ui64DataID,格式如下：
     * 站号(2)/分系统号(1)/单机号(2)/数据块标识(2)/序号(1)。
     * 站号(2)/分系统号(1)/单机号(2)不用时填0。
     * @param aData：数据缓冲区；
     * @param uiDataLen：数据缓冲区字节长度。
     * @param uiReloadCount：加载计数，计数变化意味着状态的改变。
     * @return  大于等于0：实际读取的数据长度；-1：失败。
     */
    virtual int ReadData(quint64 ui64DataID, uchar* aData, uint& uiDataLen, uint& uiReloadCount);

protected:
    //对象信息
    TLxTsspObjectInfo m_ObjectInfo;
    //错误列表
    QMap<int, QString> m_ErrInfoList;
    //错误代码
    int m_iErrCode;
    //参数列表
    QMap<quint64, QByteArray> m_ParamList;
    //状态列表
    QMap<quint64, TLxTsspDeviceState> m_StateList;

    UserLoginWidget* mUserLoginWidget = nullptr;
    UserRegisterWidget* mUserRegisterWidget = nullptr;
    UserListWidget* mUserListWidget = nullptr;
};

#endif  // MODULETEMPLATE_H
