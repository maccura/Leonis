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
/// @author   4170/TangChuXian
/// @date     2021年9月17日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月17日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "mcqcljgraphic.h"
#include <qwt_legend.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_grid.h>
#include <qwt_symbol.h>
#include <qwt_scale_div.h>
#include <qwt_scale_draw.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_marker.h>

#include <cfloat>
#include <algorithm>
#include <QMouseEvent>
#include <QMetaType>
#include <QPoint>

#define _MATH_DEFINES_DEFINED
#include "ui_mcqcljgraphic.h"
#include "shared/mccustomscaledraw.h"
#include "shared/mccustomqwtmarker.h"
#include "shared/datetimefmttool.h"
#include "shared/uicommon.h"
#include "shared/uidef.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "manager/UserInfoManager.h"
#include "thrift/DcsControlProxy.h"
#include "src/common/Mlog/mlog.h"

// 选中灵敏度
#define  SELECT_SENSITIVITY          (20)

// 是否显示不计算点
#define  IS_SHOW_NO_CALC_POINT       (ui->ShowNoCalcQcCB->isChecked() && (ui->ShowSubPointCB->isChecked() || !ui->ShowSubPointCB->isVisible()))

// 自画坐标轴LJ图X轴
class McQcLJGraphicX : public McCustomIntegerScaleDraw
{
public:
    explicit McQcLJGraphicX() { };

    // 重写函数:设置坐标轴的刻度值
    virtual QwtText label(double v) const
    {
        // 初始化数值显示
        QString StrOffset("");

        // 如果是整数，则显示整数，否则不显示
        if (qAbs(v - int(v)) <= DBL_EPSILON)
        {
            if (m_startDate.isValid())
            {
                QDate curDate = m_startDate.addDays(int(v) - 1);
				StrOffset = DateTimeFmtTool::GetInstance()->GetMonthDayFormatStr(curDate);
               
            }
            else
            {
                StrOffset.setNum(int(v));
            }
        }

        // 构造返回值
        QwtText qwtText(StrOffset);

        // 显示刻度值
        return qwtText;
    }

    // 设置起始日期
    void SetStartDate(const QDate& startDate)
    {
        m_startDate = startDate;
        this->invalidateCache();
    }

    // 清空起始日期
    void ClearStartDate()
    {
        m_startDate = QDate();
        this->invalidateCache();
    }

    // 重写函数:重绘坐标轴的刻度值
    virtual void drawLabel(QPainter* painter, double val) const
    {
        QwtScaleDraw::drawLabel(painter, val);
        if (m_color.isValid())
        {
            QwtScaleDraw::drawLabel(painter, m_iSelVal);
        }
    }

    // 设置特定文本颜色
    void SetSelLabelColor(int val, const QColor& labelColor)
    {
        m_iSelVal = val;
        m_color = labelColor;
    }

protected:
    QColor m_color;
    double m_iSelVal;

private:
    QDate   m_startDate;        // 起始日期
};

// 自定义多符号曲线类(抄录所需静态函数)
static void qwtUpdateLegendIconSize(QwtPlotCurve *curve)
{
    if (curve->symbol() &&
        curve->testLegendAttribute(QwtPlotCurve::LegendShowSymbol))
    {
        QSize sz = curve->symbol()->boundingRect().size();
        sz += QSize(2, 2); // margin

        if (curve->testLegendAttribute(QwtPlotCurve::LegendShowLine))
        {
            // Avoid, that the line is completely covered by the symbol

            int w = qCeil(1.5 * sz.width());
            if (w % 2)
                w++;

            sz.setWidth(qMax(8, w));
        }

        curve->setLegendIconSize(sz);
    }
}

// 自定义多符号曲线类(抄录所需静态函数)
static int qwtVerifyRange(int size, int &i1, int &i2)
{
    if (size < 1)
        return 0;

    i1 = qBound(0, i1, size - 1);
    i2 = qBound(0, i2, size - 1);

    if (i1 > i2)
        qSwap(i1, i2);

    return (i2 - i1 + 1);
}

// 自定义多符号曲线类
class MultiSymbolCurve : public QwtPlotCurve
{
public:
    ///
    /// @brief
    ///     覆盖基类设置符号函数
    ///
    /// @param[in]  pSymbol  符号
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年7月27日，新建函数
    ///
    void setSymbol(QwtSymbol *pSymbol)
    {
        if ((pSymbol != Q_NULLPTR) && !m_vSymbols.isEmpty() && (pSymbol->style() != QwtSymbol::NoSymbol))
        {
            m_vSymbols.clear();
        }
        QwtPlotCurve::setSymbol(pSymbol);
    }

    ///
    /// @brief
    ///     设置曲线符号数组
    ///
    /// @param[in]  symbols  符号数组
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年7月27日，新建函数
    ///
    void setSymbols(const QVector<QwtSymbol*>& symbols)
    {
        for (QwtSymbol* pSymbol : m_vSymbols)
        {
            delete pSymbol;
        }
        m_vSymbols.clear();

        m_vSymbols = std::move(symbols);
        if (!m_vSymbols.isEmpty() && (symbol()->style() != QwtSymbol::NoSymbol))
        {
            setSymbol(new QwtSymbol(QwtSymbol::NoSymbol));
        }
        qwtUpdateLegendIconSize(this);
        legendChanged();
        itemChanged();
    }

    ///
    /// @brief
    ///     获取符号数组
    ///
    /// @return 符号数组
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年7月27日，新建函数
    ///
    const QVector<QwtSymbol*>& symbols()
    {
        return m_vSymbols;
    }

    ///
    /// @brief
    ///     重写虚函数
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年7月27日，新建函数
    ///
    virtual void drawSeries(QPainter* painter,
        const QwtScaleMap& xMap, const QwtScaleMap& yMap,
        const QRectF& canvasRect, int from, int to) const override
    {
        // 调用基类函数, 如果符号数组为空则返回
        QwtPlotCurve::drawSeries(painter, xMap, yMap, canvasRect, from, to);
        if (m_vSymbols.isEmpty())
        {
            QwtPlotCurve::drawSeries(painter, xMap, yMap, canvasRect, from, to);
            return;
        }

        // 如果符号数组不为空，逐个处理符号
        const size_t numSamples = dataSize();
        if (!painter || numSamples <= 0 || from < 0)
        {
            return;
        }

        if (to < 0)
        {
            to = numSamples - 1;
        }

        // 逐个处理符号
        if (qwtVerifyRange(numSamples, from, to) > 0)
        {
            for (int i = from; i <= to; i++)
            {
                // 符号数组已到末尾，退出循环
                if (i >= m_vSymbols.size())
                {
                    break;
                }

                // 逐个画符号
                if (m_vSymbols[i]->style() != QwtSymbol::NoSymbol)
                {
                    painter->save();
                    drawSymbols(painter, *m_vSymbols[i], xMap, yMap, canvasRect, i, i);
                    painter->restore();
                }
            }
        }
    }

private:
    // 记录符号数组
    QVector<QwtSymbol*> m_vSymbols;
};

