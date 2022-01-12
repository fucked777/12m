#include "SystemBlockDiagramWidget.h"
#include "BlockDiagramConfig.h"
#include "BlockDiagramDataSource.h"
#include "BlockDiagramDataSourceThread.h"
#include "BlockDiagramDefine.h"
#include "CppMicroServicesUtility.h"
#include "DeviceProcessMessageSerialize.h"
#include "GlobalData.h"
#include "ItemDefaultDataSetChange.h"
#include "PackDataBuilder.h"
#include "PlatformInterface.h"
#include "ServiceHelper.h"
#include "SystemGraphicsView.h"
#include <QDebug>
#include <QElapsedTimer>
#include <QHBoxLayout>
#include <QIcon>
#include <QMessageBox>
#include <QThread>

/*
 * 20210331 框图的逻辑 wp??
 * 前置约定 所有的图元数据只能是颜色且只能是单色
 * 前置约定 所有的图元数据只能是颜色且只能是单色
 * 前置约定 所有的图元数据只能是颜色且只能是单色
 *
 * 首先是图元加载部分,此部分现在完全与业务逻辑分离,绝对不要加入任何的业务逻辑
 * 现在图元部分加载成功后会提供两组数据
 * 第一组是所有图元的id与图元item的map  所有的图元都在了包括线段 文本以及tips都在 并且所有的图元都有点击事件
 * 第二组是图元分组的id映射表就是id对应一个list
 *
 * 第一组数据可以用来向图元设置数据,然后点击事件等等..
 * 第二组数据是预留给链路的，因为现在所有的图元都在了所以可以当打一个链路的时候整个链路都变颜色
 *
 *
 * 框图的业务逻辑
 * 第一步
 *      启动加载数据配置
 *      数据配置包括哪个图元对应的设备  然后点击事件的类型
 * 第二步
 *      加载框图
 * 第三步
 *      数据源定时发送数据
 *
 *
 *
 * 定时发送数据这个比较好处理 就是配置把两个图元id关联上，来数据就直接设置item的数据就OK
 *
 * 点击事件是触发后先根据图元ID找当前图元是对应的什么事件 是控开关 还是显示当前设备
 * 然后根据事件执行操作
 *
 */
class SystemBlockDiagramWidgetImpl
{
public:
    cppmicroservices::BundleContext context;
    SystemGraphicsView* diagram{ nullptr };
    /* 加载完成后的图元分组 */
    GraphicsItemInfo graphicsItemInfo;
    /* 框图的数据信息 */
    SystemBlockInfo systemBlockInfo;
    /* 数据源 */
    BlockDiagramDataSource dataSource;
    QThread* dataSourceThread = nullptr;
    BlockDiagramDataSourceThread* dataSourceThreadSend = nullptr;  //发送当前redisID对应状态上报的线程
    QMutex refreshMutex;
    DeviceReportMessageMap* currentDataMap = nullptr;

    template<typename T>
    static void showSystemDialog(const T& info, QWidget* parent);
};

static QMap<QString, QDialog*> widgetCache = QMap<QString, QDialog*>();
static QMap<QString, QString> ribbonIcons = QMap<QString, QString>();
static QString iconPath = "";

