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
/// @file     chQcResultDetailsDlg.h
/// @brief    生化质控结果详情
///
/// @author   8580/GongZhiQiang
/// @date     2023年2月2日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年2月2日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <QObject>
#include <qwt_plot_grid.h>

#include "shared/basedlg.h"
#include "src/thrift/gen-cpp/defs_types.h"
#include "src/thrift/ch/gen-cpp/ch_types.h"
#include "src/thrift/ise/gen-cpp/ise_types.h"
#include "uidcsadapter/adaptertypedef.h"
#include "boost/optional.hpp"

class QwtPlotMarker;
class QwtPlotCurve;
class QwtPlotMagnifier;
class QwtPlotPanner;
class QwtSymbol;

namespace Ui {
    class chQcResultDetailsDlg;
};

namespace CH_QC_DETAIL_PLOT {

class QReactionCurveData : public QObject
{
	Q_OBJECT
public:
	QReactionCurveData();
	~QReactionCurveData() {};
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

class CustomGrid : public QwtPlotGrid
{
public:
	CustomGrid() : QwtPlotGrid()
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
};


class chQcResultDetailsDlg : public BaseDlg
{
	Q_OBJECT

public:
	chQcResultDetailsDlg(QWidget *parent = Q_NULLPTR);
	~chQcResultDetailsDlg();

	///
	/// @brief 显示曲线
	///
	struct DetectCurve
	{
		QVector<double> detectPointers;
		QVector<double> absorbValues;
		QColor color;
		QString curveName;
	};

	///
	/// @brief 设置生化质控结果id
	///
	/// @param[in]  chQcReusltID 生化质控结果ID  
	///
	/// @return true:设置成功 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年2月3日，新建函数
	///
	bool setChAssayResultID(const int& chReusltID, const bool& bCalculated, const QC_STATE& enQcState);

	///
	/// @brief 设置ISE质控结果id
	///
	/// @param[in]  iseQcReusltID ISE质控结果ID  
	///
	/// @return true:设置成功 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年2月3日，新建函数
	///
	bool setIseAssayResultID(const int& iseReusltID, const bool& bCalculated, const QC_STATE& enQcState);

private:

	///
	/// @brief 更新生化界面
	///
	/// @return true:更新成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年2月3日，新建函数
	///
	bool updateViewCH();

	///
	/// @brief 更新ISE界面
	///
	/// @return true:更新成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年2月3日，新建函数
	///
	bool updateViewISE();

	///
	/// @brief 更新生化校准信息
	///
	///
	/// @par History:
	/// @li  8580/GongZhiQiang，2023年2月2日，新建函数
	///
	void updateCalibrationInfoCH();

	///
	/// @brief 更新生化质控结果
	///
	///
	/// @par History:
	/// @li  8580/GongZhiQiang，2023年2月2日，新建函数
	///
	void updateQcResultCH();

	///
	/// @brief 更新生化项目信息
	///
	///
	/// @par History:
	/// @li  8580/GongZhiQiang，2023年2月2日，新建函数
	///
	void updateAssayInfoCH();

	///
	/// @brief 更新生化试剂信息
	///
	///
	/// @par History:
	/// @li  8580/GongZhiQiang，2023年2月2日，新建函数
	///
	void updateReagentInfoCH();

	///
	/// @brief 更新ISE质控结果
	///
	///
	/// @par History:
	/// @li  8580/GongZhiQiang，2023年2月2日，新建函数
	///
	void updateQcResultISE();

	///
	/// @brief 更新ISE耗材信息
	///
	///
	/// @par History:
	/// @li  8580/GongZhiQiang，2023年2月2日，新建函数
	///
	void updateSuppliesInfoISE();

	///
	/// @brief 更新ISE校准信息
	///
	///
	/// @par History:
	/// @li  8580/GongZhiQiang，2023年2月2日，新建函数
	///
	void updateCalibrationInfoISE();

	///
	/// @brief 更新ISE项目信息
	///
	///
	/// @par History:
	/// @li  8580/GongZhiQiang，2023年2月2日，新建函数
	///
	void updateAssayInfoISE();

	///
	/// @brief 清空生化界面
	///
	///
	/// @par History:
	/// @li  8580/GongZhiQiang，2023年2月2日，新建函数
	///
	void clearViewCH();

	///
	/// @brief 清空ISE界面
	///
	///
	/// @par History:
	/// @li  8580/GongZhiQiang，2023年2月2日，新建函数
	///
	void clearViewISE();

