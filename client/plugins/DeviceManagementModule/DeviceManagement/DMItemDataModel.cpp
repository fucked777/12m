#include "DMItemDataModel.h"
#include "DMMessageSerialize.h"
#include "DynamicConfigToWidgetUtility.h"
#include "JsonHelper.h"
#include <QDebug>

struct DMDataModelInfo
{
    DMDataItem item;
    QVariantList valueList;
};

class DMItemDataModelImpl
{
public:
    int pageCount{ 0 };
    int curPage{ 0 };
    int singlePagesCount{ 7 };
    QMap<QString, CommonToWidgetInfo> convertDataMap;
    QList<DMDataModelInfo> totalRecords;
    QList<DMDataModelInfo*> pageRecords;
    QStringList header;
    QStringList findKey;
};
static DMDataModelInfo toDMDataModelInfo(const DMDataItem& value, const DMItemDataModelImpl* impl)
{
    DMDataModelInfo dmDataModelInfo;
    for (auto& key : impl->findKey)
    {
        /* 枚举要把显示的值转换成描述信息 */
        auto find = impl->convertDataMap.find(key);
        auto curValue = value.parameterConfig.value(key, "-");
        if (find == impl->convertDataMap.end() || find->type != DynamicConfigToWidgetType::Enum)
        {
            dmDataModelInfo.valueList << curValue;
            continue;
        }

        auto& widgetInfo = find.value();
        Q_ASSERT(widgetInfo.toWidgetInfo.canConvert<EnumToWidgetInfo>());
        auto info = widgetInfo.toWidgetInfo.value<EnumToWidgetInfo>();

        QString showString;
        for (auto iter = info.enumMap.begin(); iter != info.enumMap.end(); ++iter)
        {
            if (iter.value() == curValue)
            {
                showString = iter.key();
                break;
            }
        }
        if (showString.isEmpty())
        {
            dmDataModelInfo.valueList << curValue;
        }
        else
        {
            dmDataModelInfo.valueList << showString;
        }
    }
    dmDataModelInfo.item = value;
    return dmDataModelInfo;
}

DMItemDataModel::DMItemDataModel(QObject* parent)
    : QAbstractTableModel(parent)
    , m_impl(new DMItemDataModelImpl)
{
}
DMItemDataModel::~DMItemDataModel() { delete m_impl; }

void DMItemDataModel::setShowConvertData(QList<CommonToWidgetInfo>& fromList)
{
    QMap<QString, CommonToWidgetInfo> infoMap;
    for (auto& item : fromList)
    {
        infoMap.insert(item.id, item);
    }
    m_impl->convertDataMap = infoMap;
}
void DMItemDataModel::updateData(const QList<DMDataItem>& logs)
{
    decltype(DMItemDataModelImpl::totalRecords) temp;
    for (auto& item : logs)
    {
        temp << toDMDataModelInfo(item, m_impl);
    }

    beginResetModel();
    m_impl->totalRecords = temp;
    calcPage();
    changePage(1);
    endResetModel();
}
void DMItemDataModel::updateData(const QMap<QString, DMDataItem>& logs)
{
    decltype(DMItemDataModelImpl::totalRecords) temp;
    for (auto& item : logs)
    {
        temp << toDMDataModelInfo(item, m_impl);
    }

    beginResetModel();
    m_impl->totalRecords = temp;
    calcPage();
    changePage(1);
    endResetModel();
}
void DMItemDataModel::updateData()
{
    beginResetModel();
    endResetModel();
}
void DMItemDataModel::addItem(const DMDataItem& item)
{
    auto info = toDMDataModelInfo(item, m_impl);

    beginResetModel();
    m_impl->totalRecords << info;
    calcPage();
    changePage(1);
    endResetModel();
}
void DMItemDataModel::setSinglePagesCount(int count)
{
    if (count <= 0)
    {
        return;
    }
    if (m_impl->singlePagesCount == count)
    {
        return;
    }
    m_impl->singlePagesCount = count;
    beginResetModel();
    changePage(1);
    endResetModel();
}
bool DMItemDataModel::dataExist(const QString& devID) const
{
    for (auto& item : m_impl->totalRecords)
    {
        if (item.item.devID == devID)
        {
            return true;
        }
    }
    return false;
}
bool DMItemDataModel::isValid() const { return !m_impl->totalRecords.isEmpty(); }
bool DMItemDataModel::dmDataItem(const QModelIndex& index, DMDataItem& res)
{
    if (!index.isValid())
    {
        return false;
    }
    auto row = index.row();
    if (row < 0 || row >= rowCount())
    {
        return false;
    }

    auto& record = m_impl->pageRecords.at(row);
    res = record->item;
    return true;
}
void DMItemDataModel::updateItem(const DMDataItem& value)
{
    for (auto& item : m_impl->totalRecords)
    {
        auto& info = item.item;
        if (info.id == value.id)
        {
            beginResetModel();
            item = toDMDataModelInfo(value, m_impl);
            changePage(1);
            endResetModel();
            return;
        }
    }
}
void DMItemDataModel::deleteItem(const DMDataItem& item)
{
    for (auto iter = m_impl->totalRecords.begin(); iter != m_impl->totalRecords.end(); ++iter)
    {
        auto& info = iter->item;
        if (info.id == item.id)
        {
            beginResetModel();
            m_impl->totalRecords.erase(iter);
            calcPage();
            changePage(1);
            endResetModel();
            return;
        }
    }
}
void DMItemDataModel::deleteItem(const QModelIndex& index)
{
    if (!index.isValid())
    {
        return;
    }
    auto row = index.row();
    if (row < 0 || row > m_impl->totalRecords.size())
    {
        return;
    }
    beginResetModel();
    m_impl->totalRecords.removeAt(row);
    calcPage();
    changePage(1);
    endResetModel();
}
void DMItemDataModel::setObjList(const QList<DynamicWidgetInfo>& objList)
{
    for (auto& item : objList)
    {
        m_impl->header << item.widgetInfo.name;
        m_impl->findKey << item.widgetInfo.id;
    }
}
QVariant DMItemDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (role)
    {
    case Qt::TextAlignmentRole: return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);
    case Qt::DisplayRole:
    {
        if (orientation == Qt::Horizontal)
        {
            return m_impl->header.at(section);
        }
    }
    }
    return QVariant();
}

