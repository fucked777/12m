#ifndef XCOMBOBOX_H
#define XCOMBOBOX_H

#include <QComboBox>
#include <QKeyEvent>
#include <QListView>
#include <QStandardItemModel>
#include <QVariant>
class QLineEdit;
class QListView;

struct ItemInfo
{
    int idx;
    QString str;
    QVariant userData;
    bool bChecked;

    ItemInfo()
    {
        idx = -1;
        str = QString("");
        userData = QVariant();
        bChecked = false;
    }
};

// 事件过滤器
class KeyPressEater : public QObject
{
    Q_OBJECT
public:
    KeyPressEater(QObject* parent = nullptr)
        : QObject(parent)
    {
    }
    ~KeyPressEater() {}
signals:
    void sigActivated(int idx);

protected:
    bool eventFilter(QObject* obj, QEvent* event)
    {
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Space)
            {
                QListView* lstV = qobject_cast<QListView*>(obj);
                if (nullptr != lstV)
                {
                    int idx = lstV->currentIndex().row();
                    if (-1 != idx)
                    {
                        emit sigActivated(idx);
                    }
                }
            }
            else if (keyEvent->key() == Qt::Key_Up || keyEvent->key() == Qt::Key_Down)
            {
                return QObject::eventFilter(obj, event);
            }

            return true;
        }
        else
        {
            // standard event processing
            return QObject::eventFilter(obj, event);
        }
    }
};

class XComboBox : public QComboBox
{
    Q_OBJECT

public:
    XComboBox(QWidget* parent = Q_NULLPTR);
    ~XComboBox();

    // 添加item
    void AddItem(const QString& str, bool bChecked, QVariant userData = QVariant());
    void AddItems(const QList<ItemInfo>& lstItemInfo);
    void AddItems(const QMap<QString, bool>& mapStrChk);
    void AddItems(const QList<QString>& lstStr);
    // 删除item
    void RemoveItem(int idx);
    // 清空item
    void Clear();
    void ClearSelect();
    // 获取选中的数据字符串列表
    QStringList GetSelItemsText();
    // 获取选中item的信息
    QList<ItemInfo> GetSelItemsInfo();
    //获取当前的选中信息
    quint8 GetSelItemsDatas();
    // 获取item文本
    QString GetItemText(int idx);
    // 获取item文本
    QString GetItemTextByData(QVariant data);
    // 获取item信息
    ItemInfo GetItemInfo(int idx);
    //设置item选中状态
    void SetItemChecked(int idx, bool isChecked);

    //设置Item 选中状态
    void SetItemCheckByData(QVariant data, bool isChecked);

    void SetItemChecked(const QString& itemName, bool isChecked);

    void SetItemChecked(QMap<int, bool> map);
signals:
    // popup显示信号
    void showingPopup();
    // popup隐藏信号
    void hidingPopup();
    // 选择值改变信号
    void signalValueChanged(int index);

protected:
    void showPopup();
    // 重写QComboBox的hidePopup函数
    // 目的选择过程中，不隐藏listview
    void hidePopup();
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);

private:
    void UpdateText();

private slots:
    void sltActivated(int idx);

private:
    QLineEdit* pLineEdit;
    QListView* pListView;
    QStandardItemModel m_model;
};
#endif  // XCOMBOBOX_H
