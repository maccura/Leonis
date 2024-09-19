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
/// @file     calhiscurve.cpp
/// @brief    校准历史曲线
///
/// @author   4170/TangChuXian
/// @date     2021年3月25日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2021年3月25日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "calhiscurve.h"
#include "ui_calhiscurve.h"
#include "mccustomscaledraw.h"
#include "src/common/Mlog/mlog.h"
#include <QMouseEvent>

#include <qwt_legend.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_grid.h>
#include <qwt_symbol.h>
#include <qwt_scale_div.h>
#include <qwt_scale_draw.h>
#include <qwt_scale_widget.h>

CalHisCurve::CalHisCurve(QWidget *parent)
    : QWidget(parent),
      m_bInit(false),
      m_iCalCnt(0),
      m_iSelIndex(0)
{
    // 初始化UI对象
    ui = new Ui::CalHisCurve();
    ui->setupUi(this);

    // 界面显示前初始化
    InitBeforeShow();
}

CalHisCurve::~CalHisCurve()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
    delete ui;
}

void CalHisCurve::TestRangeChanged(int iPos)
{
    ui->plot->setAxisScale(QwtPlot::xBottom, iPos + 0.5, iPos + 25.5, 1.0);
}

///
/// @brief
///     设置校准历史曲线的数据
///
/// @param[in]  CalCnt          校准次数
/// @param[in]  mapCal1HisData  校准品1的校准历史数据
/// @param[in]  mapCal2HisData  校准品2的校准历史数据
///
/// @par History:
/// @li 4170/TangChuXian，2021年3月26日，新建函数
///
void CalHisCurve::SetHisCount(int CalCnt)
{
    // 校准次数至少为1
    if (CalCnt < 0)
    {
        return;
    }

    // 设置校准历史曲线的数据
    m_iCalCnt           = CalCnt;

    // 重置选中项索引
    m_iSelIndex = 0;
    emit SigCurrentSelCalHisInfoChanged(m_iSelIndex);

    // 更新图表
    UpdatePlot();
}

///
/// @brief
///     重置曲线图
///
/// @par History:
/// @li 4170/TangChuXian，2021年8月10日，新建函数
///
void CalHisCurve::Reset()
{
    // 设置校准历史曲线的数据
    m_iCalCnt = 0;

    // 重置选中项索引
    m_iSelIndex = 0;
    emit SigCurrentSelCalHisInfoChanged(m_iSelIndex);

    // 更新图表
    UpdatePlot();
}

///
/// @brief
///     设置选中项索引
///
/// @param[in]  iIndex  索引
///
/// @par History:
/// @li 4170/TangChuXian，2021年8月10日，新建函数
///
void CalHisCurve::SetSelIndex(int iIndex)
{
    // 检查参数
    if ((iIndex <= 0) || (iIndex > m_iCalCnt))
    {
        return;
    }

    // 更新选中索引
    m_iSelIndex = iIndex;
	m_pVLineMarker->setXValue(m_iSelIndex);
    emit SigCurrentSelCalHisInfoChanged(m_iSelIndex);
}

///
/// @brief
///     获取选中项索引
///
/// @par History:
/// @li 4170/TangChuXian，2021年8月10日，新建函数
///
int CalHisCurve::GetSelIndex()
{
    return m_iSelIndex;
}

///
/// @brief 获取qwt plot
///
/// @return plot指针
///
/// @par History:
/// @li 8090/YeHuaNing，2022年12月9日，新建函数
///
QwtPlot* CalHisCurve::GetQwtPlot() {
    return ui->plot;
}

///
/// @brief
///     界面显示前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2021年5月13日，新建函数
///
void CalHisCurve::InitBeforeShow()
{
    // 初始化图例
    InitLegend();
}

///
/// @brief
///     显示之后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2021年3月25日，新建函数
///
void CalHisCurve::InitAfterShow()
{

}