McQcLjGraphic::McQcLjGraphic(QWidget *parent, bool bPrintMode)
    : QWidget(parent),
      m_bInit(false),
      m_bPrintMode(bPrintMode),
      m_iNoCalcPointSize(10)
{
    // 初始化UI对象
    ui = new Ui::McQcLjGraphic();
    ui->setupUi(this);

    // 界面显示前初始化
    InitBeforeShow();

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);
}

McQcLjGraphic::~McQcLjGraphic()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
    delete ui;
}

///
/// @brief
///     设置曲线点集
///
/// @param[in]  enCurveId      曲线ID
/// @param[in]  vecPoint       曲线上的点集
/// @param[in]  vecPointID     曲线上的点集ID
/// @param[in]  vecPointSymbol 曲线上的点集符号
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月17日，新建函数
/// @li 4170/TangChuXian，2022年5月23日，增加参数vecPointID
/// @li 4170/TangChuXian，2022年7月27日，增加参数vecPointSymbol
///
void McQcLjGraphic::SetPoints(CURVE_ID enCurveId, const QVector<QPointF>& vecPoint, 
                              const QVector<long long>& vecPointID, 
                              const QVector<QwtSymbol::Style>& vecPointSymbol)
{
    // 构造操作曲线
    MultiSymbolCurve* pOpCurve = Q_NULLPTR;

    switch (enCurveId)
    {
    case McQcLjGraphic::CURVE_ID_QC_1:
        pOpCurve = m_pQc1Curve;
        break;
    case McQcLjGraphic::CURVE_ID_QC_2:
        pOpCurve = m_pQc2Curve;
        break;
    case McQcLjGraphic::CURVE_ID_QC_3:
        pOpCurve = m_pQc3Curve;
        break;
    case McQcLjGraphic::CURVE_ID_QC_4:
        pOpCurve = m_pQc4Curve;
        break;
    case McQcLjGraphic::CURVE_ID_QC_1_SUB_PT:
        pOpCurve = m_pQc1SubPoints;
        break;
    case McQcLjGraphic::CURVE_ID_QC_2_SUB_PT:
        pOpCurve = m_pQc2SubPoints;
        break;
    case McQcLjGraphic::CURVE_ID_QC_3_SUB_PT:
        pOpCurve = m_pQc3SubPoints;
        break;
    case McQcLjGraphic::CURVE_ID_QC_4_SUB_PT:
        pOpCurve = m_pQc4SubPoints;
        break;
    case McQcLjGraphic::CURVE_ID_QC_NO_CALC:
        pOpCurve = m_pQcNoCalcPoints;
        break;
    default:
        break;
    }

    // 无操作曲线
    if (pOpCurve == Q_NULLPTR)
    {
        return;
    }

    // 更新点集
    pOpCurve->setSamples(vecPoint);

    // 更新符号
    if (!vecPointSymbol.isEmpty())
    {
        pOpCurve->setSymbols(ConstructSymbols(enCurveId, vecPointSymbol));
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
    bool bNoCalcPtVisible = IS_SHOW_NO_CALC_POINT;

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
        int iMaxStep = iMaxXUpper - 20;
        ui->GraphicScrollBar->setRange(0, iMaxStep);
        SetGraphicScrollBarInWork(true);
        //ui->QcGraphic->setAxisScale(QwtPlot::xBottom, 0.5, iMaxXUpper + 4.5, 1.0);
        //ui->QcGraphic->setGeometry(-44, -4, 1203 + ((iMaxXUpper - 16) * 40), 450);
        //ui->ScrollContent->setFixedSize(1155 + ((iMaxXUpper - 16) * 40), 446);
    }
    else
    {
        ui->GraphicScrollBar->setRange(0, 0);
        SetGraphicScrollBarInWork(false);
        ui->QcGraphic->setAxisScale(QwtPlot::xBottom, 0.5, 20.5, 1.0);
        //ui->QcGraphic->setGeometry(-44, -4, 1203, 450);
        //ui->ScrollContent->setFixedSize(1155, 446);
    }
}

///
/// @brief
///     重置曲线图
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月18日，新建函数
///
void McQcLjGraphic::Reset()
{
    QVector<QPointF> vEmptyPt;
    m_pQc1Curve->setSamples(vEmptyPt);
    m_pQc2Curve->setSamples(vEmptyPt);
    m_pQc3Curve->setSamples(vEmptyPt);
    m_pQc4Curve->setSamples(vEmptyPt);
    m_pQc1SubPoints->setSamples(vEmptyPt);
    m_pQc2SubPoints->setSamples(vEmptyPt);
    m_pQc3SubPoints->setSamples(vEmptyPt);
    m_pQc4SubPoints->setSamples(vEmptyPt);
    m_pQcNoCalcPoints->setSamples(vEmptyPt);

    // 重置选中点
    m_stuSelPointInfo.Reset();
    m_mapCurvePoints.clear();
    m_mapCurvePointsID.clear();
    m_pQcSelRing->setSamples(vEmptyPt);

    // 显示隐藏点信息显示框
    UpdateSelMarker();

    // 通知选中点改变
    //emit SigSelPointChanged(m_stuSelPointInfo.enCurveId, m_stuSelPointInfo.pointID);
}

