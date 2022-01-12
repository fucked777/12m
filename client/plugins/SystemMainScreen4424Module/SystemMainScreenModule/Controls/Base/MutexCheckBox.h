#ifndef MUTEXCHECKBOX_H
#define MUTEXCHECKBOX_H

#include <QCheckBox>
#include <QDomElement>
#include <QVariant>

#include "BaseWidget.h"

#include "CheckBox.h"

typedef struct MutexControl
{
    int deviceId;
    int modeId;
    int unitId;
    int targetId;

    QString paramId;
}Mutex;

class MutexCheckBox : public BaseWidget
{
    Q_OBJECT
    Q_PROPERTY(QVariant value READ getValue WRITE setValue)
public:
    explicit MutexCheckBox(const QDomElement& domEle, QWidget* parent = nullptr, const int deviceId = 0, const int modeId = 0);

    void setText(const QString& text);
    QString getText() const;

    void setValue(const QVariant& value);
    QVariant getValue() const;

    void setCheckable(const bool enabled);
    bool getCheckable() const;

    int getId() const;
    QList<int> getRelateId() const;

    bool isControlType();

signals:
    void signalsCmdDeviceJson(QString json);

public slots:
    void onStateChanged(int);

private:
    void initLayout();
    void parseNode(const QDomElement& domEle);

private:
    QCheckBox* mCheckBox = nullptr;
    QString d_text;

    bool d_checkable;
    bool d_isControlType;
    int d_deviceId;
    int d_modeId;
    int d_id;
    QList<int> d_childId;
    QList<MutexCheckBox*> d_childBoxs;
    Control d_control;
    Update d_update;

    QList<Mutex> d_mutexs;
};

#endif  // CHECKBOX_H
