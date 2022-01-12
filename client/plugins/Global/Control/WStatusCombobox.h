
#ifndef WSTATUSCOMBOBOX_H
#define WSTATUSCOMBOBOX_H

#include "DevProtocol.h"
#include "WWidget.h"
#include <QWidget>

class WStatusComboboxImpl;
class WStatusCombobox : public WWidget
{
    Q_OBJECT

public:
    explicit WStatusCombobox(QWidget* parent = nullptr);
    ~WStatusCombobox() override;

    void setAttr(const ParameterAttribute& attr, const QMap<QString, QVariant>& map) override;
    void setValue(const QVariant& value) override;
    QVariant value() override;
    QVariant value(bool& isValid) override;
    void setName(const QString&) override;
    void setUnit(const QString&) override;
    void setValueStyleSheet(const QString& qss) override;
    void setNameWidth(int w) override;
    void setValueWidth(int w) override;
    void setControlHeight(int h) override;

    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    WStatusComboboxImpl* m_impl;
};

#endif  // WSTATUSCOMBOBOX_H
