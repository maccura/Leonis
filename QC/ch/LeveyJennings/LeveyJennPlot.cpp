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
/// @file     mcqcljgraphic.cpp
/// @brief    质控LJ曲线图
///
/// @author   8090/YeHuaNing
/// @date     2022年11月15日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月15日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "LeveyJennPlot.h"

#include "ui_LeveyJennPlot.h"
#include "shared/mccustomscaledraw.h"
#include "shared/uicommon.h"
#include "src/common/Mlog/mlog.h"

#include "qwt_legend.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_marker.h"
#include "qwt_plot_grid.h"
#include "qwt_symbol.h"
#include "qwt_scale_div.h"
#include "qwt_scale_draw.h"
#include "qwt_scale_widget.h"
#include "qwt_plot_panner.h"

#include <cfloat>
#include <algorithm>
#include <QMouseEvent>
#include <QMetaType>
#include <QPoint>

#define  SELECT_SENSITIVITY                        (20)                 // 选中灵敏度

// 自画坐标轴LJ图X轴
class McQcLJGraphicX : public McCustomIntegerScaleDraw
{
public:
    explicit McQcLJGraphicX() {};

    // 重写函数:设置坐标轴的刻度值
    virtual QwtText label(double v) const
    {
        // 初始化数值显示
        QString StrOffset("");

        // 如果是整数，则显示整数，否则不显示
        if (qAbs(v - int(v)) <= DBL_EPSILON)
        {
            if ((int(v) == 1) || ((int(v) % 5) == 0))
            {
                StrOffset.setNum(int(v));
            }
        }

        // 构造返回值
        QwtText qwtText(StrOffset);

        // 显示刻度值
        return qwtText;
    }
};

LeveyJennPlot::LeveyJennPlot(QWidget *parent)
    : QWidget(parent),
    m_bInit(false)
{
    // 初始化UI对象
    ui = new Ui::LeveyJennPlot();
    ui->setupUi(this);

    // 界面显示前初始化
    InitBeforeShow();
}

LeveyJennPlot::~LeveyJennPlot()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
    delete ui;
}

///
/// @brief
///     设置曲线点集
///
/// @param[in]  enCurveId  曲线ID
/// @param[in]  vecPoint   曲线上的点集
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月15日，新建函数
///
void LeveyJennPlot::SetPoints(CURVE_ID enCurveId, const QVector<QPointF>& vecPoint, const QVector<long long>& vecPointID)
{
    switch (enCurveId)
    {
    case LeveyJennPlot::CURVE_ID_QC_1:
        m_pQc1Curve->setSamples(vecPoint);
        break;
    case LeveyJennPlot::CURVE_ID_QC_2:
        m_pQc2Curve->setSamples(vecPoint);
        break;
    case LeveyJennPlot::CURVE_ID_QC_3:
        m_pQc3Curve->setSamples(vecPoint);
        break;
    case LeveyJennPlot::CURVE_ID_QC_1_SUB_PT:
        m_pQc1SubPoints->setSamples(vecPoint);
        break;
    case LeveyJennPlot::CURVE_ID_QC_2_SUB_PT:
        m_pQc2SubPoints->setSamples(vecPoint);
        break;
    case LeveyJennPlot::CURVE_ID_QC_3_SUB_PT:
        m_pQc3SubPoints->setSamples(vecPoint);
        break;
    case LeveyJennPlot::CURVE_ID_QC_NO_CALC:
        m_pQcNoCalcPoints->setSamples(vecPoint);
        break;
    default:
        break;
    }

    // 更新曲线点集记录
    m_mapCurvePoints.insert(enCurveId, vecPoint);
    m_mapCurvePointsID.insert(enCurveId, vecPointID);

    // 获取上界限
    int iUpperX = 0;
    for (const QPointF& pt : vecPoint)
    {
        if (int(pt.x()) > iUpperX)
        {
            iUpperX = int(pt.x());
        }
    }

    // 更新曲线X上界
    m_iArrayUpperX[enCurveId] = iUpperX;

    // 判断是否显示不计算的点
    bool bNoCalcPtVisible = ui->ShowNoCalcQcCB->isChecked();

    // 获取所有曲线最大X上界
    int iMaxXUpper = 0;
    for (int i = 0; i < CURVE_ID_SIZE; i++)
    {
        if (!bNoCalcPtVisible && (i == CURVE_ID_QC_NO_CALC))
        {
            continue;
        }

        if (m_iArrayUpperX[i] > iMaxXUpper)
        {
            iMaxXUpper = m_iArrayUpperX[i];
        }
    }

    // 设置横坐标范围
    if (iMaxXUpper > 20)
    {
        ui->QcGraphic->setAxisScale(QwtPlot::xBottom, 0.5, iMaxXUpper + 4.5, 1.0);
        //ui->QcGraphic->setGeometry(-44, -4, 854 + ((iMaxXUpper - 16) * 40), 314);
        //ui->ScrollContent->setFixedSize(806 + ((iMaxXUpper - 16) * 40), 310);
    }
    else
    {
        ui->QcGraphic->setAxisScale(QwtPlot::xBottom, 0.5, 20.5, 1.0);
        //ui->QcGraphic->setGeometry(-44, -4, 854, 314);
        //ui->ScrollContent->setFixedSize(806, 310);
    }
}

