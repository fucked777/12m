#ifndef COPYPARAMMACRODIALOG_H
#define COPYPARAMMACRODIALOG_H

#include <QDialog>

#include "ParamMacroMessage.h"

enum class CopyMode
{
    NewParamMacro = 1,  // 复制新参数宏
    ExistParamMacro     // 复制已存在的参数宏
};

namespace Ui
{
    class CopyParamMacroDialog;
}

class QCheckBox;
class CopyParamMacroDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CopyParamMacroDialog(QWidget* parent = nullptr);
    ~CopyParamMacroDialog();

    ParamMacroData getCopyParamMacroData() const;

    CopyMode getCopyMode();

private:
    void init();
    void initSlot();

    // 更新可选模式
    void updateSelectableMode();

private slots:
    // 复制参数宏模式改变
    void slotCopyModeChanged(const QString& text);
    // 复制到新参数宏界面的源参数宏改变
    void slotNewCreateScrParamMacroComboBoxCurrentTextChanged(const QString& text);

    void slotExistSrcParamMacroComboBoxCurrentTextChanged(const QString& text);
    void slotExistDescParamMacroComboBoxCurrentTextChanged(const QString& text);

    // 确定按钮点击
    void slotOkBtnClicked();

    void slotCancelBtnClicked();

private:
    Ui::CopyParamMacroDialog* ui;

    QList<ParamMacroData> mParamMacroDataList;
    ParamMacroData mCopyParamMacro;

    QList<QCheckBox*> mModeCheckBoxList;

    ParamMacroData mSrcParamMacroData;   // 复制到已存在参数宏界面 的源参数宏数据
    ParamMacroData mDestParamMacroData;  // 复制到已存在参数宏界面 的目的参数宏数据
};

#endif  // COPYPARAMMACRODIALOG_H
