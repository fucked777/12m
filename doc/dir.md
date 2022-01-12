
```
.
├── business                                            客户端与服务端公共的引用部分
├── doc                                                 文档
├── README.md                                           文档入口
├── script                                              CI/CD相关脚本
├── client                                              客户端结构同服务端
├── server                                              服务端
│   ├── bin                                             生成的结果文件
│   ├── ConfigFile                                      
│   │   ├── Platform                                    平台的配置文件
│   │   └── Resources                                   业务相关的配置文件
│   ├── core                                            平台源码(20210206暂时的还没分离出去)
│   ├── Framework.pro                                   主工程文件
│   ├── lib                                             依赖库
│   │   ├── Bak                                         备份库
│   │   ├── Linux                                       注意此linux在不同linux平台使用的时应留意库是否可用 比如ubuntu与中标麒麟(龙芯版)库是不通用的
│   │   ├── Src                                         当前的所有的库对应的源码,如果升级库请注意一定要替换头文件
│   │   └── Win
│   ├── plugins                                         模块源码
│   │   ├── Global                                      此目录是模块的全局引用目录
│   ├── public                                          此目录是平台的开放公共接口目录
│   └── tools                                           此工具与代码的生成相关 比如usResourceCompiler3是cppmicroservices的框架相关工具
├── test                                                测试代码/测试工程/测试例程
├── tool                                                外部工具(例如:mips版本的cmake...)
└── wizards                                             qt工程向导

```