template<typename T>
void SystemBlockDiagramWidgetImpl::showSystemDialog(const T& info, QWidget* parent)
{
    /* 显示状态的图元双击 这里得从设备监控获取界面,这里也是主进程所以不存在跨线程的问题 */

    QString systemName = info.systemName;
    QElapsedTimer timer;
    timer.start();
    auto containerWidget = widgetCache.value(systemName);
    QString systemID = QString("0x%1").arg(info.systemID);
    int deviceID = systemID.toInt(nullptr, 16);
    DeviceID id(deviceID);
    if (containerWidget == nullptr)
    {
        auto module = getService<IGuiService>(info.statusTriggerModule);
        if (module == nullptr)
        {
            QMessageBox::warning(parent, "提示", "加载服务模块失败", "确定");
            return;
        }
        auto widget = module->createWidget(systemName);
        if (widget == nullptr)
        {
            QMessageBox::critical(nullptr, QString("错误"), QString("打开失败"), QString("确定"));
            return;
        }
        widget->setObjectName(systemName);

        auto vLayout = new QVBoxLayout;
        vLayout->addWidget(widget);
        vLayout->setMargin(0);
        containerWidget = new QDialog;
        containerWidget->setMinimumSize(1600, 900);
        containerWidget->setLayout(vLayout);

        QString windowTitle = GlobalData::getSystemName(id.sysID);
        containerWidget->setWindowTitle(windowTitle);
        containerWidget->setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
        QString iconName = ribbonIcons.value(systemName);
        containerWidget->setWindowIcon(QIcon(iconPath + iconName));
        widgetCache.insert(systemName, containerWidget);
    }
    QWidget* child = containerWidget->findChild<QWidget*>(systemName);
    child->setProperty("deviceID", deviceID);

    if (containerWidget)
    {
        if (containerWidget->isHidden())
        {
            containerWidget->show();
        }
        if (containerWidget->isMinimized())
        {
            containerWidget->showNormal();
        }
        // 置于顶层，并设置为活动窗口获取焦点
        containerWidget->raise();
        containerWidget->activateWindow();
    }
}
/************************************************************************************/
/************************************************************************************/
/************************************************************************************/
/************************************************************************************/
/************************************************************************************/
SystemBlockDiagramWidget::SystemBlockDiagramWidget(cppmicroservices::BundleContext context, QWidget* parent)
    : QWidget(parent)
    , m_impl(new SystemBlockDiagramWidgetImpl)
{
    m_impl->context = context;
    /* 更改图元的默认设置数据函数 */
    ItemDefaultDataSetChange::changeFunc();
    /* 加载图元 */
    m_impl->diagram = new SystemGraphicsView(this);
    auto loadResult = m_impl->diagram->reload(PlatformConfigTools::configBusiness("GraphConfig/GraphConfig.xml"));
    if (loadResult.success())
    {
        m_impl->graphicsItemInfo = loadResult.value();
    }
    /* 加载配置数据关联配置文件 */
    BlockDiagramConfig config;
    auto loadRes = config.reload();
    if (!loadRes)
    {
        qWarning() << "关联配置加载失败:" << loadRes.msg();
    }
    m_impl->systemBlockInfo = config.getParseResult();
    SystemBlockInfo* systemBlockInfo = new SystemBlockInfo;
    *systemBlockInfo = config.getParseResult();
    ribbonIcons = m_impl->systemBlockInfo.ribbonIcons;
    iconPath = PlatformConfigTools::configBusiness("RibbonIcon/");

    if (m_impl->dataSourceThread == nullptr)
    {
        m_impl->dataSourceThread = new QThread;
    }

    if (m_impl->dataSourceThreadSend == nullptr)
    {
        m_impl->dataSourceThreadSend = new BlockDiagramDataSourceThread;
    }

    if (m_impl->currentDataMap == nullptr)
    {
        m_impl->currentDataMap = new DeviceReportMessageMap;
    }

    m_impl->dataSource.moveToThread(m_impl->dataSourceThread);
    connect(m_impl->diagram, &SystemGraphicsView::sg_doubleClick, this, &SystemBlockDiagramWidget::itemDoubleClick);
    connect(&(m_impl->dataSource), &BlockDiagramDataSource::sg_sendParamData, this, &SystemBlockDiagramWidget::refreshItem);
    connect(m_impl->dataSourceThreadSend, &BlockDiagramDataSourceThread::sg_refresh, &(m_impl->dataSource), &BlockDiagramDataSource::slotRefreshData);
    m_impl->dataSource.setSystemBlockInfo(systemBlockInfo, m_impl->currentDataMap, &(m_impl->refreshMutex));
    m_impl->dataSourceThreadSend->setSystemBlockInfo(systemBlockInfo, m_impl->currentDataMap, &(m_impl->refreshMutex));
    m_impl->dataSourceThread->start();
    m_impl->dataSourceThreadSend->start();

    auto mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    //隐藏系统框图的滚动条
    m_impl->diagram->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_impl->diagram->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_impl->diagram->setScaleValue(m_impl->systemBlockInfo.scaleX, m_impl->systemBlockInfo.scaleY);

    mainLayout->addWidget(m_impl->diagram);
}

