#include "ResourceRestructuringDialog.h"
#include "GlobalData.h"
#include "ResourceRestructuringXMLReader.h"
#include "ServiceCheck.h"
#include "ui_ResourceRestructuringDialog.h"
#include <QComboBox>
#include <QDebug>
#include <QMessageBox>
#include <QSet>
#include <QTreeWidget>
#include <QUuid>

const char* UniqueID = "UniqueID";

ResourceRestructuringDialog::ResourceRestructuringDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::ResourceRestructuringDialog)

{
    ui->setupUi(this);
    // 只显示关闭按钮
    this->setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);

    init();
}

ResourceRestructuringDialog::~ResourceRestructuringDialog() { delete ui; }

void ResourceRestructuringDialog::reset()
{
    ui->name->clear();
    ui->workMode->setCurrentIndex(0);
    ui->mainStandby->setCurrentText(0);
    ui->descs->clear();

    auto wokMode = (SystemWorkMode)(ui->workMode->currentData().toInt());
    ui->tabwidget->clear();
    ui->tabwidget->addTab(mWorkModeTreeWidgetMap[wokMode], mWorkModeNameMap[wokMode]);
}

void ResourceRestructuringDialog::setMode(const ResourceRestructuringDialog::Mode& mode)
{
    mMode = mode;
    switch (mMode)
    {
    case Mode::Add:
    {
        setWindowTitle("添加基带配置");
        mTipsMessage = QString("添加");
        ui->workMode->setEnabled(true);
    }
    break;
    case Mode::Edit:
    {
        setWindowTitle("编辑基带配置");
        mTipsMessage = QString("编辑");
        ui->workMode->setEnabled(false);
    }
    break;
    }
}

ResourceRestructuringDialog::Mode ResourceRestructuringDialog::getMode() const { return mMode; }

void ResourceRestructuringDialog::setData(const ResourceRestructuringData& resourceRestructuringData)
{
    mEditResourceRestructuringData = resourceRestructuringData;

    setMainData(resourceRestructuringData);
    setSubData(resourceRestructuringData);
}

void ResourceRestructuringDialog::setMainData(const ResourceRestructuringData& resourceRestructuringData)
{
    ui->name->setText(resourceRestructuringData.name);
    for (int i = 0; i < ui->workMode->count(); ++i)
    {
        if (ui->workMode->itemData(i).toInt() == resourceRestructuringData.workMode)
        {
            ui->workMode->setCurrentIndex(i);
        }
    }

    ui->mainStandby->setCurrentIndex(resourceRestructuringData.isMaster == MasterSlave::Master ? 0 : 1);
    ui->descs->setPlainText(resourceRestructuringData.desc);
}

void ResourceRestructuringDialog::setSubData(const ResourceRestructuringData& resourceRestructuringData)
{
    QTreeWidget* treeWidget = mWorkModeTreeWidgetMap[resourceRestructuringData.workMode];
    if (treeWidget == nullptr)
    {
        return;
    }
    QList<QComboBox*> comBoxs = treeWidget->findChildren<QComboBox*>();
    QMap<int, DeviceID> info = resourceRestructuringData.deviceMap;
    int uniqueID = -1;
    for (QComboBox* comBox : comBoxs)
    {
        uniqueID = comBox->property(UniqueID).toInt();
        if (uniqueID != -1)
        {
            for (auto id : info.keys())
            {
                if (id == uniqueID)
                {
                    DeviceID deviceId = info[id];
                    int iDeviceId;
                    deviceId >> iDeviceId;
                    comBox->setCurrentIndex(getComBoxIndex(comBox, iDeviceId));
                }
            }
        }
    }

    ui->tabwidget->clear();
    ui->tabwidget->addTab(treeWidget, mWorkModeNameMap[resourceRestructuringData.workMode]);
}

QTreeWidget* ResourceRestructuringDialog::getTreeWidget(SystemWorkMode workMode)
{
    if (mWorkModeTreeWidgetMap.contains(workMode))
        return mWorkModeTreeWidgetMap[workMode];
    return nullptr;
}

QString ResourceRestructuringDialog::getWorkModeName(SystemWorkMode workMode) { return mWorkModeNameMap[workMode]; }

QMap<SystemWorkMode, QString> ResourceRestructuringDialog::getWorkModeNameMap() { return mWorkModeNameMap; }

