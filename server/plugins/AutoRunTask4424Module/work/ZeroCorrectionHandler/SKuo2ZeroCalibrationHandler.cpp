#include "SKuo2ZeroCalibrationHandler.h"
#include "SingleCommandHelper.h"

SKuo2ZeroCalibrationHandler::SKuo2ZeroCalibrationHandler(SystemWorkMode pSystemWorkMode)
    : BaseZeroCalibrationHandler(pSystemWorkMode)
{
}

bool SKuo2ZeroCalibrationHandler::handle()
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

    //前端测试开关网络关机
    singleCmdHelper.packSingleCommand("StepQDMNY_OFF_POWER", packCommand);
    waitExecSuccess(packCommand);

    // 测控基带设置为偏馈校零
    if (mTaskTimeList.m_dataMap.contains(mSystemWorkMode)) {}
    QThread::sleep(1);
    return false;
}
