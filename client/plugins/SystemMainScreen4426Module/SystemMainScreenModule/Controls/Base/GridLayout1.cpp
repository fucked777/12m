#include "GridLayout.h"

#include <QDebug>
#include <QGridLayout>

#include "MainScreenControlFactory.h"

GridLayout::GridLayout(const QDomElement& domEle, QWidget* parent)
    : QWidget(parent)
{
    parseNode(domEle);
}
void GridLayout::parseNode(const QDomElement& domEle)
{
    if (domEle.isNull())
    {
        return;
    }

    auto gridLayout = new QGridLayout;
    gridLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(gridLayout);

    auto alignment = domEle.attribute("Alignment");

    if (domEle.hasAttribute("Space"))
    {
        auto space = domEle.attribute("Space").toInt();
        gridLayout->setSpacing(space);
    }

    QDomNode domNode = domEle.firstChild();
    while (!domNode.isNull())
    {
        QDomElement subDomElement = domNode.toElement();
        if (!subDomElement.isNull())
        {
            QString tagName = subDomElement.tagName();

            QWidget* widget = nullptr;
            widget = MainScreenControlFactory::createControl(tagName, subDomElement);

            // 检查是否是有效的节点
            QString errorMsg;
            if (!isValidNode(subDomElement, errorMsg))
            {
                qDebug() << QString("GridLayout子节点%1无效:%2").arg(subDomElement.tagName()).arg(errorMsg);
                domNode = domNode.nextSibling();
                continue;
            }

            if (widget != nullptr)
            {
                auto row = subDomElement.attribute("row").toInt();
                auto col = subDomElement.attribute("col").toInt();
                auto rowSpan = subDomElement.attribute("rowSpan").toInt();
                auto columnSpan = subDomElement.attribute("columnSpan").toInt();

                gridLayout->setRowStretch(row, 1);
                gridLayout->setColumnStretch(col, 1);

                if (alignment == "Center")
                {
                    gridLayout->addWidget(widget, row, col, rowSpan, columnSpan, Qt::AlignCenter);
                }
                else if (alignment == "Left")
                {
                    gridLayout->addWidget(widget, row, col, rowSpan, columnSpan, Qt::AlignLeft);
                }
                else if (alignment == "Right")
                {
                    gridLayout->addWidget(widget, row, col, rowSpan, columnSpan, Qt::AlignRight);
                }
                else
                {
                    gridLayout->addWidget(widget, row, col, rowSpan, columnSpan);
                }
            }
        }
        domNode = domNode.nextSibling();
    }
}

bool GridLayout::isValidNode(const QDomElement& domEle, QString& error) const
{
    bool ok;
    domEle.attribute("row").toInt(&ok);
    if (!ok)
    {
        error = QString("row属性配置错误，配置值为：%1").arg(domEle.attribute("row", "空"));
        return false;
    }

    domEle.attribute("col").toInt(&ok);
    if (!ok)
    {
        error = QString("col属性配置错误，配置值为：%1").arg(domEle.attribute("col", "空"));
        return false;
    }

    domEle.attribute("rowSpan").toInt(&ok);
    if (!ok)
    {
        error = QString("rowSpan属性配置错误，配置值为：%1").arg(domEle.attribute("rowSpan", "空"));
        return false;
    }

    domEle.attribute("columnSpan").toInt(&ok);
    if (!ok)
    {
        error = QString("columnSpan属性配置错误，配置值为：%1").arg(domEle.attribute("columnSpan", "空"));
        return false;
    }

    return true;
}
