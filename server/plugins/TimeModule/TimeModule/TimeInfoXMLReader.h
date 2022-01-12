#ifndef TIMEINFOCONFIGREADER_H
#define TIMEINFOCONFIGREADER_H

#include "TimeMessageDefine.h"

class TimeInfoXMLReader
{
public:
    TimeInfoXMLReader();

    bool getTimeConfigInfo(TimeConfigInfo& timeConfigInfo, QString* errorMsg = nullptr) const;
    bool saveTimeConfigInfo(const TimeConfigInfo& timeConfigInfo, QString* errorMsg = nullptr) const;

private:
    bool parseTimeConfigInfo(const QString& filePath, TimeConfigInfo& timeConfigInfo, QString* errorMsg = nullptr) const;
    bool saveTimeConfigInfo(const TimeConfigInfo& timeConfigInfo, const QString& filePath, QString* errorMsg = nullptr) const;

    QString mFilePath;
};

#endif  // TIMEINFOCONFIGREADER_H
