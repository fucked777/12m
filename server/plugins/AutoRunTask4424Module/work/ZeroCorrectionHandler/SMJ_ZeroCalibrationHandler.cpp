#include "SMJ_ZeroCalibrationHandler.h"
#include "SingleCommandHelper.h"

SMJ_ZeroCalibrationHandler::SMJ_ZeroCalibrationHandler(SystemWorkMode pSystemWorkMode)
    : BaseZeroCalibrationHandler(pSystemWorkMode)
{
}

bool SMJ_ZeroCalibrationHandler::handle()
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
    if (mTaskTimeList.m_dataMap.contains(STTC)) {}
    return false;
}
