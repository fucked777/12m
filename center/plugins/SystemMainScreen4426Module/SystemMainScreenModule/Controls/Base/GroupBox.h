#ifndef GROUPBOX_H
#define GROUPBOX_H

#include <QDomElement>

#include "BaseWidget.h"

class QGroupBox;
class GroupBox : public BaseWidget
{
    Q_OBJECT
    struct GroupBoxInfo
    {
        QString text;
    };

public:
    explicit GroupBox(const QDomElement& domEle, QWidget* parent = nullptr);

    void setTitle(const QString& title);
    QString getTitle() const;

private:
    void initLayout();
    void parseNode(const QDomElement& domEle);

private:
    QGroupBox* mGroupBox = nullptr;
    GroupBoxInfo mGroupBoxInfo;
};

#endif  // GROUPBOX_H
