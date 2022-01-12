# libzmq编译
***

代码仓库地址 : https://github.com/zeromq/libzmq

版本        : 4.x
***

```Shell
# Windows
cd libzmq
md build
cd build
cmake -G "Visual Studio 16 2019" -A x64  ..
devenv  ZeroMQ.sln /Build "Debug|x64"
也可以直接打开目录双击ZeroMQ.sln使用Visual Studio编译

-G  后面根据实际的msvc编译器版本填写

# Linux
cd libzmq
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug .. 或者 cmake -DCMAKE_BUILD_TYPE=Release ..
make

```


## 错误1
```Text
当前错误是在ubuntu下出现的
CMake Error at /usr/share/cmake-3.10/Modules/FindPackageHandleStandardArgs.cmake:137 (message):
  Could NOT find PkgConfig (missing: PKG_CONFIG_EXECUTABLE)
Call Stack (most recent call first):
  /usr/share/cmake-3.10/Modules/FindPackageHandleStandardArgs.cmake:378 (_FPHSA_FAILURE_MESSAGE)
  /usr/share/cmake-3.10/Modules/FindPkgConfig.cmake:36 (find_package_handle_standard_args)
  builds/cmake/Modules/FindSodium.cmake:7 (find_package)
  CMakeLists.txt:267 (find_package)
```

```Text
apt install pkg-config

如果其他linux平台出现类似错误也是同样的方法缺啥安啥
没有的就下载源码安装
```


