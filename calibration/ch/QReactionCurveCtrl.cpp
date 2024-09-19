/***************************************************************************
*   This file is part of the leonis project                               *
*   Copyright (C) 2024 by Mike Medical Electronics Co., Ltd               *
*   zhouguangming@maccura.com                                             *
*                                                                         *
**                   GNU General Public License Usage                    **
*                                                                         *
*   This library is free software: you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation, either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*   You should have received a copy of the GNU General Public License     *
*   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
*                                                                         *
**                  GNU Lesser General Public License                    **
*                                                                         *
*   This library is free software: you can redistribute it and/or modify  *
*   it under the terms of the GNU Lesser General Public License as        *
*   published by the Free Software Foundation, either version 3 of the    *
*   License, or (at your option) any later version.                       *
*   You should have received a copy of the GNU Lesser General Public      *
*   License along with this library.                                      *
*   If not, see <http://www.gnu.org/licenses/>.                           *
*                                                                         *
*   This library is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
****************************************************************************/

///////////////////////////////////////////////////////////////////////////
/// @file     QReactionCurveCtrl.cpp
/// @brief    曲线控制类（仅适用于QwtPlot显示反应曲线信息）
///
/// @author   8090/YeHuaNing
/// @date     2023年12月7日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8090/YeHuaNing，2023年12月7日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include <qwt_plot.h>
#include <qwt_legend.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_symbol.h>
#include <qwt_scale_div.h>
#include <qwt_scale_draw.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_panner.h>
#include <QMouseEvent>

#include "QReactionCurveCtrl.h"
#include "src/common/Mlog/mlog.h"
#include "shared/mccustomscaledraw.h"

QCurveData::QCurveData()
	:QObject()
{
	m_curves = Q_NULLPTR;
	m_plotMarker = Q_NULLPTR;
	m_plotInnerMarker = Q_NULLPTR;
    m_plotMessageMarker = Q_NULLPTR;
    m_plotMessageMarkerText = Q_NULLPTR;
    m_markerPixmap.load(":/Leonis/resource/image/bk-curve-arrow.png");
    m_markerTextPixmap.load(":/Leonis/resource/image/bk-curve-wave.png");
}

void QCurveData::showCurve(DetectCurveData& curveData)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	if (curveData.qwtPlot == Q_NULLPTR)
	{
		return;
	}

	m_curveData = curveData;
	m_curves = std::make_shared<QwtPlotCurve>();
	m_curves->setZ(19);
	// 设置坐标轴
	m_curves->setAxes(QwtPlot::xBottom, QwtPlot::yLeft);
	//m_curves->setTitle(curveData.curveName);
	// 设置图例属性
	//m_curves->setLegendAttribute(QwtPlotCurve::LegendShowSymbol, !curveData.curveName.isEmpty());
	// 设置曲线点符号
	QwtSymbol* pCircleSymbol = new QwtSymbol(QwtSymbol::Ellipse);
	pCircleSymbol->setBrush(QBrush(curveData.color));
	pCircleSymbol->setPen(QPen(Qt::NoPen));
	pCircleSymbol->setSize(12);
	m_curves->setSymbol(pCircleSymbol);
	// 设置曲线样式为点
	m_curves->setStyle(QwtPlotCurve::NoCurve);
	m_curves->setSamples(curveData.detectPointers, curveData.absorbValues);
	m_curves->attach(curveData.qwtPlot);

	// 波长详细信息
	//QString makerDetail(":/Leonis/resource/image/bk-wave-show.png");
	m_plotMessageMarker = std::make_shared<QwtPlotMarker>();
	//QwtSymbol* pMakerSymbolInfo = new QwtSymbol(QwtSymbol::Pixmap);
	//pMakerSymbolInfo->setPixmap(QPixmap(makerDetail));
	//pMakerSymbolInfo->setBrush(QBrush(QColor(0,0,255,128)));
	//pMakerSymbolInfo->setSize(120,50);
	//m_plotMessageMarker->setSymbol(pMakerSymbolInfo);
	m_plotMessageMarker->attach(curveData.qwtPlot);
    m_plotMessageMarker->setZ(22);
    m_plotMessageMarker->hide();

    m_plotMessageMarkerText = std::make_shared<QwtPlotMarker>();
    QwtSymbol* tmpMarker = new QwtSymbol(QwtSymbol::Pixmap);
    tmpMarker->setPixmap(m_markerTextPixmap);
    tmpMarker->setBrush(QBrush(QColor(0, 0, 255, 128)));
    m_plotMessageMarkerText->setSymbol(tmpMarker);
    m_plotMessageMarkerText->attach(curveData.qwtPlot);
    m_plotMessageMarkerText->setZ(22);
    m_plotMessageMarkerText->hide();

	// 渐变色marker
	QwtSymbol* pMakerInnerSymbol = new QwtSymbol(QwtSymbol::Ellipse);
	pMakerInnerSymbol->setBrush(QBrush(Qt::white));
	pMakerInnerSymbol->setPen(QPen(Qt::NoPen));
	pMakerInnerSymbol->setSize(14);
	m_plotInnerMarker = std::make_shared<QwtPlotMarker>();
	m_plotInnerMarker->setSymbol(pMakerInnerSymbol);
	m_plotInnerMarker->attach(curveData.qwtPlot);
    m_plotInnerMarker->setZ(17);
    m_plotInnerMarker->hide();

	// 外层marker
	QwtSymbol* pMakerSymbol = new QwtSymbol(QwtSymbol::Ellipse);
	pMakerSymbol->setBrush(QBrush(curveData.markerColor));
	pMakerSymbol->setPen(QPen(Qt::NoPen));
	pMakerSymbol->setSize(26);
	m_plotMarker = std::make_shared<QwtPlotMarker>();
	m_plotMarker->setSymbol(pMakerSymbol);
	m_plotMarker->attach(curveData.qwtPlot);
    m_plotMarker->setZ(18);
    m_plotMarker->hide();
}

