#ifndef NAMELABEL_H
#define NAMELABEL_H

#include <QDomElement>
#include <QLabel>

#include "BaseWidget.h"

// 名称显示控件
class NameLabel : public BaseWidget
{
    Q_OBJECT
    struct NameLabelInfo
    {
        QString text;
    };

public:
    NameLabel(const QDomElement& domEle, QWidget* parent = nullptr);

    void setText(const QString& text);
    QString getText() const;

private:
    void initLayout();
    void parseNode(const QDomElement& domEle);

private:
    QLabel* mLabel = nullptr;

    NameLabelInfo mNameLableInfo;
};

#endif  // NAMELABEL_H
