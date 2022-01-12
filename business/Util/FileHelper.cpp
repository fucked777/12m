#include "FileHelper.h"
#include <QDebug>
#include <QFileInfoList>
void FileHelper::GetDirFileByFilters(const QDir &fromDir,
                                     const QStringList &filters,
                                     QList<QString> &filePathList)
{

    QFileInfoList fileInfoList = fromDir.entryInfoList(filters, QDir::AllDirs | QDir::Files);
    for (auto &fileInfo : fileInfoList) {
        if (fileInfo.fileName() == "." || fileInfo.fileName() == "..")
            continue;
        if (fileInfo.isDir()) {
            GetDirFileByFilters(fileInfo.filePath(), filters, filePathList);
        }
        else {
            filePathList.append(fileInfo.absoluteFilePath());
        }
    }
}