///
/// @brief
///     初始化图例
///
/// @par History:
/// @li 4170/TangChuXian，2021年3月25日，新建函数
///
void CalHisCurve::InitLegend()
{
    // plot自动更新
    ui->plot->setAutoReplot(true);

    // 设置背景色
    ui->plot->setCanvasBackground(QColor(0xf5, 0xf7, 0xfb));

    // 初始化网格分割线
    m_pSplitGrid = new QwtPlotGrid();
//     m_pSplitGrid->enableY(false);
//     m_pSplitGrid->enableXMin(true);
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

    // 校准历史横坐标只能为整型
    /*McCustomIntegerScaleDraw *pScaleDrawX = new McCustomIntegerScaleDraw();
    if (pScaleDrawX)
    {
        pScaleDrawX->enableComponent(QwtAbstractScaleDraw::Backbone, false);
        pScaleDrawX->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    }
    ui->plot->setAxisScaleDraw(QwtPlot::xBottom, pScaleDrawX);*/

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

    // 插入图例
    //ui->plot->insertLegend(new QwtLegend(), QwtPlot::TopLegend);

    // 设置坐标轴刻度
    ui->plot->setAxisScale(QwtPlot::xBottom, 0.5, 25.5, 1.0);
    ui->plot->setAxisAutoScale(QwtPlot::yLeft);
    ui->plot->setAxisAutoScale(QwtPlot::yRight);
    ui->plot->setAxisAutoScale(QwtPlot::xBottom);
}

///
/// @brief
///     窗口显示事件
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 4170/TangChuXian，2021年3月25日，新建函数
///
void CalHisCurve::showEvent(QShowEvent *event)
{
    // 让基类处理事件
    QWidget::showEvent(event);

    // 第一次显示时初始化
    if (!m_bInit)
    {
        m_bInit = true;
        InitAfterShow();
    }
}

///
/// @brief
///     事件过滤器
///
/// @param[in]  obj    目标对象
/// @param[in]  event  事件对象
///
/// @return true表示事件已处理
///
/// @par History:
/// @li 4170/TangChuXian，2021年3月26日，新建函数
///
bool CalHisCurve::eventFilter(QObject *obj, QEvent *event)
{
    if (obj != ui->plot)
    {
        return QWidget::eventFilter(obj, event);
    }

    // 获取鼠标点击事件
    if (event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent* mouseEvent = (QMouseEvent*)event;
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
            double valX = ui->plot->invTransform(QwtPlot::xBottom, posX);
            int iValX = valX + 0.5;
            if ((iValX <= 0) || (iValX > m_iCalCnt))
            {
                return QWidget::eventFilter(obj, event);
            }

            // 如果选中索引改变，则发送信号
            if (m_iSelIndex != iValX)
            {
                m_iSelIndex = iValX;
                m_pVLineMarker->setXValue(m_iSelIndex);
                //McCustomIntegerScaleDraw *pScaleDrawX = new McCustomIntegerScaleDraw();
                //pScaleDrawX->enableComponent(QwtAbstractScaleDraw::Backbone, false);
                //pScaleDrawX->enableComponent(QwtAbstractScaleDraw::Ticks, false);
                //pScaleDrawX->SetSelLabelColor(m_iSelIndex, QColor(0xfa, 0x64, 0x31));
                //ui->plot->setAxisScaleDraw(QwtPlot::xBottom, pScaleDrawX);
                emit SigCurrentSelCalHisInfoChanged(m_iSelIndex);
            }

            emit SigOnSelectPoint(QPoint(posX, mouseEvent->pos().y() + 2));
        }
    }

    return QWidget::eventFilter(obj, event);
}

