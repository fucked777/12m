#include "ZeroCalibrationRoutor.h"
#include "GML_K2_ZeroCalibrationHandler.h"
#include "GML_ZeroCalibrationHandler.h"
#include "K2_BGZB_ZeroCalibrationHandler.h"
#include "K2_GZB_ZeroCalibrationHandler.h"
#include "KT_ZeroCalibrationHandler.h"
#include "KaKuo2ZeroCalibrationHandler.h"
#include "SKuo2ZeroCalibrationHandler.h"
#include "SMJ_ZeroCalibrationHandler.h"
#include "SttcZeroCalibrationHandler.h"
#include "WX_K2_ZeroCalibrationHandler.h"
#include "WX_ZeroCalibrationHandler.h"
#include "ZeroCalibrationRoutor.h"

ZeroCalibrationRoutor::ZeroCalibrationRoutor(QObject* parent)
    : QObject(parent)
{
}

BaseZeroCalibrationHandler* ZeroCalibrationRoutor::factory(const SystemWorkMode systemWorkMode)
{
    if (systemWorkMode == STTC)
    {
        return new SttcZeroCalibrationHandler(systemWorkMode);
    }
    else if (systemWorkMode == Skuo2)
    {
        return new SKuo2ZeroCalibrationHandler(systemWorkMode);
    }
    else if (systemWorkMode == KaKuo2)
    {
        return new KaKuo2ZeroCalibrationHandler(systemWorkMode);
    }
    else if (systemWorkMode == SYTHSMJ)
    {
        return new SMJ_ZeroCalibrationHandler(systemWorkMode);
    }
    else if (systemWorkMode == SYTHWX)
    {
        return new WX_ZeroCalibrationHandler(systemWorkMode);
    }
    else if (systemWorkMode == SYTHGML)
    {
        return new GML_ZeroCalibrationHandler(systemWorkMode);
    }
    else if (systemWorkMode == SYTHSMJK2BGZB)  // 一体化上面级+扩二不共载波
    {
        return new K2_BGZB_ZeroCalibrationHandler(systemWorkMode);
    }
    else if (systemWorkMode == SYTHSMJK2GZB)  // 一体化上面级+扩二共载波
    {
        return new K2_GZB_ZeroCalibrationHandler(systemWorkMode);
    }
    else if (systemWorkMode == SYTHWXSK2)  // 一体化卫星+扩二
    {
        return new WX_K2_ZeroCalibrationHandler(systemWorkMode);
    }
    else if (systemWorkMode == SYTHGMLSK2)  // 一体化高码率+扩二
    {
        return new WX_K2_ZeroCalibrationHandler(systemWorkMode);
    }
    else if (systemWorkMode == SKT)  // 扩跳
    {
        return new KT_ZeroCalibrationHandler(systemWorkMode);
    }
    else
    {
        return nullptr;
    }
}