void QCurveData::reset()
{
	if (m_curves != Q_NULLPTR)
	{
		m_curves->detach();
	}
	
	if (m_plotMarker != Q_NULLPTR)
	{
		m_plotMarker->detach();
	}

	if (m_plotInnerMarker != Q_NULLPTR)
	{
		m_plotInnerMarker->detach();
	}

	if (m_plotMessageMarker != Q_NULLPTR)
	{
		m_plotMessageMarker->detach();
	}

	m_curves = Q_NULLPTR;
	m_plotMarker = Q_NULLPTR;
	m_plotInnerMarker = Q_NULLPTR;
	m_plotMessageMarker = Q_NULLPTR;

	if (m_curveData.qwtPlot != Q_NULLPTR)
	{
		m_curveData.qwtPlot->setAxisAutoScale(QwtPlot::xBottom, true);
		m_curveData.qwtPlot->setAxisAutoScale(QwtPlot::yLeft, true);
		m_curveData.qwtPlot->replot();
		m_curveData.qwtPlot->show();
	}
}

double QCurveData::GetAvge(QVector<double>& resultVec)
{
	double min = 0;
	double max = 0;
	for (auto result : resultVec)
	{
		if (min > result)
		{
			min = result;
		}
		else if(result > max)
		{
			max = result;
		}
	}

	return (max - min) / 10;
}