///
/// @brief
///     更新图表
///
/// @par History:
/// @li 4170/TangChuXian，2021年3月26日，新建函数
///
void CalHisCurve::UpdatePlot()
{
    // 更新X轴刻度
    double dStep = 1.0;
    if (m_iCalCnt > 25)
    {
        dStep = 5.0;
    }
    ui->plot->setAxisScale(QwtPlot::xBottom, 0.5, m_iCalCnt + 0.5, dStep);
//     ui->plot->setAxisMaxMajor(QwtPlot::xBottom, m_iCalCnt - 1);
//     ui->plot->setAxisMaxMinor(QwtPlot::xBottom, 2);

    // 更新选中线标记
    m_pVLineMarker->setXValue(m_iSelIndex);

    // 设置选中刻度颜色
    /*McCustomIntegerScaleDraw *pScaleDrawX = new McCustomIntegerScaleDraw();
    pScaleDrawX->enableComponent(QwtAbstractScaleDraw::Backbone, false);
    pScaleDrawX->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    pScaleDrawX->SetSelLabelColor(m_iSelIndex, QColor(0xfa, 0x64, 0x31));
    ui->plot->setAxisScaleDraw(QwtPlot::xBottom, pScaleDrawX);*/

    // 如果y轴右侧没有设置坐标，则不进行刻度计算
    if (!ui->plot->axisEnabled(QwtPlot::yRight))
        return;

    // 调整右坐标轴使其与左轴刻度数量一致
    // 获取左轴刻度数量
    int iTickCnt = ui->plot->axisScaleDiv(QwtPlot::yLeft).ticks(QwtScaleDiv::MajorTick).size();
    if (iTickCnt <= 1)
    {
        return;
    }

    // 获取右轴上下界限
    double dLeftLower = ui->plot->axisScaleDiv(QwtPlot::yLeft).lowerBound();
    double dLeftUpper = ui->plot->axisScaleDiv(QwtPlot::yLeft).upperBound();
    double dRightLower = ui->plot->axisScaleDiv(QwtPlot::yRight).lowerBound();
    double dRightUpper = ui->plot->axisScaleDiv(QwtPlot::yRight).upperBound();

    // 计算间隔
    double dStepLeft = qAbs(ui->plot->axisScaleDiv(QwtPlot::yLeft).ticks(QwtScaleDiv::MajorTick)[0] - ui->plot->axisScaleDiv(QwtPlot::yLeft).ticks(QwtScaleDiv::MajorTick)[1]);
    double dStepRight = qAbs(ui->plot->axisScaleDiv(QwtPlot::yRight).ticks(QwtScaleDiv::MajorTick)[0] - ui->plot->axisScaleDiv(QwtPlot::yRight).ticks(QwtScaleDiv::MajorTick)[1]);

    // 如果左右坐标轴范围大小和刻度间隔一致，则无需处理
    if (((dLeftUpper - dLeftLower) == (dRightUpper - dRightLower)) && (dStepLeft == dStepRight))
    {
        return;
    }

    // 调整左轴右轴使刻度对齐
    if ((dLeftUpper - dLeftLower) > (dRightUpper - dRightLower))
    {
        double dRightLowerOffset = int(qAbs(dRightLower - dLeftLower) / dStepLeft) * dStepLeft;
        if (dRightLower > dLeftLower)
        {
            dRightLower = dLeftLower + dRightLowerOffset;
        }
        else
        {
            dRightLower = dLeftLower - dRightLowerOffset - dStepLeft;
        }
        dRightUpper = dRightLower + (dLeftUpper - dLeftLower);

        ui->plot->setAxisScale(QwtPlot::yRight, dRightLower, dRightUpper, dStepLeft);
    }
    else
    {
        double dLeftLowerOffset = int(qAbs(dLeftLower - dRightLower) / dStepRight) * dStepRight;
        if (dLeftLower > dRightLower)
        {
            dLeftLower = dRightLower + dLeftLowerOffset;
        }
        else
        {
            dLeftLower = dRightLower - dLeftLowerOffset - dStepRight;
        }
        dLeftUpper = dLeftLower + (dRightUpper - dRightLower);

        ui->plot->setAxisScale(QwtPlot::yLeft, dLeftLower, dLeftUpper, dStepRight);
    }
}

///
/// @brief 设置右侧Y轴是否显示
///
/// @par History:
/// @li 8090/YeHuaNing，2022年12月13日，新建函数
///
void CalHisCurve::SetYAxisEnable()
{
    ui->plot->enableAxis(QwtPlot::yRight);
}

void CalHisCurve::SetAxisColor(int axisId, const QColor & color)
{
    // QColor(0x1f, 0xb5, 0x58)
    McCustomColorScaleDraw* pScaleDraw = new McCustomColorScaleDraw();
    pScaleDraw->SetLabelColor(color);
    ui->plot->setAxisScaleDraw(axisId, pScaleDraw);
}

///
/// @brief 设置坐标轴标题
///
/// @param[in]  axisId  坐标轴
/// @param[in]  title  标题
///
/// @par History:
/// @li 8090/YeHuaNing，2022年12月14日，新建函数
///
void CalHisCurve::SetAxisTitle(int axisId, const QString & title)
{
    // 设置坐标轴标题
    ui->plot->setAxisTitle(axisId, title);
}

///
/// @brief  设置Y轴范围
///
/// @param[in]  min  最小值
/// @param[in]  max  最大值
///
/// @return 
///
/// @par History:
/// @li 1226/zhangjing，2023年3月4日，新建函数
///
void CalHisCurve::SetYRange(double min, double max)
{
	int step = (max - min + 4) / 10;
	if (step < 1)
	{
		step = 1;
	}
	ui->plot->setAxisScale(QwtPlot::yLeft, min - 2, max + 2, (double)step);
	ui->plot->setAxisScale(QwtPlot::yRight, min - 2, max + 2, (double)step);
}

