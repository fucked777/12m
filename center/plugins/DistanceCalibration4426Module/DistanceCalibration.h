#ifndef DISTANCECALIBRATION_H
#define DISTANCECALIBRATION_H
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class DistanceCalibration;
}
QT_END_NAMESPACE

namespace cppmicroservices
{
    class BundleContext;
}
class DistanceCalibrationImpl;
class DistanceCalibration : public QWidget
{
    Q_OBJECT

public:
    DistanceCalibration(cppmicroservices::BundleContext context, QWidget* parent = nullptr);
    ~DistanceCalibration();

private:
    void init();

private:
    Ui::DistanceCalibration* ui;
    DistanceCalibrationImpl* m_impl;
};
#endif  // DISTANCECALIBRATION_H