///
/// @brief
///     重置曲线图但保留选中效果
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月18日，新建函数
///
void McQcLjGraphic::ResetButSel()
{
    QVector<QPointF> vEmptyPt;
    m_pQc1Curve->setSamples(vEmptyPt);
    m_pQc2Curve->setSamples(vEmptyPt);
    m_pQc3Curve->setSamples(vEmptyPt);
    m_pQc4Curve->setSamples(vEmptyPt);
    m_pQc1SubPoints->setSamples(vEmptyPt);
    m_pQc2SubPoints->setSamples(vEmptyPt);
    m_pQc3SubPoints->setSamples(vEmptyPt);
    m_pQc4SubPoints->setSamples(vEmptyPt);
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
/// @li 4170/TangChuXian，2021年9月18日，新建函数
///
void McQcLjGraphic::SetCurveVisible(CURVE_ID enCurveId, bool bVisible)
{
    QwtPlotCurve* pOpCurve = Q_NULLPTR;
    switch (enCurveId)
    {
    case McQcLjGraphic::CURVE_ID_QC_1:
        pOpCurve = m_pQc1Curve;
        break;
    case McQcLjGraphic::CURVE_ID_QC_2:
        pOpCurve = m_pQc2Curve;
        break;
    case McQcLjGraphic::CURVE_ID_QC_3:
        pOpCurve = m_pQc3Curve;
        break;
    case McQcLjGraphic::CURVE_ID_QC_4:
        pOpCurve = m_pQc4Curve;
        break;
    case McQcLjGraphic::CURVE_ID_QC_1_SUB_PT:
        pOpCurve = m_pQc1SubPoints;
        break;
    case McQcLjGraphic::CURVE_ID_QC_2_SUB_PT:
        pOpCurve = m_pQc2SubPoints;
        break;
    case McQcLjGraphic::CURVE_ID_QC_3_SUB_PT:
        pOpCurve = m_pQc3SubPoints;
        break;
    case McQcLjGraphic::CURVE_ID_QC_4_SUB_PT:
        pOpCurve = m_pQc4SubPoints;
        break;
    case McQcLjGraphic::CURVE_ID_QC_NO_CALC:
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
/// @li 4170/TangChuXian，2022年5月23日，新建函数
///
void McQcLjGraphic::SelPointByIndex(CURVE_ID enCurveID, int iIndex)
{
    // 重置选中点信息
    m_stuSelPointInfo.Reset();

    auto it = m_mapCurvePointsID.find(enCurveID);
    if (it == m_mapCurvePointsID.end())
    {
        QVector<QPointF> vecPoints;
        m_pQcSelRing->setSamples(vecPoints);
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
///     设置选中提示标记是否选中
///
/// @param[in]  bVisible  是否显示
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月6日，新建函数
///
void McQcLjGraphic::SetSelMarkerTipVisible(bool bVisible)
{
    if (bVisible)
    {
        m_pQcSelTipMarker->attach(ui->QcGraphic);
    }
    else
    {
        m_pQcSelTipMarker->detach();
    }
}

///
/// @brief
///     更新质控图选中点提示
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月6日，新建函数
///
void McQcLjGraphic::UpdateSelMarker()
{
    // 判断是否有选中点
    McQcLJGraphicX* pXScale = dynamic_cast<McQcLJGraphicX*>(ui->QcGraphic->axisScaleDraw(QwtPlot::xBottom));
    if (pXScale == Q_NULLPTR)
    {
        return;
    }

    // 判断是否有选中点
    if (m_stuSelPointInfo.enCurveId == CURVE_ID_SIZE)
    {
        // 没有选中点则隐藏
        SetSelMarkerTipVisible(false);
        m_pVLineMarker->setXValue(-1);
        pXScale->SetSelLabelColor(0, QColor());
    }
    else
    {
        // 有选中点，反之显示
        SetSelMarkerTipVisible(true);

        // 选中点
        QPointF   selPt;

        // 找到对应曲线点集
        auto it = m_mapCurvePoints.find(m_stuSelPointInfo.enCurveId);
        if (it == m_mapCurvePoints.end())
        {
            return;
        }

        // 找到选中点坐标
        for (int i = 0; i < it.value().size(); i++)
        {
            // 索引不匹配则跳过
            if (i != m_stuSelPointInfo.iIndex)
            {
                continue;
            }

            // 更新选中点信息
            selPt = it.value()[i];
        }

        // 设置坐标轴选中效果
        pXScale->SetSelLabelColor(selPt.x(), QColor());
        m_pVLineMarker->setXValue(selPt.x());

        // 构造qwtText
        m_pQcSelTipMarker->SetQcTipContent(m_strSelPtTextList);

        // 根据坐标判断标签位置
        int iAlign = 0;
        if (selPt.y() > 0)
        {
            iAlign |= Qt::AlignBottom;
        }
        else
        {
            iAlign |= Qt::AlignTop;
        }

        if ((selPt.x() - ui->QcGraphic->axisScaleDiv(QwtPlot::xBottom).lowerBound()) < 3)
        {
            iAlign |= Qt::AlignRight;
        }
        else if ((ui->QcGraphic->axisScaleDiv(QwtPlot::xBottom).upperBound() - selPt.x()) < 3)
        {
            iAlign |= Qt::AlignLeft;
        }
        else
        {
            iAlign |= Qt::AlignHCenter;
        }

        // 构造marker
        m_pQcSelTipMarker->setValue(selPt);
        m_pQcSelTipMarker->setLabelAlignment(Qt::Alignment(iAlign));
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
/// @li 4170/TangChuXian，2022年5月23日，新建函数
///
void McQcLjGraphic::SelPointByID(CURVE_ID enCurveID, long long iPointID)
{
    // 重置选中点信息
    m_stuSelPointInfo.Reset();

    auto it = m_mapCurvePointsID.find(enCurveID);
    if (it == m_mapCurvePointsID.end())
    {
        QVector<QPointF> vecPoints;
        m_pQcSelRing->setSamples(vecPoints);

        // 通知选中点改变
        //emit SigSelPointChanged(m_stuSelPointInfo.enCurveId, m_stuSelPointInfo.pointID);

        // 显示隐藏点信息显示框
        SetSelMarkerTipVisible(false);
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
    UpdateSelMarker();

    // 通知选中点改变
    //emit SigSelPointChanged(m_stuSelPointInfo.enCurveId, m_stuSelPointInfo.pointID);
}

///
/// @brief
///     获取选中点
///
/// @par History:
/// @li 4170/TangChuXian，2022年5月23日，新建函数
///
const McQcLjGraphic::SelPointInfo& McQcLjGraphic::GetSelectPoint()
{
    return m_stuSelPointInfo;
}

///
/// @brief
///     设置打印模式
///
/// @param[in]  bPrintMode    打印模式
/// @param[in]  enPrintCurve  要打印的曲线（其他曲线打印时隐藏）
///
/// @par History:
/// @li 4170/TangChuXian，2022年7月29日，新建函数
///
void McQcLjGraphic::SetPrintMode(bool bPrintMode, PRINT_CURVE enPrintCurve)
{
    // 更新打印模式
    m_bPrintMode = bPrintMode;

    // 所有曲线全部detach
    m_pQc1Curve->detach();
    m_pQc2Curve->detach();
    m_pQc3Curve->detach();
    m_pQc4Curve->detach();
    m_pQc1SubPoints->detach();
    m_pQc2SubPoints->detach();
    m_pQc3SubPoints->detach();
    m_pQc4SubPoints->detach();
    m_pQcNoCalcPoints->detach();
    m_pQcSelRing->detach();
    m_pQcSelTipMarker->detach();

    // 更新X上界限
    auto UpdateAxisXUpper = [this, enPrintCurve]()
    {
        // 获取所有曲线最大X上界
        int iMaxXUpper = 0;
        for (int i = 0; i < CURVE_ID_SIZE; i++)
        {
            if (enPrintCurve == PRINT_CURVE_QC_1 && i != CURVE_ID_QC_1 && i != CURVE_ID_QC_1_SUB_PT)
            {
                continue;
            }

            if (enPrintCurve == PRINT_CURVE_QC_2 && i != CURVE_ID_QC_2 && i != CURVE_ID_QC_2_SUB_PT)
            {
                continue;
            }

            if (enPrintCurve == PRINT_CURVE_QC_3 && i != CURVE_ID_QC_3 && i != CURVE_ID_QC_3_SUB_PT)
            {
                continue;
            }

            if (enPrintCurve == PRINT_CURVE_QC_4 && i != CURVE_ID_QC_4 && i != CURVE_ID_QC_4_SUB_PT)
            {
                continue;
            }

            if (enPrintCurve == PRINT_CURVE_NONE && !IS_SHOW_NO_CALC_POINT && i == CURVE_ID_QC_NO_CALC)
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
            int iMaxStep = iMaxXUpper - 20;
            ui->GraphicScrollBar->setRange(0, iMaxStep);
            SetGraphicScrollBarInWork(true);
            //ui->QcGraphic->setAxisScale(QwtPlot::xBottom, 0.5, iMaxXUpper + 4.5, 1.0);
            //ui->QcGraphic->setGeometry(-44, -4, 1203 + ((iMaxXUpper - 16) * 40), 450);
            //ui->ScrollContent->setFixedSize(1155 + ((iMaxXUpper - 16) * 40), 446);
        }
        else
        {
            ui->GraphicScrollBar->setRange(0, 0);
            SetGraphicScrollBarInWork(false);
            ui->QcGraphic->setAxisScale(QwtPlot::xBottom, 0.5, 20.5, 1.0);
            //ui->QcGraphic->setGeometry(-44, -4, 1203, 450);
            //ui->ScrollContent->setFixedSize(1155, 446);
        }
    };

    // 打印选择效果不显示
    if (m_bPrintMode)
    {
        SetSelMarkerTipVisible(false);

        // 判断是否显示不计算的点
        bool bNoCalcPtVisible = IS_SHOW_NO_CALC_POINT;

        // 隐藏滚动条
        ui->GraphicScrollBar->setVisible(false);

        // 要打印的曲线显示
        if (enPrintCurve == PRINT_CURVE_QC_1)
        {
            m_pQc1Curve->attach(ui->QcGraphic);
            m_pQc1SubPoints->attach(ui->QcGraphic);
        }
        else if (enPrintCurve == PRINT_CURVE_QC_2)
        {
            m_pQc2Curve->attach(ui->QcGraphic);
            m_pQc2SubPoints->attach(ui->QcGraphic);
        }
        else if (enPrintCurve == PRINT_CURVE_QC_3)
        {
            m_pQc3Curve->attach(ui->QcGraphic);
            m_pQc3SubPoints->attach(ui->QcGraphic);
        }
        else if (enPrintCurve == PRINT_CURVE_QC_4)
        {
            m_pQc4Curve->attach(ui->QcGraphic);
            m_pQc4SubPoints->attach(ui->QcGraphic);
        }

        // 如果要显示不计算点
        if (bNoCalcPtVisible)
        {
            m_pQcNoCalcPoints->attach(ui->QcGraphic);
        }
    }
    else
    {
        // 挂载到图表上
        m_pQc1Curve->attach(ui->QcGraphic);
        m_pQc2Curve->attach(ui->QcGraphic);
        m_pQc3Curve->attach(ui->QcGraphic);
        m_pQc4Curve->attach(ui->QcGraphic);
        m_pQc1SubPoints->attach(ui->QcGraphic);
        m_pQc2SubPoints->attach(ui->QcGraphic);
        m_pQc3SubPoints->attach(ui->QcGraphic);
        m_pQc4SubPoints->attach(ui->QcGraphic);
        m_pQcNoCalcPoints->attach(ui->QcGraphic);
        m_pQcSelRing->attach(ui->QcGraphic);
        UpdateSelMarker();

        // 显示滚动条
        ui->GraphicScrollBar->setVisible(true);
    }

    // 更新X轴上界限
    //UpdateAxisXUpper();
}

///
/// @brief
///     设置X轴起始日期
///
/// @param[in]  startDate  起始日期
///
/// @par History:
/// @li 4170/TangChuXian，2023年5月6日，新建函数
///
void McQcLjGraphic::SetXAxisStartDate(const QDate& startDate)
{
    // 获取横坐标轴
    QwtScaleDraw* pScaleDrawX = ui->QcGraphic->axisScaleDraw(QwtPlot::xBottom);
    McQcLJGraphicX* pMcQcLJGraphicX = dynamic_cast<McQcLJGraphicX*>(pScaleDrawX);
    if (pMcQcLJGraphicX == Q_NULLPTR)
    {
        return;
    }

    // 设置起始日期
    pMcQcLJGraphicX->SetStartDate(startDate);
    ui->QcGraphic->repaint();
}

///
/// @brief
///     清空起始日期
///
/// @par History:
/// @li 4170/TangChuXian，2023年5月6日，新建函数
///
void McQcLjGraphic::ClearXAxisStartDate()
{
    // 获取横坐标轴
    QwtScaleDraw* pScaleDrawX = ui->QcGraphic->axisScaleDraw(QwtPlot::xBottom);
    McQcLJGraphicX* pMcQcLJGraphicX = dynamic_cast<McQcLJGraphicX*>(pScaleDrawX);
    if (pMcQcLJGraphicX == Q_NULLPTR)
    {
        return;
    }

    // 设置起始日期
    pMcQcLJGraphicX->ClearStartDate();
    ui->QcGraphic->repaint();
}

///
/// @brief
///     设置选中点提示文本
///
/// @param[in]  strTextList  选中点文本列表
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月6日，新建函数
///
void McQcLjGraphic::SetSelPointTipText(const QStringList& strTextList)
{
    // 成员变量赋值
    m_strSelPtTextList = strTextList;
    UpdateSelMarker();
}

///
/// @brief
///     设置滚动条工作状态
///
/// @param[in]  bInWork  是否在工作
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月15日，新建函数
///
void McQcLjGraphic::SetGraphicScrollBarInWork(bool bInWork)
{
    // 更新属性
    ui->GraphicScrollBar->setProperty("InWork", bInWork);

    // 更新状态
    ui->GraphicScrollBar->style()->unpolish(ui->GraphicScrollBar);
    ui->GraphicScrollBar->style()->polish(ui->GraphicScrollBar);
    ui->GraphicScrollBar->update();
}

///
/// @brief
///     设置不计算点大小
///
/// @param[in]  iPointSize  点大小
///
/// @par History:
/// @li 4170/TangChuXian，2023年10月26日，新建函数
///
void McQcLjGraphic::SetNoCalcPointSize(int iPointSize)
{
    QwtSymbol* pQcNoCalcSymbol = new QwtSymbol(QwtSymbol::Ellipse);
    pQcNoCalcSymbol->setBrush(QBrush(QColor(0x80, 0x80, 0x80)));
    pQcNoCalcSymbol->setSize(iPointSize);
    m_iNoCalcPointSize = iPointSize;
    m_pQcNoCalcPoints->setSymbol(pQcNoCalcSymbol);
}

///
/// @brief
///     是否显示不计算点
///
/// @return true表示显示
///
/// @par History:
/// @li 4170/TangChuXian，2024年4月19日，新建函数
///
bool McQcLjGraphic::IsShowNoCalcPoint()
{
    return IS_SHOW_NO_CALC_POINT;
}

///
/// @brief
///     界面显示前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2021年5月13日，新建函数
///
void McQcLjGraphic::InitBeforeShow()
{
    // 初始化子点与不显示计算点
    ui->ShowSubPointCB->setChecked(DcsControlProxy::GetInstance()->GetQcLjSubPointFlag());
    ui->ShowNoCalcQcCB->setChecked(DcsControlProxy::GetInstance()->GetQcLjNoCalculateFlag());

    // 初始化滚动条
    ui->GraphicScrollBar->setRange(0, 0);
    SetGraphicScrollBarInWork(false);

    ui->GraphicScrollBar->setSingleStep(1);
    ui->GraphicScrollBar->setPageStep(20);

    // 初始化图例
    InitLegend();

    // 初始化信号槽连接
    InitConnect();

    //多语言处理
    QString tmpStr;
    tmpStr = ui->ShowNoCalcQcCB->fontMetrics().elidedText(ui->ShowNoCalcQcCB->text(), Qt::ElideRight, 85);
    ui->ShowNoCalcQcCB->setToolTip(ui->ShowNoCalcQcCB->text());
    ui->ShowNoCalcQcCB->setText(tmpStr);

    tmpStr = ui->ShowSubPointCB->fontMetrics().elidedText(ui->ShowSubPointCB->text(), Qt::ElideRight, 85);
    ui->ShowSubPointCB->setToolTip(ui->ShowSubPointCB->text());
    ui->ShowSubPointCB->setText(tmpStr);
}

///
/// @brief
///     构造点符号数组
///
/// @param[in]  enCurveId       曲线ID
/// @param[in]  vecPointSymbol  点符号形状数组
///
/// @return 点符号数组
///
/// @par History:
/// @li 4170/TangChuXian，2022年7月27日，新建函数
///
QVector<QwtSymbol*> McQcLjGraphic::ConstructSymbols(CURVE_ID enCurveId, const QVector<QwtSymbol::Style>& vecPointSymbol)
{
    // 构造原型
    auto ConstructSymbolsByCurve = [this](CURVE_ID enCurveId, QwtSymbol& symbolModel)
    {
        symbolModel.setPen(QPen(Qt::NoPen));

        switch (enCurveId)
        {
        case McQcLjGraphic::CURVE_ID_QC_1:
            symbolModel.setBrush(QBrush(QColor(0x1b, 0x7d, 0xf5)));
            symbolModel.setSize(10);
            break;
        case McQcLjGraphic::CURVE_ID_QC_2:
            symbolModel.setBrush(QBrush(QColor(0x1f, 0xb5, 0x58)));
            symbolModel.setSize(10);
            break;
        case McQcLjGraphic::CURVE_ID_QC_3:
            symbolModel.setBrush(QBrush(QColor(0x9f, 0x65, 0xe6)));
            symbolModel.setSize(10);
            break;
        case McQcLjGraphic::CURVE_ID_QC_4:
            symbolModel.setBrush(QBrush(QColor(0xff, 0x14, 0x93)));
            symbolModel.setSize(10);
            break;
        case McQcLjGraphic::CURVE_ID_QC_1_SUB_PT:
            symbolModel.setBrush(QBrush(QColor(0x1b, 0x7d, 0xf5)));
            symbolModel.setSize(8);
            break;
        case McQcLjGraphic::CURVE_ID_QC_2_SUB_PT:
            symbolModel.setBrush(QBrush(QColor(0x1f, 0xb5, 0x58)));
            symbolModel.setSize(8);
            break;
        case McQcLjGraphic::CURVE_ID_QC_3_SUB_PT:
            symbolModel.setBrush(QBrush(QColor(0x9f, 0x65, 0xe6)));
            symbolModel.setSize(8);
            break;
        case McQcLjGraphic::CURVE_ID_QC_4_SUB_PT:
            symbolModel.setBrush(QBrush(QColor(0xff, 0x14, 0x93)));
            symbolModel.setSize(8);
            break;
        case McQcLjGraphic::CURVE_ID_QC_NO_CALC:
            symbolModel.setBrush(QBrush(QColor(0x80, 0x80, 0x80)));
            symbolModel.setSize(m_iNoCalcPointSize);
            break;
        default:
            break;
        }

        // 如果风格为Triangle
        if (symbolModel.style() == QwtSymbol::Triangle)
        {
            // 设置图标
            symbolModel.setStyle(QwtSymbol::Pixmap);
            symbolModel.setPixmap(QPixmap(":/Leonis/resource/image/icon-qc-warn.png"));
        }
        else if (symbolModel.style() == QwtSymbol::XCross)
        {
            // 设置图标
            symbolModel.setStyle(QwtSymbol::Pixmap);
            symbolModel.setPixmap(QPixmap(":/Leonis/resource/image/icon-qc-outctrl.png"));
        }
    };

    // 构造符号数组
    QVector<QwtSymbol*> vSymbol;
    for (QwtSymbol::Style enStyle : vecPointSymbol)
    {
        QwtSymbol* pSymbolModel = new QwtSymbol(enStyle);
        ConstructSymbolsByCurve(enCurveId, *pSymbolModel);
        vSymbol.push_back(pSymbolModel);
    }

    return vSymbol;
}

///
/// @brief
///     显示之后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2021年3月25日，新建函数
///
void McQcLjGraphic::InitAfterShow()
{
    // 初始化字符串资源
    InitStrResource();

    // 判断是否显示子点
    bool bSubPtVisible = ui->ShowSubPointCB->isChecked();

    // 判断是否显示不计算的点
    bool bNoCalcPtVisible = IS_SHOW_NO_CALC_POINT;

    // 更新对应曲线（子点）显示
    SetCurveVisible(CURVE_ID_QC_1_SUB_PT, bSubPtVisible);
    SetCurveVisible(CURVE_ID_QC_2_SUB_PT, bSubPtVisible);
    SetCurveVisible(CURVE_ID_QC_3_SUB_PT, bSubPtVisible);
    SetCurveVisible(CURVE_ID_QC_4_SUB_PT, bSubPtVisible);

    // 更新对应曲线（不计算结果点）显示
    SetCurveVisible(CURVE_ID_QC_NO_CALC, bNoCalcPtVisible);
}

///
/// @brief
///     初始化字符串资源
///
/// @par History:
/// @li 4170/TangChuXian，2022年1月17日，新建函数
///
void McQcLjGraphic::InitStrResource()
{
//     ui->Qc1TextLab->setText(LoadStrFromLanguage(CHAR_CODE::IDS_QC_GRAPHIC_1));
//     ui->Qc2TextLab->setText(LoadStrFromLanguage(CHAR_CODE::IDS_QC_GRAPHIC_2));
//     ui->Qc3TextLab->setText(LoadStrFromLanguage(CHAR_CODE::IDS_QC_GRAPHIC_3));
//     ui->Qc4TextLab->setText(LoadStrFromLanguage(CHAR_CODE::IDS_QC_GRAPHIC_4));
//     ui->NoCalcQcTextLab->setText(LoadStrFromLanguage(CHAR_CODE::IDS_QC_NO_CALC_POINT));
//     ui->ShowNoCalcQcCB->setText(LoadStrFromLanguage(CHAR_CODE::IDS_QC_SHOW_NO_CALC_POINT));
//     ui->ShowSubPointCB->setText(LoadStrFromLanguage(CHAR_CODE::IDS_QC_SHOW_SUB_POINT));
//     ui->ResultNameLab->setText(LoadStrFromLanguage(CHAR_CODE::IDS_RLT_NAME));
//     ui->TimeNameLab->setText(LoadStrFromLanguage(CHAR_CODE::IDS_TIME) + LoadStrFromLanguage(CHAR_CODE::IDS_SYMBOL_COLON));
}

///
/// @brief
///     初始化图例
///
/// @par History:
/// @li 4170/TangChuXian，2021年3月25日，新建函数
///
void McQcLjGraphic::InitLegend()
{
    // 注册元类型
    qRegisterMetaType<CURVE_ID>("McQcLjGraphic::CURVE_ID");

    // 初始化X轴上界限
    for (int i = 0; i < CURVE_ID_SIZE; i++)
    {
        m_iArrayUpperX[i] = 0;
    }

    // 安装事件过滤器
    ui->QcGraphic->installEventFilter(this);
    ui->ShowSubPointCB->installEventFilter(this);

    // plot自动更新
    ui->QcGraphic->setAutoReplot(true);

    // 设置背景色
    ui->QcGraphic->setCanvasBackground(QColor(0xf5, 0xf7, 0xfb));

    // 设置没有小间隔
    ui->QcGraphic->setAxisMaxMinor(QwtPlot::xBottom, 0);

    // 设置自定义坐标轴
    McCustomScaleDraw *pScaleDrawY = new McCustomScaleDraw();
    if (pScaleDrawY)
    {
        pScaleDrawY->enableComponent(QwtAbstractScaleDraw::Backbone, false);
        pScaleDrawY->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    }
    ui->QcGraphic->setAxisScaleDraw(QwtPlot::yLeft, pScaleDrawY);

    McQcLJGraphicX *pScaleDrawX = new McQcLJGraphicX();
    if (pScaleDrawX)
    {
        pScaleDrawX->enableComponent(QwtAbstractScaleDraw::Backbone, false);
        pScaleDrawX->enableComponent(QwtAbstractScaleDraw::Ticks, true);
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
//     ui->QcGraphicBk->enableAxis(QwtPlot::xTop);
//     ui->QcGraphicBk->enableAxis(QwtPlot::yRight);
//     ui->QcGraphicBk->axisScaleDraw(QwtPlot::xTop)->enableComponent(QwtAbstractScaleDraw::Backbone, false);
//     ui->QcGraphicBk->axisScaleDraw(QwtPlot::xTop)->enableComponent(QwtAbstractScaleDraw::Ticks, false);
//     ui->QcGraphicBk->axisScaleDraw(QwtPlot::xTop)->enableComponent(QwtAbstractScaleDraw::Labels, false);
//     ui->QcGraphicBk->axisScaleDraw(QwtPlot::yRight)->enableComponent(QwtAbstractScaleDraw::Backbone, false);
//     ui->QcGraphicBk->axisScaleDraw(QwtPlot::yRight)->enableComponent(QwtAbstractScaleDraw::Ticks, false);
//     ui->QcGraphicBk->axisScaleDraw(QwtPlot::yRight)->enableComponent(QwtAbstractScaleDraw::Labels, false);

    // 设置边框
    ui->QcGraphic->canvas()->setStyleSheet("border:1px solid #a3a3a3;background:#f5f7fb;");
    //ui->QcGraphicBk->canvas()->setStyleSheet("border:1px solid #a3a3a3;background:#f5f7fb;");

    // 使用鼠标左键平移
    //QwtPlotPanner* pPanner = new QwtPlotPanner(ui->QcGraphic->canvas());
    //pPanner->setAxisEnabled(QwtPlot::yLeft, false);  //y轴坐标不平移

    // 初始化网格分割线
    m_pGrid = new QwtPlotGrid();
    m_pGrid->setMinorPen(Qt::transparent);
    m_pGrid->setMajorPen(QColor(0xd4, 0xd4, 0xd4), 1.0, Qt::DashLine);
    m_pGrid->attach(ui->QcGraphic);
    m_pGrid->enableX(false);

    // 初始化SD标记选中线
    m_pP2SDLineMarker = new QwtPlotMarker();
    m_pP3SDLineMarker = new QwtPlotMarker();
    m_pN2SDLineMarker = new QwtPlotMarker();
    m_pN3SDLineMarker = new QwtPlotMarker();
    m_pVLineMarker = new QwtPlotMarker();

    // 设置为水平线
    m_pP2SDLineMarker->setLineStyle(QwtPlotMarker::HLine);
    m_pP3SDLineMarker->setLineStyle(QwtPlotMarker::HLine);
    m_pN2SDLineMarker->setLineStyle(QwtPlotMarker::HLine);
    m_pN3SDLineMarker->setLineStyle(QwtPlotMarker::HLine);
    m_pVLineMarker->setLineStyle(QwtPlotMarker::VLine);

    // 设置颜色
    m_pP2SDLineMarker->setLinePen(QColor(0xfa, 0x90, 0x16), 1.0, Qt::DashLine);
    m_pP3SDLineMarker->setLinePen(QColor(0xfa, 0x37, 0x41), 1.0, Qt::DashLine);
    m_pN2SDLineMarker->setLinePen(QColor(0xfa, 0x90, 0x16), 1.0, Qt::DashLine);
    m_pN3SDLineMarker->setLinePen(QColor(0xfa, 0x37, 0x41), 1.0, Qt::DashLine);
    m_pVLineMarker->setLinePen(QColor(0xec, 0x69, 0x41), 2.0, Qt::SolidLine);

    // 设置Z轴坐标
    m_pP2SDLineMarker->setZ(20);
    m_pP3SDLineMarker->setZ(20);
    m_pN2SDLineMarker->setZ(20);
    m_pN3SDLineMarker->setZ(20);
    m_pVLineMarker->setZ(19);

    // 设置Y轴坐标
    m_pP2SDLineMarker->setYValue(2);
    m_pP3SDLineMarker->setYValue(3);
    m_pN2SDLineMarker->setYValue(-2);
    m_pN3SDLineMarker->setYValue(-3);
    m_pVLineMarker->setXValue(-1);

    // 添加到曲线图上
    m_pP2SDLineMarker->attach(ui->QcGraphic);
    m_pP3SDLineMarker->attach(ui->QcGraphic);
    m_pN2SDLineMarker->attach(ui->QcGraphic);
    m_pN3SDLineMarker->attach(ui->QcGraphic);
    m_pVLineMarker->attach(ui->QcGraphic);

    // 初始化曲线成员对象
    // 质控曲线
    m_pQc1Curve = new MultiSymbolCurve();
    m_pQc2Curve = new MultiSymbolCurve();
    m_pQc3Curve = new MultiSymbolCurve();
    m_pQc4Curve = new MultiSymbolCurve();
    m_pQc1SubPoints = new MultiSymbolCurve();
    m_pQc2SubPoints = new MultiSymbolCurve();
    m_pQc3SubPoints = new MultiSymbolCurve();
    m_pQc4SubPoints = new MultiSymbolCurve();
    m_pQcNoCalcPoints = new MultiSymbolCurve();

    // 选中效果圆环
    m_pQcSelRing = new QwtPlotCurve();
    QwtSymbol* pQcSelSymbol = new QwtSymbol(QwtSymbol::Pixmap);
    pQcSelSymbol->setPixmap(QPixmap(":/Leonis/resource/image/img-qc-graphic-sel-ring.png"));
    m_pQcSelRing->setSymbol(pQcSelSymbol);

    // 选中标记
    m_pQcSelTipMarker = new QcGraphicSelTipMaker();

    // 设置曲线颜色
    m_pQc1Curve->setPen(QColor(0x1b, 0x7d, 0xf5), 1.0);
    m_pQc2Curve->setPen(QColor(0x1f, 0xb5, 0x58), 1.0);
    m_pQc3Curve->setPen(QColor(0x9f, 0x65, 0xe6), 1.0);
    m_pQc4Curve->setPen(QColor(0xff, 0x14, 0x93), 1.0);

    // 挂载到图表上
    m_pQc1Curve->attach(ui->QcGraphic);
    m_pQc2Curve->attach(ui->QcGraphic);
    m_pQc3Curve->attach(ui->QcGraphic);
    m_pQc4Curve->attach(ui->QcGraphic);
    m_pQc1SubPoints->attach(ui->QcGraphic);
    m_pQc2SubPoints->attach(ui->QcGraphic);
    m_pQc3SubPoints->attach(ui->QcGraphic);
    m_pQc4SubPoints->attach(ui->QcGraphic);
    m_pQcNoCalcPoints->attach(ui->QcGraphic);
    m_pQcSelRing->attach(ui->QcGraphic);

    // 设置曲线点符号
    QwtSymbol* pQc1Symbol = new QwtSymbol(QwtSymbol::Ellipse);
    QwtSymbol* pQc2Symbol = new QwtSymbol(QwtSymbol::Ellipse);
    QwtSymbol* pQc3Symbol = new QwtSymbol(QwtSymbol::Ellipse);
    QwtSymbol* pQc4Symbol = new QwtSymbol(QwtSymbol::Ellipse);
    QwtSymbol* pQc1SubSymbol = new QwtSymbol(QwtSymbol::Ellipse);
    QwtSymbol* pQc2SubSymbol = new QwtSymbol(QwtSymbol::Ellipse);
    QwtSymbol* pQc3SubSymbol = new QwtSymbol(QwtSymbol::Ellipse);
    QwtSymbol* pQc4SubSymbol = new QwtSymbol(QwtSymbol::Ellipse);
    QwtSymbol* pQcNoCalcSymbol = new QwtSymbol(QwtSymbol::Ellipse);
    pQc1Symbol->setBrush(QBrush(QColor(0x1b, 0x7d, 0xf5)));
    pQc2Symbol->setBrush(QBrush(QColor(0x1f, 0xb5, 0x58)));
    pQc3Symbol->setBrush(QBrush(QColor(0x9f, 0x65, 0xe6)));
    pQc4Symbol->setBrush(QBrush(QColor(0xff, 0x14, 0x93)));
    pQc1SubSymbol->setBrush(QBrush(QColor(0x1b, 0x7d, 0xf5)));
    pQc2SubSymbol->setBrush(QBrush(QColor(0x1f, 0xb5, 0x58)));
    pQc3SubSymbol->setBrush(QBrush(QColor(0x9f, 0x65, 0xe6)));
    pQc4SubSymbol->setBrush(QBrush(QColor(0xff, 0x14, 0x93)));
    pQcNoCalcSymbol->setBrush(QBrush(QColor(0x80, 0x80, 0x80)));
    pQc1Symbol->setPen(QPen(Qt::NoPen));
    pQc2Symbol->setPen(QPen(Qt::NoPen));
    pQc3Symbol->setPen(QPen(Qt::NoPen));
    pQc4Symbol->setPen(QPen(Qt::NoPen));
    pQc1SubSymbol->setPen(QPen(Qt::NoPen));
    pQc2SubSymbol->setPen(QPen(Qt::NoPen));
    pQc3SubSymbol->setPen(QPen(Qt::NoPen));
    pQc4SubSymbol->setPen(QPen(Qt::NoPen));
    pQcNoCalcSymbol->setPen(QPen(Qt::NoPen));
    pQc1Symbol->setSize(10);
    pQc2Symbol->setSize(10);
    pQc3Symbol->setSize(10);
    pQc4Symbol->setSize(10);
    pQcNoCalcSymbol->setSize(m_iNoCalcPointSize);
    pQc1SubSymbol->setSize(8);
    pQc2SubSymbol->setSize(8);
    pQc3SubSymbol->setSize(8);
    pQc4SubSymbol->setSize(8);
    m_pQc1Curve->setSymbol(pQc1Symbol);
    m_pQc2Curve->setSymbol(pQc2Symbol);
    m_pQc3Curve->setSymbol(pQc3Symbol);
    m_pQc4Curve->setSymbol(pQc4Symbol);
    m_pQc1SubPoints->setSymbol(pQc1SubSymbol);
    m_pQc2SubPoints->setSymbol(pQc2SubSymbol);
    m_pQc3SubPoints->setSymbol(pQc3SubSymbol);
    m_pQc4SubPoints->setSymbol(pQc4SubSymbol);
    m_pQcNoCalcPoints->setSymbol(pQcNoCalcSymbol);

    // 设置曲线样式为点
    m_pQc1SubPoints->setStyle(QwtPlotCurve::NoCurve);
    m_pQc2SubPoints->setStyle(QwtPlotCurve::NoCurve);
    m_pQc3SubPoints->setStyle(QwtPlotCurve::NoCurve);
    m_pQc4SubPoints->setStyle(QwtPlotCurve::NoCurve);
    m_pQcNoCalcPoints->setStyle(QwtPlotCurve::NoCurve);

    // 设置坐标轴刻度
    ui->QcGraphic->setAxisScale(QwtPlot::yLeft, -3.9, 3.9, 1.0);
    //ui->QcGraphicBk->setAxisScale(QwtPlot::yLeft, -3.9, 3.9, 1.0);
    ui->QcGraphic->setAxisScale(QwtPlot::xBottom, 0.5, 20.5, 1.0);
//    ui->QcGraphic->setAxisAutoScale(QwtPlot::yLeft);
    ui->QcGraphic->setAxisAutoScale(QwtPlot::xBottom);
    //ui->QcGraphic->setGeometry(-44, -4, 1203, 450);
    //ui->ScrollContent->setFixedSize(1155 , 446);
}

///
/// @brief
///     初始化信号槽连接
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月18日，新建函数
///
void McQcLjGraphic::InitConnect()
{
    // 滚动条滑块移动
    connect(ui->GraphicScrollBar, SIGNAL(valueChanged(int)), this, SLOT(OnScrollBarSliderMove(int)));

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
/// @li 4170/TangChuXian，2022年5月23日，新建函数
///
void McQcLjGraphic::UpdateSelLine()
{
    // 打印模式不画选中线
    if (m_bPrintMode)
    {
        return;
    }

    if (m_stuSelPointInfo.enCurveId == CURVE_ID_SIZE)
    {
        QVector<QPointF> vecPoints;
        m_pQcSelRing->setSamples(vecPoints);
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

    // 直接设置数据即可
    QVector<QPointF> vecSelRing;
    vecSelRing.push_back(selPt);
    m_pQcSelRing->setSamples(vecSelRing);

    // 更新选中曲线
    m_pQcSelRing->setZ(200);
    m_pQcSelRing->setSamples(vecSelRing);
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
void McQcLjGraphic::showEvent(QShowEvent *event)
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
/// @li 4170/TangChuXian，2022年5月23日，新建函数
///
bool McQcLjGraphic::eventFilter(QObject *obj, QEvent *event)
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

    // 显示不计算点复选框显示隐藏事件
    if (obj == ui->ShowSubPointCB)
    {
        if (event->type() == QEvent::Show || event->type() == QEvent::Hide)
        {
            // 刷新曲线显示
            OnCurveVisibleChanged();
        }
    }

    return QWidget::eventFilter(obj, event);
}

///
/// @brief
///     曲线显示状态改变
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月18日，新建函数
///
void McQcLjGraphic::OnCurveVisibleChanged()
{
    // 判断是否显示子点
    bool bSubPtVisible = ui->ShowSubPointCB->isChecked();
    DcsControlProxy::GetInstance()->SetQcLjSubPointFlag(bSubPtVisible);

    // 判断是否显示不计算的点
    bool bNoCalcPtVisible = IS_SHOW_NO_CALC_POINT;
    DcsControlProxy::GetInstance()->SetQcLjNoCalculateFlag(bNoCalcPtVisible);

    // 更新对应曲线（子点）显示
    SetCurveVisible(CURVE_ID_QC_1_SUB_PT, bSubPtVisible);
    SetCurveVisible(CURVE_ID_QC_2_SUB_PT, bSubPtVisible);
    SetCurveVisible(CURVE_ID_QC_3_SUB_PT, bSubPtVisible);
    SetCurveVisible(CURVE_ID_QC_4_SUB_PT, bSubPtVisible);

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
        int iMaxStep = iMaxXUpper - 20;
        ui->GraphicScrollBar->setRange(0, iMaxStep);
        SetGraphicScrollBarInWork(true);
        //ui->QcGraphic->setAxisScale(QwtPlot::xBottom, 0.5, iMaxXUpper + 4.5, 1.0);
        //ui->QcGraphic->setGeometry(-44, -4, 1203 + ((iMaxXUpper - 16) * 40), 450);
        //ui->ScrollContent->setFixedSize(1155 + ((iMaxXUpper - 16) * 40), 446);
    }
    else
    {
        ui->GraphicScrollBar->setRange(0, 0);
        SetGraphicScrollBarInWork(false);
        ui->QcGraphic->setAxisScale(QwtPlot::xBottom, 0.5, 20.5, 1.0);
        //ui->QcGraphic->setGeometry(-44, -4, 1203, 450);
        //ui->ScrollContent->setFixedSize(1155, 446);
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
/// @li 4170/TangChuXian，2022年5月23日，新建函数
///
void McQcLjGraphic::HandleQcGraphicClicked(int iX, int iY)
{
    // 误差调整
    iY -= 5;

    // 重置选中点
    m_stuSelPointInfo.Reset();
    QVector<QPointF> vecPoints;
    m_pQcSelRing->setSamples(vecPoints);

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
        HandleCurveClicked(CURVE_ID_QC_4_SUB_PT, iX, iY);
    }

    if (!ui->ShowSubPointCB->isVisible())
    {
        HandleCurveClicked(CURVE_ID_QC_1, iX, iY);
        HandleCurveClicked(CURVE_ID_QC_2, iX, iY);
        HandleCurveClicked(CURVE_ID_QC_3, iX, iY);
        HandleCurveClicked(CURVE_ID_QC_4, iX, iY);
    }

    // 显示不计算点
    if (IS_SHOW_NO_CALC_POINT)
    {
        HandleCurveClicked(CURVE_ID_QC_NO_CALC, iX, iY);
    }

    // 显示隐藏点信息显示框
    UpdateSelMarker();

    // 通知选中点改变
    emit SigSelPointChanged(m_stuSelPointInfo.enCurveId, m_stuSelPointInfo.pointID);
}

///
/// @bref
///		响应权限变化
///
/// @par History:
/// @li 8276/huchunli, 2023年8月18日，新建函数
///
void McQcLjGraphic::OnPermisionChanged()
{
    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();
    userPms->IsPermisson(PMS_QC_RESULT_SET_SHOWNOCALCULATEPT) ? ui->ShowNoCalcQcCB->show() : ui->ShowNoCalcQcCB->hide();
}

///
/// @brief
///     滚动条滑块移动
///
/// @param[in]  iValue  滑块当前位置
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月15日，新建函数
///
void McQcLjGraphic::OnScrollBarSliderMove(int iValue)
{
    // 如果范围为0，则设置默认显示
    if (ui->GraphicScrollBar->maximum() == ui->GraphicScrollBar->minimum())
    {
        ui->QcGraphic->setAxisScale(QwtPlot::xBottom, 0.5, 20.5, 1.0);
        return;
    }

    // 检查参数
    if (iValue <= 0)
    {
        ui->QcGraphic->setAxisScale(QwtPlot::xBottom, 0.5, 20.5, 1.0);
        return;
    }

    // 根据滑块位置设置当前坐标范围
    ui->QcGraphic->setAxisScale(QwtPlot::xBottom, 0.5 + iValue, 20.5 + iValue, 1.0);
}