	///
	/// @brief 获取项目模块名称
	///
	/// @param[in]  deviceSN  设备编号
	///
	/// @return 模块名称
	///
	/// @par History:
	/// @li  8580/GongZhiQiang，2023年2月2日，新建函数
	///
	QString getModuleIndex(const std::string deviceSN);

	///
	/// @brief 获取同杯前反应
	///
	/// @param[in]  previousAssayCode  同杯前反应编号
	///
	/// @return 编号
	///
	/// @par History:
	/// @li  8580/GongZhiQiang，2023年2月2日，新建函数
	///
	QString getLastAssay(const int previousAssayCode);

	///
	/// @brief 是否过期
	///
	/// @param[in]  leftDate  左时间
	/// @param[in]  rightDate 右时间
	///
	/// @return leftDate > rightDate = true
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年10月4日，新建函数
	///
	bool IsExpired(const std::string& leftDate, const std::string& rightDate);

	///
	/// @brief 显示曲线
	///
	/// @param[in]  assayResult  结果
	///
	/// @return true成功
	///
	/// @par History:
	/// @li  8580/GongZhiQiang，2023年2月2日，新建函数
	///
	bool DisplayCurve(ch::tf::AssayTestResult& assayResult);

	///
	/// @brief 显示曲线
	///
	/// @param[in]  assayResult  结果
	///
	/// @return true成功
	///
	/// @par History:
	/// @li  8580/GongZhiQiang，2023年2月2日，新建函数
	///
	bool DisplayAnalyPointInfo(const ch::tf::AssayTestResult& AssayResult, 
		std::shared_ptr<CH_QC_DETAIL_PLOT::CustomGrid>& spCurve);

	///
	/// @brief 显示反应过程
	///
	/// @param[in]  detectPointer  显示过程的点
	///
	/// @par History:
	/// @li  8580/GongZhiQiang，2023年2月2日，新建函数
	///
	void ShowProcessCurve(DetectCurve& detectPointer);

	///
	/// @brief 绘图初始化
	///
	///
	/// @par History:
	/// @li  8580/GongZhiQiang，2023年2月2日，新建函数
	///
	void initPlot();

	///
	/// @brief 初始化信号槽
	///
	///
	/// @par History:
	/// @li  8580/GongZhiQiang，2023年10月26日，新建函数
	///
	void initConnect();

	///
	/// @brief 初始化子控件
	///
	///
	/// @par History:
	/// @li  8580/GongZhiQiang，2023年10月26日，新建函数
	///
	void initChildControl();

	///
	/// @brief 事件处理函数
	///
	/// @param[in]  obj   目标对象
	/// @param[in]  event 事件对象
	///
	/// @return true表示事件已处理
	///
	/// @par History:
	/// @li  8580/GongZhiQiang，2023年2月2日，新建函数
	///
	bool eventFilter(QObject *obj, QEvent *event) override;

protected slots:

	///
	/// @brief 恢复原图
	///
	///
	/// @par History:
	/// @li  8580/GongZhiQiang，2023年2月2日，新建函数
	///
	void ResetCurve();

	void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;

private:
	Ui::chQcResultDetailsDlg* ui;

	ch::tf::AssayTestResult         m_chAssayResult;            ///< ch测试结果信息
	ise::tf::AssayTestResult		m_iseAssayResult;			///< ISE测试结果信息

	int                             m_chReusltID;			    ///< 当前生化质控结果测试数据库主键
	int                             m_iseResultID;			    ///< 当前ISE质控结果测试数据库主键

	bool							m_bCalculated;				///< 计算选择
	QC_STATE						m_enQcState;				///< 质控状态

	bool                            m_controlModify;            ///< control被选中
	bool                            m_shiftKey;					///< shiftKey被选中
	boost::optional<double>			m_startPostion;
	QwtPlotMarker*                  m_plotMarker;
	std::shared_ptr<CH_QC_DETAIL_PLOT::CustomGrid>		m_pSplitGrid;
	std::vector<std::shared_ptr<CH_QC_DETAIL_PLOT::QReactionCurveData>> m_reactionDatas;
	std::vector<std::shared_ptr<QwtPlotCurve>> m_waveCurves;
	QwtPlotMagnifier*               m_plotMagnifier;
	QwtPlotPanner*                  m_plotPanner;

};
