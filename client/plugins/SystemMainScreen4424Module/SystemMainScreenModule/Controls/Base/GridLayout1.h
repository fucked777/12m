#ifndef GRIDLAYOUT_H
#define GRIDLAYOUT_H

#include <QDomElement>
#include <QWidget>

class GridLayout : public QWidget
{
    Q_OBJECT
public:
    explicit GridLayout(const QDomElement& domEle, QWidget* parent = nullptr);

private:
    void parseNode(const QDomElement& domEle);

    bool isValidNode(const QDomElement& domEle, QString& error) const;
};
#endif  // GRIDLAYOUT_H
