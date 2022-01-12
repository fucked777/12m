#ifndef USERREGISTERWIDGET_H
#define USERREGISTERWIDGET_H

#include "UserManagerMessageDefine.h"
#include <QWidget>

namespace Ui
{
    class UserRegisterWidget;
}

class UserRegisterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit UserRegisterWidget(QWidget* parent = nullptr);
    ~UserRegisterWidget();

private:
    void initWidget();
    void initSlot();

signals:
    void signalUserRegister(const QByteArray& data);

public slots:
    void slotUserRegisterResponse(const QByteArray& data);

private slots:
    void on_confirmBtn_clicked();

    void on_cancleBtn_clicked();

private:
    Ui::UserRegisterWidget* ui;
};

#endif  // USERREGISTERWIDGET_H
