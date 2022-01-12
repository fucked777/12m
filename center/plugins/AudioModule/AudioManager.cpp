#include "AudioManager.h"
#include "AudioConfig.h"
#include "AudioPlayServer.h"
#include "CppMicroServicesUtility.h"
#include "RedisHelper.h"
#include "RepeatAlarmInterval.h"
#include "ui_AudioManager.h"
#include <QApplication>
#include <QCheckBox>
#include <QDebug>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QMap>
#include <QMessageBox>
#include <QMutex>
#include <QPushButton>
#include <QTabWidget>
#include <QThread>
#include <QVBoxLayout>

struct AudioUIInfo
{
    AudioInfo info;
    QList<QCheckBox*> checkBoxList;
};

class AudioManagerImpl
{
public:
    QThread thread;
    AudioPlayServer audioPlayServer;
    QTabWidget* tabWidget{ nullptr };
    QList<AudioUIInfo> audioInfoListToUI;
    QHBoxLayout* managerLayout{ nullptr };
    int curIndex{ 0 };
};

AudioManager::AudioManager(cppmicroservices::BundleContext context, QWidget* parent)
    : QWidget(parent)
    , m_impl{ new AudioManagerImpl }
{
    m_impl->managerLayout = new QHBoxLayout(this);
    m_impl->managerLayout->setSpacing(10);
    m_impl->managerLayout->setContentsMargins(10, 10, 10, 10);
    this->resize(900, 750);

    m_impl->audioPlayServer.moveToThread(&(m_impl->thread));
    m_impl->thread.start();
    emit m_impl->audioPlayServer.sg_start();
}

