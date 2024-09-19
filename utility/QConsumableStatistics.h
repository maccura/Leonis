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
/// @file     QCalibrationStatistics.h
/// @brief 	  试剂/耗材统计界面
///
/// @author   7656/zhang.changjiang
/// @date      2023年7月11日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年7月11日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include "shared/QStatisBaseWgt.h"
#include "src/thrift/gen-cpp/statis_types.h"
#include "shared/PrintExportDefine.h"

class QStandardItemModel;
class QConsumableCurve;
class QScrollBar;

namespace Ui { class QConsumableStatistics; };

class QConsumableStatistics : public QStatisBaseWgt
{
	Q_OBJECT

public:

	// 显示模式
	enum TABLE_SHOW_MODE
	{
		REAGENT,					// 试剂
		CONSUMEABLE,				// 耗材
	};

	QConsumableStatistics(QWidget *parent = Q_NULLPTR);
	~QConsumableStatistics();
	virtual void showEvent(QShowEvent *event) override;

    ///
    ///  @brief 获取打印数据
    ///
    ///
    ///
    ///  @return	
    ///
    ///  @par History: 
    ///  @li 6889/ChenWei，2024年1月9日，新建函数
    ///
    bool GetStatisticsInfo(ReagentStatisticsInfo& Info);

protected slots:
    ///
    /// @bref
    ///		权限变化响应
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年7月26日，新建函数
    ///
    void OnPermisionChanged();

private:
	
	///
	///  @brief 显示前初始化
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月17日，新建函数
	///
	void InitBeforeShow();

	///
	/// @brief  初始化模块下拉列表
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月7日，新建函数
	///
	void InitDevComboBox();

	///
	/// @brief  初始化试剂/耗材下拉列表
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月9日，新建函数
	///
	void InitSupComboBox();

	///
	/// @brief  初始化表格
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月16日，新建函数
	///
	void InitSupTab();
	
	///
	/// @brief  初始化曲线图
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月16日，新建函数
	///
	void InitCurve();

	///
	///  @brief 连接信号槽
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月17日，新建函数
	///
	void ConnectSlots() override;

	///
	///  @brief 切换显示模式
	///
	///
	///  @param[in]   mode  显示模式
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月17日，新建函数
	///
	void SwitchMode(TABLE_SHOW_MODE mode);

	///
	/// @brief  更新表格数据
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年12月14日，新建函数
	///
	void UpdateTable();
	
	///
	/// @brief  查询选中的行的曲线数据
	///
	/// @param[in]  row  选中的行
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年12月14日，新建函数
	///
	void QueryDataByRow(int row);

	///
	/// @brief  计算X轴对应的值
	///
	/// @param[in]  groupByType           分组类型
	/// @param[in]  statisReagentCurve    曲线数据
	///
	/// @return X 轴对应的值
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2024年3月1日，新建函数
	///
	int CalCuXValue(const ::tf::GroupByType::type groupByType, const ::tf::StatisReagentCurve &curveData);

	///
	/// @brief  解码试剂统计曲线数据
	///
	/// @param[in]  outCurveData  曲线数据返回值
	/// @param[in]  groupByType   分组类型
	/// @param[in]  curveData     数据库查询出的数据
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年12月15日，新建函数
	///
	void DecodeReagData(QVector<QVector<QPointF>>& outCurveData, ::tf::GroupByType::type groupByType, const std::vector<::tf::StatisReagentCurve> &curveData);
	
	///
	/// @brief  解码耗材统计曲线数据
	///
	/// @param[in]  outCurveData  曲线数据返回值
	/// @param[in]  groupByType   分组类型
	/// @param[in]  curveData     数据库查询出的数据
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年12月15日，新建函数
	///
	void DecodeSupData(QVector<QVector<QPointF>>& outCurveData, ::tf::GroupByType::type groupByType, const std::vector<::tf::StatisSupCurve> &curveData);

	///
	/// @brief  计算X轴坐标范围
	///
	/// @param[in]  mode		显示模式（按周、按月、按天）
	/// @param[in]  startDate   起始日期
	/// @param[in]  endDate     截止日期
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年12月18日，新建函数
	///
	int GetXRange(const::tf::GroupByType::type mode, const QDate& startDate, const QDate& endDate);
	
	///
	/// @brief  更新水平滑动条范围
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年12月18日，新建函数
	///
	void UpdateScrollBarRange();
	
	///
	/// @brief  切换曲线图显示模式（按周、按月、按天）
	///
	/// @param[in]  mode  显示模式
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年12月19日，新建函数
	///
	void ShowModeChanged(const ::tf::GroupByType::type mode);

	///
	/// @brief		清除表格内容
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 1556/Chenjianlin，2024年3月30日，新建函数
	///
	void ClearTableData();

private slots:

	///
	/// @brief  筛选按钮被点击
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月7日，新建函数
	///
	void OnQueryBtnClicked() override;

	///
	/// @brief  重置按钮被点击
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月7日，新建函数
	///
	void OnResetBtnClicked() override;

	///
	/// @brief  点击耗材表的一行
	///
	/// @param[in]  index  选中行所在的索引
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月9日，新建函数
	///
	void OnSupTabCliked(const QModelIndex &index);

	///
	/// @brief  起止日期改变
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月27日，新建函数
	///
	void DateEditChanged() override;

	///
	/// @brief  平移处理函数
	///
	/// @param[in]  dx  x 轴偏移量
	/// @param[in]  dy  y 轴偏移量
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年12月19日，新建函数
	///
	void HandlePanned(int dx, int dy);
private:
	Ui::QConsumableStatistics *ui;
	QScrollBar*								m_pScrollBar;			// 滚动条
	QConsumableCurve*						m_pConsumableCurve;     // 试剂/耗材统计曲线图
	QStandardItemModel *					m_pModel;				// 表格模型
};
