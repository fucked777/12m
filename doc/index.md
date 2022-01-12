# 目录结构

[目录结构](./dir.md)

# 工具
***
[cmake](./cmake.md)


# 依赖库编译
***
[CppMicroServices](./cms_build.md)

[zmq](./libzmq_build.md)

[cpp_redis](./cpp_redis.md)


# 使用/编码说明

[使用/编码](./code.md)

# 注意事项

1. git大小不识别问题

```shell
git config core.ignorecase false

#全局
git config --global core.ignorecase false

```

# 程序配置

无论客户端还是服务器都需要将ConfigFile下的所有文件拷贝到bin目录下

## 数据库配置
配置文件 DBConnectInfo.xml
有两个节点 Default 和 DeviceStatus

Default 是所有业务的配置
DeviceStatus 是状态入库的数据库 截止20211228 状态数据库会崩溃

数据库里面的内容和odbc配置一致就行

## redis配置
配置文件 RedisConfig.xml

必须有密码 填写对应的ip端口即可

## 主备配置
配置文件: Heartbeat/HeartbeatConfig.xml

在windows上未配置 直接把节点 AIP BIP配置成localhost即可 

然后在服务器的Heartbeat模块将pro文件中的 
DEFINES += TEST_MASTER_SLAVE 取消注释

在windows下操作在启动程序后需要点击主备然后切换为主机才能使用