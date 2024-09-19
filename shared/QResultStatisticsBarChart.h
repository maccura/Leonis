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
#pragma once

#include "uidef.h"

#include <QWidget>
#include <qwt_date_scale_draw.h>
#include <qwt_plot_multi_barchart.h>
#include <qwt_column_symbol.h>

class QwtPlotGrid;
class BarCustomScaleDraw;
class GJHPlotMultiBarChart;
class ColumnSymbol;

namespace Ui { class QResultStatisticsBarChart; };

class QResultStatisticsBarChart : public QWidget
{
	Q_OBJECT

public:
	enum BAR_CHART_SHOW_MODE
	{
		NORMAL_BAR_ONLY,		// 只显示常规
		RECHECK_BAR_ONLY,		// 只显示复查
		NORMAL_AND_RECHECK,		// 复查和常规都显示
	};
	QResultStatisticsBarChart(QWidget *parent = Q_NULLPTR);
	~QResultStatisticsBarChart();
	void SetCanvasBackground(const QColor &color);

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
	void SetSamples(QVector< QVector<double> >& series);

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
	void SetSpacing(int spacing);
private:

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
	void InitBeforeShow();

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
	void ConnectSlots();

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
	void RePaintBarChart(BAR_CHART_SHOW_MODE mode);
private:
	Ui::QResultStatisticsBarChart *ui;
	QVector<QVector<double>>				m_data;
	QColor									m_CanvasBackground;// 画布背景颜色
	GJHPlotMultiBarChart *					m_pBarChart;
	QVector<QColor>							m_colors;
	QwtPlotGrid*							m_pSplitGrid;
	BarCustomScaleDraw*						m_pXBottomScaleDraw;
};

///////////////////////////////////////////////////////////////////////////
/// @file     BarCustomScaleDraw
/// @brief 	  条形统计图自定义坐标
///
/// @author   7656/zhang.changjiang
/// @date      2023年7月20日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年7月20日，新建文件
///
///////////////////////////////////////////////////////////////////////////
class BarCustomScaleDraw : public QwtScaleDraw
{
public:

	explicit BarCustomScaleDraw():QwtScaleDraw(){};

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
	virtual QwtText label(double v) const;

};

///////////////////////////////////////////////////////////////////////////
/// @file     ColumnSymbol
/// @brief 	  
///
/// @author   7656/zhang.changjiang
/// @date      2023年7月26日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年7月26日，新建文件
///
///////////////////////////////////////////////////////////////////////////
class ColumnSymbol :public QwtColumnSymbol
{
public:
	ColumnSymbol(int hit, QColor color) :QwtColumnSymbol(), m_color(color), m_hit(hit) {}
	
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
	virtual void draw(QPainter *painter, const QwtColumnRect & rect) const;

private:
	QColor m_color;				// 柱子颜色	
	int m_hit;					// 柱子上方的数字
};

///////////////////////////////////////////////////////////////////////////
/// @file     GJHPlotMultiBarChart
/// @brief 	  自定义直方图
///
/// @author   7656/zhang.changjiang
/// @date      2023年7月26日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年7月26日，新建文件
///
///////////////////////////////////////////////////////////////////////////
class GJHPlotMultiBarChart :public QwtPlotMultiBarChart
{
public:
	GJHPlotMultiBarChart(const QString &title = QString::null) :QwtPlotMultiBarChart() {}
	
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
	QwtColumnSymbol* specialSymbol(int sampleIndex, int valueIndex)const;
	
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
	void SetSamples(const QVector<QVector<double>>& series);

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
	void SetColors(const QVector<QColor>& colors);
	
private:
	QVector<QVector<double>>	m_series;
	QVector<QColor>				m_colors;
};
