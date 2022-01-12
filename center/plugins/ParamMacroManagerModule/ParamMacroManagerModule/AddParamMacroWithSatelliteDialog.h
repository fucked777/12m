#ifndef ADDPARAMMACROWITHSATELLITEDIALOG_H
#define ADDPARAMMACROWITHSATELLITEDIALOG_H

#include <QDialog>

namespace Ui
{
    class AddParamMacroWithSatelliteDialog;
}
#include "ParamMacroMessage.h"
#include "SystemWorkMode.h"

class AddParamMacroWithSatelliteDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddParamMacroWithSatelliteDialog(QWidget* parent = nullptr);
    ~AddParamMacroWithSatelliteDialog();

    ParamMacroData getCurrentParamMacro() const;

private:
    void init();

private slots:
    void on_add_clicked();

    void on_cancel_clicked();

    void on_desc_textChanged();

    void on_satelliteCombo_currentIndexChanged(const QString& arg1);

private:
    Ui::AddParamMacroWithSatelliteDialog* ui;
    ParamMacroData mCurrentParamMacro;
    QMap<QString, QString> m_currentWorkMode;
    QMap<QString, QStringList> mWorkModeDescMap;
    QMap<QString, QMap<SystemWorkMode, int>> m_codeWorkModeMap;
    ParamMacroDataList mParamMacroList;
};

#endif  // ADDPARAMMACROWITHSATELLITEDIALOG_H
