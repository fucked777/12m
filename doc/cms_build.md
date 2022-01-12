#  CppMicroServices编译

代码仓库地址 : https://github.com/CppMicroServices/CppMicroServices

版本        : 3.x
***

```Shell
# Windows
# -DUS_ENABLE_THREADING_SUPPORT 要改为FALSE  因为这个框架本身是多线程
# 在QT中线程的线程创建的对象没有事件循环
cd CppMicroServices
md build
cd build
cmake -G "Visual Studio 16 2019" -A x64  ..
devenv  CppMicroServices.sln /Build "Debug|x64"
也可以直接打开目录双击CppMicroServices.sln使用Visual Studio编译

-G  后面根据实际的msvc编译器版本填写

# Linux
cd CppMicroServices
mkdir build
cd build
cmake -DUS_ENABLE_THREADING_SUPPORT=FALSE -DCMAKE_BUILD_TYPE=Debug .. 或者 cmake -DUS_ENABLE_THREADING_SUPPORT=FALSE -DCMAKE_BUILD_TYPE=Release ..
make

```


# 错误解决
## 错误1

```Text
   节点 1 上的项目“D:\Build\CppMicroServices\bbb\CMakeFiles\3.18.4\VCTargetsPath.vcxproj”(默认目标)。
    C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\MSBuild\Current\Bin\Microsoft.Common.CurrentVersion.targets(780,5): error : The OutputPath property is not set for project 'VCTargetsPath.vcxproj'.  Please check to make sure that you have specified a valid combination of Configuration and Platform for this project.  Configuration='Debug'  Platform='Win64'.  You may be seeing this message because you are trying to build a project without a solution file, and have specified a non-default Configuration or Platform that doesn't exist for this project. [D:\Build\CppMicroServices\bbb\CMakeFiles\3.18.4\VCTargetsPath.vcxproj]
    已完成生成项目“D:\Build\CppMicroServices\bbb\CMakeFiles\3.18.4\VCTargetsPath.vcxproj”(默认目标)的操作 - 失败。

    生成失败。

    “D:\Build\CppMicroServices\bbb\CMakeFiles\3.18.4\VCTargetsPath.vcxproj”(默认目标) (1) ->
    (_CheckForInvalidConfigurationAndPlatform 目标) ->
      C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\MSBuild\Current\Bin\Microsoft.Common.CurrentVersion.targets(780,5): error : The OutputPath property is not set for project 'VCTargetsPath.vcxproj'.  Please check to make sure that you have specified a valid combination of Configuration and Platform for this project.  Configuration='Debug'  Platform='Win64'.  You may be seeing this message because you are trying to build a project without a solution file, and have specified a non-default Configuration or Platform that doesn't exist for this project. [D:\Build\CppMicroServices\bbb\CMakeFiles\3.18.4\VCTargetsPath.vcxproj]
```
```Text
在msvc2017之前是这么写的

cmake -G "Visual Studio 15 2017" -A x64
cmake -G "Visual Studio 15 2017 Win64"

msvc2019只能这么写了
cmake -G "Visual Studio 15 2019" -A x64

```

## 错误2

```Text
CMake Error: Error: generator platform: x64
Does not match the platform used previously: Win64
Either remove the CMakeCache.txt file and CMakeFiles directory or choose a different binary directory.

```

```Text
删除缓存重来
```

## 错误3
```Text
错误	C2220	以下警告被视为错误
```
```Text
在代码根目录 找到CMakeLists.txt文件
在文件中查找 /WX  然后删除这个编译选项
大约在409行

然后清除缓存重新cmake

Linux 下查找 -Werror 然后删除这个编译选项
```