///
/// @brief
///     重置曲线图
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月15日，新建函数
///
void LeveyJennPlot::Reset()
{
    QVector<QPointF> vEmptyPt;
    m_pQc1Curve->setSamples(vEmptyPt);
    m_pQc2Curve->setSamples(vEmptyPt);
    m_pQc3Curve->setSamples(vEmptyPt);
    m_pQc1SubPoints->setSamples(vEmptyPt);
    m_pQc2SubPoints->setSamples(vEmptyPt);
    m_pQc3SubPoints->setSamples(vEmptyPt);
    m_pQcNoCalcPoints->setSamples(vEmptyPt);

    // 重置选中点
    m_stuSelPointInfo.Reset();
    m_mapCurvePoints.clear();
    m_mapCurvePointsID.clear();
    m_pQcSelLineLeft->setSamples(vEmptyPt);
    m_pQcSelLineTop->setSamples(vEmptyPt);
    m_pQcSelLineRight->setSamples(vEmptyPt);
    m_pQcSelLineBottom->setSamples(vEmptyPt);

    // 显示隐藏点信息显示框
    ui->PtInfoViewFrame->setVisible(m_stuSelPointInfo.enCurveId != CURVE_ID_SIZE);

    // 通知选中点改变
    emit SigSelPointChanged(m_stuSelPointInfo.enCurveId, m_stuSelPointInfo.pointID);
}

///
/// @brief
///     重置曲线图但保留选中效果
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月15日，新建函数
///
void LeveyJennPlot::ResetButSel()
{
    QVector<QPointF> vEmptyPt;
    m_pQc1Curve->setSamples(vEmptyPt);
    m_pQc2Curve->setSamples(vEmptyPt);
    m_pQc3Curve->setSamples(vEmptyPt);
    m_pQc1SubPoints->setSamples(vEmptyPt);
    m_pQc2SubPoints->setSamples(vEmptyPt);
    m_pQc3SubPoints->setSamples(vEmptyPt);
    m_pQcNoCalcPoints->setSamples(vEmptyPt);
}

///
/// @brief
///     设置曲线是否可见
///
/// @param[in]  enCurveId  曲线ID
/// @param[in]  bVisible   是否可见
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月15日，新建函数
///
void LeveyJennPlot::SetCurveVisible(CURVE_ID enCurveId, bool bVisible)
{
    QwtPlotCurve* pOpCurve = Q_NULLPTR;
    switch (enCurveId)
    {
    case LeveyJennPlot::CURVE_ID_QC_1:
        pOpCurve = m_pQc1Curve;
        break;
    case LeveyJennPlot::CURVE_ID_QC_2:
        pOpCurve = m_pQc2Curve;
        break;
    case LeveyJennPlot::CURVE_ID_QC_3:
        pOpCurve = m_pQc3Curve;
        break;
    case LeveyJennPlot::CURVE_ID_QC_1_SUB_PT:
        pOpCurve = m_pQc1SubPoints;
        break;
    case LeveyJennPlot::CURVE_ID_QC_2_SUB_PT:
        pOpCurve = m_pQc2SubPoints;
        break;
    case LeveyJennPlot::CURVE_ID_QC_3_SUB_PT:
        pOpCurve = m_pQc3SubPoints;
        break;
    case LeveyJennPlot::CURVE_ID_QC_NO_CALC:
        pOpCurve = m_pQcNoCalcPoints;
        break;
    default:
        break;
    }

    // 如果无操作曲线，则直接返回
    if (pOpCurve == Q_NULLPTR)
    {
        return;
    }

    // 判断是显示操作曲线还是隐藏操作曲线
    if (bVisible)
    {
        pOpCurve->attach(ui->QcGraphic);
    }
    else
    {
        pOpCurve->detach();
    }
}

///
/// @brief
///     通过索引选中点
///
/// @param[in]  enCurveID  曲线ID
/// @param[in]  iIndex     点索引
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月15日，新建函数
///
void LeveyJennPlot::SelPointByIndex(CURVE_ID enCurveID, int iIndex)
{
    // 重置选中点信息
    m_stuSelPointInfo.Reset();

    auto it = m_mapCurvePointsID.find(enCurveID);
    if (it == m_mapCurvePointsID.end())
    {
        QVector<QPointF> vecPoints;
        m_pQcSelLineLeft->setSamples(vecPoints);
        m_pQcSelLineRight->setSamples(vecPoints);
        m_pQcSelLineTop->setSamples(vecPoints);
        m_pQcSelLineBottom->setSamples(vecPoints);
        return;
    }

    for (int i = 0; i < it.value().size(); i++)
    {
        if (i != iIndex)
        {
            continue;
        }

        // 更新选中点信息
        m_stuSelPointInfo.enCurveId = enCurveID;
        m_stuSelPointInfo.iIndex = iIndex;
        m_stuSelPointInfo.pointID = it.value()[iIndex];

        // 更新选中线
        UpdateSelLine();
        break;
    }
}

