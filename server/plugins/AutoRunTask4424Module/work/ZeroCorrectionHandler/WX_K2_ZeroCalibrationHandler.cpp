#include "WX_K2_ZeroCalibrationHandler.h"
#include "SingleCommandHelper.h"

WX_K2_ZeroCalibrationHandler::WX_K2_ZeroCalibrationHandler(SystemWorkMode pSystemWorkMode)
    : BaseZeroCalibrationHandler(pSystemWorkMode)
{
}

bool WX_K2_ZeroCalibrationHandler::handle()
{
    SingleCommandHelper singleCmdHelper;
    PackCommand packCommand;
    auto m_ckDeviceMap = mTaskTimeList.m_ckDeviceMap;
    auto m_ckQdDeviceMap = mTaskTimeList.m_ckQdDeviceMap;
    auto m_hpaDeviceMap = mTaskTimeList.m_hpaDeviceMap;

    auto m_upFrequencyMap = mTaskTimeList.m_upFrequencyMap;
    auto m_downFrequencyMap = mTaskTimeList.m_downFrequencyMap;
    auto m_upFrequencyListMap = mTaskTimeList.m_upFrequencyListMap;
    auto m_downFrequencyListMap = mTaskTimeList.m_downFrequencyListMap;

    // 测控基带设置为偏馈校零
    if (mTaskTimeList.m_dataMap.contains(mSystemWorkMode)) {}
    return false;
}
