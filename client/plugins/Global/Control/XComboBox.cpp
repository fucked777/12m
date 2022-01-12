#include "XComboBox.h"

#include <QDebug>
#include <QLineEdit>
#include <QMouseEvent>

XComboBox::XComboBox(QWidget* parent)
    : QComboBox(parent)
{
    pLineEdit = new QLineEdit(this);
    pLineEdit->setReadOnly(true);
    this->setLineEdit(pLineEdit);
    this->lineEdit()->disconnect();

    KeyPressEater* keyPressEater = new KeyPressEater(this);
    pListView = new QListView(this);
    pListView->installEventFilter(keyPressEater);
    this->setView(pListView);

    this->setModel(&m_model);

    connect(this, SIGNAL(activated(int)), this, SLOT(sltActivated(int)));
    connect(keyPressEater, SIGNAL(sigActivated(int)), this, SLOT(sltActivated(int)));
}

XComboBox::~XComboBox() {}

void XComboBox::AddItem(const QString& str, bool bChecked /*= false*/, QVariant userData /*= QVariant()*/)
{
    QStandardItem* item = new QStandardItem(str);
    item->setCheckable(true);
    item->setCheckState(bChecked ? Qt::Checked : Qt::Unchecked);
    item->setData(userData, Qt::UserRole + 1);
    m_model.appendRow(item);

    UpdateText();
    emit signalValueChanged(m_model.rowCount() - 1);
}

void XComboBox::AddItems(const QList<ItemInfo>& lstItemInfo)
{
    for (auto a : lstItemInfo)
    {
        AddItem(a.str, a.bChecked, a.userData);
    }
}

void XComboBox::AddItems(const QMap<QString, bool>& mapStrChk)
{
    for (auto it = mapStrChk.begin(); it != mapStrChk.end(); ++it)
    {
        AddItem(it.key(), it.value());
    }
}

void XComboBox::AddItems(const QList<QString>& lstStr)
{
    for (auto a : lstStr)
    {
        AddItem(a, false);
    }
}

void XComboBox::RemoveItem(int idx)
{
    m_model.removeRow(idx);
    UpdateText();
    // emit signalValueChanged(idx);
}

void XComboBox::Clear()
{
    m_model.clear();
    UpdateText();
    // emit signalValueChanged(-1);
}

void XComboBox::ClearSelect()
{
    auto totalCount = m_model.rowCount();
    for (auto i = 0; i < totalCount; i++)
    {
        QStandardItem* item = m_model.item(i);
        item->setCheckState(Qt::Unchecked);
    }

    UpdateText();
}

QStringList XComboBox::GetSelItemsText()
{
    QStringList lst;
    QString str = pLineEdit->text();
    if (str.isEmpty())
    {
        return lst;
    }
    else
    {
        return pLineEdit->text().split(",");
    }
}

QList<ItemInfo> XComboBox::GetSelItemsInfo()
{
    QList<ItemInfo> lstInfo;
    for (int i = 0; i < m_model.rowCount(); i++)
    {
        QStandardItem* item = m_model.item(i);
        if (item->checkState() == Qt::Unchecked)
            continue;

        ItemInfo info;
        info.idx = i;
        info.str = item->text();
        info.bChecked = true;
        info.userData = item->data(Qt::UserRole + 1);

        lstInfo << info;
    }

    return lstInfo;
}

quint8 XComboBox::GetSelItemsDatas()
{
    QList<ItemInfo> lstInfo = GetSelItemsInfo();
    quint8 result = 0x00;
    bool ok;
    int temp = 0;
    for (auto item : lstInfo)
    {
        temp = item.userData.toInt(&ok) - 1;
        if (ok)
        {
            quint8 value = 1;
            while (temp--)
            {
                value = value << 1;
            }
            result |= value;
        }
    }
    return result;
}

QString XComboBox::GetItemText(int idx)
{
    if (idx < 0 || idx >= m_model.rowCount())
    {
        return QString("");
    }

    return m_model.item(idx)->text();
}