///
/// @brief
///     通过ID选中点
///
/// @param[in]  enCurveID  曲线ID
/// @param[in]  iPointID   点ID
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月15日，新建函数
///
void LeveyJennPlot::SelPointByID(CURVE_ID enCurveID, long long iPointID)
{
    // 重置选中点信息
    m_stuSelPointInfo.Reset();

    auto it = m_mapCurvePointsID.find(enCurveID);
    if (it == m_mapCurvePointsID.end())
    {
        QVector<QPointF> vecPoints;
        m_pQcSelLineLeft->setSamples(vecPoints);
        m_pQcSelLineRight->setSamples(vecPoints);
        m_pQcSelLineTop->setSamples(vecPoints);
        m_pQcSelLineBottom->setSamples(vecPoints);

        // 通知选中点改变
        //emit SigSelPointChanged(m_stuSelPointInfo.enCurveId, m_stuSelPointInfo.pointID);

        // 显示隐藏点信息显示框
        ui->PtInfoViewFrame->setVisible(false);
        return;
    }

    for (int iIndex = 0; iIndex < it.value().size(); iIndex++)
    {
        if (it.value()[iIndex] != iPointID)
        {
            continue;
        }

        // 更新选中点信息
        m_stuSelPointInfo.enCurveId = enCurveID;
        m_stuSelPointInfo.iIndex = iIndex;
        m_stuSelPointInfo.pointID = iPointID;

        // 更新选中线
        UpdateSelLine();
        break;
    }

    // 显示隐藏点信息显示框
    ui->PtInfoViewFrame->setVisible(m_stuSelPointInfo.enCurveId != CURVE_ID_SIZE);

    // 通知选中点改变
    //emit SigSelPointChanged(m_stuSelPointInfo.enCurveId, m_stuSelPointInfo.pointID);
}

///
/// @brief
///     获取选中点
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月15日，新建函数
///
const LeveyJennPlot::SelPointInfo& LeveyJennPlot::GetSelectPoint()
{
    return m_stuSelPointInfo;
}

///
/// @brief
///     界面显示前初始化
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月15日，新建函数
///
void LeveyJennPlot::InitBeforeShow()
{
    // 初始化图例
    InitLegend();

    // 初始化信号槽连接
    InitConnect();
}

///
/// @brief
///     显示之后初始化
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月15日，新建函数
///
void LeveyJennPlot::InitAfterShow()
{
    // 初始化字符串资源
    InitStrResource();

    // 判断是否显示子点
    bool bSubPtVisible = ui->ShowSubPointCB->isChecked();

    // 判断是否显示不计算的点
    bool bNoCalcPtVisible = ui->ShowNoCalcQcCB->isChecked();

    // 更新对应曲线（子点）显示
    SetCurveVisible(CURVE_ID_QC_1_SUB_PT, bSubPtVisible);
    SetCurveVisible(CURVE_ID_QC_2_SUB_PT, bSubPtVisible);
    SetCurveVisible(CURVE_ID_QC_3_SUB_PT, bSubPtVisible);

    // 更新对应曲线（不计算结果点）显示
    SetCurveVisible(CURVE_ID_QC_NO_CALC, bNoCalcPtVisible);
}

///
/// @brief
///     初始化字符串资源
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月15日，新建函数
///
void LeveyJennPlot::InitStrResource()
{
    /*ui->Qc1TextLab->setText(LoadStrFromLanguage(CHAR_CODE::IDS_QC_GRAPHIC_1));
    ui->Qc2TextLab->setText(LoadStrFromLanguage(CHAR_CODE::IDS_QC_GRAPHIC_2));
    ui->Qc3TextLab->setText(LoadStrFromLanguage(CHAR_CODE::IDS_QC_GRAPHIC_3));
    ui->NoCalcQcTextLab->setText(LoadStrFromLanguage(CHAR_CODE::IDS_QC_NO_CALC_POINT));
    ui->ShowNoCalcQcCB->setText(LoadStrFromLanguage(CHAR_CODE::IDS_QC_SHOW_NO_CALC_POINT));
    ui->ShowSubPointCB->setText(LoadStrFromLanguage(CHAR_CODE::IDS_QC_SHOW_SUB_POINT));
    ui->ResultNameLab->setText(LoadStrFromLanguage(CHAR_CODE::IDS_RLT_NAME));
    ui->TimeNameLab->setText(LoadStrFromLanguage(CHAR_CODE::IDS_TIME) + LoadStrFromLanguage(CHAR_CODE::IDS_SYMBOL_COLON));*/
}

