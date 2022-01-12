#ifndef CHECKBOXCOMBOBOX_H
#define CHECKBOXCOMBOBOX_H

#include <QComboBox>

class QListWidget;
class QListWidgetItem;
class QCheckBox;
class CheckBoxComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit CheckBoxComboBox(QWidget* parent = nullptr);

    void addItem(const QString& text, const QVariant& userData = QVariant());

    QStringList getCheckedItemsText();

    void clearSelected();

    void setCheckedItem(const QString& text);
    void setCheckedItem(const QVariant& userData);

    void hidePopup() override;

signals:
    // 选中改变时
    void signalSelectedChanged(QCheckBox* checkBox);

private slots:
    void slotStateChanged();
    void slotTextChanged();

private:
    QListWidget* mListWidget = nullptr;
    QLineEdit* mLineEdit = nullptr;
    QString mSelectedText;

    // QComboBox interface
public:
    void showPopup();
};

#endif  // CHECKBOXCOMBOBOX_H
