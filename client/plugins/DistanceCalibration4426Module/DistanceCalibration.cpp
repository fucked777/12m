#include "DistanceCalibration.h"
#include "CppMicroServicesUtility.h"
#include "GlobalData.h"
#include "ui_DistanceCalibration.h"
#include <QButtonGroup>

class DistanceCalibrationImpl
{
public:
    /* 通道单选的分组,这里的策略是无论用不用的上都是正常初始化 */
    QButtonGroup* channelGroup{ nullptr };
};

DistanceCalibration::DistanceCalibration(cppmicroservices::BundleContext context, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::DistanceCalibration)
    , m_impl(new DistanceCalibrationImpl)
{
    Q_UNUSED(context)
    ui->setupUi(this);
    init();
}

DistanceCalibration::~DistanceCalibration()
{
    delete ui;
    delete m_impl;
}

void DistanceCalibration::init()
{
    /* 首先隐藏掉,通道选项,只有在24的一体化上有这个 */
    ui->channelGroupBox->setVisible(false);
    m_impl->channelGroup = new QButtonGroup(this);
    /*
     * 1 通道1(一体化)
     * 2 通道2(扩频）
     */
    m_impl->channelGroup->addButton(ui->channel_yth, 1);
    m_impl->channelGroup->addButton(ui->channel_kp, 2);

    QDateTime currentTime = GlobalData::currentDateTime();
    ui->beginTime_2->setDateTime(currentTime.addMonths(-1));
    ui->endTime_2->setDateTime(currentTime);
}