void ResourceRestructuringDialog::init()
{
    ui->editBtn->setVisible(false);

    connect(ui->okBtn, &QPushButton::clicked, this, &ResourceRestructuringDialog::slotOkBtnClicked);
    connect(ui->cancelBtn, &QPushButton::clicked, this, &ResourceRestructuringDialog::reject);

    ui->mainStandby->addItem(QString("主"), static_cast<int>(MasterSlave::Master));
    ui->mainStandby->addItem(QString("备"), static_cast<int>(MasterSlave::Slave));

    connect(ui->workMode, &QComboBox::currentTextChanged, this, &ResourceRestructuringDialog::slotWorkModeSelectedChanged);

    ResourceRestructuringXMLReader reader;
    auto workSystems = reader.getBBEConfig();
    d_BBE_ID_Type = reader.getBBEIDAndType();
    for (auto& workSystem : workSystems)
    {
        auto workMode = (SystemWorkMode)workSystem.id;
        mWorkModeNameMap[workMode] = workSystem.desc;

        auto modeName = workSystem.desc;
        ui->workMode->addItem(modeName, workMode);

        auto treeWidget = new QTreeWidget(this);
        treeWidget->setHidden(true);
        treeWidget->setColumnCount(3);
        treeWidget->setColumnWidth(0, 200);
        treeWidget->setColumnWidth(1, 200);
        treeWidget->setHeaderLabels(QStringList() << QString("名称") << QString("值") << QString("描述"));
        treeWidget->setSelectionMode(QTreeWidget::NoSelection);

        for (auto& deviceTypes : workSystem.devicesTypes)
        {
            auto root = new QTreeWidgetItem;
            root->setText(0, deviceTypes.desc);
            treeWidget->addTopLevelItem(root);

            for (auto& deviceType : deviceTypes.deviceTypes)
            {
                auto child1 = new QTreeWidgetItem;
                child1->setText(0, deviceType.desc);
                root->addChild(child1);

                for (auto& dev : deviceType.devs)
                {
                    auto child2 = new QTreeWidgetItem;
                    child2->setText(0, dev.desc);
                    child1->addChild(child2);

                    auto comboBox = new QComboBox(this);
                    for (auto key : dev.enums.keys())
                    {
                        comboBox->addItem(dev.enums.value(key), key);
                        comboBox->setProperty(UniqueID, dev.id);
                    }
                    treeWidget->setItemWidget(child2, 1, comboBox);
                }
            }
        }
        treeWidget->expandAll();
        mWorkModeTreeWidgetMap[(SystemWorkMode)workSystem.id] = treeWidget;
    }
}

int ResourceRestructuringDialog::getComBoxIndex(QComboBox* comBox, const int target)
{
    int i = 0;
    for (; i < comBox->count(); i++)
    {
        if (comBox->itemData(i).toInt() == target)
            break;
    }
    return i;
}

void ResourceRestructuringDialog::slotOperationResult(bool result, const QString& msg)
{
    if (result)
    {
        QMessageBox::information(this, "提示", mTipsMessage + "成功", "确定");
        this->close();
    }
    else
    {
        QMessageBox::critical(this, mTipsMessage + "失败", msg, "确定");
    }
}

void ResourceRestructuringDialog::slotWorkModeSelectedChanged(const QString& text)
{
    Q_UNUSED(text)
    auto workMode = (SystemWorkMode)ui->workMode->currentData().toInt();
    auto treeWidget = mWorkModeTreeWidgetMap[workMode];
    ui->tabwidget->clear();
    ui->tabwidget->addTab(treeWidget, mWorkModeNameMap[workMode]);
}

