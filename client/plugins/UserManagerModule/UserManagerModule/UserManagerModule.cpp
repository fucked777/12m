#include "UserManagerModule.h"

#include <QApplication>
#include <QCoreApplication>
#include <QHBoxLayout>

#include "GlobalUserData.h"
#include "IUiModuleManager.h"
#include "UserListWidget.h"
#include "UserLoginWidget.h"
#include "UserRegisterWidget.h"

UserManagerModule::UserManagerModule(TLxTsspObjectInfo ObjectInfo)
{
    //初始化
    memcpy(&m_ObjectInfo, &ObjectInfo, sizeof(TLxTsspObjectInfo));

    //错误代码初始化为无错误
    m_iErrCode = 0;

    //对于本对象的错误列表进行初始化
    m_ErrInfoList.insert(0, QString("无错误"));
    //根据对象实际情况添加。
    // m_ErrInfoList.insert(1, "...");
}

/*--------------对象信息接口部分-------------------------------*/
/**
 * 获取对象所属的模块信息：信息包括模块的公司名称、模块名称、配置标识、版本、开发人员等
 * ，用于实时运行时查看。
 * @param[out] ModuleInfo：模块信息。
 * @return 1：成功；0：未定义；-1：失败。
 */
int UserManagerModule::GetModuleInfo(TLxTsspModuleInfo& moduleInfo) const
{
    memset(&moduleInfo, 0, sizeof(TLxTsspModuleInfo));

    // 请按照实际模块的名称、标识、版本、开发人员填写。
    /// 公司名称,固定为“中国航天科技集团九院七〇四所”
    strcpy(moduleInfo.szCompany, QString("中国航天科技集团九院七〇四所").toUtf8().data());

    /// 模块名称，如：系统工作参数设置
    strcpy(moduleInfo.szModuleName, QString("UserManagerModule").toUtf8().data());

    /// 配置标识，如：D_Y17-61_008
    strcpy(moduleInfo.szConfigID, QString("UM_001").toUtf8().data());

    /// 版本
    strcpy(moduleInfo.szVersion, QString("1.00").toUtf8().data());

    /// 开发者，如：大卫.奥巴马
    strcpy(moduleInfo.szCreator, QString("LiuLin").toUtf8().data());

    /// 备注
    strcpy(moduleInfo.szComment, QString("客户端日志管理模块").toUtf8().data());

    return 1;
}

/**
 * 获取对象相关信息。对象相关信息包括对象名称，对象的分系统号、设备号、流号，
 * 本地对象还是远程对象，主用还是备用等。
 * @param[out] ObjInfo：对象信息。
 * @return 1：成功；0：未定义；-1：失败。
 */
int UserManagerModule::GetObjectInfo(TLxTsspObjectInfo& ObjectInfo) const
{
    memcpy(&ObjectInfo, &m_ObjectInfo, sizeof(TLxTsspObjectInfo));

    return 1;
}

/*--------------对象控制接口部分-------------------------------*/
/**
 * 预初始化：模块在此函数中执行必要的预初始化工作，如查找需要的对象，
 *  向对象管理器添加私有对象等。主用工作是准备对象运行的外界资源条件。
 * @return 1：成功；0：未定义；-1：失败。
 */
int UserManagerModule::PreInitialize() { return 1; }

/**
 * 初始化 ：模块在此函数中执行必要的初始化工作。
 * @return 1：成功；0：未定义；-1：失败。
 */
int UserManagerModule::Initialize()
{
    GlobalUserData* globalUserData = new GlobalUserData();
    LxTsspObjectManager::Instance()->AddObject(GLOBAL_USER_DATA_MANAGER_OBJ_ID, globalUserData);

    return 1;
}

/**
 * 启动：模块在此函数中启动功能的执行，尤其是包含线程的模块，应该在此启动线程；
 * @return 1：成功；0：未定义；-1：失败。
 */
