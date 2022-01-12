#ifndef COPYPOINTFREQDIALOG_H
#define COPYPOINTFREQDIALOG_H

#include <QDialog>
#include <QMap>

namespace Ui
{
    class CopyPointFreqDialog;
}

class QTreeWidget;
class CopyPointFreqDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CopyPointFreqDialog(QWidget* parent = nullptr);
    ~CopyPointFreqDialog();

    void setPointFreqMap(const QMap<QString, QTreeWidget*> pointFreqMap);

private slots:
    void on_cancleBtn_clicked();

    void on_okBtn_clicked();

private:
    Ui::CopyPointFreqDialog* ui;

    QMap<QString, QTreeWidget*> mPointFreqMap;
};

#endif  // COPYPOINTFREQDIALOG_H
