#ifndef AGCVOLTAGEMAP_H
#define AGCVOLTAGEMAP_H

#include <QVector>
#include <QWidget>

class QCustomPlot;
class QCPGraph;
struct AGCResult;
using AGCResultList = QList<AGCResult>;
class AGCVoltageMap : public QWidget
{
public:
    explicit AGCVoltageMap(QWidget* parent);

    void setBackground(const QBrush& brush);
    void addData(const AGCResult&);
    void resetData(const AGCResultList& result);

private:
    QVector<double> m_sbf; /* x */
    QVector<double> m_agc; /* y */

    QCustomPlot* m_plot;
    QCPGraph* m_agcsbfGraph;
};

#endif  // AGCVOLTAGEMAP_H
