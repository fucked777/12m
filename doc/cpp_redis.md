# cpp_redis编译
***

代码仓库地址 : https://github.com/cpp-redis/cpp_redis

版本        : 4.3.x
***

```Shell
# Windows
cd cpp_redis
md build
cd build
cmake -G "Visual Studio 16 2019" -A x64  ..
devenv  cpp_redis.sln /Build "Debug|x64"
也可以直接打开目录双击cpp_redis.sln使用Visual Studio编译

-G  后面根据实际的msvc编译器版本填写

# Linux
cd cpp_redis
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug .. 或者 cmake -DCMAKE_BUILD_TYPE=Release ..
make

```


## 错误1
```Text
CMake Error at CMakeLists.txt:216 (add_subdirectory):
  add_subdirectory given source "tacopie" which is not an existing directory.
```

```Text
git submodule update --init --recursive
下载子模块 也可以手动去下载
```


