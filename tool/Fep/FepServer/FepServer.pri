QT -= gui
QT += network xml
QT += core widgets

INCLUDEPATH+=$$PWD
include($$PWD/../FepCommon/FepCommon.pri)

SOURCES += \
    $$PWD/FepFileOperate.cpp \
    $$PWD/FepInitRecv.cpp \
    $$PWD/FepRecvData.cpp \
    $$PWD/FepServer.cpp \
    $$PWD/IStateRecv.cpp \
    $$PWD/TcpServer.cpp \
    $$PWD/TcpSocket.cpp

#    $$PWD/dataReceive/cfepcloseprocess.cpp \
#    $$PWD/dataReceive/cfepdatareceiveprocess.cpp \
#    $$PWD/dataReceive/cfepdatatransfer.cpp \
#    $$PWD/dataReceive/cfeplinkedstate.cpp \
#    $$PWD/dataReceive/creceivecontext.cpp \
#    $$PWD/dataSend/csendcontext.cpp \
#    $$PWD/dataReceive/istate.cpp \
#    $$PWD/dataSend/cfepsendinital.cpp \
#    $$PWD/dataSend/cfsenddata.cpp \
#    $$PWD/dataSend/cfsendclose.cpp \
#    $$PWD/dataSend/cfepsendlinkinital.cpp \
#    $$PWD/cfepfileoperate.cpp \
#    $$PWD/cffilerecvoperate.cpp


HEADERS += \
    $$PWD/FepFileOperate.h \
    $$PWD/FepInitRecv.h \
    $$PWD/FepRecvData.h \
    $$PWD/FepServer.h \
    $$PWD/IStateRecv.h \
    $$PWD/TcpServer.h \
    $$PWD/TcpSocket.h
#    $$PWD/dataReceive/cfepcloseprocess.h \
#    $$PWD/dataReceive/cfepdatareceiveprocess.h \
#    $$PWD/dataReceive/cfepdatatransfer.h \
#    $$PWD/dataReceive/cfeplinkedstate.h \
#    $$PWD/dataReceive/creceivecontext.h \
#    $$PWD/definedatastruct.h \
#    $$PWD/dataSend/csendcontext.h \
#    $$PWD/dataReceive/istate.h \
#    $$PWD/dataSend/cfepsendinital.h \
#    $$PWD/dataSend/cfsenddata.h \
#    $$PWD/dataSend/cfsendclose.h \
#    $$PWD/dataSend/cfepsendlinkinital.h \
#    $$PWD/cfepfileoperate.h \
#    $$PWD/cffilerecvoperate.h