int UserManagerModule::Start()
{
    mUserListWidget = new UserListWidget();
    LxTsspObjectManager::Instance()->AddObject(USERLIST_MANAGER_OBJ_ID, mUserListWidget);

    mUserLoginWidget = new UserLoginWidget();
    LxTsspObjectManager::Instance()->AddObject(USERLOGIN_MANAGER_OBJ_ID, mUserLoginWidget);

    mUserRegisterWidget = new UserRegisterWidget();
    LxTsspObjectManager::Instance()->AddObject(0x000F30FA, mUserRegisterWidget);
    //    QDialog dialog;
    //    QHBoxLayout* layout = new QHBoxLayout();
    //    layout->addWidget(new UserWidget());
    //    dialog.setLayout(layout);

    //    if (dialog.exec() != QDialog::Accepted)
    //    {
    //        return 1;
    //    }
    //    else
    //    {
    //        //#if defined(Q_OS_WIN32) || defined(Q_WS_WIN)
    //        //        qint64 pid = QCoreApplication::applicationPid();  //获取当前进程的PID
    //        //        /**
    //        //         * 强制杀死当前进程，同时推出所有子线程
    //        //         * TASKKILL 参数解释：
    //        //         * /T : 杀死当前进程启动和其启动的所有子线程
    //        //         * /F: 强制杀死进程
    //        //         * */
    //        //        QString cmd = QString("TASKKILL /PID %1 /F").arg(pid);  // Windows 系统下，杀死当前进程命令，不杀死其开启的子进程
    //        //        //    QString cmd = QString("TASKKILL /PID %1 /T /F").arg(pid);//Windows 系统下，杀死当前进程树，包括其开启的子进程

    //        //        //@ 可以，无dos黑窗口
    //        //        // WinExec(cmd.toLocal8Bit().data(), 0);  // SW_HIDE：控制运行cmd时，不弹出cmd运行窗口

    //        //        QProcess p;

    //        //        p.execute(cmd);
    //        //        p.close();

    //        //#else
    //        //        qint64 pid = QCoreApplication::applicationPid();
    //        //        QProcess::startDetached("kill -9 " + QString::number(pid));
    //        //#endif
    //        return 1;
    //    }
    return 1;
}

/**
 * 停止 ：模块在此函数中停止功能的执行，尤其是包含线程的模块，应该在此结束线程。
 * @return 1：成功；0：未定义；-1：失败。
 */
int UserManagerModule::Stop() { return 1; }

/**
 * 清理：模块在此函数中清理申请的资源，为进程退出做准备。
 * @return 1：成功；0：未定义；-1：失败。
 */
bool UserManagerModule::ClearUp()
{
    if (NULL != mUserListWidget)
    {
        delete mUserListWidget;
        mUserListWidget = NULL;
    }

    if (NULL != mUserLoginWidget)
    {
        delete mUserLoginWidget;
        mUserLoginWidget = NULL;
    }

    if (NULL != mUserRegisterWidget)
    {
        delete mUserRegisterWidget;
        mUserRegisterWidget = NULL;
    }

    return 1;
}

/*--------------对象故障诊断接口部分----------------------------*/
/**
 * 取错误码： 取对象的错误码。
 * @return： 返回模块的错误码，用于判断模块不能执行的原因:
 *           0表示运行正常；-1表示没有配置信息；-2表示初始化失败；
 *           -3表示启动操作失败；-4表示停止操作失败；-5表示清理资源失败；
 *           其它错误依情况增加；
 */
int UserManagerModule::GetErrorNumber() { return m_iErrCode; }

/**
 * 取错误信息
 * @param iErrorNum：错误码。
 * @param[out] szErrorMsg：错误信息，描述性的错误信息，用于判断对象不能执行的原因。
 * @return 1：成功；0：未定义；-1：表示没有对应的错误信息；
 */
int UserManagerModule::GetErrorMsg(int iErrorNo, QString& strErrorMsg)
{
    if (m_ErrInfoList.contains(iErrorNo))
    {
        strErrorMsg = m_ErrInfoList[iErrorNo];
        return 1;
    }
    else
    {
        strErrorMsg = QString("无对应错误信息");
        return -1;
    }
}

/**
 * 是否在线 ：对于远程对象可能存在不在线情况，普通模块直接返回true即可。
 *          对于代理模块需要判断远程实际对象响应情况、通道通断情况来判断。
 * @return true：在线；false：不在线。
 */
bool UserManagerModule::IsOnline() { return true; }

/**
 * 发送消息 ：通过此函数可实现信息的直接发送，对于对象管理器，当此函数被调用时，
 *          向管理的所有对象发送此消息。
 * @param msg：消息。对象收到消息后，可以拷贝消息内容，但不应更改。
 */
void UserManagerModule::SendMessage(TLxTsspMessage msg)
{
    //处理消息

    return;
}

/**
 * 设置组参数 ：通过此函数把参数设置给对象，设备对象加载到具体设备；
 * @param parameterList：参数块列表,
 * quint64参数块标识，格式为：
 * 站号(2)/分系统号(1)/单机号(2)/数据块标识(2)/序号(1)。
 * 站号(2)/分系统号(1)/单机号(2)不用时填0。
 * @return 1：成功；-1：失败。
 */
int UserManagerModule::SetParameter(QMap<quint64, QByteArray> parameterList)
{
    //对于参数组中的参数，首先判断是否本对象参数
    foreach (quint64 key, parameterList.keys())
    {
        //参数加载到硬件、本地保存、本地使用。
    }

    return 1;
}

