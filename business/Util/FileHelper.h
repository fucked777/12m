#ifndef FILEHELPER_H
#define FILEHELPER_H

#include <QDir>
#include <QStringList>
class FileHelper
{
public:
    static void GetDirFileByFilters(const QDir &fromDir,
                                    const QStringList &filters,
                                    QList<QString> &filePathList);
};

#endif  // FILEHELPER_H