///
/// @brief
///     初始化图例
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月15日，新建函数
///
void LeveyJennPlot::InitLegend()
{
    // 注册元类型
    qRegisterMetaType<CURVE_ID>("LeveyJennPlot::CURVE_ID");

    // 初始化X轴上界限
    for (int i = 0; i < CURVE_ID_SIZE; i++)
    {
        m_iArrayUpperX[i] = 0;
    }

    // 初始化点信息显示框尺寸
    int iWidthOffset = ui->TimeNameLab->width() + ui->TimeValLab->width() - 96;
    if (iWidthOffset > 0)
    {
        ui->PtInfoViewFrame->setGeometry(375 - (iWidthOffset / 2), ui->PtInfoViewFrame->y(), 127 + iWidthOffset, 53);
        ui->PointInfoFrame->move(-300 + (iWidthOffset / 2), 0);
    }

    // 隐藏点信息显示框
    ui->PtInfoViewFrame->setVisible(false);

    // 安装事件过滤器
    ui->QcGraphic->installEventFilter(this);
    ui->TimeNameLab->installEventFilter(this);
    ui->TimeValLab->installEventFilter(this);

    // plot自动更新
    ui->QcGraphic->setAutoReplot(true);

    // 设置背景色
    ui->QcGraphic->setCanvasBackground(QColor(0xf5, 0xf7, 0xfb));

    // 设置自定义坐标轴
    McCustomScaleDraw *pScaleDrawY = new McCustomScaleDraw();
    McCustomScaleDraw *pScaleDrawBkY = new McCustomScaleDraw();
    if (pScaleDrawY)
    {
        pScaleDrawY->enableComponent(QwtAbstractScaleDraw::Backbone, false);
        pScaleDrawY->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    }
    if (pScaleDrawBkY)
    {
        pScaleDrawBkY->enableComponent(QwtAbstractScaleDraw::Backbone, false);
        pScaleDrawBkY->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    }
    ui->QcGraphicBk->setAxisScaleDraw(QwtPlot::yLeft, pScaleDrawBkY);

    McQcLJGraphicX *pScaleDrawX = new McQcLJGraphicX();
    if (pScaleDrawX)
    {
        pScaleDrawX->enableComponent(QwtAbstractScaleDraw::Backbone, false);
        pScaleDrawX->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    }
    ui->QcGraphic->setAxisScaleDraw(QwtPlot::xBottom, pScaleDrawX);

    // 坐标无刻度
    ui->QcGraphic->enableAxis(QwtPlot::xTop);
    ui->QcGraphic->enableAxis(QwtPlot::yRight);
    ui->QcGraphic->axisScaleDraw(QwtPlot::xTop)->enableComponent(QwtAbstractScaleDraw::Backbone, false);
    ui->QcGraphic->axisScaleDraw(QwtPlot::xTop)->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    ui->QcGraphic->axisScaleDraw(QwtPlot::xTop)->enableComponent(QwtAbstractScaleDraw::Labels, false);
    ui->QcGraphic->axisScaleDraw(QwtPlot::yRight)->enableComponent(QwtAbstractScaleDraw::Backbone, false);
    ui->QcGraphic->axisScaleDraw(QwtPlot::yRight)->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    ui->QcGraphic->axisScaleDraw(QwtPlot::yRight)->enableComponent(QwtAbstractScaleDraw::Labels, false);

    // 背景图无刻度
    ui->QcGraphicBk->enableAxis(QwtPlot::xTop);
    ui->QcGraphicBk->enableAxis(QwtPlot::yRight);
    ui->QcGraphicBk->axisScaleDraw(QwtPlot::xTop)->enableComponent(QwtAbstractScaleDraw::Backbone, false);
    ui->QcGraphicBk->axisScaleDraw(QwtPlot::xTop)->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    ui->QcGraphicBk->axisScaleDraw(QwtPlot::xTop)->enableComponent(QwtAbstractScaleDraw::Labels, false);
    ui->QcGraphicBk->axisScaleDraw(QwtPlot::yRight)->enableComponent(QwtAbstractScaleDraw::Backbone, false);
    ui->QcGraphicBk->axisScaleDraw(QwtPlot::yRight)->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    ui->QcGraphicBk->axisScaleDraw(QwtPlot::yRight)->enableComponent(QwtAbstractScaleDraw::Labels, false);

    // 设置边框
    ui->QcGraphic->canvas()->setStyleSheet("border:1px solid #a3a3a3;background:#f5f7fb;");
    ui->QcGraphicBk->canvas()->setStyleSheet("border:1px solid #a3a3a3;background:#f5f7fb;");

    // 使用鼠标左键平移
    //QwtPlotPanner* pPanner = new QwtPlotPanner(ui->QcGraphic->canvas());
    //pPanner->setAxisEnabled(QwtPlot::yLeft, false);  //y轴坐标不平移

    // 初始化网格分割线
    m_pGrid = new QwtPlotGrid();
    m_pGrid->setMinorPen(Qt::transparent);
    m_pGrid->setMajorPen(QColor(0xd4, 0xd4, 0xd4), 1.0, Qt::DashLine);
    m_pGrid->attach(ui->QcGraphic);

    // 初始化SD标记选中线
    m_pP2SDLineMarker = new QwtPlotMarker();
    m_pP3SDLineMarker = new QwtPlotMarker();
    m_pN2SDLineMarker = new QwtPlotMarker();
    m_pN3SDLineMarker = new QwtPlotMarker();

    // 设置为水平线
    m_pP2SDLineMarker->setLineStyle(QwtPlotMarker::HLine);
    m_pP3SDLineMarker->setLineStyle(QwtPlotMarker::HLine);
    m_pN2SDLineMarker->setLineStyle(QwtPlotMarker::HLine);
    m_pN3SDLineMarker->setLineStyle(QwtPlotMarker::HLine);

    // 设置颜色
    m_pP2SDLineMarker->setLinePen(QColor(0xfa, 0x90, 0x16), 1.0, Qt::DashLine);
    m_pP3SDLineMarker->setLinePen(QColor(0xfa, 0x37, 0x41), 1.0, Qt::DashLine);
    m_pN2SDLineMarker->setLinePen(QColor(0xfa, 0x90, 0x16), 1.0, Qt::DashLine);
    m_pN3SDLineMarker->setLinePen(QColor(0xfa, 0x37, 0x41), 1.0, Qt::DashLine);

    // 设置Z轴坐标
    m_pP2SDLineMarker->setZ(20);
    m_pP3SDLineMarker->setZ(20);
    m_pN2SDLineMarker->setZ(20);
    m_pN3SDLineMarker->setZ(20);

    // 设置Y轴坐标
    m_pP2SDLineMarker->setYValue(2);
    m_pP3SDLineMarker->setYValue(3);
    m_pN2SDLineMarker->setYValue(-2);
    m_pN3SDLineMarker->setYValue(-3);

    // 添加到曲线图上
    m_pP2SDLineMarker->attach(ui->QcGraphic);
    m_pP3SDLineMarker->attach(ui->QcGraphic);
    m_pN2SDLineMarker->attach(ui->QcGraphic);
    m_pN3SDLineMarker->attach(ui->QcGraphic);

    // 初始化曲线成员对象
    // 质控曲线
    m_pQc1Curve = new QwtPlotCurve();
    m_pQc2Curve = new QwtPlotCurve();
    m_pQc3Curve = new QwtPlotCurve();
    m_pQc1SubPoints = new QwtPlotCurve();
    m_pQc2SubPoints = new QwtPlotCurve();
    m_pQc3SubPoints = new QwtPlotCurve();
    m_pQcNoCalcPoints = new QwtPlotCurve();

    // 选中效果线
    m_pQcSelLineLeft = new QwtPlotCurve();
    m_pQcSelLineRight = new QwtPlotCurve();
    m_pQcSelLineTop = new QwtPlotCurve();
    m_pQcSelLineBottom = new QwtPlotCurve();

    // 设置曲线颜色
    m_pQc1Curve->setPen(QColor(0x1b, 0x7d, 0xf5), 1.0);
    m_pQc2Curve->setPen(QColor(0x1f, 0xb5, 0x58), 1.0);
    m_pQc3Curve->setPen(QColor(0x9f, 0x65, 0xe6), 1.0);
    m_pQcSelLineLeft->setPen(QColor(0xfa, 0x64, 0x31), 2.0);
    m_pQcSelLineRight->setPen(QColor(0xfa, 0x64, 0x31), 2.0);
    m_pQcSelLineTop->setPen(QColor(0xfa, 0x64, 0x31), 2.0);
    m_pQcSelLineBottom->setPen(QColor(0xfa, 0x64, 0x31), 2.0);

    // 挂载到图表上
    m_pQc1Curve->attach(ui->QcGraphic);
    m_pQc2Curve->attach(ui->QcGraphic);
    m_pQc3Curve->attach(ui->QcGraphic);
    m_pQc1SubPoints->attach(ui->QcGraphic);
    m_pQc2SubPoints->attach(ui->QcGraphic);
    m_pQc3SubPoints->attach(ui->QcGraphic);
    m_pQcNoCalcPoints->attach(ui->QcGraphic);
    m_pQcSelLineLeft->attach(ui->QcGraphic);
    m_pQcSelLineRight->attach(ui->QcGraphic);
    m_pQcSelLineTop->attach(ui->QcGraphic);
    m_pQcSelLineBottom->attach(ui->QcGraphic);

    // 设置曲线点符号
    QwtSymbol* pQc1Symbol = new QwtSymbol(QwtSymbol::Ellipse);
    QwtSymbol* pQc2Symbol = new QwtSymbol(QwtSymbol::Ellipse);
    QwtSymbol* pQc3Symbol = new QwtSymbol(QwtSymbol::Ellipse);
    QwtSymbol* pQc1SubSymbol = new QwtSymbol(QwtSymbol::Ellipse);
    QwtSymbol* pQc2SubSymbol = new QwtSymbol(QwtSymbol::Ellipse);
    QwtSymbol* pQc3SubSymbol = new QwtSymbol(QwtSymbol::Ellipse);
    QwtSymbol* pQcNoCalcSymbol = new QwtSymbol(QwtSymbol::Ellipse);
    pQc1Symbol->setBrush(QBrush(QColor(0x1b, 0x7d, 0xf5)));
    pQc2Symbol->setBrush(QBrush(QColor(0x1f, 0xb5, 0x58)));
    pQc3Symbol->setBrush(QBrush(QColor(0x9f, 0x65, 0xe6)));
    pQc1SubSymbol->setBrush(QBrush(QColor(0x1b, 0x7d, 0xf5)));
    pQc2SubSymbol->setBrush(QBrush(QColor(0x1f, 0xb5, 0x58)));
    pQc3SubSymbol->setBrush(QBrush(QColor(0x9f, 0x65, 0xe6)));
    pQcNoCalcSymbol->setBrush(QBrush(QColor(0x80, 0x80, 0x80)));
    pQc1Symbol->setPen(QPen(Qt::NoPen));
    pQc2Symbol->setPen(QPen(Qt::NoPen));
    pQc3Symbol->setPen(QPen(Qt::NoPen));
    pQc1SubSymbol->setPen(QPen(Qt::NoPen));
    pQc2SubSymbol->setPen(QPen(Qt::NoPen));
    pQc3SubSymbol->setPen(QPen(Qt::NoPen));
    pQcNoCalcSymbol->setPen(QPen(Qt::NoPen));
    pQc1Symbol->setSize(10);
    pQc2Symbol->setSize(10);
    pQc3Symbol->setSize(10);
    pQcNoCalcSymbol->setSize(10);
    pQc1SubSymbol->setSize(8);
    pQc2SubSymbol->setSize(8);
    pQc3SubSymbol->setSize(8);
    m_pQc1Curve->setSymbol(pQc1Symbol);
    m_pQc2Curve->setSymbol(pQc2Symbol);
    m_pQc3Curve->setSymbol(pQc3Symbol);
    m_pQc1SubPoints->setSymbol(pQc1SubSymbol);
    m_pQc2SubPoints->setSymbol(pQc2SubSymbol);
    m_pQc3SubPoints->setSymbol(pQc3SubSymbol);
    m_pQcNoCalcPoints->setSymbol(pQcNoCalcSymbol);

    // 设置曲线样式为点
    m_pQc1SubPoints->setStyle(QwtPlotCurve::NoCurve);
    m_pQc2SubPoints->setStyle(QwtPlotCurve::NoCurve);
    m_pQc3SubPoints->setStyle(QwtPlotCurve::NoCurve);
    m_pQcNoCalcPoints->setStyle(QwtPlotCurve::NoCurve);

    // 设置坐标轴刻度
    ui->QcGraphic->setAxisScale(QwtPlot::yLeft, -3.9, 3.9, 1.0);
    ui->QcGraphicBk->setAxisScale(QwtPlot::yLeft, -3.9, 3.9, 1.0);
    ui->QcGraphic->setAxisScale(QwtPlot::xBottom, 0.5, 21.5, 1.0);
    //    ui->QcGraphic->setAxisAutoScale(QwtPlot::yLeft);
    ui->QcGraphic->setAxisAutoScale(QwtPlot::xBottom);
    //ui->QcGraphic->setGeometry(-44, -4, 854, 314);
    //ui->ScrollContent->setFixedSize(806 , 310);
}

