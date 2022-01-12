INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

INCLUDEPATH += $$PWD/OverallSystem/SCTTTest
INCLUDEPATH += $$PWD/OverallSystem/SCTTTest/SCTTTestProduct

INCLUDEPATH += $$PWD/OverallSystem/RMRETTest
INCLUDEPATH += $$PWD/OverallSystem/RMRETTest/RMRETTestProduct

INCLUDEPATH += $$PWD/OverallSystem/SRETTest
INCLUDEPATH += $$PWD/OverallSystem/SRETTest/SRETTestProduct


HEADERS += \
    $$PWD/AutomateTestHelper.h \
    $$PWD/AutomateTestBase.h \
    $$PWD/AutomateTestFactory.h \
    $$PWD/AutomateTestFactoryRegisterTemplate.h \
    $$PWD/OverallSystem/EIRPTest.h \
    $$PWD/OverallSystem/GTTest.h \
    $$PWD/OverallSystem/RMRETTest/RMRETTest.h \
    $$PWD/OverallSystem/RMRETTest/IRMRETTestProduct.h \
    $$PWD/OverallSystem/RMRETTest/RMRETTestFactory.h \
    $$PWD/OverallSystem/RMRETTest/RMRETTestProduct/RMRETTestBase.h \
    $$PWD/OverallSystem/RMRETTest/RMRETTestProduct/RMRETTestSTTCProduct.h \
    $$PWD/OverallSystem/RMRETTest/RMRETTestProduct/RMRETTestSYTHGMLProduct.h \
    $$PWD/OverallSystem/RMRETTest/RMRETTestProduct/RMRETTestSYTHSMJProduct.h \
    $$PWD/OverallSystem/RMRETTest/RMRETTestProduct/RMRETTestSYTHWXProduct.h \
    $$PWD/OverallSystem/RMRETTest/RMRETTestProduct/RMRETTestSkuo2Product.h \
    $$PWD/OverallSystem/SCTTTest/SCTTTest.h \
    $$PWD/OverallSystem/SCTTTest/SCTTTestFactory.h \
    $$PWD/OverallSystem/SCTTTest/ISCTTTestProduct.h \
    $$PWD/OverallSystem/SCTTTest/SCTTTestProduct/SCTTTestBase.h \
    $$PWD/OverallSystem/SCTTTest/SCTTTestProduct/SCTTTestSKTProduct.h \
    $$PWD/OverallSystem/SCTTTest/SCTTTestProduct/SCTTTestSTTCProduct.h \
    $$PWD/OverallSystem/SCTTTest/SCTTTestProduct/SCTTTestSYTHGMLProduct.h \
    $$PWD/OverallSystem/SCTTTest/SCTTTestProduct/SCTTTestSYTHSMJProduct.h \
    $$PWD/OverallSystem/SCTTTest/SCTTTestProduct/SCTTTestSYTHWXProduct.h \
    $$PWD/OverallSystem/SCTTTest/SCTTTestProduct/SCTTTestSkuo2Product.h \
    $$PWD/OverallSystem/SRETTest/ISRETTestProduct.h \
    $$PWD/OverallSystem/SRETTest/SRETTest.h \
    $$PWD/OverallSystem/SRETTest/SRETTestFactory.h \
    $$PWD/OverallSystem/SRETTest/SRETTestProduct/SRETTestBase.h \
    $$PWD/OverallSystem/SRETTest/SRETTestProduct/SRETTestSTTCProduct.h \
    $$PWD/OverallSystem/SRETTest/SRETTestProduct/SRETTestSYTHGMLProduct.h \
    $$PWD/OverallSystem/SRETTest/SRETTestProduct/SRETTestSYTHSMJProduct.h \
    $$PWD/OverallSystem/SRETTest/SRETTestProduct/SRETTestSYTHWXProduct.h \
    $$PWD/OverallSystem/SRETTest/SRETTestProduct/SRETTestSkuo2Product.h \
    $$PWD/SendSystem/LPNTTest.h \
    $$PWD/SendSystem/PSTTest.h \
    $$PWD/SendSystem/SASHTTest.h \
    $$PWD/RecvSystem/FRTTest.h \
    $$PWD/RecvSystem/RPNTTest.h \
    $$PWD/RecvSystem/STTTest.h \
    $$PWD/FrontSystem/TFTTTest.h \
    $$PWD/FrontSystem/AGCCRATTest.h \
    $$PWD/BBESystem/TDDTTest.h \
    $$PWD/BBESystem/LBTTest.h \
    $$PWD/BBESystem/TSTTest.h

