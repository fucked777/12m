cd ../..
::rd /S /Q .\client\core\App\SARibbonBar  .\server\core\App\SARibbonBar  .\client\plugins\Global\QCustomPlot .\business\RapidJson\rapidjson .\business\SARibbonBar
cppcheck -j 4 -DZMQ_DEPRECATED -DZMQ_BUILD_DRAFT_API -D_MSC_VER --enable=warning --language=c++ --std=c++14 --library=qt ./server ./client ./business -i.\client\core\App\SARibbonBar -i.\server\core\App\SARibbonBar -i.\client\plugins\Global\QCustomPlot -i.\business\RapidJson\rapidjson -i.\business\SARibbonBar  --suppress=preprocessorErrorDirective:business\RapidJson\rapidjson\rapidjson.h 2> error.txt --template=gcc --error-exitcode=1

for %%a in ("error.txt") do (
     if "%%~za" equ "0" (
        echo "no error"
        ) else (
        call ".\CI\windows10\send_mail.bat"
                exit 1
     )
)

