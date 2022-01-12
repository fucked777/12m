#ifndef ADDDATATRANSMISSIONCENTER_H
#define ADDDATATRANSMISSIONCENTER_H

#include "StationResManagementDefine.h"
#include <QDialog>

namespace Ui
{
    class AddStationResManagement;
}
class StationResManagement;
class AddStationResManagement : public QDialog
{
    Q_OBJECT

public:
    explicit AddStationResManagement(StationResManagement* parent);
    ~AddStationResManagement();

    StationResManagementData data() const;
    void setData(const StationResManagementData& data);

private slots:
    void on_desc_textChanged();

private:
    void ok();
    void addACK(const QByteArray& data);
    void modifyACK(const QByteArray& data);

private:
    Ui::AddStationResManagement* ui;
    StationResManagementData m_centerData;
    StationResManagement* m_parent;
    bool m_isAdd{ true };
};

#endif  // ADDDATATRANSMISSIONCENTER_H
