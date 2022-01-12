#include "DeviceControlXMLReader.h"

DeviceControlXMLReader::DeviceControlXMLReader() {}

void DeviceControlXMLReader::loadControlAlignmentXML(QString filePath, ControlAlignment& controlAlignment)
{
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return;
    }
    QDomDocument doc;
    if (!doc.setContent(&file))
    {
        file.close();
        return;
    }
    QDomElement root = doc.documentElement();
    //遍历
    parseControlAlignmentXML(&root, controlAlignment);

    file.close();
}

void DeviceControlXMLReader::parseControlAlignmentXML(QDomElement* docElem, ControlAlignment& controlAlignment)
{
    QDomNode node = docElem->firstChild();
    while (!node.isNull())
    {
        QDomElement element = node.toElement();
        if (!element.isNull())
        {
            QString tagName = element.tagName();
            if (tagName == "currentProjectID")
            {
                controlAlignment.currentProjectID = element.attribute("ID", "4424");
            }
            else if (tagName == "NameControl")
            {
                controlAlignment.nameControlAlignmentType = element.attribute("Alignment", "0x02");
            }
            parseControlAlignmentXML(&element, controlAlignment);
        }
        node = node.nextSibling();
    }
}
