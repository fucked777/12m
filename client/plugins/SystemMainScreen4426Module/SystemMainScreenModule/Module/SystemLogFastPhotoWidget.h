#ifndef SYSTEMLOGFASTPHOTOWIDGET_H
#define SYSTEMLOGFASTPHOTOWIDGET_H

#include <QWidget>

namespace Ui
{
    class SystemLogFastPhotoWidget;
}

struct SystemLogData;
class SystemLogTableModel;
class QMenu;
class SystemLogFastPhotoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SystemLogFastPhotoWidget(QWidget* parent = nullptr);
    ~SystemLogFastPhotoWidget();

    void setFastLogData(QList<SystemLogData> systemLogDatas);

private:
    void init();

public slots:
    // 表格点击
    void slotLogTableClicked(const QModelIndex& index);

private:
    Ui::SystemLogFastPhotoWidget* ui;

    QMenu* m_menu = nullptr;
    SystemLogTableModel* m_systemLogTableFastPhotoModel = nullptr;
};

#endif  // SYSTEMLOGFASTPHOTOWIDGET_H
