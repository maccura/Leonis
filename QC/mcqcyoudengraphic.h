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
/// @file     mcqcyoudengraphic.h
/// @brief    
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

#pragma once

#include <QWidget>
#include <QMap>
#include <QVector>
namespace Ui { class McQcYoudenGraphic; };

// 前置声明
class QwtPlotCurve;                     // Qwt曲线类
class QwtPlotMarker;                    // Qwt标记线类
class QPoint;                           // 点坐标
class QcGraphicSelTipMaker;             // 选中效果标记

class McQcYoudenGraphic : public QWidget
{
    Q_OBJECT

public:
    enum CURVE_ID
    {
        CURVE_ID_QC_1 = 0,
        CURVE_ID_QC_2,
        CURVE_ID_QC_NO_CALC,
        CURVE_ID_QC_1_SD,
        CURVE_ID_QC_2_SD,
        CURVE_ID_QC_3_SD,
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
    McQcYoudenGraphic(QWidget *parent = Q_NULLPTR, bool bPrintMode = false);
    ~McQcYoudenGraphic();

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
    void SetPoints(CURVE_ID enCurveId, const QVector<QPointF>& vecPoint, const QVector<long long>& vecPointID = QVector<long long>());

    ///
    /// @brief
    ///     重置曲线图
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年9月18日，新建函数
    ///
    void Reset();

    ///
    /// @brief
    ///     重置曲线图但保留选中效果
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年5月25日，新建函数
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
    /// @li 4170/TangChuXian，2021年9月18日，新建函数
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
    /// @li 4170/TangChuXian，2022年5月23日，新建函数
    ///
    void SelPointByID(CURVE_ID enCurveID, long long iPointID);

    ///
    /// @brief
    ///     获取选中点
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年5月23日，新建函数
    ///
    const SelPointInfo& GetSelectPoint();

    ///
    /// @brief
    ///     设置打印模式
    ///
    /// @param[in]  bPrintMode  true表示是打印模式
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年7月29日，新建函数
    ///
    void SetPrintMode(bool bPrintMode);

    ///
    /// @brief
    ///     设置选中点提示文本
    ///
    /// @param[in]  strTextList  选中点文本列表
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月10日，新建函数
    ///
    void SetSelPointTipText(const QStringList& strTextList);

protected:
    ///
    /// @brief
    ///     界面显示前初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年5月13日，新建函数
    ///
    void InitBeforeShow();

    ///
    /// @brief
    ///     显示之后初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年3月25日，新建函数
    ///
    void InitAfterShow();

    ///
    /// @brief
    ///     初始化字符串资源
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年1月17日，新建函数
    ///
    void InitStrResource();

    ///
    /// @brief
    ///     初始化图例
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年3月25日，新建函数
    ///
    void InitLegend();

    ///
    /// @brief
    ///     初始化信号槽连接
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年9月18日，新建函数
    ///
    void InitConnect();

    ///
    /// @brief
    ///     更新选中效果线
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年5月23日，新建函数
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
    /// @li 4170/TangChuXian，2022年5月23日，新建函数
    ///
    void SelPointByIndex(CURVE_ID enCurveID, int iIndex);

    ///
    /// @brief
    ///     设置选中提示标记是否选中
    ///
    /// @param[in]  bVisible  是否显示
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月10日，新建函数
    ///
    void SetSelMarkerTipVisible(bool bVisible);

    ///
    /// @brief
    ///     更新质控图选中点提示
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月10日，新建函数
    ///
    void UpdateSelMarker();

    ///
    /// @brief
    ///     窗口显示事件
    ///
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年3月25日，新建函数
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
    /// @li 4170/TangChuXian，2022年5月23日，新建函数
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
    /// @li 4170/TangChuXian，2022年5月25日，新建函数
    ///
    void SigSelPointChanged(McQcYoudenGraphic::CURVE_ID enCurveID, long long iPointID);

protected Q_SLOTS:
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
    void HandleQcGraphicClicked(int iX, int iY);

    ///
    /// @brief
    ///     曲线显示状态改变
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年6月6日，新建函数
    ///
    void OnCurveVisibleChanged();

    ///
    /// @bref
    ///		响应权限变化
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年8月18日，新建函数
    ///
    void OnPermisionChanged();

private:
    Ui::McQcYoudenGraphic              *ui;                            // UI对象指针
    bool                                m_bInit;                       // 是否已经初始化
    bool                                m_bPrintMode;                  // 打印模式

    // 质控点集
    QwtPlotCurve*                       m_pQc1Points;                  // 质控图表1点集
    QwtPlotCurve*                       m_pQc2Points;                  // 质控图表2点集
    QwtPlotCurve*                       m_pQcNoCalcPoints;             // 质控不计算点

    // SD曲线
    QwtPlotCurve*                       m_p1SDCurve;                   // 质控图1倍SD
    QwtPlotCurve*                       m_p2SDCurve;                   // 质控图2倍SD
    QwtPlotCurve*                       m_p3SDCurve;                   // 质控图3倍SD

    // 特殊线
    QwtPlotMarker*                      m_pHLineMarker;                // 水平标记线
    QwtPlotMarker*                      m_pVLineMarker;                // 垂直标记线

    // 选中点信息记录
    SelPointInfo                        m_stuSelPointInfo;             // 选中点信息
    QMap<CURVE_ID, QVector<QPointF>>    m_mapCurvePoints;              // 曲线包含点集
    QMap<CURVE_ID, QVector<long long>>  m_mapCurvePointsID;            // 曲线包含点ID

    // 选中效果
    QStringList                         m_strSelPtTextList;            // 选中点文本列表
    QcGraphicSelTipMaker*               m_pQcSelTipMarker;             // 选中点悬浮框标志
    QwtPlotCurve*                       m_pQcSelRing;                  // 质控图选中效果（圆环）

    // 友元类
    friend class                        QcGraphYoudenWidget;           // 质控Youden图界面
};
