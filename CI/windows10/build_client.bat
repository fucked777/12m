cd ../..
mkdir build-client
cd build-client
call "%VS140COMNTOOLS%..\..\VC\vcvarsall.bat" amd64
qmake ..\client\Framework.pro -spec win32-msvc "CONFIG+=debug" "CONFIG+=qml_debug"
echo "qmake success"
jom -f .\Makefile qmake_all
jom -j 6
echo "over"


:: cd ..
:: mkdir build-client
:: cd build-client
:: "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64  
:: C:\Qt\Qt5.14.2\5.14.2\msvc2015_64\bin\qmake.exe ./client/Framework/Framework.pro -spec win32-msvc "CONFIG+=debug" "CONFIG+=qml_debug"
:: C:\Qt\Qt5.14.2\Tools\QtCreator\bin\jom.exe -f .\Makefile qmake_all
:: C:\Qt\Qt5.14.2\Tools\QtCreator\bin\jom.exe -j 6
