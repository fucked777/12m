#include "GridLayout.h"

#include <QDebug>
#include <QGridLayout>

#include "MainScreenControlFactory.h"

GridLayout::GridLayout(const QDomElement& domEle, QWidget* parent)
    : BaseWidget(domEle, parent)
{
    initLayout();

    parseNode(domEle);
}

void GridLayout::initLayout()
{
    auto gridLayout = new QGridLayout;

    setLayout(gridLayout);
}
void GridLayout::parseNode(const QDomElement& domEle)
{
    if (domEle.isNull())
    {
        return;
    }

    auto gridLayout = qobject_cast<QGridLayout*>(layout());

    // 设置间隔
    if (domEle.hasAttribute("space"))
    {
        auto space = domEle.attribute("space").toInt();
        gridLayout->setSpacing(space);
    }

    if (domEle.hasAttribute("horizontalSpacing"))
    {
        auto hSpace = domEle.attribute("horizontalSpacing").toInt();
        gridLayout->setHorizontalSpacing(hSpace);
    }
    else
    {
        gridLayout->setHorizontalSpacing(3);
    }
    if (domEle.hasAttribute("verticalSpacing"))
    {
        auto vSpace = domEle.attribute("verticalSpacing").toInt();
        gridLayout->setVerticalSpacing(vSpace);
    }
    else
    {
        gridLayout->setVerticalSpacing(3);
    }

    // 设置内容边距
    if (domEle.hasAttribute("contentsMargins"))
    {
        QStringList list = domEle.attribute("contentsMargins").split(",", QString::SkipEmptyParts);
        if (list.size() == 4)
        {
            gridLayout->setContentsMargins(list[0].toInt(), list[1].toInt(), list[2].toInt(), list[3].toInt());
        }
    }
    else
    {
        gridLayout->setContentsMargins(3, 3, 3, 3);
    }

    auto alignment = domEle.attribute("Alignment");

    QDomNode domNode = domEle.firstChild();
    while (!domNode.isNull())
    {
        QDomElement subDomElement = domNode.toElement();
        if (!subDomElement.isNull())
        {
            QString tagName = subDomElement.tagName();

            QWidget* widget = nullptr;
            widget = MainScreenControlFactory::createControl(tagName, subDomElement, this);

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