SOURCES += \
    $$PWD/AutomateTestHelper.cpp \
    $$PWD/AutomateTestBase.cpp \
    $$PWD/AutomateTestFactory.cpp \
    $$PWD/OverallSystem/EIRPTest.cpp \
    $$PWD/OverallSystem/GTTest.cpp \
    $$PWD/OverallSystem/RMRETTest/RMRETTest.cpp \
    $$PWD/OverallSystem/RMRETTest/RMRETTestFactory.cpp \
    $$PWD/OverallSystem/RMRETTest/RMRETTestProduct/RMRETTestBase.cpp \
    $$PWD/OverallSystem/RMRETTest/RMRETTestProduct/RMRETTestSTTCProduct.cpp \
    $$PWD/OverallSystem/RMRETTest/RMRETTestProduct/RMRETTestSYTHGMLProduct.cpp \
    $$PWD/OverallSystem/RMRETTest/RMRETTestProduct/RMRETTestSYTHSMJProduct.cpp \
    $$PWD/OverallSystem/RMRETTest/RMRETTestProduct/RMRETTestSYTHWXProduct.cpp \
    $$PWD/OverallSystem/RMRETTest/RMRETTestProduct/RMRETTestSkuo2Product.cpp \
    $$PWD/OverallSystem/SCTTTest/SCTTTest.cpp \
    $$PWD/OverallSystem/SCTTTest/SCTTTestFactory.cpp \
    $$PWD/OverallSystem/SCTTTest/SCTTTestProduct/SCTTTestBase.cpp \
    $$PWD/OverallSystem/SCTTTest/SCTTTestProduct/SCTTTestSKTProduct.cpp \
    $$PWD/OverallSystem/SCTTTest/SCTTTestProduct/SCTTTestSTTCProduct.cpp \
    $$PWD/OverallSystem/SCTTTest/SCTTTestProduct/SCTTTestSYTHGMLProduct.cpp \
    $$PWD/OverallSystem/SCTTTest/SCTTTestProduct/SCTTTestSYTHSMJProduct.cpp \
    $$PWD/OverallSystem/SCTTTest/SCTTTestProduct/SCTTTestSYTHWXProduct.cpp \
    $$PWD/OverallSystem/SCTTTest/SCTTTestProduct/SCTTTestSkuo2Product.cpp \
    $$PWD/OverallSystem/SRETTest/SRETTest.cpp \
    $$PWD/OverallSystem/SRETTest/SRETTestFactory.cpp \
    $$PWD/OverallSystem/SRETTest/SRETTestProduct/SRETTestBase.cpp \
    $$PWD/OverallSystem/SRETTest/SRETTestProduct/SRETTestSTTCProduct.cpp \
    $$PWD/OverallSystem/SRETTest/SRETTestProduct/SRETTestSYTHGMLProduct.cpp \
    $$PWD/OverallSystem/SRETTest/SRETTestProduct/SRETTestSYTHSMJProduct.cpp \
    $$PWD/OverallSystem/SRETTest/SRETTestProduct/SRETTestSYTHWXProduct.cpp \
    $$PWD/OverallSystem/SRETTest/SRETTestProduct/SRETTestSkuo2Product.cpp \
    $$PWD/SendSystem/LPNTTest.cpp \
    $$PWD/SendSystem/PSTTest.cpp \
    $$PWD/SendSystem/SASHTTest.cpp \
    $$PWD/RecvSystem/FRTTest.cpp \
    $$PWD/RecvSystem/RPNTTest.cpp \
    $$PWD/RecvSystem/STTTest.cpp \
    $$PWD/FrontSystem/AGCCRATTest.cpp \
    $$PWD/FrontSystem/TFTTTest.cpp \
    $$PWD/BBESystem/TDDTTest.cpp \
    $$PWD/BBESystem/LBTTest.cpp \
    $$PWD/BBESystem/TSTTest.cpp


