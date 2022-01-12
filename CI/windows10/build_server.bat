cd ../..
mkdir build-server
cd build-server
call  "%VS140COMNTOOLS%..\..\VC\vcvarsall.bat" amd64
::"C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64
qmake ..\server\Framework.pro -spec win32-msvc "CONFIG+=debug" "CONFIG+=qml_debug"
echo "qmake success"
jom -f .\Makefile qmake_all
jom -j 6
echo "over"

:: 复制安装包到h5ai项目库
:: scp -r -P 12322 ..\README.md root@192.168.3.227:/application/h5ai/data/12m_station/windows_qt5.14.2