int DMItemDataModel::rowCount(const QModelIndex& /* parent */) const { return m_impl->pageRecords.count(); }
int DMItemDataModel::columnCount(const QModelIndex& /* parent */) const { return m_impl->header.size(); }
int DMItemDataModel::page() const { return m_impl->pageCount; }
int DMItemDataModel::totalCount() const { return m_impl->totalRecords.size(); }
void DMItemDataModel::changePage(int page)
{
    if (page <= 0 || page > m_impl->pageCount)
    {
        return;
    }

    m_impl->curPage = page;
    auto singlePagesCount = m_impl->singlePagesCount;
    auto totalNum = totalCount();
    auto beginIndex = (page - 1) * singlePagesCount;
    auto endIndex = (((totalNum - beginIndex - singlePagesCount) <= 0) ? totalNum : beginIndex + singlePagesCount);

    m_impl->pageRecords.clear();
    for (auto i = beginIndex; i != endIndex; ++i)
    {
        m_impl->pageRecords.append(&(m_impl->totalRecords)[i]);
    }
}
void DMItemDataModel::changePageUpdate(int page)
{
    beginResetModel();
    changePage(page);
    endResetModel();
}
void DMItemDataModel::calcPage()
{
    auto totalNum = totalCount();
    auto singlePagesCount = m_impl->singlePagesCount;
    m_impl->pageCount = (totalNum / singlePagesCount) + ((totalNum % singlePagesCount != 0) ? 1 : 0);
    m_impl->pageCount = (m_impl->pageCount <= 0 ? 1 : m_impl->pageCount);
}

Qt::ItemFlags DMItemDataModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return QAbstractItemModel::flags(index);

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return flags;
}
bool DMItemDataModel::judgeData(int row)
{
    if (row >= 0 && row <= m_impl->pageRecords.count())
    {
        return true;
    }
    else
    {
        return false;
    }
}

QVariant DMItemDataModel::data(const QModelIndex& index, int role) const
{
    int row = index.row();
    int col = index.column();
    if (row >= m_impl->pageRecords.size())
    {
        return QVariant();
    }
    auto& record = m_impl->pageRecords.at(row);
    auto& curItem = record->valueList;
    if (col >= curItem.size())
    {
        qWarning() << __FUNCTION__ << "列数量不匹配";
        return QVariant();
    }

    switch (role)
    {
    case Qt::TextAlignmentRole: return QVariant(Qt::AlignCenter);
    case Qt::DisplayRole:
    {
        return curItem.at(col);
    }
    }
    return QVariant();
}
Optional<DMDataItem> DMItemDataModel::getItem(const QModelIndex& index)
{
    using ResType = Optional<DMDataItem>;
    int row = index.row();
    if (row < 0 || row >= rowCount())
    {
        return ResType(ErrorCode::InvalidArgument, "无效的索引");
    }

    return ResType(m_impl->pageRecords.at(row)->item);
}
