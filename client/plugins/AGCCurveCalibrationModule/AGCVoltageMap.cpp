#include "AGCVoltageMap.h"
#include "AGCDefine.h"
#include "qcustomplot.h"
#include <QDebug>
#include <QTime>
#include <QVBoxLayout>

#define widthBegin  (-10.0)
#define widthEnd    (200.0)
#define heightBegin (-1.0)
#define heightEnd   (5.0)

AGCVoltageMap::AGCVoltageMap(QWidget* parent)
    : QWidget(parent)
    , m_plot(new QCustomPlot(this))
{
    m_plot->setInteraction(QCP::iRangeDrag, true);  //鼠标单击拖动
    m_plot->setInteraction(QCP::iRangeZoom, true);  //滚轮滑动缩放
    m_plot->resize(parent->size());

    m_plot->xAxis->setRange(widthBegin, widthEnd);
    m_plot->yAxis->setRange(heightBegin, heightEnd);
    m_plot->setBackground(QBrush(QColor("#FFFAFA")));

    m_plot->xAxis->setLabel(QString("S/Ф(dBHz)"));
    m_plot->yAxis->setLabel(QString("AGC电压值(V)"));

    // m_plot->xAxis->scaleRange(10, -2.3);
    // m_plot->yAxis->scaleRange(10, 0.05);

    m_plot->xAxis->setTickLength(0, 4);
    m_plot->yAxis->setTickLength(0, 4);

    m_plot->xAxis->setSubTickLength(0, 2);
    m_plot->yAxis->setSubTickLength(0, 2);

    m_plot->xAxis->grid()->setVisible(false);
    m_plot->xAxis->grid()->setSubGridVisible(false);
    m_plot->yAxis->grid()->setVisible(true);
    m_plot->yAxis->grid()->setPen(QPen(Qt::black));
    m_plot->yAxis->grid()->setZeroLinePen(QPen(Qt::black));

    QVBoxLayout* vlayout = new QVBoxLayout(this);

    vlayout->addWidget(m_plot);

    setLayout(vlayout);

    m_agcsbfGraph = m_plot->addGraph();
    Q_ASSERT(m_agcsbfGraph != nullptr);

    QPen penLine;
    penLine.setColor(Qt::blue);
    penLine.setWidth(1);
    m_agcsbfGraph->setPen(penLine);
    m_agcsbfGraph->setLineStyle(QCPGraph::lsLine);
    m_agcsbfGraph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCross, 2));
}

void AGCVoltageMap::setBackground(const QBrush& brush)
{
    if (m_plot)
        m_plot->setBackground(brush);
}
void AGCVoltageMap::addData(const AGCResult& result)
{
    m_sbf << result.sbf;
    m_agc << result.agc;

    m_agcsbfGraph->setData(m_sbf, m_agc);
    m_plot->replot();
}
void AGCVoltageMap::resetData(const AGCResultList& result)
{
    m_plot->xAxis->setRange(widthBegin, widthEnd);
    m_plot->yAxis->setRange(heightBegin, heightEnd);
    m_sbf.clear();
    m_agc.clear();

    for (auto& item : result)
    {
        m_sbf << item.sbf;
        m_agc << item.agc;
    }

    m_agcsbfGraph->setData(m_sbf, m_agc);
    m_plot->replot();
}
