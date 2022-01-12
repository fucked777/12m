#include "VoltageRadar4426.h"
#include "CustomPacketMessageDefine.h"
#include "ExtendedConfig.h"
#include "GlobalData.h"
#include <QCheckBox>
#include <QDomElement>
#include <QHBoxLayout>
#include <QVBoxLayout>

#define rangeWidthL  (-5)
#define rangeWidthR  (5)
#define rangeHeightT (5)
#define rangeHeightB (-5)
#define DOT_NUMBERS  (1000)

class VoltageRadar4426Impl
{
public:
    QCustomPlot* pCustomplot{ nullptr };

    QCheckBox* sCheck{ nullptr };
    QCheckBox* kaRCheck{ nullptr };
    QCheckBox* kaDCheck{ nullptr };

    QCPGraph* graphS{ nullptr };   /* S图层 */
    QCPGraph* graphKaR{ nullptr }; /* Ka图层 */
    QCPGraph* graphKaD{ nullptr }; /* Ka图层 */

    QVector<double> sKeyVector;
    QVector<double> sValVector;
    QVector<double> kaRKeyVector;
    QVector<double> kaRValVector;
    QVector<double> kaDKeyVector;
    QVector<double> kaDValVector;

    QString projectId;

    VoltageRadar4426Impl()
    {
        sKeyVector.resize(1);
        sValVector.resize(1);
        kaRKeyVector.resize(1);
        kaRValVector.resize(1);
        kaDKeyVector.resize(1);
        kaDValVector.resize(1);
    }
};

VoltageRadar4426::VoltageRadar4426(const QDomElement& domEle, QWidget* parent, const int deviceId, const int modeId)
    : QWidget(parent)
    , m_impl(new VoltageRadar4426Impl)
    , d_deviceId(deviceId)
    , d_modeId(modeId)
{
    // this->setMinimumWidth(200);
    // this->setWindowFlags(Qt::FramelessWindowHint);
    parseNode(domEle);
    initUI();
}

VoltageRadar4426::~VoltageRadar4426() { delete m_impl; }

void VoltageRadar4426::setSPoint(double x, double y)
{
    m_impl->sKeyVector[0] = x;
    m_impl->sValVector[0] = y;
    updateStarMap();
}
void VoltageRadar4426::setKaRPoint(double x, double y)
{
    m_impl->kaRKeyVector[0] = x;
    m_impl->kaRValVector[0] = y;
    updateStarMap();
}
void VoltageRadar4426::setKaDPoint(double x, double y)
{
    m_impl->kaDKeyVector[0] = x;
    m_impl->kaDValVector[0] = y;
    updateStarMap();
}
void VoltageRadar4426::setSVisible(bool visible)
{
    m_impl->graphS->setVisible(visible);
    m_impl->pCustomplot->replot();
}
void VoltageRadar4426::setKaRVisible(bool visible)
{
    m_impl->graphKaR->setVisible(visible);
    m_impl->pCustomplot->replot();
}
void VoltageRadar4426::setKaDVisible(bool visible)
{
    m_impl->graphKaD->setVisible(visible);
    m_impl->pCustomplot->replot();
}
void VoltageRadar4426::resetValue(bool value)
{
    m_impl->kaRCheck->setChecked(value);
    m_impl->kaDCheck->setChecked(value);
    m_impl->sCheck->setChecked(value);
    m_impl->pCustomplot->replot();
}

void VoltageRadar4426::timerEvent(QTimerEvent* /*event*/)
{
    auto statusReportMsg = GlobalData::getExtenStatusReportDataByObject(d_deviceId);
    double s_x = statusReportMsg.unitReportMsgMap[d_sPointInfo.unitId].paramMap[d_sPointInfo.x_point].toDouble();
    double s_y = statusReportMsg.unitReportMsgMap[d_sPointInfo.unitId].paramMap[d_sPointInfo.y_point].toDouble();

    double kaR_x = statusReportMsg.unitReportMsgMap[d_kaRPointInfo.unitId].paramMap[d_kaRPointInfo.x_point].toDouble();
    double kaR_y = statusReportMsg.unitReportMsgMap[d_kaRPointInfo.unitId].paramMap[d_kaRPointInfo.y_point].toDouble();

    double kaD_x = statusReportMsg.unitReportMsgMap[d_kaDPointInfo.unitId].paramMap[d_kaDPointInfo.x_point].toDouble();
    double kaD_y = statusReportMsg.unitReportMsgMap[d_kaDPointInfo.unitId].paramMap[d_kaDPointInfo.y_point].toDouble();

    setSPoint(s_x, s_y);
    setKaRPoint(kaR_x, kaR_y);
    setKaDPoint(kaD_x, kaD_y);
}
/*
 * S blue
 * Ka green
 */
