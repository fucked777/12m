#ifndef SYSTEMBLOCKDIAGRAMWIDGET_H
#define SYSTEMBLOCKDIAGRAMWIDGET_H

#include <QWidget>

namespace cppmicroservices
{
    class BundleContext;
}

struct TwoValueSwitchTypeInfo;
class SystemBlockDiagramWidgetImpl;
class SystemBlockDiagramWidget : public QWidget
{
    Q_OBJECT

public:
    SystemBlockDiagramWidget(cppmicroservices::BundleContext context, QWidget* parent = nullptr);
    ~SystemBlockDiagramWidget();

private:
    /* 图元双击事件 */
    void itemDoubleClick(const QString& id, const QVariant&);
    /* 双击的图元-->状态显示的图元 */
    void deviceStatus(const QVariant& info);
    /* 双击的图元-->二值开关图元 */
    void twoValueSwitch(const QVariant& info, const QVariant&);
    /* 双击的图元-->分系统的图元 */
    void subSystem(const QVariant& info);

    /* 刷新图元 */
    /**
     * @brief refreshItem
     * @param id
     * @param status
     * @param type
     * 0:TwoValueSwitchType表示开关类型
     * 1:DeviceStatusType表示设备类型
     * 2:BorderColorType表示边框颜色类型
     */
    void refreshItem(const QString& id, const QVariant& status, const int type);

signals:
    void sg_sendUnitCmd(const QString&);
    void sg_sendProcessCmd(const QString&);

private:
    SystemBlockDiagramWidgetImpl* m_impl;
};
#endif  // SYSTEMBLOCKDIAGRAMWIDGET_H