void ResourceRestructuringDialog::slotOkBtnClicked()
{
    SERVICEONLINECHECK();  //判断服务器是否离线

    QString errorMsg;
    if (!GlobalData::checkUserLimits(errorMsg))  //检查用户权限
    {
        QMessageBox::information(this, QString("提示"), QString(errorMsg), QString("确定"));
        return;
    }

    ResourceRestructuringData data;
    if (mMode == Edit)
    {
        data.resourceRestructuringID = mEditResourceRestructuringData.resourceRestructuringID;
    }
    else
    {
        data.resourceRestructuringID = QUuid::createUuid().toString();
    }

    data.name = ui->name->text();
    data.workMode = (SystemWorkMode)ui->workMode->currentData().toInt();
    data.isMaster = (MasterSlave)ui->mainStandby->currentData().toInt();
    data.desc = ui->descs->toPlainText();

    if (data.name.isEmpty())
    {
        QMessageBox::information(this, QString("提示"), QString("基带配置名称不能为空"), QString("确定"));
        return;
    }

    if (data.name.size() > 80)
    {
        QMessageBox::information(this, QString("提示"), QString("基带配置名称过长"), QString("确定"));
        return;
    }

    if (ui->workMode->currentText() == QString(""))
    {
        QMessageBox::warning(this, QString("提示"), QString("请选择工作模式"), QString("确定"));
        return;
    }
    if (data.desc.size() > 80)
    {
        QMessageBox::information(this, QString("提示"), QString("基带配置描述过长"), QString("确定"));
        return;
    }

    // 已经存在就不添加
    ResourceRestructuringDataList dataList;
    GlobalData::getResourceRestructuringData(dataList);
    for (auto item : dataList)
    {
        // 编辑时id才可能相等，编辑时不需要判断和自己之前是否相等(能够保存成功，说明约束条件是唯一的)
        if (item.resourceRestructuringID == data.resourceRestructuringID)
        {
            continue;
        }

        if (item.name == data.name)
        {
            QMessageBox::information(this, QString("提示"), QString("已存在相同名称的基带配置方案"), QString("确定"));
            return;
        }

        if (item.workMode == data.workMode && item.isMaster == data.isMaster)
        {
            QMessageBox::information(this, QString("提示"), QString("该工作模式的基带配置案已存在"), QString("确定"));
            return;
        }
    }

    QTreeWidget* treeWidget = mWorkModeTreeWidgetMap[(SystemWorkMode)ui->workMode->currentData().toInt()];
    QList<QComboBox*> comBoxs = treeWidget->findChildren<QComboBox*>();
    int uniqueId = -1;
    for (QComboBox* comBox : comBoxs)
    {
        uniqueId = comBox->property(UniqueID).toInt();
        if (uniqueId != -1)
        {
            DeviceID deviceId;
            int iDeviceId = comBox->currentData().toInt();
            deviceId << iDeviceId;
            data.deviceMap.insert(uniqueId, deviceId);
        }
    }

    DeviceID gsddp_main_1 = data.getDeviceId(GSDDP_BBE | Main | First);
    DeviceID gssdp_main_1 = data.getDeviceId(GSSDP_BBE | Main | First);
    if (!gsddp_main_1.isValid()) {}
    else
    {
        DeviceID gsddp_bak_1 = data.getDeviceId(GSDDP_BBE | Bak | First);
        DeviceID gsddp_bak_2 = data.getDeviceId(GSDDP_BBE | Bak | Second);
        if (gsddp_main_1 == gsddp_bak_1 || gsddp_main_1 == gsddp_bak_2 || gsddp_bak_1 == gsddp_bak_2)
        {
            QMessageBox::information(this, QString("提示"), QString("高速基带(单点频)主用和备用不能一样"), QString("确定"));
            return;
        }
    }
    if (!gssdp_main_1.isValid()) {}
    else
    {
        DeviceID gssdp_main_2 = data.getDeviceId(GSSDP_BBE | Main | Second);
        DeviceID gssdp_bak_1 = data.getDeviceId(GSSDP_BBE | Bak | First);
        if (gssdp_main_1 == gssdp_main_2 || gssdp_main_1 == gssdp_bak_1 || gssdp_main_2 == gssdp_bak_1)
        {
            QMessageBox::information(this, QString("提示"), QString("高速基带(双点频)主用和备用不能一样"), QString("确定"));
            return;
        }
    }

    QMap<DeviceID, int> duplicateCheck;
    QMapIterator<int, DeviceID> resourceInfoIt(data.deviceMap);
    while (resourceInfoIt.hasNext())
    {
        resourceInfoIt.next();
        int id = resourceInfoIt.key();
        DeviceID deviceId = resourceInfoIt.value();
        QString BBEName = d_BBE_ID_Type[id];
        if (BBEName.contains("点频"))
        {
            continue;
        }
        if (duplicateCheck.contains(deviceId))
        {
            QMessageBox::information(this, QString("提示"), QString("%1主用和备用不能一样").arg(d_BBE_ID_Type[id]), QString("确定"));
            return;
        }
        else
        {
            duplicateCheck[deviceId] = 0;
        }
    }

    emit signalDataReady(data);
}

void ResourceRestructuringDialog::on_descs_textChanged()
{
    QString textContent = ui->descs->toPlainText();
    int length = textContent.count();
    int maxLength = 40;  // 最大字符数
    if (length > maxLength)
    {
        int position = ui->descs->textCursor().position();
        QTextCursor textCursor = ui->descs->textCursor();
        textContent.remove(position - (length - maxLength), length - maxLength);
        ui->descs->setPlainText(textContent);
        textCursor.setPosition(position - (length - maxLength));
        ui->descs->setTextCursor(textCursor);
    }
}
