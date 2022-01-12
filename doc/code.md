# 配置说明
## 平台配置
``` shell
# <Main> 主配置部分是固定不变的,代码写死了
#     <InstanceID>Server</InstanceID>                       实例ID 与程序单例相关
#     <PlatformConfigDir>Platform</PlatformConfigDir>       平台配置文件目录,代码中的平台配置目录都是其子目录
#     <BusinessConfigDir>Resources/4412</BusinessConfigDir> 与业务相关的配置文件的目录
# </Main>
# <Extended> 扩展配置 格式固定内容随意 
#     格式统一为
#     <Key>Value</Key>
#     <LocalIP>192.168.137.77</LocalIP>                 本地IP地址
#     <CommunicationIP>192.168.137.77</CommunicationIP> 通讯IP windows上使用
#     <CommunicationPort>9999</CommunicationPort>       通讯端口 windows上使用
#     <CommunicationFile>localmsg.ipc</CommunicationFile> 通讯文件 Linux上使用
# </Extended>
```

## 模块加载
## ui加载


# 编码说明
## ZMQ

### 逻辑代码框架

客户端与服务器的交互全部通过ZMQ转发来完成

转发是ZMQRouterServer模块来完成的

其他的模块需要使用只需要继承INetMsg即可

只有通过registerSubscribe注册的函数在通信时

才能被指定的key调用


```cpp

class Demo:public INetMsg
{
public:
    Demo(cppmicroservices::BundleContext context);
    ~Demo();
};
Demo::Demo(cppmicroservices::BundleContext context)
    : INetMsg(nullptr)
{
    registerSubscribe("test1",&Demo::test1,this);
    registerSubscribe("test2",&Demo::test2,this);
    registerSubscribe("test3",&Demo::test3,this);
    registerSubscribe("test4",&Demo::test4,this);
    registerSubscribe("test5",&Demo::test5,this);
}

```

### 启动服务
当功能模块完成后,需要让他运行起来

需要在框架的接口Start中书写
```cpp
    class US_ABI_LOCAL DemoActivator : public BundleActivator
    {
    public:
        void Start(BundleContext context)
        {
            /* 创建并注册对象 */
            auto server = std::make_shared<Demo>(context);
            ServiceProperties props;
            props["Type"] = std::string("Demo");
            context.RegisterService<Demo>(server, props);

            /* 这里是通过平台的扩展配置读取相关通讯的信息
             * 这里要注意服务端和客户端在windows中通讯端口不能一样,否则结果是未定义的
             * linux中通讯文件只要不是同一个文件,同名是不影响的
             */
#ifdef Q_OS_WIN
            auto communicationIP = PlatformTools::valueStr("CommunicationIP");
            auto communicationPort = PlatformTools::valueStr("CommunicationPort").toUShort();
            auto connectAddr = ZMQHelper::syntheticAddress("tcp", communicationIP, communicationPort);
#else
            auto communicationFile = PlatformTools::valueStr("communicationFile");
            auto connectAddr = ZMQHelper::syntheticAddress("ipc", communicationFile);

#endif
            /* 设置连接地址 */
            server->setConnectAddr(connectAddr);
            /* 设置源地址 别的模块就是通过此地址来发送消息的 */
            server->setSrcAddr("Demo");
            /* 最后启动服务 */
            QString msg;
            if (!server->startServer(&msg))
            {
                qWarning() << msg;
            }
        }

        void Stop(BundleContext /*context*/)
        {
            // NOTE: The service is automatically unregistered
        }
    };

```

### 发送消息

发送消息分为两种
一种是主动发送
一种是应答

```cpp
// 主动发送的消息调用send函数就行
// silenceSend是个宏,错误时把错误信息打印出来了

ProtocolPack sendPack;
sendPack.desID = "Demo";                // 给谁发
sendPack.operation = "getAll";          // 发过去调用对方什么方法
sendPack.operationACK = "getAllACK" ;   // 应答时调用本类什么方法
sendPack.module = true;                 // 是服务器或者客户端的模块自己给自己发消息,还是两者对发送
sendPack.data = data;                   // 附带的数据
silenceSend(sendPack);                  // 发送


// 应答时可以手动拼装消息,也可以直接将收到的数据包处理一下回发
// sendACK 只是简单的把operation与operationACK互换了一下
// silenceSendACK是个宏,错误时把错误信息打印出来了

auto recvPack=pack;/* 收到的数据包 */
/* 做了一堆事儿 */
recvPack.data=ACKData;/* 回复的数据,此处尽量不要更改别的成员,否则的话手动拼包更好一些 */
silenceSendACK(recvPack); // 发送

```

### 消息接收

现在总体逻辑没问题,有bug还在想
现在总体逻辑没问题,有bug还在想
现在总体逻辑没问题,有bug还在想
现在总体逻辑没问题,有bug还在想
现在总体逻辑没问题,有bug还在想

