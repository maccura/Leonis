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
/// @file     QConsumableCurve.h
/// @brief 	  试剂耗材统计曲线图
///
/// @author   7656/zhang.changjiang
/// @date      2023年7月13日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年7月13日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <cfloat>
#include <QWidget>
#include <qwt_plot.h>
#include <qwt_scale_engine.h>
#include <qwt_date.h>
#include <qwt_date_scale_draw.h>
#include <qwt_date_scale_engine.h>
#include <qwt_plot_panner.h>
#include "shared/mccustomqwtmarker.h"
#include "src/thrift/gen-cpp/statis_types.h"

namespace Ui { class QConsumableCurve; };
// 前置声明
class QwtPlot;
class QwtPlotCurve;                   // Qwt 曲线类
class QwtPlotMarker;                  // Qwt 标记线类
class QwtPlotGrid;                    // Qwt 网格类
class CustomScaleDraw;				  // 自定义坐标

enum CURVE_ID
{
	ID_REAG_CONSUMPTION,		// 试剂消耗量曲线
	ID_REAG_WORK,				// 试剂工作量曲线
	ID_SAMPLE_WORK,				// 样本工作量曲线
	ID_RECHECK_WORK,			// 复查工作量曲线
	ID_QC_WORK,					// 质控工作量曲线
	ID_CALI_WORK,				// 校准工作量曲线
	ID_CONSUMPTION,				// 耗材消耗量曲线
};

class QConsumableCurve : public QWidget
{
	Q_OBJECT

public:

	QConsumableCurve(QWidget *parent = Q_NULLPTR);
	~QConsumableCurve();
	
	///
	///  @brief 显示前初始化
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月13日，新建函数
	///
	void InitBeforeShow();
	
	///
	///  @brief 创建曲线
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月17日，新建函数
	///
	void CreateCurves();
	
	///
	///  @brief 设置数据
	///
	///
	///  @param[in]   data  曲线数据列表
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月24日，新建函数
	///
	void SetData(const QVector<QVector<QPointF>>& data);

    ///
    ///  @brief 获取数据
    ///
    ///
    ///  @param[in]   data  曲线数据列表
    ///
    ///  @return	
    ///
    ///  @par History: 
    ///  @li 6889/ChenWei，2024年1月10日，新建函数
    ///
    const QVector<QVector<QPointF>>& GetData();

    ///
    ///  @brief 重写函数:设置坐标轴的刻度值
    ///
    ///
    ///  @param[in]   v  坐标值
    ///
    ///  @return	
    ///
    ///  @par History: 
    ///  @li 6889/ChenWei，2024年1月10日，新建函数
    ///
    QString label(double v);
	
	///
	/// @brief  清除曲线数据
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年12月18日，新建函数
	///
	void ClearData();

	///
	///  @brief 隐藏所有选中效果
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月24日，新建函数
	///
	void HideAllSelSymbol();

	///
	/// @brief  X 值对应的位置是否有数据
	///
	/// @param[in]  curveId  曲线ID
	/// @param[in]  iValX    X 轴的值
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2024年3月1日，新建函数
	///
	bool HasPointAtIndex(CURVE_ID curveId, const int iValX);
	
	///
	/// @brief  获取指定id和x轴值的数据点
	///
	/// @param[in]  curveId  曲线ID
	/// @param[in]  iValX    X 轴的值
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2024年3月1日，新建函数
	///
	QPointF GetDataPointByIdAndXVal(CURVE_ID curveId, const int iValX);

	///
	///  @brief 显示选中气泡
	///
	///
	///  @param[in]   iValX  选中线对应的X周坐标值
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月24日，新建函数
	///
	void ShowSelSymbol(const int iValX);

	///
	/// @brief  切换统计模式
	///
	/// @param[in]  statisType 统计模式
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年12月15日，新建函数
	///
	void SwitchStatisType(::tf::StatisType::type statisType);

	///
	///  @brief 设置曲线显示模式
	///
	///
	///  @param[in]   mode  显示模式
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月17日，新建函数
	///
	void SetShowMode(::tf::GroupByType::type mode, int maxRange = 12);
	
	///
	/// @brief  刷新X轴坐标
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年12月15日，新建函数
	///
	void UpdateAxisScaleX(int maxRange);

	///
	/// @brief  获取x轴刻度间宽度
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年12月19日，新建函数
	///
	int GetXAxisScaleIntervalWidth();

	///
	/// @brief  获取显示模式
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年12月15日，新建函数
	///
	::tf::GroupByType::type GetShowMode();