/**
 * 设置参数 ：设置单个参数。
 * @param ui64ParamID：参数ID,格式为：
 * 站号(2)/分系统号(1)/单机号(2)/数据块标识(2)/序号(1)。
 * 站号(2)/分系统号(1)/单机号(2)不用时填0。
 * @param parameter：参数块数据
 * @return 1：成功；0：非本对象参数；-1：失败。
 */
int UserManagerModule::SetParameter(quint64 ui64ParamID, QByteArray parameter)
{
    if (m_ParamList.contains(ui64ParamID))
    {
        m_ParamList[ui64ParamID] = parameter;
        //参数加载到硬件、本地保存、本地使用。
        return 1;
    }

    return 0;
}

/**
 * 取对象的参数ID ：取本对象需要的参数块的ID。在参数宏加载时，
 *      调用此函数确定对象需要的参数块，然后从参数宏中取出相应的参数，设置给该对象。
 * @param[out] pParameteIDList：参数块ID的列表。
 * @return 大于等于0：参数块个数。-1：失败。
 */
int UserManagerModule::GetParameterID(QList<quint64>& pParameterIDList) const
{
    pParameterIDList += m_ParamList.keys();

    return m_ParamList.count();
}

/**
 * 取参数 ：获取本对象的参数，可以一次读取所有参数。
 * @param[out] parameterList：参数块ID和参数值的键值对的列表。
 * @return 大于等于0：参数块个数。-1：失败。
 */
int UserManagerModule::GetParameter(QMap<quint64, QByteArray>& parameterList) const
{
    parameterList.unite(m_ParamList);

    return m_ParamList.count();
}

/**
 * 取参数 ：获取本对象的参数，按照指定的ID读取。
 * @param ui64ParamID：参数块ID。
 * 站号(2)/分系统号(1)/单机号(2)/数据块标识(2)/序号(1)。
 * 站号(2)/分系统号(1)/单机号(2)不用时填0。
 * @param[out] parameter：参数块的值；
 * @return 1：成功；-1：失败。
 */
int UserManagerModule::GetParameter(quint64 ui64ParamID, QByteArray& parameter) const
{
    if (m_ParamList.contains(ui64ParamID))
    {
        parameter = m_ParamList[ui64ParamID];
        return 1;
    }

    return -1;
}

/**
 * 获取状态 ：获取本对象的状态，可以一次读取所有状态。
 * @param[out] stateList：状态块ID和状态值的键值对的列表。quint64为状态号，格式为：
 * 站号(2)/分系统号(1)/单机号(2)/数据块标识(2)/序号(1)。
 * 站号(2)/分系统号(1)/单机号(2)不用时填0。
 * @return 大于等于0：状态块个数。-1：失败。
 */
int UserManagerModule::GetState(QMap<quint64, TLxTsspDeviceState>& stateList) const
{
    stateList.unite(m_StateList);

    return m_StateList.count();
}

/**
 * 取单个状态 ：根据状态ID取相应的状态，注意如果参数不加载到硬件，也要作为状态返回。
 * @param ui64StatusID:状态块ID。
 * 站号(2)/分系统号(1)/单机号(2)/数据块标识(2)/序号(1)。
 * 站号(2)/分系统号(1)/单机号(2)不用时填0。
 * @param[out] status：状态值。
 * @return 1：成功；-1：失败。
 */
int UserManagerModule::GetState(quint64 ui64StatusID, TLxTsspDeviceState& status) const
{
    if (m_StateList.contains(ui64StatusID))
    {
        status = m_StateList[ui64StatusID];
        return 1;
    }
    return -1;
}

/**
 * 直接取状态 ：直接从硬件读取状态，不取当前缓存的状态，强调实时性时使用。
 * @param ui64StatusID:状态块ID。
 * @param[out] status：状态值。
 * @return 1：成功；-1：失败。
 */
int UserManagerModule::GetStateDirect(quint64 ui64StatusID, TLxTsspDeviceState& status)
{
    if (m_StateList.contains(ui64StatusID))
    {
        //
    }

    return 1;
}

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
int UserManagerModule::WriteData(quint64 ui64DataID, uchar* aData, uint uiDataLen) { return 0; }

/**
 * 从模块读取数据 ：从模块读取数据
 * @param ui64DataID：数据ID；
 * 站号(2)/分系统号(1)/单机号(2)/数据块标识(2)/序号(1)。
 * 站号(2)/分系统号(1)/单机号(2)不用时填0。
 * @param[out] aData：数据缓冲区；
 * @param[out] uiDataLen：数据缓冲区字节长度。
 * @param[out] uiReloadCount：加载计数，计数变化意味着状态的改变。
 * @return 大于等于0：实际读取的数据长度；-1：失败。
 */
int UserManagerModule::ReadData(quint64 ui64DataID, uchar* aData, uint& uiDataLen, uint& uiReloadCount) { return 0; }
