#ifndef ADDEPHEMERISDATADIALOG_H
#define ADDEPHEMERISDATADIALOG_H

#include "EphemerisDataDefine.h"
#include <QDialog>

namespace Ui
{
    class AddEphemerisDataDialog;
}
class EphemerisData;
struct SatelliteManagementData;
using SatelliteManagementMap = QMap<QString, SatelliteManagementData>;
using SatelliteManagementList = QList<SatelliteManagementData>;
class AddEphemerisDataDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddEphemerisDataDialog(EphemerisData* parent);
    ~AddEphemerisDataDialog();

    EphemerisDataData data() const;
    void setData(const SatelliteManagementMap& smm, const EphemerisDataData& data);
    void setData(const SatelliteManagementMap& smm);

private:
    void ok();
    void satellitCodeChange(const QString& arg1);
    void addACK(const QByteArray& data);
    void modifyACK(const QByteArray& data);

private:
    Ui::AddEphemerisDataDialog* ui;
    EphemerisData* m_parent;
    EphemerisDataData m_taskCenter;
    SatelliteManagementMap m_map;
    bool m_isAdd{ true };
};

#endif  // ADDEPHEMERISDATADIALOG_H
