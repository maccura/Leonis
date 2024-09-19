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
/// @file     calhiscurve.h
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
#pragma once

#include <QVector>
#include <QWidget>
namespace Ui { class CalHisCurve; };

// 前置声明
class QwtPlot;
class QwtPlotCurve;                   // Qwt曲线类
class QwtPlotMarker;                  // Qwt标记线类
class QwtPlotGrid;                    // Qwt网格类

class CalHisCurve : public QWidget
{
    Q_OBJECT

public:
    CalHisCurve(QWidget *parent = Q_NULLPTR);
    ~CalHisCurve();

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
    void SetHisCount(int CalCnt);

    ///
    /// @brief
    ///     重置曲线图
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年8月10日，新建函数
    ///
    void Reset();

    ///
    /// @brief
    ///     设置选中项索引
    ///
    /// @param[in]  iIndex  索引
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年8月10日，新建函数
    ///
    void SetSelIndex(int iIndex);

    ///
    /// @brief
    ///     获取选中项索引
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年8月10日，新建函数
    ///
    int GetSelIndex();

    ///
    /// @brief 获取qwt plot
    ///
    /// @return plot指针
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年12月9日，新建函数
    ///
    QwtPlot* GetQwtPlot();

    ///
    /// @brief
    ///     更新图表
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年3月26日，新建函数
    ///
    void UpdatePlot();

    ///
    /// @brief 设置右侧Y轴是否显示
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年12月13日，新建函数
    ///
    void SetYAxisEnable();

    ///
    /// @brief 纵坐标刻度值颜色
    ///
    /// @param[in]  axisId  轴
    /// @param[in]  color  颜色
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年12月14日，新建函数
    ///
    void SetAxisColor(int axisId, const QColor& color);

    ///
    /// @brief 设置坐标轴标题
    ///
    /// @param[in]  axisId  坐标轴
    /// @param[in]  title  标题
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年12月14日，新建函数
    ///
    void  SetAxisTitle(int axisId, const QString& title);

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
	void SetYRange(double min, double max);

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
    ///     初始化图例
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年3月25日，新建函数
    ///
    void InitLegend();

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
    /// @li 4170/TangChuXian，2021年3月26日，新建函数
    ///
    bool eventFilter(QObject *obj, QEvent *event) override;

Q_SIGNALS:
    ///
    /// @brief
    ///     当前选中校准历史信息改变
    ///
    /// @param[in]  iIndex  改变后选中校准历史信息索引
    /// @param[in]  pos  鼠标点击的位置
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年4月28日，新建函数
    ///
    void SigCurrentSelCalHisInfoChanged(int iIndex);

    void SigOnSelectPoint(QPoint pos);

public Q_SLOTS :
    void TestRangeChanged(int);

private:
    Ui::CalHisCurve        *ui;                            // UI对象指针
    bool                    m_bInit;                       // 是否已经初始化
    int                     m_iSelIndex;                   // 校准历史曲线选中索引

    // 校准历史数据
    int                     m_iCalCnt;                     // 校准次数
    QVector<QwtPlotCurve*>          m_curves;                   ///< 显示的曲线

    // 分割线
    QwtPlotGrid*            m_pSplitGrid;                  // 校准次数之间的网格分割线
    QwtPlotMarker*          m_pVLineMarker;                // 标记选中的校准次数序号
};