AudioManager::~AudioManager()
{
    emit m_impl->audioPlayServer.sg_stop();
    m_impl->audioPlayServer.waitQuit();
    delete m_impl;
}
void AudioManager::init() { reload(); }
void AudioManager::saveCurPage(qint32 defaultCoiledPlayInterval)
{
    if (m_impl->tabWidget == nullptr)
    {
        return;
    }
    auto index = m_impl->tabWidget->currentIndex();
    auto configData = m_impl->audioPlayServer.configData();
    auto& changePage = m_impl->audioInfoListToUI[index];
    for (auto iter = changePage.info.audioMap.begin(); iter != changePage.info.audioMap.end(); ++iter)
    {
        auto find = configData.find(iter.key());
        if (find != configData.end())
        {
            iter.value() = find.value();
        }
    }
    QString errMsg;
    if (!AudioConfig::saveConfig(changePage.info, errMsg, defaultCoiledPlayInterval))
    {
        QMessageBox::information(this, "保存配置文件失败", errMsg);
    }
}
void AudioManager::reload()
{
    /* 先加载配置 */
    QString errMsg;
    QList<AudioInfo> audioInfoList;
    if (!AudioConfig::parseConfig(audioInfoList, errMsg))
    {
        qWarning() << QString("音频配置文件解析失败:").arg(errMsg);
        return;
    }

    m_impl->audioInfoListToUI.clear();
    AudioUIInfo uiinfo;
    QMap<QString, AudioItem> audioMap;
    for (auto& item : audioInfoList)
    {
        uiinfo.info = item;
        m_impl->audioInfoListToUI << uiinfo;
        for (auto iter = item.audioMap.begin(); iter != item.audioMap.end(); ++iter)
        {
            audioMap.insert(iter.key(), iter.value());
        }
    }
    emit m_impl->audioPlayServer.sg_setConfigData(audioMap);

    /********************************************************************************************/
    /********************************************************************************************/
    /********************************************************************************************/
    /********************************************************************************************/
    /* 重新加载ui */
    if (m_impl->tabWidget != nullptr)
    {
        m_impl->tabWidget->setParent(nullptr);
        delete m_impl->tabWidget;
        m_impl->tabWidget = nullptr;
    }
    /* 根据配置初始化ui */
    auto tabWidget = new QTabWidget;
    GetValueHelper<QTabWidget> getValueHelper(tabWidget);
    for (auto& itemInfo : m_impl->audioInfoListToUI)
    {
        auto widget = new QWidget;
        auto controlLayout = new QGridLayout;
        controlLayout->setVerticalSpacing(10);
        controlLayout->setHorizontalSpacing(10);
        controlLayout->setContentsMargins(0, 0, 0, 0);
        int col = 0;
        int row = 0;
        for (auto iter = itemInfo.info.audioMap.begin(); iter != itemInfo.info.audioMap.end(); ++iter)
        {
            auto itemLayout = new QHBoxLayout();
            itemLayout->setSpacing(5);
            itemLayout->setContentsMargins(0, 0, 0, 0);

            auto checkBox = new QCheckBox(widget);
            checkBox->setText("");
            checkBox->setChecked(iter->isEnable);
            checkBox->setProperty("key", iter.key());
            itemInfo.checkBoxList << checkBox;

            auto changeInterval = new QPushButton(widget);
            changeInterval->setText(iter->desc);

            itemLayout->addWidget(checkBox);
            itemLayout->addWidget(changeInterval);
            itemLayout->setStretch(0, 0);
            itemLayout->setStretch(1, 1);
            controlLayout->addLayout(itemLayout, row, col);
            ++col;
            if (col >= itemInfo.info.layoutColumn)
            {
                col = 0;
                ++row;
            }
            connect(checkBox, &QCheckBox::stateChanged, [=](int status) {
                auto key = iter.key();
                emit m_impl->audioPlayServer.sg_enableItem(key, Qt::Checked == status);
                saveCurPage();
            });
            connect(changeInterval, &QPushButton::clicked, [=]() {
                auto key = iter.key();
                RepeatAlarmInterval repeatAlarmInterval(m_impl->audioPlayServer.interval(key), this);
                if (QDialog::Accepted == repeatAlarmInterval.exec())
                {
                    emit m_impl->audioPlayServer.sg_changeInterval(key, repeatAlarmInterval.interval());
                }
                saveCurPage();
            });
        }
        auto mainLayout = new QVBoxLayout(widget);
        mainLayout->setSpacing(10);
        mainLayout->setContentsMargins(10, 10, 10, 10);
        mainLayout->addLayout(controlLayout);

        auto buttonLayout = new QHBoxLayout();
        buttonLayout->setSpacing(10);
        buttonLayout->setContentsMargins(0, 0, 0, 0);

        auto allEnable = new QPushButton(widget);
        allEnable->setText("全部启用");
        buttonLayout->addStretch();
        buttonLayout->addWidget(allEnable);

        auto allDisable = new QPushButton(widget);
        allDisable->setText("全部禁用");
        buttonLayout->addWidget(allDisable);

        auto defaultInterval = new QPushButton(widget);
        defaultInterval->setText("更改当前默认重复告警间隔");
        buttonLayout->addWidget(defaultInterval);

        auto reset = new QPushButton(widget);
        reset->setText("重置当前页");
        buttonLayout->addWidget(reset);

        auto clearAllAudio = new QPushButton(widget);
        clearAllAudio->setText("清空当前所有音频");
        buttonLayout->addWidget(clearAllAudio);

        auto reload = new QPushButton(widget); /* 调试用 */
        reload->setText("重载");
        buttonLayout->addWidget(reload);
        reload->setVisible(false);

        mainLayout->addStretch();
        mainLayout->addLayout(buttonLayout);
        tabWidget->addTab(widget, itemInfo.info.title);

        connect(allEnable, &QPushButton::clicked, [=]() {
            auto index = tabWidget->currentIndex();
            auto& changePage = m_impl->audioInfoListToUI[index];
            for (auto& item : changePage.checkBoxList)
            {
                item->setChecked(true);
            }
        });
        connect(allDisable, &QPushButton::clicked, [=]() {
            auto index = tabWidget->currentIndex();
            auto& changePage = m_impl->audioInfoListToUI[index];
            for (auto& item : changePage.checkBoxList)
            {
                item->setChecked(false);
            }
        });

        connect(defaultInterval, &QPushButton::clicked, [=]() {
            auto index = tabWidget->currentIndex();
            auto& changePage = m_impl->audioInfoListToUI[index];

            RepeatAlarmInterval repeatAlarmInterval(changePage.info.defaultCoiledPlayInterval, this);
            if (QDialog::Accepted != repeatAlarmInterval.exec())
            {
                return;
            }

            saveCurPage(repeatAlarmInterval.interval());
            this->reload();
        });
        connect(clearAllAudio, &QPushButton::clicked, [=]() { emit m_impl->audioPlayServer.sg_clearAllAudio(); });
        connect(reset, &QPushButton::clicked, [=]() {
            static qint32 defaultCoiledPlayInterval = 5 * 60;
            auto index = tabWidget->currentIndex();
            auto& changePage = m_impl->audioInfoListToUI[index];

            for (auto iter = changePage.info.audioMap.begin(); iter != changePage.info.audioMap.end(); ++iter)
            {
                iter->coiledPlayInterval = defaultCoiledPlayInterval;
                iter->isEnable = true;
            }
            QString errMsg;
            if (!AudioConfig::saveConfig(changePage.info, errMsg, defaultCoiledPlayInterval))
            {
                QMessageBox::information(this, "重置配置文件失败", errMsg);
            }
            this->reload();
        });
        connect(tabWidget, &QTabWidget::currentChanged, [=](int index) { m_impl->curIndex = index; });
        connect(reload, &QPushButton::clicked, [=]() { this->reload(); });
    }
    m_impl->managerLayout->addWidget(tabWidget);
    m_impl->tabWidget = getValueHelper.get();
    m_impl->tabWidget->setCurrentIndex(m_impl->curIndex);
}
