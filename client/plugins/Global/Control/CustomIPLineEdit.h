#ifndef CustomIPLineEdit_H
#define CustomIPLineEdit_H

#include <QEvent>
#include <QLineEdit>

class CustomIPLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    CustomIPLineEdit(QWidget* parent = 0);
    ~CustomIPLineEdit();

    void setText(const QString& strIP);
    QString text() const;
    bool isValidIP();

signals:
    void editingFinished();

protected:
    void paintEvent(QPaintEvent* event);
    bool eventFilter(QObject* obj, QEvent* ev);

    int getIndex(QLineEdit* pEdit);

private:
    void onEditingFinished();
    void onTextChanged();

private:
    QList<QLineEdit*> mLineEidtList;
    bool mIsValid = false;
};

#endif  // CustomIPLineEdit_H
