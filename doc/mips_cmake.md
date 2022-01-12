# uname -a
Linux slave 3.10.0 #1 SMP PREEMPT Thu Dec 5 16:01:33 CST 2019 00001-g8a289a9 mips64 mips64 mips64 GNU/Linux
# gcc -v
```Text
使用内建 specs。
COLLECT_GCC=gcc
COLLECT_LTO_WRAPPER=/usr/libexec/gcc/mips64el-neokylin-linux/4.9.3/lto-wrapper
目标：mips64el-neokylin-linux
配置为：../configure --prefix=/usr --mandir=/usr/share/man --infodir=/usr/share/info --with-bugurl=http://bugzilla.cs2c.com/bugzilla --enable-bootstrap --enable-shared --enable-threads=posix --enable-checking=release --build=mips64el-neokylin-linux --enable-multilib --with-system-zlib --enable-__cxa_atexit --disable-libunwind-exceptions --enable-gnu-unique-object --enable-linker-build-id --with-arch=loongson3a --enable-languages=c,c++,objc,obj-c++,fortran,go,lto --enable-plugin --disable-libgcj --with-isl=/builddir/build/BUILD/gcc-4.9.3/obj-mips64el-neokylin-linux/isl-install --with-cloog=/builddir/build/BUILD/gcc-4.9.3/obj-mips64el-neokylin-linux/cloog-install --enable-gnu-indirect-function --with-long-double-128 --build=mips64el-neokylin-linux
线程模型：posix
gcc 版本 4.9.3 20150626 (NeoKylin 4.9.3-5) (GCC) 
```
# cat /proc/cpuinfo
```Text
system type		: generic-loongson-machine
machine			: LEMOTE-LS3B3000-SR5690-2w-V1.00-server
processor		: 0
cpu model		: ICT Loongson-3 V0.13  FPU V0.1
model name		: Loongson-3A R3 (Loongson-3B3000) @ 1450MHz @ 1450MHz
BogoMIPS		: 2887.52
cpu MHz			: 1450.00
wait instruction	: yes
microsecond timers	: yes
tlb_entries		: 1088
extra interrupt vector	: no
hardware watchpoint	: yes, count: 0, address/irw mask: []
isa			: mips1 mips2 mips3 mips4 mips5 mips32r1 mips32r2 mips64r1 mips64r2
ASEs implemented	: vz
shadow register sets	: 1
kscratch registers	: 6
package			: 0
core			: 0
VCED exceptions		: not available
VCEI exceptions		: not available

```

***
***
***
***
***
***

# 编译原因
在开发时我们拿到的这个版本的系统内置的cmake版本是2.x的编译一些较新的软件直接版本校验过不了

# 编译
源码: https://github.com/Kitware/CMake

版本: 3.x 当前编译时的版本是3.19.1

编译时间: 2020年12月10日16:02:46


```Shell
cd cmake
mkdir build
cd build
../bootstrap
gmake

```

***
***
***
***

# 错误解决
## 错误1
```Text
configure CMake with -DCMAKE_USE_OPENSSL=OFF to build without OpenSSL
```

```Text
这里先不安装openssl,有问题再说
在根目录的CMakeLists.txt中

在开头添加 set(CMAKE_USE_OPENSSL OFF)
不能添加在函数里面
```