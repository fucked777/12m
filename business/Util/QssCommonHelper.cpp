#include "QssCommonHelper.h"
#include "PlatformInterface.h"
#include <QApplication>
#include <QFile>
#include <QWidget>

void QssCommonHelper::setWidgetStyle(QWidget* widget, const QString& fileName)
{
    auto filePath = PlatformConfigTools::configBusiness(QString("QssFile/%1").arg(fileName));
    QFile qss(filePath);
    if (!qss.open(QFile::ReadOnly))  // 20201217
    {
        return;
    }
    widget->setStyleSheet(qss.readAll());
}