///
/// @brief
///     初始化信号槽连接
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月15日，新建函数
///
void LeveyJennPlot::InitConnect()
{
    // 显示子点复选框状态改变
    connect(ui->ShowSubPointCB, SIGNAL(stateChanged(int)), this, SLOT(OnCurveVisibleChanged()));

    // 显示子点复选框状态改变
    connect(ui->ShowNoCalcQcCB, SIGNAL(stateChanged(int)), this, SLOT(OnCurveVisibleChanged()));
}

///
/// @brief
///     更新选中效果线
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月15日，新建函数
///
void LeveyJennPlot::UpdateSelLine()
{
    if (m_stuSelPointInfo.enCurveId == CURVE_ID_SIZE)
    {
        QVector<QPointF> vecPoints;
        m_pQcSelLineLeft->setSamples(vecPoints);
        m_pQcSelLineRight->setSamples(vecPoints);
        m_pQcSelLineTop->setSamples(vecPoints);
        m_pQcSelLineBottom->setSamples(vecPoints);
        return;
    }

    const int iSelLineWidth = 15;
    QPointF   selPt;

    auto it = m_mapCurvePoints.find(m_stuSelPointInfo.enCurveId);
    if (it == m_mapCurvePoints.end())
    {
        return;
    }

    // 找到选中点坐标
    for (int i = 0; i < it.value().size(); i++)
    {
        if (i != m_stuSelPointInfo.iIndex)
        {
            continue;
        }

        // 更新选中点信息
        selPt = it.value()[i];
    }

    // 无效点返回
    if (selPt.isNull())
    {
        return;
    }

    // 根据选中点位置，判断点信息显示框位置
    if (selPt.y() > 0)
    {
        ui->PtInfoViewFrame->move(ui->PtInfoViewFrame->x(), 228);
    }
    else
    {
        ui->PtInfoViewFrame->move(ui->PtInfoViewFrame->x(), 3);
    }

    int posX = ui->QcGraphic->transform(QwtPlot::xBottom, selPt.x());
    int posY = ui->QcGraphic->transform(QwtPlot::yLeft, selPt.y());

    // 构造坐标点
    QPointF leftPt1(posX - 22, posY + 1), leftPt2(posX - 10, posY + 1);
    QPointF topPt1(posX, posY - 20), topPt2(posX, posY - 8);
    QPointF rightPt1(posX + 10, posY + 1), rightPt2(posX + 22, posY + 1);
    QPointF bottomPt1(posX, posY + 9), bottomPt2(posX, posY + 21);

    auto TransFormPt = [this](QPointF& point)
    {
        point = QPointF(ui->QcGraphic->invTransform(QwtPlot::xBottom, point.x()), ui->QcGraphic->invTransform(QwtPlot::yLeft, point.y()));
    };

    // 点坐标转换
    TransFormPt(leftPt1);
    TransFormPt(leftPt2);
    TransFormPt(topPt1);
    TransFormPt(topPt2);
    TransFormPt(rightPt1);
    TransFormPt(rightPt2);
    TransFormPt(bottomPt1);
    TransFormPt(bottomPt2);

    // 构造点集
    QVector<QPointF> leftSelLine;
    QVector<QPointF> topSelLine;
    QVector<QPointF> rightSelLine;
    QVector<QPointF> bottomSelLine;
    leftSelLine << leftPt1 << leftPt2;
    topSelLine << topPt1 << topPt2;
    rightSelLine << rightPt1 << rightPt2;
    bottomSelLine << bottomPt1 << bottomPt2;

    // 更新选中曲线
    m_pQcSelLineLeft->setZ(200);
    m_pQcSelLineTop->setZ(200);
    m_pQcSelLineRight->setZ(200);
    m_pQcSelLineBottom->setZ(200);
    m_pQcSelLineLeft->setSamples(leftSelLine);
    m_pQcSelLineTop->setSamples(topSelLine);
    m_pQcSelLineRight->setSamples(rightSelLine);
    m_pQcSelLineBottom->setSamples(bottomSelLine);
}

