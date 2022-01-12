#ifndef PARAMMACROTABLEVIEWDELEGATE_H
#define PARAMMACROTABLEVIEWDELEGATE_H
#include <QItemDelegate>
#include <QPushButton>
class ParamMacroTableViewDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    explicit ParamMacroTableViewDelegate(QObject* parent = nullptr);

public:
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;

signals:
    void signal_show(const QModelIndex& index);
    void signal_edit(const QModelIndex& index);
    void signal_del(const QModelIndex& index);

private:
    QPushButton* m_pbtn = nullptr;
    QMap<QModelIndex, QVector<QStyleOptionButton*>> m_mapStyleBtn;  ///< 每个单元格管理的按钮
    QPair<bool, QStyleOptionButton*> m_pairClickedButton;           ///< 记录被按压的按钮坐标

    int m_iBtnHeight = 25;     ///< 按钮的高度
    int m_iBtnWidth = 40;      ///< 按钮的宽度
    int m_iIntervalWidth = 5;  ///< 按钮间隔距离
};

#endif  // PARAMMACROTABLEVIEWDELEGATE_H