bool QCurveData::showDataDetail(double x, QString showMessage)
{
    if (m_plotMessageMarker != Q_NULLPTR)
    {
        m_plotMessageMarker->setXValue(x);
        m_plotMessageMarkerText->setXValue(x);
        if ((x <= 0) || (x > 34) || x > m_curveData.absorbValues.size())
        {
            return false;
        }

        int iValXLeft = m_curveData.qwtPlot->transform(QwtPlot::xBottom, x) - 180;
        if (x <= 2 && iValXLeft < 0)
        {
            m_plotMessageMarkerText->setXValue(3);
        }

        int iValXRight = m_curveData.qwtPlot->transform(QwtPlot::xBottom, x) + 180;
        int width = m_curveData.qwtPlot->width();
        if (x >= (m_curveData.absorbValues.size() - 1))
        {
            m_plotMessageMarkerText->setXValue(m_curveData.absorbValues.size() - 2);
        }

        auto y = m_curveData.absorbValues[x - 1];
        int iValY = m_curveData.qwtPlot->transform(QwtPlot::yLeft, y) - 52;
        int arrowY = m_curveData.qwtPlot->transform(QwtPlot::yLeft, y) - 13;
        if (iValY <= 15)
        {
            iValY = m_curveData.qwtPlot->transform(QwtPlot::yLeft, y) + 52;
            arrowY = m_curveData.qwtPlot->transform(QwtPlot::yLeft, y) + 13;
            QMatrix rm;
            rm.rotate(180);
            auto pixmap = m_markerPixmap.transformed(rm);
            QwtSymbol* tmp = new QwtSymbol(QwtSymbol::Pixmap);
            tmp->setPixmap(pixmap);
            tmp->setBrush(QBrush(QColor(0, 0, 255, 128)));
            m_plotMessageMarker->setSymbol(tmp);
        }
        else
        {
            QwtSymbol* tmp = new QwtSymbol(QwtSymbol::Pixmap);
            tmp->setPixmap(m_markerPixmap);
            tmp->setBrush(QBrush(QColor(0, 0, 255, 128)));
            m_plotMessageMarker->setSymbol(tmp);
        }

        y = m_curveData.qwtPlot->invTransform(QwtPlot::yLeft, iValY);
        int arrowy = m_curveData.qwtPlot->invTransform(QwtPlot::yLeft, arrowY);

        QFont font;
        font.setPixelSize(14);
        QwtText textShow;
        textShow.setFont(font);
        textShow.setColor(Qt::white);
        textShow.setText(showMessage);
        m_plotMessageMarkerText->setLabel(textShow);
        m_plotMessageMarkerText->setYValue(y);
        m_plotMessageMarker->setYValue(arrowy);
        m_plotMessageMarker->show();
        m_plotMessageMarkerText->show();
    }

	return true;
}

void QCurveData::showMarker(double x, double y)
{
    if ((x <= 0) || /*(x > 34) ||*/ x > m_curveData.absorbValues.size())
    {
        return;
    }

	if (m_plotMarker != Q_NULLPTR)
	{
		m_plotMarker->setXValue(x);
		m_plotMarker->setYValue(m_curveData.absorbValues[x - 1]);
		m_plotMarker->show();
	}

	if (m_plotInnerMarker != Q_NULLPTR)
	{
		m_plotInnerMarker->setXValue(x);
		m_plotInnerMarker->setYValue(m_curveData.absorbValues[x - 1]);
		m_plotInnerMarker->show();
	}
}

QString QCurveData::GetCurrentWaveAbsorbs(double x)
{
	QString ret = m_curveData.curveName;
	if ((x <= 0) || /*(x > 34) ||*/ x > m_curveData.absorbValues.size())
	{
		return ret + '\n';
	}
	else
	{
		return ((ret += QString::number(m_curveData.absorbValues[x-1], 'f', 2)) + '\n');
	}
}

void QCustomGrid::draw(QPainter *p, const QwtScaleMap &xMap, const QwtScaleMap &yMap, const QRectF &canvasRect) const
{
    QwtPlotGrid::draw(p, xMap, yMap, canvasRect);

    for (const auto& shleid : m_shiledMap)
    {
        double x1 = shleid.first; // 开始段的X坐标
        double x2 = shleid.second; // 结束段的X坐标
        p->fillRect(xMap.transform(x1), qRound(canvasRect.top()), xMap.transform(x2) - xMap.transform(x1), qRound(canvasRect.height()), QBrush(QColor("#b7dbfd")));
    }
}

