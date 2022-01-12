#ifndef DEVICECONTROLXMLREADER_H
#define DEVICECONTROLXMLREADER_H

#include "ProtocolXmlTypeDefine.h"

#include <QDir>
#include <QDomElement>
#include <QFileInfoList>

class DeviceControlXMLReader
{
public:
    DeviceControlXMLReader();

    void loadControlAlignmentXML(QString filePath, ControlAlignment& controlAlignment);

    void parseControlAlignmentXML(QDomElement* docElem, ControlAlignment& controlAlignment);
};

#endif  // DEVICECONTROLXMLREADER_H
