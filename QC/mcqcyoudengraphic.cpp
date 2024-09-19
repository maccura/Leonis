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
/// @file     mcqcyoudengraphic.cpp
/// @brief    质控Youden图
///
/// @author   4170/TangChuXian
/// @date     2021年9月18日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月18日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "mcqcyoudengraphic.h"
#include "ui_mcqcyoudengraphic.h"
#include "shared/mccustomscaledraw.h"
#include "shared/mccustomqwtmarker.h"
#include "shared/uicommon.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "thrift/DcsControlProxy.h"
#include "manager/UserInfoManager.h"
#include "src/common/Mlog/mlog.h"

#include <qwt_legend.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_grid.h>
#include <qwt_symbol.h>
#include <qwt_scale_div.h>
#include <qwt_scale_draw.h>
#include <qwt_scale_widget.h>

#include <QMouseEvent>
#include <QMetaType>
#include <QPoint>


#define  SELECT_SENSITIVITY                        (20)                 // 选中灵敏度

McQcYoudenGraphic::McQcYoudenGraphic(QWidget *parent, bool bPrintMode)
    : QWidget(parent),
      m_bInit(false),
      m_bPrintMode(bPrintMode)
{
    // 界面显示前初始化
    ui = new Ui::McQcYoudenGraphic();
    ui->setupUi(this);

    // 界面显示前初始化
    InitBeforeShow();

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);
}

McQcYoudenGraphic::~McQcYoudenGraphic()
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
/// @param[in]  vecPointID 曲线上的点集ID
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月17日，新建函数
/// @li 4170/TangChuXian，2022年5月23日，新建函数
///
void McQcYoudenGraphic::SetPoints(CURVE_ID enCurveId, const QVector<QPointF>& vecPoint, const QVector<long long>& vecPointID /*= QVector<long long>()*/)
{
    switch (enCurveId)
    {
    case McQcYoudenGraphic::CURVE_ID_QC_1:
        m_pQc1Points->setSamples(vecPoint);
        break;
    case McQcYoudenGraphic::CURVE_ID_QC_2:
        m_pQc2Points->setSamples(vecPoint);
        break;
    case McQcYoudenGraphic::CURVE_ID_QC_NO_CALC:
        m_pQcNoCalcPoints->setSamples(vecPoint);
        break;
    default:
        break;
    }

    // 更新曲线点集记录
    m_mapCurvePoints.insert(enCurveId, vecPoint);
    m_mapCurvePointsID.insert(enCurveId, vecPointID);
}

///
/// @brief
///     重置曲线图
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月18日，新建函数
///
void McQcYoudenGraphic::Reset()
{
    QVector<QPointF> vEmptyPt;
    m_pQc1Points->setSamples(vEmptyPt);
    m_pQc2Points->setSamples(vEmptyPt);
    m_pQcNoCalcPoints->setSamples(vEmptyPt);

    // 重置选中点
    m_stuSelPointInfo.Reset();
    m_mapCurvePoints.clear();
    m_mapCurvePointsID.clear();
    m_pQcSelRing->setSamples(vEmptyPt);

    // 隐藏点信息显示框
    SetSelMarkerTipVisible(false);

    // 通知选中点改变
    emit SigSelPointChanged(m_stuSelPointInfo.enCurveId, m_stuSelPointInfo.pointID);
}

