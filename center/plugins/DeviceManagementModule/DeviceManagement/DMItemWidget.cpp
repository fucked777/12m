#include "DMItemWidget.h"
#include "DMItemDataModel.h"
#include "DMMessageSerialize.h"
#include "DMParamInputOrDisplay.h"
#include "DMUIUtility.h"
#include "DynamicConfigToWidgetUtility.h"
#include "PageNavigator.h"
#include "ui_DMItemWidget.h"
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QTabWidget>
#include <QTableView>
#include <QUuid>

class DMItemWidgetImpl
{
public:
    DMParamInputOrDisplay* dmParamInputOrDisplay{ nullptr };
    DMItemDataModel dataModel;               /* 数据模型 */
    PageNavigator* pageNavigator{ nullptr }; /* 翻页控件 */
    QTreeWidget* attributeTreeWidget{ nullptr };

    UIDataParameter parameter;
    quint32 channel{ 0 };
};

DMItemWidget::DMItemWidget(const UIDataParameter& parameter, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::DMItemWidget)
    , m_impl(new DMItemWidgetImpl)
{
    m_impl->parameter = parameter;
    ui->setupUi(this);
    init();
}
DMItemWidget::~DMItemWidget()
{
    delete ui;
    delete m_impl;
}
void DMItemWidget::init()
{
    auto& parameter = m_impl->parameter;
    m_impl->dmParamInputOrDisplay = new DMParamInputOrDisplay(parameter, this, false);
    m_impl->dmParamInputOrDisplay->setReadOnly();
    ui->parameter->addWidget(m_impl->dmParamInputOrDisplay);

    m_impl->dataModel.setShowConvertData(m_impl->parameter.fromInfo.fromList);
    m_impl->dataModel.setObjList(m_impl->dmParamInputOrDisplay->fromWidgetInfo());

    ui->dataTable->setFixedHeight(300);
    ui->dataTable->setModel(&(m_impl->dataModel));
    ui->dataTable->setSelectionMode(QAbstractItemView::SingleSelection);  //设置单行选择
    ui->dataTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->dataTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->dataTable->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->dataTable->verticalHeader()->setVisible(false);
    ui->dataTable->horizontalHeader()->setHighlightSections(false);

    m_impl->pageNavigator = new PageNavigator(3, this);
    ui->pagingLayout->addStretch();
    ui->pagingLayout->addWidget(m_impl->pageNavigator);
    ui->pagingLayout->addStretch();

    connect(m_impl->pageNavigator, &PageNavigator::currentPageChanged, [=](int page) { m_impl->dataModel.changePageUpdate(page); });
    connect(ui->dataTable, &QTableView::clicked, this, &DMItemWidget::viewItem);
}

void DMItemWidget::viewItem(const QModelIndex& index)
{
    auto res = m_impl->dataModel.getItem(index);
    if (!res)
    {
        return;
    }
    setUIValue(res.value());
}
Optional<DMDataItem> DMItemWidget::getSelectItem()
{
    using ResType = Optional<DMDataItem>;
    DMDataItem curItem;

    auto res = m_impl->dataModel.dmDataItem(ui->dataTable->currentIndex(), curItem);
    return res ? ResType(curItem) : ResType(ErrorCode::NotFound, "当前未选中数据");
}
Optional<DMDataItem> DMItemWidget::getUIValue() { return m_impl->dmParamInputOrDisplay->getUIValue(); }
void DMItemWidget::setUIValue(const DMDataItem& value)
{
    m_impl->dmParamInputOrDisplay->setUIValue(value);
    m_impl->dmParamInputOrDisplay->setReadOnly();
}
void DMItemWidget::addItem(const DMDataItem& value)
{
    m_impl->dataModel.addItem(value);
    m_impl->pageNavigator->setMaxPage(m_impl->dataModel.page());
    m_impl->pageNavigator->setCurrentPage(1);
}
void DMItemWidget::deleteItem(const DMDataItem& value)
{
    m_impl->dataModel.deleteItem(value);
    m_impl->pageNavigator->setMaxPage(m_impl->dataModel.page());
    m_impl->pageNavigator->setCurrentPage(1);
}
void DMItemWidget::updateItem(const DMDataItem& value)
{
    m_impl->dataModel.updateItem(value);
    m_impl->pageNavigator->setMaxPage(m_impl->dataModel.page());
    m_impl->pageNavigator->setCurrentPage(1);
}
void DMItemWidget::setItems(const QList<DMDataItem>& value)
{
    m_impl->dataModel.updateData(value);
    m_impl->pageNavigator->setMaxPage(m_impl->dataModel.page());
    m_impl->pageNavigator->setCurrentPage(1);
}
QString DMItemWidget::type() const { return m_impl->dmParamInputOrDisplay->type(); }
UIDataParameter DMItemWidget::uiDataParameter() const { return m_impl->parameter; }
bool DMItemWidget::dataExist(const QString& devID) const { return m_impl->dataModel.dataExist(devID); }

void DMItemWidget::clearInput()
{
    m_impl->dmParamInputOrDisplay->clearInput();
    m_impl->dmParamInputOrDisplay->setReadOnly();
    ui->dataTable->clearSelection();
}
