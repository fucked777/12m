#ifndef IGUISERVICE_H
#define IGUISERVICE_H

#include "CppMicroServicesUtility.h"
#include <QDialog>
#include <QWidget>

class IGuiService
{
public:
    inline IGuiService() {}
    virtual inline ~IGuiService() {}
    /* 此接口创建的控件,创建完成后就不归此函数管了 */
    virtual QWidget* createWidget(const QString& /*subWidgetName*/) = 0;
};

#endif  // IGUISERVICE_H