void VoltageRadar4426::initUI()
{
    m_impl->sCheck = new QCheckBox(this);
    m_impl->kaRCheck = new QCheckBox(this);
    m_impl->kaDCheck = new QCheckBox(this);
    m_impl->pCustomplot = new QCustomPlot(this);
    m_impl->sCheck->setText("S跟踪");
    m_impl->kaRCheck->setText("Ka遥测");
    m_impl->kaDCheck->setText("Ka数传");
    m_impl->sCheck->setChecked(true);
    m_impl->kaRCheck->setChecked(true);
    m_impl->kaDCheck->setChecked(true);
    /* 221*279 */
    m_impl->pCustomplot->setFixedSize(220, 210);

    m_impl->kaRCheck->setStyleSheet(".QCheckBox{background-color: rgb(124,252,0);}");
    m_impl->kaDCheck->setStyleSheet(".QCheckBox{background-color: rgb(255,99,71);}");
    m_impl->sCheck->setStyleSheet(".QCheckBox{background-color: rgb(240,230,140);}");

    auto checkLayout = new QHBoxLayout;
    checkLayout->setSpacing(0);
    checkLayout->setContentsMargins(5, 0, 5, 0);
    checkLayout->addWidget(m_impl->kaRCheck);
    checkLayout->addWidget(m_impl->kaDCheck);
    checkLayout->addWidget(m_impl->sCheck);

    auto layout = new QVBoxLayout(this);
    layout->setSpacing(5);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addLayout(checkLayout);
    layout->addWidget(m_impl->pCustomplot);

    connect(m_impl->sCheck, &QCheckBox::stateChanged, [=](int status) { setSVisible(status == Qt::Checked); });
    connect(m_impl->kaRCheck, &QCheckBox::stateChanged, [=](int status) { setKaRVisible(status == Qt::Checked); });
    connect(m_impl->kaDCheck, &QCheckBox::stateChanged, [=](int status) { setKaDVisible(status == Qt::Checked); });

    m_impl->pCustomplot->xAxis->setRange(rangeWidthL, rangeWidthR);
    m_impl->pCustomplot->yAxis->setRange(rangeHeightB, rangeHeightT);
    m_impl->pCustomplot->setBackground(QBrush(QColor("#515151")));

    QPen penSGrid(QBrush(QColor(240, 230, 140)), 1); /* S */
    QPen penKaRGrid(QBrush(QColor(124, 252, 0)), 1); /* Ka遥测 */
    QPen penKaDGrid(QBrush(QColor(255, 99, 71)), 1); /* Ka数传 */
    // QPen penCircleGrid(QBrush(QColor(0, 255, 255)), 2); /* 范围圈 */

    auto yAxis = m_impl->pCustomplot->yAxis;
    yAxis->setTickLabels(true);
    auto yFont = yAxis->tickLabelFont();
    yFont.setPointSize(8);
    yAxis->setTickLabelFont(yFont);
    // yAxis->setTickLength(0);
    // yAxis->setSubTickLength(0);
    /* 设置大刻度在外边长度 */
    yAxis->setTickLengthIn(1);
    yAxis->setTickLengthOut(3);
    /* 设置小刻度在坐标轴线里边长度 */
    yAxis->setSubTickLengthIn(0);
    yAxis->setSubTickLengthOut(0);
    /* 刻度的文字颜色 */
    yAxis->setTickLabelColor(Qt::white);
    /* 原点偏移 */
    auto yTicker = yAxis->ticker();
    yTicker->setTickOrigin(0);
    yTicker->setTickCount(13);
    auto yGrid = yAxis->grid();
    /* 不显示零线 */
    yGrid->setZeroLinePen(QPen(QColor(0, 206, 209)));
    yGrid->setVisible(true);
    yGrid->setSubGridVisible(false);

    auto xAxis = m_impl->pCustomplot->xAxis;
    auto xFont = yAxis->tickLabelFont();
    xFont.setPointSize(8);
    xAxis->setTickLabelFont(xFont);

    xAxis->setTickLabels(true);
    /* 设置大刻度在外边长度 */
    xAxis->setTickLength(1);
    xAxis->setSubTickLength(3);
    /* 设置小刻度在坐标轴线里边长度 */
    xAxis->setSubTickLengthIn(0);
    xAxis->setSubTickLengthOut(0);
    /* 刻度的文字颜色 */
    xAxis->setTickLabelColor(Qt::white);
    /* 原点偏移 */
    auto xTicker = xAxis->ticker();
    xTicker->setTickOrigin(0);
    xTicker->setTickCount(13);
    auto xGrid = xAxis->grid();
    /* 不显示零线 */
    xGrid->setZeroLinePen(QPen(QColor(0, 206, 209)));
    xGrid->setVisible(true);
    xGrid->setSubGridVisible(false);

    /* 范围圈 */
    //    QCPItemEllipse* ellipse = new QCPItemEllipse(m_impl->pCustomplot);
    //    ellipse->setVisible(true);
    //    ellipse->setPen(penCircleGrid);
    //    auto topLeft = ellipse->topLeft;
    //    auto bottomRight = ellipse->bottomRight;
    //    topLeft->setType(QCPItemPosition::ptPlotCoords);
    //    bottomRight->setType(QCPItemPosition::ptPlotCoords);
    //    topLeft->setAxes(m_impl->pCustomplot->xAxis, m_impl->pCustomplot->yAxis);
    //    topLeft->setCoords(-5, -5);
    //    bottomRight->setCoords(5, 5);

    // 增加图层
    m_impl->graphS = m_impl->pCustomplot->addGraph(m_impl->pCustomplot->xAxis, m_impl->pCustomplot->yAxis);
    m_impl->graphKaR = m_impl->pCustomplot->addGraph(m_impl->pCustomplot->xAxis, m_impl->pCustomplot->yAxis);
    m_impl->graphKaD = m_impl->pCustomplot->addGraph(m_impl->pCustomplot->xAxis, m_impl->pCustomplot->yAxis);

    // 绘制散点
    m_impl->graphS->setPen(penSGrid);
    m_impl->graphS->setLineStyle(QCPGraph::lsNone);
    m_impl->graphS->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCrossCircle, 5));

    m_impl->graphKaR->setPen(penKaRGrid);
    m_impl->graphKaR->setLineStyle(QCPGraph::lsNone);
    m_impl->graphKaR->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCrossCircle, 5));

    m_impl->graphKaD->setPen(penKaDGrid);
    m_impl->graphKaD->setLineStyle(QCPGraph::lsNone);
    m_impl->graphKaD->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCrossCircle, 5));

    startTimer(1000);
}

