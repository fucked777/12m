#ifndef ADDDATATRANSMISSIONCENTER_H
#define ADDDATATRANSMISSIONCENTER_H

#include "DataTransmissionCenterDefine.h"
#include <QDialog>

namespace Ui
{
    class AddDataTransmissionCenter;
}

class DataTransmissionCenter;
class AddDataTransmissionCenter : public QDialog
{
    Q_OBJECT

public:
    explicit AddDataTransmissionCenter(DataTransmissionCenter* parent);
    ~AddDataTransmissionCenter();

    DataTransmissionCenterData data() const;
    void setData(const DataTransmissionCenterData& data);

private slots:
    void on_desc_textChanged();

    void on_cancelBtn_clicked();

private:
    void ok();
    void addACK(const QByteArray& data);
    void modifyACK(const QByteArray& data);

private:
    Ui::AddDataTransmissionCenter* ui;
    DataTransmissionCenter* m_parent;
    DataTransmissionCenterData m_centerData;
    bool m_isAdd{ true };
};

#endif  // ADDDATATRANSMISSIONCENTER_H