	///
	///  @brief  设置起止日期
	///
	///
	///  @param[in]   startDate  
	///  @param[in]   endDate  
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月13日，新建函数
	///
	void SetDateInterval(const QDate& startDate, const QDate& endDate);

	///
	///  @brief 设置曲线的可见性
	///
	///
	///  @param[in]   index    曲线索引
	///  @param[in]   visable  是否可见
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月17日，新建函数
	///
	void SetCurveVisable(CURVE_ID index, bool visable);

	///
	/// @brief 事件处理函数
	///
	/// @param[in]  obj   目标对象
	/// @param[in]  event 事件对象
	///
	/// @return true 表示事件已处理
	///
	/// @par History:
	///  @li 7656/zhang.changjiang，2023年7月17日，新建函数
	///
	bool eventFilter(QObject *obj, QEvent *event) override;
	
	///
	///  @brief 获取悬浮框显示列表
	///
	///
	///  @param[in]   iValX  选中线x轴
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月25日，新建函数
	///
	QStringList GetTipList(const int iValX);

	///
	///  @brief 获取y值最大的点
	///
	///
	///  @param[in]   iValX  选中线x轴
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月25日，新建函数
	///
	QPointF GetMaxPointF(const int iValX);

	///
	/// @brief  重置坐标轴
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年12月19日，新建函数
	///
	void ResetScaleDraw();
Q_SIGNALS:
	void Panned(int dx, int dy);
public slots:
	///
	/// @brief  水平滑动条值改变
	///
	/// @param[in]  iValue  滑动条当前值
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年12月18日，新建函数
	///
	void OnSliderValueChanged(int iValue);

private:
	Ui::QConsumableCurve *ui;
	QwtPlotGrid*					m_pSplitGrid;               // 网格分割线
	QwtPlotMarker*					m_pVLineMarker;             // 标记选中数据
	QVector<QwtPlotCurve*>          m_curves;                   // 显示的曲线
	CustomScaleDraw*				m_pXDateScaleDraw;			// X 轴坐标
	StatisticsGraphicSelTipMarker*  m_pSelMarker;				// 悬浮提示气泡
	QVector<QwtPlotCurve*>          m_pSelPoints;				// 图选中效果（圆点）
	QVector<QVector<QPointF>>		m_datas;					// 曲线数据
	::tf::StatisType::type          m_statisType;               // 统计模式
	QwtPlotPanner*                  m_pPanner;                  // 平移器
    int                             m_rangMax;                  // x轴最大值
};

///////////////////////////////////////////////////////////////////////////
/// @file     QConsumableCurve.h
/// @brief 	  试剂耗材统计曲线图自定义坐标轴
///
/// @author   7656/zhang.changjiang
/// @date      2023年7月13日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年7月13日，新建文件
///
///////////////////////////////////////////////////////////////////////////

class CustomScaleDraw : public QwtScaleDraw
{
public:

	explicit CustomScaleDraw();

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

	///
	///  @brief  设置起止日期
	///
	///
	///  @param[in]   startDate  
	///  @param[in]   endDate  
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月13日，新建函数
	///
	void SetDateInterval(const QDate& startDate, const QDate& endDate);
	
	///
	/// @brief  获取X坐标轴起始日期
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年12月18日，新建函数
	///
	QDate& GetStartDate();
	
	///
	/// @brief  获取X坐标轴截止日期
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年12月18日，新建函数
	///
	QDate& GetEndDate();

	///
	///  @brief  清空起始日期
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月13日，新建函数
	///
	void ClearStartDate();
	
	///
	///  @brief 设置显示模式
	///
	///
	///  @param[in]   showMode  
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月24日，新建函数
	///
	void SetShowMode(::tf::GroupByType::type showMode);
	
	///
	/// @brief  获取显示模式
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年12月14日，新建函数
	///
	::tf::GroupByType::type GetShowMode();

	///
	///  @brief 设置选中的标签延时
	///
	///
	///  @param[in]   val   选中的x轴坐标值
	///  @param[in]   color 颜色 
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月24日，新建函数
	///
	void SetSelLabelColor(int val, const QColor &color);
	
	///
	///  @brief 是否隐藏选中效果
	///
	///
	///  @param[in]   bShowSelSymbol  true 显示 false 不显示
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月24日，新建函数
	///
	void SetSelSymbolVisible(const bool bShowSelSymbol);
private:
	QDate                       m_startDate;        // 起始日期
	QDate                       m_endDate;          // 起始日期
	::tf::GroupByType::type     m_iMode;            // 显示模式
	int                         m_iSelVal;			// 选中的坐标值
	QColor                      m_color;			// 选中的坐标刻度颜色
	bool                        m_bShowSelSymbol;	// 是否显示选中效果
};
