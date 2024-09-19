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
/// @file     QResultStatisticsBarChart.h
/// @brief 	  结果统计条形统计图
///
/// @author   7656/zhang.changjiang
/// @date      2023年7月19日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年7月19日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include <float.h>
#include "QResultStatisticsBarChart.h"
#include "ui_QResultStatisticsBarChart.h"

#include <qwt_plot.h>
#include <qwt_plot_renderer.h>
#include <qwt_plot_canvas.h>

#include <qwt_plot_layout.h>
#include <qwt_legend.h>
#include <qwt_scale_draw.h>
#include <qwt_plot_grid.h>

QResultStatisticsBarChart::QResultStatisticsBarChart(QWidget *parent)
	: QWidget(parent)
	, m_CanvasBackground(Qt::white)
	, m_colors({ QColor(59, 143, 246), QColor(250, 144, 22) })
{
	ui = new Ui::QResultStatisticsBarChart();
	ui->setupUi(this);
	InitBeforeShow();
}

QResultStatisticsBarChart::~QResultStatisticsBarChart()
{
	delete ui;
}

void QResultStatisticsBarChart::SetCanvasBackground(const QColor & color)
{
	ui->plot->setCanvasBackground(color);
	m_CanvasBackground = color;
}

///
///  @brief 设置直方图数据
///
///
///  @param[in]   series  直方图数据
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年7月21日，新建函数
///
void QResultStatisticsBarChart::SetSamples(QVector< QVector<double> >& series)
{
	m_pBarChart->SetSamples(series);
	ui->plot->replot();
}

///
///  @brief 设置间隙大小
///
///
///  @param[in]   spacing  间隙大小
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年7月21日，新建函数
///
void QResultStatisticsBarChart::SetSpacing(int spacing)
{
	m_pBarChart->setSpacing(spacing);
}

///
///  @brief 显示前初始化
///
///
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年7月17日，新建函数
///
void QResultStatisticsBarChart::InitBeforeShow()
{
	// plot 自动更新
	ui->plot->setAutoReplot(true);

	// 设置背景色
	ui->plot->setCanvasBackground(QColor(0xf5, 0xf7, 0xfb));

	// 初始化网格分割线
	m_pSplitGrid = new QwtPlotGrid();
	m_pSplitGrid->enableX(false);
	m_pSplitGrid->enableXMin(true);
	m_pSplitGrid->setMinorPen(Qt::transparent);
	m_pSplitGrid->setMajorPen(QColor(0xd4, 0xd4, 0xd4), 1, Qt::DashLine);
	m_pSplitGrid->attach(ui->plot);

	// 安装事件过滤器
	ui->plot->installEventFilter(this);

	// 设置坐标轴刻度
	ui->plot->setAxisAutoScale(QwtPlot::yLeft);
	ui->plot->setAxisAutoScale(QwtPlot::yRight);

	// 坐标无刻度
	ui->plot->enableAxis(QwtPlot::xTop);
	ui->plot->axisScaleDraw(QwtPlot::xTop)->enableComponent(QwtAbstractScaleDraw::Backbone, false);
	ui->plot->axisScaleDraw(QwtPlot::xTop)->enableComponent(QwtAbstractScaleDraw::Ticks, false);
	ui->plot->axisScaleDraw(QwtPlot::xTop)->enableComponent(QwtAbstractScaleDraw::Labels, false);
	ui->plot->axisScaleDraw(QwtPlot::yRight)->enableComponent(QwtAbstractScaleDraw::Backbone, false);
	ui->plot->axisScaleDraw(QwtPlot::yRight)->enableComponent(QwtAbstractScaleDraw::Ticks, false);
	ui->plot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Backbone, false);
	ui->plot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Ticks, false);

	// 设置边框
	ui->plot->canvas()->setStyleSheet("border:1px solid #a3a3a3;background:#f5f7fb;");

	// 创建柱子
	m_pBarChart = new GJHPlotMultiBarChart();
	m_pBarChart->attach(ui->plot);
	m_pBarChart->SetColors(m_colors);
	m_pBarChart->setSpacing(60);

	ui->plot->setAutoReplot();
	ui->plot->setAxisScale(QwtPlot::xBottom, -0.5, 9.5, 1.0);
	ui->plot->setAxisScale(QwtPlot::yLeft, 0, 350, 50);
	m_pXBottomScaleDraw = new BarCustomScaleDraw();
	ui->plot->setAxisScaleDraw(QwtPlot::xBottom, m_pXBottomScaleDraw);
	ui->plot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Backbone, false);
	ui->plot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Ticks, false);
	ui->plot->setAxisMaxMinor(QwtPlot::xBottom, 0);
	ui->plot->plotLayout()->setAlignCanvasToScale(QwtPlot::xBottom, true);
	ui->plot->plotLayout()->setAlignCanvasToScale(QwtPlot::yLeft, false);

	ui->plot->plotLayout()->setCanvasMargin(0);
	ui->plot->updateCanvasMargins();

	// 设置柱子颜色
	const int numBars = m_colors.size();
	for (int i = 0; i < numBars; i++)
	{
		QwtColumnSymbol *symbol = new QwtColumnSymbol(QwtColumnSymbol::Box);
		symbol->setLineWidth(1);
		symbol->setFrameStyle(QwtColumnSymbol::Raised);
		symbol->setPalette(QPalette(m_colors[i]));

		m_pBarChart->setSymbol(i, symbol);
	}
}

