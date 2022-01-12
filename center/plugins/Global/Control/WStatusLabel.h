
#ifndef WSTATUSLABEL_H
#define WSTATUSLABEL_H

#include "DevProtocol.h"
#include "WWidget.h"
#include <QWidget>
namespace Ui
{
    class WStatusLabel;
}

class WStatusLabelImpl;
class WStatusLabel : public WWidget
{
    Q_OBJECT

public:
    explicit WStatusLabel(QWidget* parent = nullptr);
    ~WStatusLabel() override;

    void setAttr(const ParameterAttribute& attr, const QMap<QString, QVariant>& /* map */) override;
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
    WStatusLabelImpl* m_impl;
};

#endif  // WSTATUSLABEL_H
