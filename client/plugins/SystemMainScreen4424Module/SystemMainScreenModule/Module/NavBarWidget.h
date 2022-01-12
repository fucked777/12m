#ifndef NAVBAR_H
#define NAVBAR_H

#include <QWidget>

namespace Ui
{
    class NavBarWidget;
}

class QListWidgetItem;
// 导航栏模块
class NavBarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NavBarWidget(QWidget* parent = nullptr);
    ~NavBarWidget();

    void appendItem(const QString& label, const QString& moduleName = QString());

public slots:
    void setCurrentRow(int row);

signals:
    void signalCurrentItemClicked(QListWidgetItem* item);

private:
    Ui::NavBarWidget* ui;
};

#endif  // NAVBAR_H