///
/// @brief
///     重置曲线图但保留选中效果
///
/// @par History:
/// @li 4170/TangChuXian，2022年5月25日，新建函数
///
void McQcYoudenGraphic::ResetButSel()
{
    QVector<QPointF> vEmptyPt;
    m_pQc1Points->setSamples(vEmptyPt);
    m_pQc2Points->setSamples(vEmptyPt);
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
void McQcYoudenGraphic::SetCurveVisible(CURVE_ID enCurveId, bool bVisible)
{
    QwtPlotCurve* pOpCurve = Q_NULLPTR;
    switch (enCurveId)
    {
    case McQcYoudenGraphic::CURVE_ID_QC_1:
        pOpCurve = m_pQc1Points;
        break;
    case McQcYoudenGraphic::CURVE_ID_QC_2:
        pOpCurve = m_pQc2Points;
        break;
    case McQcYoudenGraphic::CURVE_ID_QC_NO_CALC:
        pOpCurve = m_pQcNoCalcPoints;
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
///     通过ID选中点
///
/// @param[in]  enCurveID  曲线ID
/// @param[in]  iPointID   点ID
///
/// @par History:
/// @li 4170/TangChuXian，2022年5月23日，新建函数
///
void McQcYoudenGraphic::SelPointByID(CURVE_ID enCurveID, long long iPointID)
{
    // 重置选中点信息
    m_stuSelPointInfo.Reset();

    auto it = m_mapCurvePointsID.find(enCurveID);
    if (it == m_mapCurvePointsID.end())
    {
        QVector<QPointF> vEmptyPt;
        m_pQcSelRing->setSamples(vEmptyPt);

        // 通知选中点改变
        //emit SigSelPointChanged(m_stuSelPointInfo.enCurveId, m_stuSelPointInfo.pointID);

        // 隐藏点信息显示框
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
const McQcYoudenGraphic::SelPointInfo& McQcYoudenGraphic::GetSelectPoint()
{
    return m_stuSelPointInfo;
}

///
/// @brief
///     设置打印模式
///
/// @param[in]  bPrintMode  true表示是打印模式
///
/// @par History:
/// @li 4170/TangChuXian，2022年7月29日，新建函数
///
void McQcYoudenGraphic::SetPrintMode(bool bPrintMode)
{
    // 设置打印模式
    m_bPrintMode = bPrintMode;

    // 如果是打印模式，不显示选中效果
    m_pQcSelRing->detach();

    // 打印选择效果不显示
    if (m_bPrintMode)
    {
        // 隐藏点信息显示框
        SetSelMarkerTipVisible(false);
    }
    else
    {
        // 挂载到图表上
        m_pQcSelRing->attach(ui->QcGraphic);
        UpdateSelMarker();
    }
}

///
/// @brief
///     设置选中点提示文本
///
/// @param[in]  strTextList  选中点文本列表
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月10日，新建函数
///
void McQcYoudenGraphic::SetSelPointTipText(const QStringList& strTextList)
{
    // 成员变量赋值
    m_strSelPtTextList = strTextList;
    UpdateSelMarker();
}

///
/// @brief
///     界面显示前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2021年5月13日，新建函数
///
void McQcYoudenGraphic::InitBeforeShow()
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
/// @li 4170/TangChuXian，2021年3月25日，新建函数
///
void McQcYoudenGraphic::InitAfterShow()
{
    // 初始化字符串资源
    InitStrResource();
}

///
/// @brief
///     初始化字符串资源
///
/// @par History:
/// @li 4170/TangChuXian，2022年1月17日，新建函数
///
void McQcYoudenGraphic::InitStrResource()
{
    //ui->NoCalcQcTextLab->setText(LoadStrFromLanguage(CHAR_CODE::IDS_QC_NO_CALC_POINT));
    //ui->Qc1TextLab->setText(LoadStrFromLanguage(CHAR_CODE::IDS_QC_HIS_POINT));
    //ui->Qc2TextLab->setText(LoadStrFromLanguage(CHAR_CODE::IDS_QC_NEWEST_POINT));
    //ui->ResultNameLab->setText(LoadStrFromLanguage(CHAR_CODE::IDS_RLT_NAME));
}

///
/// @brief
///     初始化图例
///
/// @par History:
/// @li 4170/TangChuXian，2021年3月25日，新建函数
///
void McQcYoudenGraphic::InitLegend()
{
    // 注册元类型
    qRegisterMetaType<CURVE_ID>("McQcYoudenGraphic::CURVE_ID");

    // 加载是否显示计算点
    ui->ShowNoCalcQcCB->setChecked(DcsControlProxy::GetInstance()->GetQcTpNoCalculateFlag());

    // 安装事件过滤器
    ui->QcGraphic->installEventFilter(this);

    // plot自动更新
    ui->QcGraphic->setAutoReplot(true);

    // 设置背景色
    ui->QcGraphic->setCanvasBackground(QColor(0xf5, 0xf7, 0xfb));

    // 设置自定义坐标轴
    McCustomScaleDraw *pScaleDrawX = new McCustomScaleDraw();
    McCustomScaleDraw *pScaleDrawY = new McCustomScaleDraw();
    if (pScaleDrawX)
    {
        pScaleDrawX->enableComponent(QwtAbstractScaleDraw::Backbone, false);
        pScaleDrawX->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    }
    if (pScaleDrawY)
    {
        pScaleDrawY->enableComponent(QwtAbstractScaleDraw::Backbone, false);
        pScaleDrawY->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    }
    ui->QcGraphic->setAxisScaleDraw(QwtPlot::xBottom, pScaleDrawX);
    ui->QcGraphic->setAxisScaleDraw(QwtPlot::yLeft, pScaleDrawY);

    // 坐标无刻度
    ui->QcGraphic->enableAxis(QwtPlot::xTop);
    ui->QcGraphic->enableAxis(QwtPlot::yRight);
    ui->QcGraphic->axisScaleDraw(QwtPlot::xTop)->enableComponent(QwtAbstractScaleDraw::Backbone, false);
    ui->QcGraphic->axisScaleDraw(QwtPlot::xTop)->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    ui->QcGraphic->axisScaleDraw(QwtPlot::xTop)->enableComponent(QwtAbstractScaleDraw::Labels, false);
    ui->QcGraphic->axisScaleDraw(QwtPlot::yRight)->enableComponent(QwtAbstractScaleDraw::Backbone, false);
    ui->QcGraphic->axisScaleDraw(QwtPlot::yRight)->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    ui->QcGraphic->axisScaleDraw(QwtPlot::yRight)->enableComponent(QwtAbstractScaleDraw::Labels, false);

    // 设置边框
    ui->QcGraphic->canvas()->setStyleSheet("border:1px solid #a3a3a3;background:#f5f7fb;");

    // 初始化SD标记选中线
    m_pHLineMarker = new QwtPlotMarker();
    m_pVLineMarker = new QwtPlotMarker();

    // 设置为水平线
    m_pHLineMarker->setLineStyle(QwtPlotMarker::HLine);
    m_pVLineMarker->setLineStyle(QwtPlotMarker::VLine);

    // 设置颜色
    m_pHLineMarker->setLinePen(QColor(0xd4, 0xd4, 0xd4), 1.0, Qt::DashLine);
    m_pVLineMarker->setLinePen(QColor(0xd4, 0xd4, 0xd4), 1.0, Qt::DashLine);

    // 设置Z轴坐标
    m_pHLineMarker->setZ(-10);
    m_pVLineMarker->setZ(-10);

    // 设置坐标
    m_pHLineMarker->setYValue(0.0);
    m_pVLineMarker->setXValue(0.0);

    // 添加到曲线图上
    m_pHLineMarker->attach(ui->QcGraphic);
    m_pVLineMarker->attach(ui->QcGraphic);

    // 初始化曲线成员对象
    // 质控曲线
    m_pQc1Points = new QwtPlotCurve();
    m_pQc2Points = new QwtPlotCurve();
    m_pQcNoCalcPoints = new QwtPlotCurve();
    m_p1SDCurve = new QwtPlotCurve();
    m_p2SDCurve = new QwtPlotCurve();
    m_p3SDCurve = new QwtPlotCurve();

    // 选中效果圆环
    m_pQcSelRing = new QwtPlotCurve();
    QwtSymbol* pQcSelSymbol = new QwtSymbol(QwtSymbol::Pixmap);
    pQcSelSymbol->setPixmap(QPixmap(":/Leonis/resource/image/img-qc-graphic-sel-ring.png"));
    m_pQcSelRing->setSymbol(pQcSelSymbol);

    // 选中标记
    m_pQcSelTipMarker = new QcGraphicSelTipMaker();

    // 设置曲线颜色
    m_p1SDCurve->setPen(QColor(0x1f, 0xb5, 0x58), 1.0, Qt::DashLine);
    m_p2SDCurve->setPen(QColor(0xfa, 0x90, 0x16), 1.0, Qt::DashLine);
    m_p3SDCurve->setPen(QColor(0xfa, 0x37, 0x41), 1.0, Qt::DashLine);

    // 构造SD曲线点集
    QVector<QPointF> vecSDPts{QPointF(-1, 1), QPointF(1, 1), QPointF(1, -1), QPointF(-1, -1), QPointF(-1, 1)};
    QVector<QPointF> vec2SDPts{QPointF(-2, 2), QPointF(2, 2), QPointF(2, -2), QPointF(-2, -2), QPointF(-2, 2) };
    QVector<QPointF> vec3SDPts{QPointF(-3, 3), QPointF(3, 3), QPointF(3, -3), QPointF(-3, -3), QPointF(-3, 3) };
    m_p1SDCurve->setSamples(vecSDPts);
    m_p2SDCurve->setSamples(vec2SDPts);
    m_p3SDCurve->setSamples(vec3SDPts);

    // 挂载到图表上
    m_pQc1Points->attach(ui->QcGraphic);
    m_pQc2Points->attach(ui->QcGraphic);
    m_p1SDCurve->attach(ui->QcGraphic);
    m_p2SDCurve->attach(ui->QcGraphic);
    m_p3SDCurve->attach(ui->QcGraphic);
    m_pQcSelRing->attach(ui->QcGraphic);
    if (ui->ShowNoCalcQcCB->isChecked())
    {
        m_pQcNoCalcPoints->attach(ui->QcGraphic);
    }

    // 设置曲线点符号
    QwtSymbol* pQc1Symbol = new QwtSymbol(QwtSymbol::Ellipse);
    QwtSymbol* pQc2Symbol = new QwtSymbol(QwtSymbol::Ellipse);
    QwtSymbol* pQcNoCalcSymbol = new QwtSymbol(QwtSymbol::Ellipse);
    pQc1Symbol->setBrush(QBrush(QColor(0x1b, 0x7d, 0xf5)));
    pQc2Symbol->setBrush(QBrush(QColor(0x1f, 0xb5, 0x58)));
    pQcNoCalcSymbol->setBrush(QBrush(QColor(0x80, 0x80, 0x80)));
    pQc1Symbol->setPen(QPen(Qt::NoPen));
    pQc2Symbol->setPen(QPen(Qt::NoPen));
    pQcNoCalcSymbol->setPen(QPen(Qt::NoPen));
    pQc1Symbol->setSize(10);
    pQc2Symbol->setSize(10);
    pQcNoCalcSymbol->setSize(10);
    m_pQc1Points->setSymbol(pQc1Symbol);
    m_pQc2Points->setSymbol(pQc2Symbol);
    m_pQcNoCalcPoints->setSymbol(pQcNoCalcSymbol);

    // 设置曲线样式为点
    m_pQc1Points->setStyle(QwtPlotCurve::NoCurve);
    m_pQc2Points->setStyle(QwtPlotCurve::NoCurve);
    m_pQcNoCalcPoints->setStyle(QwtPlotCurve::NoCurve);

    // 设置坐标轴刻度
    ui->QcGraphic->setAxisScale(QwtPlot::yLeft, -3.9, 3.9, 1.0);
    ui->QcGraphic->setAxisScale(QwtPlot::xBottom, -3.9, 3.9, 1.0);
//     ui->QcGraphic->setAxisAutoScale(QwtPlot::yLeft);
//     ui->QcGraphic->setAxisAutoScale(QwtPlot::xBottom);
}

///
/// @brief
///     初始化信号槽连接
///
/// @par History:
/// @li 4170/TangChuXian，2021年9月18日，新建函数
///
void McQcYoudenGraphic::InitConnect()
{
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
void McQcYoudenGraphic::UpdateSelLine()
{
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
    bool bExsitSelPt = false;
    for (int i = 0; i < it.value().size(); i++)
    {
        if (i != m_stuSelPointInfo.iIndex)
        {
            continue;
        }

        // 更新选中点信息
        selPt = it.value()[i];
        bExsitSelPt = true;
    }

    // 无效点返回
    if (!bExsitSelPt)
    {
        return;
    }

    // 构造点集
    QVector<QPointF> vecSelPoints;
    vecSelPoints << selPt;

    // 更新选中曲线
    m_pQcSelRing->setZ(200);
    m_pQcSelRing->setSamples(vecSelPoints);
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
void McQcYoudenGraphic::SelPointByIndex(CURVE_ID enCurveID, int iIndex)
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
/// @li 4170/TangChuXian，2023年7月10日，新建函数
///
void McQcYoudenGraphic::SetSelMarkerTipVisible(bool bVisible)
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
/// @li 4170/TangChuXian，2023年7月10日，新建函数
///
void McQcYoudenGraphic::UpdateSelMarker()
{
    // 判断是否有选中点
    if (m_stuSelPointInfo.enCurveId == CURVE_ID_SIZE)
    {
        // 没有选中点则隐藏
        SetSelMarkerTipVisible(false);
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

        if (qAbs(selPt.x()) < 2)
        {
            iAlign |= Qt::AlignHCenter;
        }
        else if (selPt.x() > 0)
        {
            iAlign |= Qt::AlignLeft;
        }
        else if (selPt.x() < 0)
        {
            iAlign |= Qt::AlignRight;
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
///     窗口显示事件
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 4170/TangChuXian，2021年3月25日，新建函数
///
void McQcYoudenGraphic::showEvent(QShowEvent *event)
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
bool McQcYoudenGraphic::eventFilter(QObject *obj, QEvent *event)
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

    return QWidget::eventFilter(obj, event);
}

///
/// @brief
///     质控图被点击，尝试选中某一点
///
/// @param[in]  iX  点击位置X坐标
/// @param[in]  iY  点击位置Y坐标
///
/// @par History:
/// @li 4170/TangChuXian，2022年5月23日，新建函数
///
void McQcYoudenGraphic::HandleQcGraphicClicked(int iX, int iY)
{
    // 误差调整
    iY -= 4;
    iX -= 2;

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

    // 处理历史点，最新点和不就算点
    HandleCurveClicked(CURVE_ID_QC_1, iX, iY);
    HandleCurveClicked(CURVE_ID_QC_2, iX, iY);
    HandleCurveClicked(CURVE_ID_QC_NO_CALC, iX, iY);

    // 显示隐藏点信息显示框
    UpdateSelMarker();

    // 通知选中点改变
    emit SigSelPointChanged(m_stuSelPointInfo.enCurveId, m_stuSelPointInfo.pointID);
}

///
/// @brief
///     曲线显示状态改变
///
/// @par History:
/// @li 4170/TangChuXian，2023年6月6日，新建函数
///
void McQcYoudenGraphic::OnCurveVisibleChanged()
{
    // 判断是否显示不计算的点
    bool bNoCalcPtVisible = ui->ShowNoCalcQcCB->isChecked();
    DcsControlProxy::GetInstance()->SetQcTpNoCalculateFlag(bNoCalcPtVisible);

    // 判断是显示操作曲线还是隐藏操作曲线
    if (bNoCalcPtVisible)
    {
        m_pQcNoCalcPoints->attach(ui->QcGraphic);
    }
    else
    {
        m_pQcNoCalcPoints->detach();
    }
}

void McQcYoudenGraphic::OnPermisionChanged()
{
    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();
    userPms->IsPermisson(PMS_QC_RESULT_SET_SHOWNOCALCULATEPT) ? ui->ShowNoCalcQcCB->show() : ui->ShowNoCalcQcCB->hide();
}
