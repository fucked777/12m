cd ../..
cppcheck --xml --xml-version=2 --enable=all -DZMQ_DEPRECATED -DZMQ_BUILD_DRAFT_API -D_MSC_VER --enable=warning --language=c++ --std=c++14 --library=qt ./server ./client ./business -i.\client\core\App\SARibbonBar -i.\server\core\App\SARibbonBar -i.\client\plugins\Global\QCustomPlot -i.\business\RapidJson\rapidjson -i.\business\SARibbonBar --suppress=preprocessorErrorDirective:business\RapidJson\rapidjson\rapidjson.h  2> cppcheck-report.xml
sonar-scanner