QReactionCurveCtrl::QReactionCurveCtrl(QwtPlot* plot, QObject *parent)
    : m_plot(plot)
    , QObject(parent)
	, m_plotMagnifier(Q_NULLPTR)
	, m_plotPanner(Q_NULLPTR)
    , m_pSplitGrid(Q_NULLPTR)
{
    // plot自动更新
    m_plot->setAutoReplot(true);
    // 设置背景色
    m_plot->setCanvasBackground(QColor(0xf5, 0xf7, 0xfb));

    // 初始化网格分割线
    m_pSplitGrid = std::make_shared<QCustomGrid>();
    m_pSplitGrid->setMinorPen(Qt::transparent);
    m_pSplitGrid->setMajorPen(QColor(0xd4, 0xd4, 0xd4), 1, Qt::DashLine);
    m_pSplitGrid->attach(m_plot);

    // 初始化标记选中线
    m_plotMarker = new QwtPlotMarker();
    m_plotMarker->setLineStyle(QwtPlotMarker::VLine);
    m_plotMarker->setLinePen(QColor(0xfa, 0x64, 0x31, 128), 2);
    m_plotMarker->attach(m_plot);
    m_plotMarker->setZ(20);

    // 安装事件过滤器
    m_plot->installEventFilter(this);

    // 设置坐标轴数据
    QwtText yLeftTitle(tr("吸光度"));
    QFont cosFont = m_plot->axisFont(QwtPlot::yLeft);
    cosFont.setPixelSize(14);
    cosFont.setFamily("HarmonyOS Sans SC");
    yLeftTitle.setFont(cosFont);
    m_plot->setAxisTitle(QwtPlot::yLeft, yLeftTitle);
    m_plot->setAxisLabelAlignment(QwtPlot::xBottom, Qt::AlignRight);
    m_plot->setAxisLabelAlignment(QwtPlot::yLeft, Qt::AlignLeft);
    m_plot->setContentsMargins(3, 3, 3, 3);

    // 横坐标只能为整型
    McCustomIntegerScaleDraw *pScaleDrawX = new McCustomIntegerScaleDraw();
    if (pScaleDrawX)
    {
        pScaleDrawX->enableComponent(QwtAbstractScaleDraw::Backbone, false);
        pScaleDrawX->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    }
    m_plot->setAxisScaleDraw(QwtPlot::xBottom, pScaleDrawX);

    // 纵坐标刻度值为彩色
    McCustomColorScaleDraw* pScaleDrawYLeft = new McCustomColorScaleDraw();
    //pScaleDrawYLeft->SetLabelColor(QColor(0x1b, 0x7d, 0xf5));
    m_plot->setAxisScaleDraw(QwtPlot::yLeft, pScaleDrawYLeft);

    // 坐标无刻度
    m_plot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Backbone, false);
    m_plot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    m_plot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Backbone, false);
    m_plot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Ticks, false);

    // 设置边框
    m_plot->canvas()->setStyleSheet("border:1px solid #a3a3a3;background:#f5f7fb;");
    // 插入图例
    //m_plot->insertLegend(new QwtLegend(), QwtPlot::BottomLegend);
    m_plot->setAxisAutoScale(QwtPlot::yLeft);
    m_plot->setAxisAutoScale(QwtPlot::xBottom);
    // 放大缩小
    m_plotMagnifier = new QwtPlotMagnifier(m_plot->canvas());
    // 平移
    m_plotPanner = new QwtPlotPanner(m_plot->canvas());
}

QReactionCurveCtrl::~QReactionCurveCtrl()
{
    
}

void QReactionCurveCtrl::ResetPlotScale()
{
    if (m_plot == nullptr)
        return;

    for (auto& curveData : m_reactionDatas)
    {
        if (curveData != Q_NULLPTR)
        {
            curveData->reset();
        }
    }

    m_reactionDatas.clear();
    DisplayCurve(m_assayResult);
}

