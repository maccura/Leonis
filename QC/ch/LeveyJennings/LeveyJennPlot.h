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
/// @file     LeveyJennPlot.h
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

#pragma once

#include <QWidget>
#include <QMap>
#include <QVector>
namespace Ui { class LeveyJennPlot; };

// 前置声明
class QPoint;
class QwtPlotCurve;                   // Qwt曲线类
class QwtPlotMarker;                  // Qwt标记线类
class QwtPlotGrid;                    // Qwt网格类

class LeveyJennPlot : public QWidget
{
    Q_OBJECT

public:
    enum CURVE_ID
    {
        CURVE_ID_QC_1 = 0,
        CURVE_ID_QC_2,
        CURVE_ID_QC_3,
        CURVE_ID_QC_1_SUB_PT,
        CURVE_ID_QC_2_SUB_PT,
        CURVE_ID_QC_3_SUB_PT,
        CURVE_ID_QC_NO_CALC,
        CURVE_ID_SIZE
    };

    // 选中点信息
    struct SelPointInfo
    {
        CURVE_ID        enCurveId;            // 所在曲线
        int             iIndex;               // 在曲线中的索引
        long long       pointID;              // 点ID（对应数据库主键）

        SelPointInfo()
        {
            enCurveId = CURVE_ID_SIZE;
            iIndex = -1;
            pointID = -1;
        }

        void Reset()
        {
            enCurveId = CURVE_ID_SIZE;
            iIndex = -1;
            pointID = -1;
        }
    };

public:
    LeveyJennPlot(QWidget *parent = Q_NULLPTR);
    ~LeveyJennPlot();

    ///
    /// @brief
    ///     设置曲线点集
    ///
    /// @param[in]  enCurveId  曲线ID
    /// @param[in]  vecPoint   曲线上的点集
    /// @param[in]  vecPointID 曲线上的点集ID
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月15日，新建函数
    ///
    void SetPoints(CURVE_ID enCurveId, const QVector<QPointF>& vecPoint, const QVector<long long>& vecPointID = QVector<long long>());

    ///
    /// @brief
    ///     重置曲线图
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月15日，新建函数
    ///
    void Reset();

    ///
    /// @brief
    ///     重置曲线图但保留选中效果
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月15日，新建函数
    ///
    void ResetButSel();

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
    void SetCurveVisible(CURVE_ID enCurveId, bool bVisible);

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
    void SelPointByID(CURVE_ID enCurveID, long long iPointID);

    ///
    /// @brief
    ///     获取选中点
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月15日，新建函数
    ///
    const SelPointInfo& GetSelectPoint();

protected:
    ///
    /// @brief
    ///     界面显示前初始化
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月15日，新建函数
    ///
    void InitBeforeShow();

    ///
    /// @brief
    ///     显示之后初始化
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，22022年11月15日，新建函数
    ///
    void InitAfterShow();

    ///
    /// @brief
    ///     初始化字符串资源
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月15日，新建函数
    ///
    void InitStrResource();

    ///
    /// @brief
    ///     初始化图例
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月15日，新建函数
    ///
    void InitLegend();

    ///
    /// @brief
    ///     初始化信号槽连接
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月15日，新建函数
    ///
    void InitConnect();

    ///
    /// @brief
    ///     更新选中效果线
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月15日，新建函数
    ///
    void UpdateSelLine();

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
    void SelPointByIndex(CURVE_ID enCurveID, int iIndex);

    ///
    /// @brief
    ///     窗口显示事件
    ///
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月15日，新建函数
    ///
    void showEvent(QShowEvent *event) override;

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
    bool eventFilter(QObject *obj, QEvent *event) override;

Q_SIGNALS:
    ///
    /// @brief
    ///     选重点改变
    ///
    /// @param[in]  enCurveID  选中点曲线ID
    /// @param[in]  iPointID   选中点ID
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月15日，新建函数
    ///
    void SigSelPointChanged(LeveyJennPlot::CURVE_ID enCurveID, long long iPointID);

    protected Q_SLOTS:
    ///
    /// @brief
    ///     曲线显示状态改变
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月15日，新建函数
    ///
    void OnCurveVisibleChanged();

    ///
    /// @brief
    ///     质控图被点击，尝试选中某一点
    ///
    /// @param[in]  iX  点击位置X坐标
    /// @param[in]  iY  点击位置Y坐标
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月15日，新建函数
    ///
    void HandleQcGraphicClicked(int iX, int iY);

private:
    Ui::LeveyJennPlot                  *ui;                            // UI对象指针
    bool                                m_bInit;                       // 是否已经初始化

                                                                       // 质控曲线
    QwtPlotCurve*                       m_pQc1Curve;                   // 质控图表1曲线
    QwtPlotCurve*                       m_pQc2Curve;                   // 质控图表2曲线
    QwtPlotCurve*                       m_pQc3Curve;                   // 质控图表3曲线

                                                                       // 质控点集
    QwtPlotCurve*                       m_pQc1SubPoints;               // 质控图表1子点
    QwtPlotCurve*                       m_pQc2SubPoints;               // 质控图表2子点
    QwtPlotCurve*                       m_pQc3SubPoints;               // 质控图表3子点
    QwtPlotCurve*                       m_pQcNoCalcPoints;             // 质控不计算点

                                                                       // 特殊线
    QwtPlotGrid*                        m_pGrid;                       // 网格线
    QwtPlotMarker*                      m_pP2SDLineMarker;             // +2SD标记线
    QwtPlotMarker*                      m_pP3SDLineMarker;             // +3SD标记线
    QwtPlotMarker*                      m_pN2SDLineMarker;             // -2SD标记线
    QwtPlotMarker*                      m_pN3SDLineMarker;             // -3SD标记线

                                                                       // 曲线X轴上限
    int                                 m_iArrayUpperX[CURVE_ID_SIZE]; // 曲线X上界

                                                                       // 选中点信息记录
    SelPointInfo                        m_stuSelPointInfo;             // 选中点信息
    QMap<CURVE_ID, QVector<QPointF>>    m_mapCurvePoints;              // 曲线包含点集
    QMap<CURVE_ID, QVector<long long>>  m_mapCurvePointsID;            // 曲线包含点ID

                                                                       // 选中效果线
    QwtPlotCurve*                       m_pQcSelLineLeft;              // 质控图选中线左
    QwtPlotCurve*                       m_pQcSelLineRight;             // 质控图选中线右
    QwtPlotCurve*                       m_pQcSelLineTop;               // 质控图选中线上
    QwtPlotCurve*                       m_pQcSelLineBottom;            // 质控图选中线下
};