SystemBlockDiagramWidget::~SystemBlockDiagramWidget()
{
    /* 不延时会导致不是同一个线程关闭定时器出现一个警告 */
    QThread::msleep(100);

    if (m_impl->dataSourceThreadSend)
    {
        m_impl->dataSourceThreadSend->stopRunning();
        m_impl->dataSourceThreadSend->quit();
        m_impl->dataSourceThreadSend->wait();
        delete m_impl->dataSourceThreadSend;
        m_impl->dataSourceThreadSend = nullptr;
    }
    if (m_impl->dataSourceThread)
    {
        m_impl->dataSourceThread->quit();
        m_impl->dataSourceThread->wait();
        delete m_impl->dataSourceThread;
        m_impl->dataSourceThread = nullptr;
    }

    delete m_impl;
}
void SystemBlockDiagramWidget::itemDoubleClick(const QString& id, const QVariant& data)
{
    auto& systemBlockInfo = m_impl->systemBlockInfo;
    /* 这个ID是图元ID */
    auto find = systemBlockInfo.dataConfigMap.find(id);
    if (find == systemBlockInfo.dataConfigMap.end())
    {
        /* 没找到则代表此图元不需要事件 */
        return;
    }
    auto type = std::get<0>(find.value());
    auto& info = std::get<1>(find.value());
    switch (type)
    {
    case ItemShowType::System:
    {
        subSystem(info);
        return;
    }
    case ItemShowType::Status:
    {
        deviceStatus(info);
        return;
    }
    case ItemShowType::TwoValueSwitch:
    {
        twoValueSwitch(info, data);
        return;
    }
    }

    QMessageBox::warning(this, "配置错误", "未能找到当前类型信息", "确定");
}
void SystemBlockDiagramWidget::subSystem(const QVariant& info)
{
    /* 这里来个错误,这个没注册 */
    Q_ASSERT(info.canConvert<SystemTypeInfo>());
    auto curContext = info.value<SystemTypeInfo>();

    SystemBlockDiagramWidgetImpl::showSystemDialog(curContext, this);
}
void SystemBlockDiagramWidget::deviceStatus(const QVariant& info)
{
    /* 这里来个错误,这个没注册 */
    Q_ASSERT(info.canConvert<DeviceStatusTypeInfo>());
    auto curContext = info.value<DeviceStatusTypeInfo>();
    SystemBlockDiagramWidgetImpl::showSystemDialog(curContext, this);
}
/* 切换开关 */
void SystemBlockDiagramWidget::twoValueSwitch(const QVariant& info, const QVariant& data)
{
    Q_ASSERT(info.canConvert<TwoValueSwitchTypeInfo>());
    auto curContext = info.value<TwoValueSwitchTypeInfo>();

    bool isOK = false;
    auto curValue = data.toDouble(&isOK);
    if (!isOK)
    {
        QMessageBox::warning(this, "配置错误", "开关信息装换失败", "确定");
        return;
    }

    QMap<QString, QVariant> replace;
    PlusCmdInfo plusCmdInfo;
    int currentValue = 0;

    /* 就两个值一个一个比较就行 */
    auto devValue1 = std::get<0>(curContext.twoValueRelational[0]);
    auto angle1 = std::get<1>(curContext.twoValueRelational[0]);

    auto devValue2 = std::get<0>(curContext.twoValueRelational[1]);
    auto angle2 = std::get<1>(curContext.twoValueRelational[1]);
    QString tipsMsg;
    /* 角度相等,那就是切换到下一个数据 */
    if (qFuzzyCompare(1.0 + curValue, 1.0 + angle1))
    {
        replace[curContext.cmdKey] = devValue2;
        tipsMsg = std::get<2>(curContext.twoValueRelational[0]);
        currentValue = devValue2;
    }
    else if (qFuzzyCompare(1.0 + curValue, 1.0 + angle2))
    {
        replace[curContext.cmdKey] = devValue1;
        tipsMsg = std::get<2>(curContext.twoValueRelational[1]);
        currentValue = devValue1;
    }
    else
    {
        QMessageBox::warning(this, "配置错误", "查找数据失败", "确定");
        return;
    }

    //如果单元参数中有和当前命令相同的参数需要发送,会合并之后发送
    if (curContext.addtionalInfo.addtionalInfo.size())
    {
        QMap<int, PlusCmdInfo> cmdInfos = curContext.addtionalInfo.addtionalInfo;
        if (cmdInfos.contains(currentValue))
        {
            plusCmdInfo = cmdInfos[currentValue];
            int unitId = curContext.cmdNum;
            if (plusCmdInfo.unitParam.contains(unitId))
            {
                replace.unite(plusCmdInfo.unitParam.take(unitId));
            }
        }
    }

    if (0 != QMessageBox::information(this, QString("开关切换"), tipsMsg, "确定", "取消"))
    {
        return;
    }

    /* 发送数据 */
    auto tempRes = PackDataBuilder::unitToCmdRequest(curContext.systemNum, curContext.deviceNum, curContext.extenNum, curContext.modeNum,
                                                     curContext.cmdNum, replace);

    if (!tempRes.success())
    {
        QMessageBox::warning(this, "提示", tempRes.msg(), "确定");
        return;
    }

    JsonWriter writer;
    writer& tempRes.value();
    qDebug() << writer.GetQString();
    emit sg_sendUnitCmd(writer.GetQString());

    //如果是不同的单元参数命令或者过程控制命令,那么会在以下循环中发送命令
    QMap<int, QMap<QString, QVariant>> unitParam = plusCmdInfo.unitParam;
    for (int unitId : unitParam.keys())
    {
        auto tempRes = PackDataBuilder::unitToCmdRequest(curContext.systemNum, curContext.deviceNum, curContext.extenNum, curContext.modeNum, unitId,
                                                         unitParam[unitId]);
        if (!tempRes.success())
        {
            QMessageBox::warning(this, "提示", tempRes.msg(), "确定");
            return;
        }

        JsonWriter writer;
        writer& tempRes.value();

        QThread::sleep(2);
        emit sg_sendUnitCmd(writer.GetQString());
    }

    QMap<int, QMap<QString, QVariant>> processControl = plusCmdInfo.processControl;
    for (int cmdId : processControl.keys())
    {
        auto tempRes = PackDataBuilder::cmdToCmdRequest(curContext.systemNum, curContext.deviceNum, curContext.extenNum, curContext.modeNum, cmdId,
                                                        unitParam[cmdId]);
        if (!tempRes.success())
        {
            QMessageBox::warning(this, "提示", tempRes.msg(), "确定");
            return;
        }

        JsonWriter writer;
        writer& tempRes.value();

        QThread::sleep(2);
        emit sg_sendProcessCmd(writer.GetQString());
    }
}

void SystemBlockDiagramWidget::refreshItem(const QString& id, const QVariant& status, const int type)
{
    auto& graphicsItemInfo = m_impl->graphicsItemInfo;
    /* 这个ID是图元ID */
    auto find = graphicsItemInfo.graphicsItemMap.find(id);
    if (find == graphicsItemInfo.graphicsItemMap.end())
    {
        /* 没找到则代表此图元不需要刷新 */
        return;
    }
    auto& info = find.value();
    switch (type)
    {
    case 2:
    {
        info.setBorderColorData(info, status);
        break;
    }
    default:
    {
        info.setItemData(info, status);
        break;
    }
    }
}
