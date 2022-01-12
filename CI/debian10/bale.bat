::@echo off
echo Setting up environment for Qt usage...
set PATH=C:\Qt\Qt5.7.1\5.7\msvc2015_64\bin;%PATH%
echo qmake... 
::qmake编译pro文件，本脚本的pro文件在上季以目录
qmake ..
echo nmake...
::qmake之后生成makefile 使用nmake编译生成可执行文件
nmake release
::下面这些是打包成安装文件的步骤
echo set app reference information...
::脚本第一参数是应用名称（注意不用带扩展）
set app_name=%1
::默认nmake生成的文件在当前脚本目录的同级release文件夹下
set app_path=%~dp0\release
::设置应用版本，格式为 v1.0.0
set app_version=%2
echo Copy Qt references...
::使用qt自带程序拷贝qt相关文件
windeployqt "%app_path%\\%app_name%.exe"
echo Copy license etc.
::拷贝许可文件，就是安装时候说的那堆责任神马的东西
copy ..\apps\package\license.txt %app_path%
::这是msvc的运行库安装文件，偷懒了，其实可以直接复制动态库的
copy ..\apps\package\vcredist_x64.exe %app_path%
::qt配置文件，我这里主要是为了配置一些高分辨率下字体大小的
copy ..\apps\package\qt.conf %app_path%
echo nsis package...
::开始用nsis打包成安装文件
"C:\Program Files (x86)\NSIS\makensis.exe" "D:\work\gitlab\apps\package\normal_package.nsi"
