#ifndef USERMODIFYWIDGET_H
#define USERMODIFYWIDGET_H

#include "UserManagerMessageDefine.h"
#include <QWidget>

namespace Ui
{
    class UserModifyWidget;
}

class UserModifyWidget : public QWidget
{
    Q_OBJECT

public:
    explicit UserModifyWidget(QWidget* parent = nullptr);
    ~UserModifyWidget();
    void setUser(User& user);

private:
    void initWidget();
    void initSlot();

signals:
    void signalModifyUser(const QByteArray& data);

public slots:
    void slotModifyUserResponse(const QByteArray& data);

private slots:
    void on_confirmBtn_clicked();

    void on_cancleBtn_clicked();

private:
    User mSelectUser;
    Ui::UserModifyWidget* ui;
};

#endif  // USERMODIFYWIDGET_H