void QReactionCurveCtrl::DisplayCurve(ch::tf::AssayTestResult& assayResult)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (m_plot == nullptr)
        return;

    ClearPlotData();

    m_assayResult = assayResult;
    // 测试结果，或者数据库id不正确
	if (!assayResult.__isset.id || assayResult.id <= 0)
		return;

    int primaryWave = -1;
    primaryWave = assayResult.primarySubWaves[0];
    int secondWave = -1;
    if (assayResult.primarySubWaves.size() >= 2)
    {
        secondWave = assayResult.primarySubWaves[1];
    }

	QCurveData::DetectCurveData primaryCurve;
	QCurveData::DetectCurveData secondCurve;
	QCurveData::DetectCurveData primarySecondCurve;
    for (const auto& detectPoint : assayResult.detectPoints)
    {
        // 主波长
        if (detectPoint.ods.count(primaryWave) != 0)
        {
            // 测光点编号
			primaryCurve.detectPointers.push_back(detectPoint.pointSN);
            // 吸光度
			primaryCurve.absorbValues.push_back(detectPoint.ods.at(primaryWave));
        }

        // 次波长
        if (detectPoint.ods.count(secondWave) != 0)
        {
            // 测光点编号
            secondCurve.detectPointers.push_back(detectPoint.pointSN);
            // 吸光度
            secondCurve.absorbValues.push_back(detectPoint.ods.at(secondWave));
        }

        // 主次波长
        if (detectPoint.ods.count(primaryWave) != 0 && detectPoint.ods.count(secondWave) != 0)
        {
            // 测光点编号
            primarySecondCurve.detectPointers.push_back(detectPoint.pointSN);
            auto value = detectPoint.ods.at(primaryWave) - detectPoint.ods.at(secondWave);
            // 吸光度差值
            primarySecondCurve.absorbValues.push_back(value);
        }
    }

    // 显示主波长
    if (!primaryCurve.detectPointers.empty())
    {
        primaryCurve.color = QColor("#3b8ff6");
		primaryCurve.markerColor = QColor("#b7dbfd");
        primaryCurve.curveName = QString(tr("主波长："));
		primaryCurve.qwtPlot = m_plot;
		auto spCurve = std::make_shared<QCurveData>();
		spCurve->showCurve(primaryCurve);
		m_reactionDatas.push_back(spCurve);
        //ShowProcessCurve(primaryCurve);
    }

    // 次波长
    if (!secondCurve.detectPointers.empty())
    {
        secondCurve.color = QColor("#fa9016");
		secondCurve.markerColor = QColor("#b7dbfd");
        secondCurve.curveName = QString(tr("次波长："));
		secondCurve.qwtPlot = m_plot;
		auto spCurve = std::make_shared<QCurveData>();
		spCurve->showCurve(secondCurve);
		m_reactionDatas.push_back(spCurve);
        //ShowProcessCurve(secondCurve);
    }

    // 主-次波长
    if (!primarySecondCurve.detectPointers.empty())
    {
        primarySecondCurve.color = QColor("#41c46a");
		primarySecondCurve.markerColor = QColor("#b7dbfd");
        primarySecondCurve.curveName = QString(tr("主-次："));
		primarySecondCurve.qwtPlot = m_plot;
		auto spCurve = std::make_shared<QCurveData>();
		spCurve->showCurve(primarySecondCurve);
		m_reactionDatas.push_back(spCurve);
        //ShowProcessCurve(primarySecondCurve);
    }
}

void QReactionCurveCtrl::SetDetectPoints(const std::vector<int>& points, ch::ANALYSIS_METHOD method)
{
    size_t vecSize = points.size();
    if (m_pSplitGrids.size() < vecSize)
    {
        m_pSplitGrids.resize(vecSize);
    }
    else if (m_pSplitGrids.size() <= 4)
    {
        m_pSplitGrids.resize(4);
    }

    for (auto& grid : m_pSplitGrids)
    {
        if (grid == nullptr)
        {
            grid = std::make_shared<QCustomGrid>();
            grid->setMinorPen(Qt::transparent);
            grid->setMajorPen(QColor(0xd4, 0xd4, 0xd4), 1, Qt::DashLine);
            grid->attach(m_plot);
        }

        grid->clearSegment();
        grid->detach();
    }

    // 新的点为空，则清理原有数据后，直接退出
    if (vecSize == 0)
        return;

    size_t pointLoop = 0;
    switch (method)
    {
        case ch::ANALYSIS_METHOD::ANALYSIS_METHOD_RATE_A:
        case ch::ANALYSIS_METHOD::ANALYSIS_METHOD_TWO_POINTS_RATE:
        {
            if ( vecSize < 2 )
                break;

            m_pSplitGrids[0]->addSegment(points[0], points[1]);
            m_pSplitGrids[0]->attach(m_plot);
        }
            break;
        case ch::ANALYSIS_METHOD::ANALYSIS_METHOD_ONE_POINT:
        case ch::ANALYSIS_METHOD::ANALYSIS_METHOD_TWO_POINTS:
        {
            for (const auto& p : points)
            {
                if (p <= 0)
                    continue;

                m_pSplitGrids[pointLoop]->addSegment(p - 0.5, p + 0.5);
                m_pSplitGrids[pointLoop++]->attach(m_plot);
            }
        }
            break;
        case ch::ANALYSIS_METHOD::ANALYSIS_METHOD_RATE_A_SAMPLE_BLANK:
        {
            if (vecSize < 4)
                break;

            m_pSplitGrids[0]->addSegment(points[0], points[1]);
            m_pSplitGrids[0]->attach(m_plot);
            m_pSplitGrids[1]->addSegment(points[2], points[3]);
            m_pSplitGrids[1]->attach(m_plot);
        }
            break;
        default:
            break;
    }
}

