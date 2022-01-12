#ifndef MMDynamicParamDialog_H
#define MMDynamicParamDialog_H

#include <QDialog>

#include "ProtocolXmlTypeDefine.h"

namespace Ui
{
    class MMDynamicParamDialog;
}

class MMDynamicParamDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MMDynamicParamDialog(QWidget* parent = nullptr);
    ~MMDynamicParamDialog();

    enum WidgetMode
    {
        Add = 0,  // 添加
        Edit,     // 编辑
        Look      // 查看
    };

    void setWidgetMode(const WidgetMode& currentUIMode);

    void setParamValueMap(const QMap<int, QMap<QString, QVariant>>& paramValueMap);
    QMap<int, QMap<QString, QVariant>> getParamValueMap();

    void setRowParamAttribute(const QList<ParameterAttribute>& paramAttriList);

    void setEnumMap(const QMap<QString, DevEnum>& enumMap);

    void setNumRange(const int minValue, const int maxValue);

    void setButtonEnable(bool isEnable);

protected:
    void showEvent(QShowEvent* event) override;

private slots:
    void on_okBtn_clicked();

    void on_cancelBtn_clicked();

    void on_addBtn_clicked();

    void on_removeBtn_clicked();

    void on_addBtn_2_clicked();

    void on_removeBtn_2_clicked();

    void on_addBtn_3_clicked();

    void on_removeBtn_3_clicked();

    void on_addBtn_4_clicked();

    void on_removeBtn_4_clicked();

private:
    Ui::MMDynamicParamDialog* ui;

    WidgetMode mCurrentMode = WidgetMode::Add;

    QList<ParameterAttribute> mParamAttriList;

    QMap<int, QMap<QString, QVariant>> mParamValueMap;
    QMap<QString, DevEnum> mEnumMap;
    int mMinValue;
    int mMaxValue;
    bool mEnabled;
};

#endif  // MMDynamicParamDialog_H