QString XComboBox::GetItemTextByData(QVariant data)
{
    QString text = "";
    for (int i = 0; i < m_model.rowCount(); i++)
    {
        QStandardItem* item = m_model.item(i);
        if (item->data() == data)
        {
            text = item->text();
        }
    }

    return text;
}

ItemInfo XComboBox::GetItemInfo(int idx)
{
    ItemInfo info;
    if (idx < 0 || idx >= m_model.rowCount())
    {
        return info;
    }

    QStandardItem* item = m_model.item(idx);
    info.idx = idx;
    info.str = item->text();
    info.bChecked = (item->checkState() == Qt::Checked);
    info.userData = item->data(Qt::UserRole + 1);

    return info;
}

void XComboBox::SetItemChecked(int idx, bool isChecked)
{
    if (idx < 0 || idx >= m_model.rowCount())
        return;

    QStandardItem* item = m_model.item(idx);
    item->setCheckState(isChecked ? Qt::Checked : Qt::Unchecked);

    UpdateText();
    // emit signalValueChanged(idx);
}

void XComboBox::SetItemCheckByData(QVariant data, bool isChecked)
{
    for (auto idx = 0; idx < m_model.rowCount(); idx++)
    {
        QStandardItem* item = m_model.item(idx);
        if (item->data() == data)
        {
            item->setCheckState(isChecked ? Qt::Checked : Qt::Unchecked);

            UpdateText();
            emit signalValueChanged(idx);
        }
    }
}

void XComboBox::SetItemChecked(const QString& itemName, bool isChecked)
{
    int totalCount = m_model.rowCount();
    for (auto i = 0; i < totalCount; i++)
    {
        QStandardItem* item = m_model.item(i);
        auto currentText = item->text();
        if (itemName == currentText)
        {
            item->setCheckState(isChecked ? Qt::Checked : Qt::Unchecked);
            UpdateText();
            // signalValueChanged(i);
            return;
        }
    }
}

void XComboBox::SetItemChecked(QMap<int, bool> map)
{
    for (auto idx : map.keys())
    {
        bool isChecked = map[idx];
        if (idx < 0 || idx >= m_model.rowCount())
            return;

        QStandardItem* item = m_model.item(idx);
        item->setCheckState(isChecked ? Qt::Checked : Qt::Unchecked);
    }
    UpdateText();
    // signalValueChanged();
}

void XComboBox::showPopup()
{
    emit showingPopup();
    QComboBox::showPopup();
}

void XComboBox::hidePopup()
{
    int width = this->view()->width();
    int height = this->view()->height();
    int x = QCursor::pos().x() - mapToGlobal(geometry().topLeft()).x() + geometry().x();
    int y = QCursor::pos().y() - mapToGlobal(geometry().topLeft()).y() + geometry().y();

    QRect rectView(0, this->height(), width, height);
    if (!rectView.contains(x, y))
    {
        emit hidingPopup();
        QComboBox::hidePopup();
    }
}

void XComboBox::mousePressEvent(QMouseEvent* event)
{
    QComboBox::mousePressEvent(event);
    event->accept();
}

void XComboBox::mouseReleaseEvent(QMouseEvent* event)
{
    QComboBox::mouseReleaseEvent(event);
    event->accept();
}

void XComboBox::mouseMoveEvent(QMouseEvent* event)
{
    QComboBox::mouseMoveEvent(event);
    event->accept();
}

void XComboBox::UpdateText()
{
    QStringList lstTxt;
    for (int i = 0; i < m_model.rowCount(); ++i)
    {
        QStandardItem* item = m_model.item(i);
        if (item->checkState() == Qt::Unchecked)
            continue;

        lstTxt << item->text();
    }

    pLineEdit->setText(lstTxt.join(","));
    pLineEdit->setToolTip(lstTxt.join("\n"));
}

void XComboBox::sltActivated(int idx)
{
    QStandardItem* item = m_model.item(idx);
    if (nullptr == item)
        return;

    Qt::CheckState state = (item->checkState() == Qt::Checked) ? Qt::Unchecked : Qt::Checked;
    item->setCheckState(state);

    UpdateText();
    emit signalValueChanged(idx);
}