///
/// @brief
///     窗口显示事件
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月15日，新建函数
///
void LeveyJennPlot::showEvent(QShowEvent *event)
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
/// @li 8090/YeHuaNing，2022年11月15日，新建函数
///
bool LeveyJennPlot::eventFilter(QObject *obj, QEvent *event)
{
    // 获取鼠标点击事件
    if (obj == ui->QcGraphic && event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent* mouseEvent = (QMouseEvent*)event;
        QwtScaleWidget *yAxisWdgt = ui->QcGraphic->axisWidget(QwtPlot::yLeft);
        int posX;
        int posY;
        if (yAxisWdgt != NULL)
        {
            posX = mouseEvent->x() - yAxisWdgt->width();
        }
        else
        {
            posX = mouseEvent->x();
        }
        posY = mouseEvent->y();

        // 处理质控图被点击
        HandleQcGraphicClicked(posX, posY);
    }

    // 点信息显示框尺寸变化
    if ((obj == ui->TimeNameLab || obj == ui->TimeValLab) && event->type() == QEvent::Resize)
    {
        int iWidthOffset = ui->TimeNameLab->width() + ui->TimeValLab->width() - 96;
        if (iWidthOffset > 0)
        {
            ui->PtInfoViewFrame->setGeometry(375 - (iWidthOffset / 2), ui->PtInfoViewFrame->y(), 127 + iWidthOffset, 53);
            ui->PointInfoFrame->move(-300 + (iWidthOffset / 2), 0);
        }
    }

    return QWidget::eventFilter(obj, event);
}

