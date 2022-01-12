#ifndef GRIDLAYOUT_H
#define GRIDLAYOUT_H

#include <QDomElement>

#include "BaseWidget.h"

class GridLayout : public BaseWidget
{
    Q_OBJECT
public:
    explicit GridLayout(const QDomElement& domEle, QWidget* parent = nullptr);

private:
    void initLayout();
    void parseNode(const QDomElement& domEle);

    bool isValidNode(const QDomElement& domEle, QString& error) const;
};
#endif  // GRIDLAYOUT_H