void VoltageRadar4426::updateStarMap()
{
    m_impl->graphS->setData(m_impl->sKeyVector, m_impl->sValVector);
    m_impl->graphKaR->setData(m_impl->kaRKeyVector, m_impl->kaRValVector);
    m_impl->graphKaD->setData(m_impl->kaDKeyVector, m_impl->kaDValVector);
    m_impl->pCustomplot->replot();
}

void VoltageRadar4426::parseNode(const QDomElement& domEle)
{
    if (domEle.isNull())
    {
        return;
    }

    auto domNode = domEle.firstChild();
    while (!domNode.isNull())
    {
        auto subDomEle = domNode.toElement();
        if (subDomEle.isNull())
        {
            continue;
        }

        if (subDomEle.tagName() == "S")
        {
            parseItemNode(subDomEle, d_sPointInfo);
        }
        if (subDomEle.tagName() == "KaR")
        {
            parseItemNode(subDomEle, d_kaRPointInfo);
        }
        if (subDomEle.tagName() == "KaD")
        {
            parseItemNode(subDomEle, d_kaDPointInfo);
        }

        domNode = domNode.nextSibling();
    }
}

void VoltageRadar4426::parseItemNode(const QDomElement& domEle, VoltageRadar4426::PointInfo& info)
{
    info.unitId = domEle.attribute("unitId", "1").toInt();
    info.x_point = domEle.attribute("x_point");
    info.y_point = domEle.attribute("y_point");
    info.isVisible = (domEle.attribute("isVisible", "true") == "true") ? (true) : (false);
}