///
/// @brief
///     曲线显示状态改变
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月15日，新建函数
///
void LeveyJennPlot::OnCurveVisibleChanged()
{
    // 判断是否显示子点
    bool bSubPtVisible = ui->ShowSubPointCB->isChecked();

    // 判断是否显示不计算的点
    bool bNoCalcPtVisible = ui->ShowNoCalcQcCB->isChecked();

    // 更新对应曲线（子点）显示
    SetCurveVisible(CURVE_ID_QC_1_SUB_PT, bSubPtVisible);
    SetCurveVisible(CURVE_ID_QC_2_SUB_PT, bSubPtVisible);
    SetCurveVisible(CURVE_ID_QC_3_SUB_PT, bSubPtVisible);

    // 更新对应曲线（不计算结果点）显示
    SetCurveVisible(CURVE_ID_QC_NO_CALC, bNoCalcPtVisible);

    // 获取所有曲线最大X上界
    int iMaxXUpper = 0;
    for (int i = 0; i < CURVE_ID_SIZE; i++)
    {
        if (!bNoCalcPtVisible && (i == CURVE_ID_QC_NO_CALC))
        {
            continue;
        }

        if (m_iArrayUpperX[i] > iMaxXUpper)
        {
            iMaxXUpper = m_iArrayUpperX[i];
        }
    }

    // 设置横坐标范围
    if (iMaxXUpper > 20)
    {
        ui->QcGraphic->setAxisScale(QwtPlot::xBottom, 0.5, iMaxXUpper + 4.5, 1.0);
        //ui->QcGraphic->setGeometry(-44, -4, 854 + ((iMaxXUpper - 16) * 40), 314);
        //ui->ScrollContent->setFixedSize(806 + ((iMaxXUpper - 16) * 40), 310);
    }
    else
    {
        ui->QcGraphic->setAxisScale(QwtPlot::xBottom, 0.5, 20.5, 1.0);
        //ui->QcGraphic->setGeometry(-44, -4, 854, 314);
        //ui->ScrollContent->setFixedSize(806, 310);
    }
}

