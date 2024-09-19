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
#include "QConsumableCurve.h"
#include "ui_QConsumableCurve.h"
#include <qwt_legend.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_grid.h>
#include <qwt_symbol.h>
#include <qwt_scale_div.h>
#include <qwt_scale_draw.h>
#include <qwt_scale_widget.h>
#include <QMouseEvent>

#define  CURVE_NUM  (7)  // 曲线数

QConsumableCurve::QConsumableCurve(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::QConsumableCurve())
	, m_pSplitGrid(new QwtPlotGrid())
	, m_pVLineMarker(new QwtPlotMarker())
	, m_statisType(::tf::StatisType::type::STATIS_REAG)
{
	ui->setupUi(this);
	InitBeforeShow();
}

QConsumableCurve::~QConsumableCurve()
{
	delete ui;
}

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
void QConsumableCurve::InitBeforeShow()
{
	// plot 自动更新
	ui->plot->setAutoReplot(true);

	// 设置背景色
	ui->plot->setCanvasBackground(QColor(0xf5, 0xf7, 0xfb));

	// 创建平移器
	m_pPanner = new QwtPlotPanner(ui->plot->canvas());
	connect(m_pPanner, SIGNAL(panned(int, int)), this, SIGNAL(Panned(int, int)));

	// 初始化网格分割线
	m_pSplitGrid = new QwtPlotGrid();
	m_pSplitGrid->enableX(false);
	m_pSplitGrid->enableXMin(true);
	m_pSplitGrid->setMinorPen(Qt::transparent);
	m_pSplitGrid->setMajorPen(QColor(0xd4, 0xd4, 0xd4), 1, Qt::DashLine);
	m_pSplitGrid->attach(ui->plot);

	// 初始化标记选中线
	m_pVLineMarker = new QwtPlotMarker();
	m_pVLineMarker->setLineStyle(QwtPlotMarker::VLine);
	m_pVLineMarker->setLinePen(QColor(0xfa, 0x64, 0x31));
	m_pVLineMarker->attach(ui->plot);
	m_pVLineMarker->setZ(20);

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
	ui->plot->setAxisTitle(QwtPlot::yLeft, tr("测试数"));
	
	// 设置边框
	ui->plot->canvas()->setStyleSheet("border:1px solid #a3a3a3;background:#f5f7fb;");

	// 初始化X轴
	m_pXDateScaleDraw = new CustomScaleDraw();
	ui->plot->setAxisScaleDraw(QwtPlot::xBottom, m_pXDateScaleDraw);
	ui->plot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Backbone, false);
	ui->plot->setAxisMaxMinor(QwtPlot::xBottom, 0);

	// 选中气泡
	m_pSelMarker = new StatisticsGraphicSelTipMarker();
	m_pSelMarker->attach(ui->plot);

	SetShowMode(::tf::GroupByType::type::GROUP_BY_DAY, 12);
	CreateCurves();
	ui->plot->replot();
}

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
void QConsumableCurve::CreateCurves()
{
	for (int i = 0; i < CURVE_NUM; ++i)
	{
		// 创建显示的曲线
		QwtPlotCurve *pCurve = new QwtPlotCurve("12");
		m_curves.append(pCurve);
		pCurve->setRenderHint(QwtPlotItem::RenderAntialiased);
		pCurve->attach(ui->plot);

		// 选中效果曲线
		auto pSelCurve = new QwtPlotCurve();
		pSelCurve->attach(ui->plot);
		pSelCurve->hide();
		m_pSelPoints.append(pSelCurve);
	}

	int width = 2;
	m_curves[ID_REAG_CONSUMPTION]->setPen(QPen(QColor(27, 125, 245), width));
	m_curves[ID_REAG_WORK]->setPen(QPen(QColor(31, 181, 88), width));
	m_curves[ID_SAMPLE_WORK]->setPen(QPen(QColor(19, 191, 191), width));
	m_curves[ID_RECHECK_WORK]->setPen(QPen(QColor(250, 144, 22), width));
	m_curves[ID_QC_WORK]->setPen(QPen(QColor(124, 63, 209), width));
	m_curves[ID_CALI_WORK]->setPen(QPen(QColor(209, 56, 204), width));
	m_curves[ID_CONSUMPTION]->setPen(QPen(QColor(27, 125, 245), width));

	// 选中效果
	QwtSymbol* pSelSymbol = new QwtSymbol(QwtSymbol::Pixmap);
	pSelSymbol->setPixmap(QPixmap(":/Leonis/resource/image/icon_reag_con_curve.png"));
	m_pSelPoints[ID_REAG_CONSUMPTION]->setSymbol(pSelSymbol);

	pSelSymbol = new QwtSymbol(QwtSymbol::Pixmap);
	pSelSymbol->setPixmap(QPixmap(":/Leonis/resource/image/icon_reag_work_curve.png"));
	m_pSelPoints[ID_REAG_WORK]->setSymbol(pSelSymbol);

	pSelSymbol = new QwtSymbol(QwtSymbol::Pixmap);
	pSelSymbol->setPixmap(QPixmap(":/Leonis/resource/image/icon_sample_work_curve.png"));
	m_pSelPoints[ID_SAMPLE_WORK]->setSymbol(pSelSymbol);

	pSelSymbol = new QwtSymbol(QwtSymbol::Pixmap);
	pSelSymbol->setPixmap(QPixmap(":/Leonis/resource/image/icon_recheck_work_curve.png"));
	m_pSelPoints[ID_RECHECK_WORK]->setSymbol(pSelSymbol);

	pSelSymbol = new QwtSymbol(QwtSymbol::Pixmap);
	pSelSymbol->setPixmap(QPixmap(":/Leonis/resource/image/icon_qc_work_curve"));
	m_pSelPoints[ID_QC_WORK]->setSymbol(pSelSymbol);

	pSelSymbol = new QwtSymbol(QwtSymbol::Pixmap);
	pSelSymbol->setPixmap(QPixmap(":/Leonis/resource/image/icon_cali_work_curve.png"));
	m_pSelPoints[ID_CALI_WORK]->setSymbol(pSelSymbol);

	pSelSymbol = new QwtSymbol(QwtSymbol::Pixmap);
	pSelSymbol->setPixmap(QPixmap(":/Leonis/resource/image/icon_reag_con_curve.png"));
	m_pSelPoints[ID_CONSUMPTION]->setSymbol(pSelSymbol);
}

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
void QConsumableCurve::SetData(const QVector<QVector<QPointF>>& data)
{
	m_datas.clear();
	m_datas = data;
	for (int i = 0; i < m_curves.size(); ++i)
	{
		if (i >= data.size())
		{
			break;
		}
		m_curves[i]->setSamples(m_datas[i]);
	}

	HideAllSelSymbol();
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
///  @li 6889/ChenWei，2024年1月10日，新建函数
///
QString QConsumableCurve::label(double v)
{
    return m_pXDateScaleDraw->label(v).text();
}

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
const QVector<QVector<QPointF>>& QConsumableCurve::GetData()
{
    return m_datas;
}

///
/// @brief  清除曲线数据
///
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年12月18日，新建函数
///
void QConsumableCurve::ClearData()
{
	for (int i = 0; i < m_curves.size(); ++i)
	{
		m_curves[i]->setSamples(QVector<QPointF>());
	}

	for (int i = 0; i < m_datas.size(); ++i)
	{
		m_datas[i].clear();
	}
	
	HideAllSelSymbol();
}

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
void QConsumableCurve::HideAllSelSymbol()
{
	for (auto it : m_pSelPoints)
	{
		it->hide();
	}
	m_pVLineMarker->hide();
	m_pSelMarker->hide();
	m_pXDateScaleDraw->SetSelSymbolVisible(false);

	ui->plot->repaint();
}

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
bool QConsumableCurve::HasPointAtIndex(CURVE_ID curveId, const int iValX)
{
	if (curveId < 0 || curveId >= m_datas.size())
	{
		return false;
	}

	for (int i = 0;i < m_datas[curveId].size();++i)
	{
		if (m_datas[curveId][i].x() == iValX)
		{
			return true;
		}
	}
	return false;
}

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
QPointF QConsumableCurve::GetDataPointByIdAndXVal(CURVE_ID curveId, const int iValX)
{
	if (curveId < 0 || curveId >= m_datas.size())
	{
		return QPointF(-1,-1);
	}

	for (int i = 0;i < m_datas[curveId].size();++i)
	{
		if (m_datas[curveId][i].x() == iValX)
		{
			return m_datas[curveId][i];
		}
	}
	return QPointF(-1, -1);
}

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
void QConsumableCurve::ShowSelSymbol(const int iValX)
{
	for (int i = 0; i < CURVE_NUM; ++i)
	{
		m_pSelPoints[i]->hide();
		if (!m_curves[i]->isVisible())
		{
			continue;
		}
		
		if (!HasPointAtIndex(static_cast<CURVE_ID>(i), iValX))
		{
			continue;
		}
		QPointF dataPointF = GetDataPointByIdAndXVal(static_cast<CURVE_ID>(i), iValX);
		if (dataPointF.x() < 0 && dataPointF.y() < 0)
		{
			continue;
		}
		m_pSelPoints[i]->setSamples({ dataPointF });
		m_pSelPoints[i]->show();
	}
}

///
/// @brief  切换数据源试剂和耗材
///
/// @param[in]  bIsReag  true 试剂 false 耗材
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年12月15日，新建函数
///
void QConsumableCurve::SwitchStatisType(::tf::StatisType::type statisType)
{
	m_statisType = statisType;
}

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
void QConsumableCurve::SetShowMode(::tf::GroupByType::type mode, int maxRange)
{
	m_pXDateScaleDraw->SetShowMode(mode);

    m_rangMax = maxRange;

	ResetScaleDraw();
}

///
/// @brief  刷新X轴坐标
///
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年12月15日，新建函数
///
void QConsumableCurve::UpdateAxisScaleX(int maxRange)
{
	SetShowMode(m_pXDateScaleDraw->GetShowMode(), maxRange);
}

///
/// @brief  获取x轴刻度间宽度
///
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年12月19日，新建函数
///
int QConsumableCurve::GetXAxisScaleIntervalWidth()
{
	return ui->plot->axisScaleDiv(QwtPlot::xBottom).interval().width();
}

///
/// @brief  获取显示模式
///
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年12月15日，新建函数
///
::tf::GroupByType::type QConsumableCurve::GetShowMode()
{
	return m_pXDateScaleDraw->GetShowMode();
}

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
void QConsumableCurve::SetDateInterval(const QDate & startDate, const QDate & endDate)
{
	if (!startDate.isValid() || !endDate.isValid())
	{
		return;
	}
	m_pXDateScaleDraw->SetDateInterval(startDate, endDate);
	ui->plot->setAxisScale(QwtPlot::xBottom, 0.5, 12, 0.5);
}

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
void QConsumableCurve::SetCurveVisable(CURVE_ID index, bool visable)
{
	if (index < 0 || index >= m_curves.size())
	{
		return;
	}

	m_curves[index]->setVisible(visable);
	HideAllSelSymbol();
}

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
bool QConsumableCurve::eventFilter(QObject * obj, QEvent * event)
{
	if (obj != ui->plot)
	{
		return QWidget::eventFilter(obj, event);
	}
	if (m_datas.empty())
	{
		return QWidget::eventFilter(obj, event);
	}
	// 获取鼠标点击事件
	if (event->type() == QEvent::MouseButtonPress)
	{
		QMouseEvent* mouseEvent = (QMouseEvent*)event;

		if (mouseEvent->button() == Qt::RightButton)
		{
			ResetScaleDraw();
			return QWidget::eventFilter(obj, event);
		}

		QwtScaleWidget *yAxisWdgt = ui->plot->axisWidget(QwtPlot::yLeft);
		int posX;
		if (yAxisWdgt != NULL)
		{
			posX = mouseEvent->x() - yAxisWdgt->width();
		}
		else
		{
			posX = mouseEvent->x();
		}

		if (posX > 0)
		{
			int iValX = ui->plot->invTransform(QwtPlot::xBottom, posX) + 0.5;
			int iAlign = 0;
			auto point = GetMaxPointF(iValX);

			if (ui->plot->canvas()->height() - point.y() < 1000)
			{
				iAlign |= Qt::AlignBottom;
			}
			else
			{
				iAlign |= Qt::AlignTop;
			}

			if (point.x() < 120)
			{
				iAlign |= Qt::AlignRight;
			}
			else if (ui->plot->canvas()->width() - point.x() < 500)
			{
				iAlign |= Qt::AlignLeft;
			}
			else
			{
				iAlign |= Qt::AlignHCenter;
			}

			// 显示高亮圆环
			ShowSelSymbol(iValX);

			// 显示气泡
			m_pSelMarker->SetTipContent(GetTipList(iValX));
			m_pSelMarker->setValue(GetMaxPointF(iValX));
			m_pSelMarker->SetLabelAlignment(Qt::Alignment(iAlign), Qt::AlignLeft);
			m_pSelMarker->show();

			// 竖直选中线
			m_pVLineMarker->setXValue(iValX);
			m_pVLineMarker->show();

			// 设置选中的x轴标签
			m_pXDateScaleDraw->SetSelSymbolVisible(true);
			m_pXDateScaleDraw->SetSelLabelColor(iValX, QColor(0xfa, 0x64, 0x31));

			// 重绘
			ui->plot->repaint();
		}
	}

	return QWidget::eventFilter(obj, event);
}

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
QStringList QConsumableCurve::GetTipList(const int iValX)
{
	QStringList list;
	std::map<int, double> idAndYMap;
	for (int i = 0; i < m_datas.size(); ++i)
	{
		if (i >= m_curves.size())
		{
			continue;
		}

		if (!HasPointAtIndex(static_cast<CURVE_ID>(i), iValX))
		{
			continue;
		}

		QPointF dataPointF = GetDataPointByIdAndXVal(static_cast<CURVE_ID>(i), iValX);
		if (dataPointF.x() < 0 && dataPointF.y() < 0)
		{
			continue;
		}

		if (m_curves[i]->isVisible())
		{
			idAndYMap[i] = dataPointF.y();
		}
	}
	for (auto it = idAndYMap.begin(); it != idAndYMap.end(); ++it)
	{
		switch (it->first)
		{
		case ID_REAG_CONSUMPTION:
			//list.append(tr("试剂消耗量") + ": " + QString::number(it->second));
			break;
		case ID_REAG_WORK:
			list.append(tr("试剂工作量") + ": " + QString::number(it->second));
			break;
		case ID_SAMPLE_WORK:
			list.append(tr("样本工作量") + ": " + QString::number(it->second));
			break;
		case ID_RECHECK_WORK:
			list.append(tr("复查工作量") + ": " + QString::number(it->second));
			break;
		case ID_QC_WORK:
			list.append(tr("质控工作量") + ": " + QString::number(it->second));
			break;
		case ID_CALI_WORK:
			list.append(tr("校准工作量") + ": " + QString::number(it->second));
			break;
		case ID_CONSUMPTION:
			list.append(tr("耗材消耗量") + ": " + QString::number(it->second));
			break;
		default:
			break;
		}
	}

	return list;
}

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
QPointF QConsumableCurve::GetMaxPointF(const int iValX)
{
	QPointF point(-1, -1);
	for (int i = 0; i < m_datas.size(); ++i)
	{
		if (i >= m_curves.size())
		{
			break;
		}

		if (!m_curves[i]->isVisible())
		{
			continue;
		}
		if (!HasPointAtIndex(static_cast<CURVE_ID>(i), iValX))
		{
			continue;
		}
		QPointF dataPointF = GetDataPointByIdAndXVal(static_cast<CURVE_ID>(i), iValX);
		if (dataPointF.x() < 0 && dataPointF.y() < 0)
		{
			continue;
		}
		if (point.y() < dataPointF.y())
		{
			point = dataPointF;
		}
	}
	return point;
}

///
/// @brief  重置坐标轴
///
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年12月19日，新建函数
///
void QConsumableCurve::ResetScaleDraw()
{
    ui->plot->setAxisScale(QwtPlot::xBottom, 0.5, m_rangMax<12? m_rangMax:12, 1.0);
	ui->plot->setAxisScale(QwtPlot::yLeft, 0.0, 2000, 200.0);
	emit Panned(0, 0);
}

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
void QConsumableCurve::OnSliderValueChanged(int iValue)
{
    if (iValue > m_rangMax || iValue < 0 || m_rangMax <= 12)
    {
        return;
    }
    if (12 + iValue > m_rangMax)
    {
        iValue = m_rangMax - 12;
    }
    ui->plot->setAxisScale(QwtPlot::xBottom, 0.5+ iValue, 12 + iValue, 1.0);
}

CustomScaleDraw::CustomScaleDraw()
	:m_iMode(::tf::GroupByType::type::GROUP_BY_DAY)
	, m_iSelVal(-1)
	, m_bShowSelSymbol(false)
{
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
QwtText CustomScaleDraw::label(double v) const
{
	// 初始化数值显示
	QString StrOffset("");
	switch (m_iMode)
	{
	case ::tf::GroupByType::type::GROUP_BY_DAY:
	{
		// 如果是整数，则显示整数，否则不显示
		if (qAbs(v - int(v)) < DBL_EPSILON)
		{
			if (m_startDate.isValid())
			{
				StrOffset = QString("%1.%2");
				QDate curDate = m_startDate.addDays(int(v) - 1);
				StrOffset = StrOffset.arg(curDate.month()).arg(curDate.day());
			}
			else
			{
				StrOffset.setNum(int(v));
			}
		}
	}
	break;
	case ::tf::GroupByType::type::GROUP_BY_WEEK:
	{
		// 如果是整数，则显示整数，否则不显示
		if (qAbs(v - int(v)) < DBL_EPSILON)
		{
			if (m_startDate.isValid())
			{
				StrOffset = QObject::tr("第") + QString("%1") + QObject::tr("周");
				QDate curDate = m_startDate.addDays(int(v) * 7 - 7);
				StrOffset = StrOffset.arg(v);
			}
			else
			{
				StrOffset.setNum(int(v));
			}
		}
	}
	break;
	case ::tf::GroupByType::type::GROUP_BY_MON:
	{
		// 如果是整数，则显示整数，否则不显示
		if (qAbs(v - int(v)) < DBL_EPSILON)
		{
			if (m_startDate.isValid())
			{
				StrOffset = QString("%1/%2");
				QDate curDate = m_startDate.addMonths(int(v) - 1);
				StrOffset = StrOffset.arg(curDate.year()).arg(curDate.month());
			}
			else
			{
				StrOffset.setNum(int(v));
			}
		}
	}
	break;
	default:
		break;
	}

	// 构造返回值
	QwtText qwtText(StrOffset);
	if (!m_bShowSelSymbol)
	{
		return qwtText;
	}

	if (m_color.isValid() && (int(v) == m_iSelVal))
	{
		qwtText.setColor(m_color);
	}

	// 显示刻度值
	return qwtText;
}

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
void CustomScaleDraw::SetDateInterval(const QDate& startDate, const QDate& endDate)
{
	m_startDate = startDate;
	m_endDate = endDate;

	this->invalidateCache();
}

///
/// @brief  获取X坐标轴起始日期
///
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年12月18日，新建函数
///
QDate & CustomScaleDraw::GetStartDate()
{
	return m_startDate;
}

///
/// @brief  获取X坐标轴截止日期
///
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年12月18日，新建函数
///
QDate & CustomScaleDraw::GetEndDate()
{
	return m_endDate;
}

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
void CustomScaleDraw::ClearStartDate()
{
	m_startDate = QDate();
	this->invalidateCache();
}

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
void CustomScaleDraw::SetShowMode(::tf::GroupByType::type showMode)
{
	m_iMode = showMode;
	this->invalidateCache();
}

///
/// @brief  获取显示模式
///
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年12月14日，新建函数
///
::tf::GroupByType::type CustomScaleDraw::GetShowMode()
{
	return m_iMode;
}

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
void CustomScaleDraw::SetSelLabelColor(int val, const QColor & color)
{
	m_iSelVal = val;
	m_color = color;
	this->invalidateCache();
}

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
void CustomScaleDraw::SetSelSymbolVisible(const bool bShowSelSymbol)
{
	m_bShowSelSymbol = bShowSelSymbol;
	this->invalidateCache();
}