void QReactionCurveCtrl::ClearPlotData()
{
    if (m_plot == nullptr)
        return;

    for (const auto& data : m_reactionDatas)
    {
        data->reset();
    }

    // 清空原有数据
    m_reactionDatas.clear();
    for (auto& grid : m_pSplitGrids)
    {
        if (grid == nullptr)
        continue;

        grid->clearSegment();
        grid->detach();
    }
    
    m_startPostion = boost::none;
    // 清理标记线
    m_plotMarker->detach();
    // 清理线坐标颜色
    McCustomIntegerScaleDraw *pScaleDrawX = new McCustomIntegerScaleDraw();
    if (pScaleDrawX)
    {
        pScaleDrawX->enableComponent(QwtAbstractScaleDraw::Backbone, false);
        pScaleDrawX->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    }
    m_plot->setAxisScaleDraw(QwtPlot::xBottom, pScaleDrawX);
}
///
/// @brief 事件处理函数
///
/// @param[in]  obj   目标对象
/// @param[in]  event 事件对象
///
/// @return true表示事件已处理
///
/// @par History:
/// @li 5774/WuHongTao，2022年11月17日，新建函数
///
bool QReactionCurveCtrl::eventFilter(QObject *obj, QEvent *event)
{
    if (obj != m_plot)
    {
        return QObject::eventFilter(obj, event);
    }

    // 获取鼠标点击事件
    if (event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent* mouseEvent = (QMouseEvent*)event;
        QwtScaleWidget *yAxisWdgt = m_plot->axisWidget(QwtPlot::yLeft);
        int posX;
        int posY;
        if (yAxisWdgt != NULL)
        {
            posX = mouseEvent->x() - yAxisWdgt->width();
            posY = mouseEvent->y();
        }
        else
        {
            posX = mouseEvent->x();
            posY = mouseEvent->y();
        }

        if (posX > 0)
        {
            int iValX = m_plot->invTransform(QwtPlot::xBottom, posX) + 0.5;
            double iValY = m_plot->invTransform(QwtPlot::yLeft, posY);
            if ((iValX <= 0) || (iValX > 34))
            {
                return QObject::eventFilter(obj, event);
            }

            // 调整背景色
            ////if (m_shiftKey)
            //{
            //    if (m_startPostion && m_pSplitGrid != Q_NULLPTR)
            //    {
            //        m_pSplitGrid->addSegment(m_startPostion.value(), iValX);
            //    }
            //    m_startPostion = iValX;
            //}

            QString message = "";
            m_plotMarker->setXValue(iValX);
            m_plotMarker->attach(m_plot);
            for (const auto& data : m_reactionDatas)
            {
                if (data == Q_NULLPTR)
                {
                    continue;
                }

                message += data->GetCurrentWaveAbsorbs(iValX);
                data->showMarker(iValX, iValY);
            }

            // 选择第一个
            if (m_reactionDatas.size() != 0)
            {
                m_reactionDatas[0]->showDataDetail(iValX, message);
            }


            McCustomIntegerScaleDraw *pScaleDrawX = new McCustomIntegerScaleDraw();
            pScaleDrawX->enableComponent(QwtAbstractScaleDraw::Backbone, false);
            pScaleDrawX->enableComponent(QwtAbstractScaleDraw::Ticks, false);
            pScaleDrawX->SetSelLabelColor(iValX, QColor(0xfa, 0x64, 0x31));
            m_plot->setAxisScaleDraw(QwtPlot::xBottom, pScaleDrawX);
        }
    }

    return QObject::eventFilter(obj, event);
}