///
/// @brief
///     质控图被点击，尝试选中某一点
///
/// @param[in]  dX  点击位置X坐标
/// @param[in]  dY  点击位置Y坐标
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月15日，新建函数
///
void LeveyJennPlot::HandleQcGraphicClicked(int iX, int iY)
{
    // 误差调整
    iY -= 5;

    // 重置选中点
    m_stuSelPointInfo.Reset();
    QVector<QPointF> vecPoints;
    m_pQcSelLineLeft->setSamples(vecPoints);
    m_pQcSelLineRight->setSamples(vecPoints);
    m_pQcSelLineTop->setSamples(vecPoints);
    m_pQcSelLineBottom->setSamples(vecPoints);

    // 处理曲线被点击
    double dMinDis = (double)SELECT_SENSITIVITY;
    auto HandleCurveClicked = [this, &dMinDis](CURVE_ID enCurveID, int iX, int iY)
    {
        auto it = m_mapCurvePoints.find(enCurveID);
        if (it == m_mapCurvePoints.end())
        {
            return;
        }

        for (int i = 0; i < it.value().size(); i++)
        {
            const QPointF& pt = it.value()[i];

            int posX = ui->QcGraphic->transform(QwtPlot::xBottom, pt.x());
            int posY = ui->QcGraphic->transform(QwtPlot::yLeft, pt.y());

            double dDis = qSqrt(((iX - posX) * (iX - posX)) + ((iY - posY) * (iY - posY)));
            if (dDis >= dMinDis)
            {
                continue;
            }

            // 选中该点
            SelPointByIndex(enCurveID, i);

            // 更新最小距离
            dMinDis = dDis;
        }

        return;
    };

    // 显示子点
    if (ui->ShowSubPointCB->isChecked() && ui->ShowSubPointCB->isVisible())
    {
        HandleCurveClicked(CURVE_ID_QC_1_SUB_PT, iX, iY);
        HandleCurveClicked(CURVE_ID_QC_2_SUB_PT, iX, iY);
        HandleCurveClicked(CURVE_ID_QC_3_SUB_PT, iX, iY);
    }

    if (!ui->ShowSubPointCB->isVisible())
    {
        HandleCurveClicked(CURVE_ID_QC_1, iX, iY);
        HandleCurveClicked(CURVE_ID_QC_2, iX, iY);
        HandleCurveClicked(CURVE_ID_QC_3, iX, iY);
    }

    // 显示不计算点
    if (ui->ShowNoCalcQcCB->isChecked())
    {
        HandleCurveClicked(CURVE_ID_QC_NO_CALC, iX, iY);
    }

    // 显示隐藏点信息显示框
    ui->PtInfoViewFrame->setVisible(m_stuSelPointInfo.enCurveId != CURVE_ID_SIZE);

    // 通知选中点改变
    emit SigSelPointChanged(m_stuSelPointInfo.enCurveId, m_stuSelPointInfo.pointID);
}