///
///  @brief 连接信号槽
///
///
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年7月17日，新建函数
///
void QResultStatisticsBarChart::ConnectSlots()
{
	// TODO
}

///
///  @brief 根据现实模式绘制柱子
///
///
///  @param[in]   mode  现实模式
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年7月20日，新建函数
///
void QResultStatisticsBarChart::RePaintBarChart(BAR_CHART_SHOW_MODE mode)
{

	switch (mode)
	{
	case QResultStatisticsBarChart::NORMAL_BAR_ONLY:
	{

	}
		break;
	case QResultStatisticsBarChart::RECHECK_BAR_ONLY:
		break;
	case QResultStatisticsBarChart::NORMAL_AND_RECHECK:
		break;
	default:
		break;
	}
}

///
///  @brief 重写函数:设置坐标轴的刻度值
///
///
///  @param[in]   v  坐标值
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年7月13日，新建函数
///
QwtText BarCustomScaleDraw::label(double v) const
{
	// 初始化数值显示
	QString StrOffset("");

	// 如果是整数，则显示整数，否则不显示
	if (qAbs(v - int(v)) < DBL_EPSILON)
	{
		if (v >= 0 && v < 9)
		{
			StrOffset = QString("%1-%2").arg(int(v) * 10).arg(int(v + 1) * 10);
		}
		else
		{
			StrOffset = QString(">%1").arg(int(v) * 10);
		}
	}

	// 构造返回值
	QwtText qwtText(StrOffset);

	// 显示刻度值
	return qwtText;
}

///
///  @brief 重绘柱子列样式
///
///
///  @param[in]   painter  画家指针
///  @param[in]   rect	   绘制区域
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年7月26日，新建函数
///
void ColumnSymbol::draw(QPainter * painter, const QwtColumnRect & rect) const
{
	// 设置字体
	QFont font;
	font.setFamily(FONT_FAMILY);
	font.setPixelSize(16);
	font.setBold(false);
	font.setWeight(QFont::Thin);
	painter->setFont(font);

	// 设置柱子颜色
	QRectF rect2 = rect.toRect();

	painter->setBrush(m_color);
	painter->setPen(m_color);
	painter->drawRect(rect2);

	// 这里暂时全部设置为黑色字体，如果需求需要跟随柱子颜色，请注释这两句代码
	QColor tempColor = QColor("black");
	painter->setPen(tempColor);

	// 设置绘制区域
	int top = rect2.top();
	int height = rect2.height();
	Qt::Alignment alignmnet;
	alignmnet = Qt::AlignTop | Qt::AlignHCenter;

	// 绘制文本
	painter->drawText(QRect(rect2.left() + 1, top - 25, rect2.width() + 2, height), alignmnet, QString::number(m_hit));
}

///
///  @brief 重绘样式
///
///
///  @param[in]   sampleIndex  
///  @param[in]   valueIndex  
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年7月26日，新建函数
///
QwtColumnSymbol * GJHPlotMultiBarChart::specialSymbol(int sampleIndex, int valueIndex) const
{
	ColumnSymbol *pSymbol = new ColumnSymbol(m_series[sampleIndex][valueIndex], m_colors[valueIndex]);
	return pSymbol;
}

///
///  @brief 设置数据
///
///
///  @param[in]   series  直方图数据
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年7月26日，新建函数
///
void GJHPlotMultiBarChart::SetSamples(const QVector<QVector<double>>& series)
{
	m_series = series;
	setSamples(series);
}

///
///  @brief 设置直方图颜色
///
///
///  @param[in]   colors  颜色列表
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年7月26日，新建函数
///
void GJHPlotMultiBarChart::SetColors(const QVector<QColor>& colors)
{
	m_colors = colors;
}
