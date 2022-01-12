#ifndef PROTOCOLXMLTYPEHELPER_H
#define PROTOCOLXMLTYPEHELPER_H

#include "ProtocolXmlTypeDefine.h"

class ProtocolXmlTypeHelper
{
public:
    static AttributeType getAttributeType(const QString& str);
    static DisplayFormat getDisplayFormat(const QString& str);
    static int getAttributeTypeByteLength(const QString& str);
    static int getAttributeTypeByteLength(const AttributeType& type);
};

#endif  // PROTOCOLXMLTYPEHELPER_H
