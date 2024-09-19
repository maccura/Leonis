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
/// @file     QReactionCurveCtrl.h
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
#pragma once

#include <memory>
#include <QObject>
#include <qwt_plot_grid.h>
#include "boost/optional.hpp"
#include "thrift/ch/c1005/C1005LogicControlProxy.h"
#include "src/alg/ch/ch_alg/ch_alg.h"

class QwtPlot;
class QwtPlotMarker;
class QwtPlotCurve;
class QwtPlotMagnifier;
class QwtPlotPanner;

class QCurveData : public QObject
{
	Q_OBJECT
public:
	QCurveData();
	~QCurveData() {};
	struct DetectCurveData
	{
		QVector<double> detectPointers;
		QVector<double> absorbValues;
		QColor			color;
		QColor			markerColor;
		QString			curveName;
		QwtPlot			*qwtPlot;
	};

	double GetAvge(QVector<double>& resultVec);
	bool showDataDetail(double x, QString showMessage);
	void showMarker(double x, double y);
	QString GetCurrentWaveAbsorbs(double x);
	void showCurve(DetectCurveData& curveData);
	void reset();
private:
	std::shared_ptr<QwtPlotMarker>  m_plotMarker;
	std::shared_ptr<QwtPlotMarker>  m_plotInnerMarker;
	std::shared_ptr<QwtPlotCurve>	m_curves;
    std::shared_ptr<QwtPlotMarker>  m_plotMessageMarker;
    std::shared_ptr<QwtPlotMarker>  m_plotMessageMarkerText;
    QPixmap							m_markerPixmap;
    QPixmap							m_markerTextPixmap;
	DetectCurveData					m_curveData;
};

class QCustomGrid : public QwtPlotGrid
{
public:
	QCustomGrid() : QwtPlotGrid()
	{
		segment1Color = Qt::yellow;
		segment2Color = Qt::blue;
	}

	void setSegmentColors(const QColor& color1, const QColor& color2)
	{
		segment1Color = color1;
		segment2Color = color2;
	}

	void addSegment(double start, double end)
	{
		m_shiledMap.push_back(std::make_pair(start, end));
	}

	bool clearSegment()
	{
		m_shiledMap.clear();
		return true;
	}

    virtual void draw(QPainter *p, const QwtScaleMap &xMap, const QwtScaleMap &yMap, const QRectF &canvasRect) const;

private:
	QColor segment1Color;
	QColor segment2Color;
	std::vector<std::pair<double, double>> m_shiledMap;
};

class QReactionCurveCtrl : public QObject
{
    Q_OBJECT
    
public:
    QReactionCurveCtrl(QwtPlot* plot, QObject *parent = Q_NULLPTR);
    ~QReactionCurveCtrl();

    // 恢复原图比例
    void ResetPlotScale();
    
    // 设置结果数据（空数据和有效曲线）
    void DisplayCurve(ch::tf::AssayTestResult& assayResult);

    ///
    /// @brief 设置测光点数据
    ///
    /// @param[in]  points  
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年12月8日，新建函数
    ///
    void SetDetectPoints(const std::vector<int>& points, ch::ANALYSIS_METHOD method/*, 分析方法*/);

protected:
    // 清空数据
    void ClearPlotData();

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
    bool eventFilter(QObject *obj, QEvent *event) override;
    
private:
    QwtPlot*                        m_plot;                     /// 基础图元
	QwtPlotMarker*                  m_plotMarker;               /// 标记线
    std::shared_ptr<QCustomGrid>    m_pSplitGrid;               /// 网格线
	std::vector<std::shared_ptr<QCustomGrid>> m_pSplitGrids;    /// 标记背景色（蓝色)  
	std::vector<std::shared_ptr<QCurveData>> m_reactionDatas;   /// 曲线数据
    boost::optional<double>			m_startPostion;
    QwtPlotMagnifier*               m_plotMagnifier;
    QwtPlotPanner*                  m_plotPanner;
    std::vector<int>                m_detectPoints;             /// 测光点（用于标记测光点）
    ch::tf::AssayTestResult         m_assayResult;              /// 测试结果
};
