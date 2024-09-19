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
/// @brief 	  结果统计界面
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
class QResultStatisticsRangeSetDlg;
class QResultRangeCard;
class QResultStatisticsBarChart;
namespace Ui { class QResultStatistics; };

class QResultStatistics : public QStatisBaseWgt
{
	Q_OBJECT

public:
	QResultStatistics(QWidget *parent = Q_NULLPTR);
	~QResultStatistics();
	virtual void showEvent(QShowEvent *event) override;

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
	/// @brief  初始化项目名称下拉列表
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月9日，新建函数
	///
	void InitAssayItemComboBox();

	///
	/// @brief  初始化试剂批号下拉列表
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月7日，新建函数
	///
	void InitReagentLotComboBox();
	
	///
	/// @brief  初始化小卡片控件
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月14日，新建函数
	///
	void InitResultRangeCard();
	
	///
	/// @brief  初始化条形统计图
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月14日，新建函数
	///
	void InitBarChart();

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
	///  @brief 设置统计数据
	///
	///
	///  @param[in]   vecData  统计数据列表
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月21日，新建函数
	///
	void SetData(QVector<QVector<double>>& vecData);

	///
	///  @brief 复查统计使能
	///
	///
	///  @param[in]   visible  true 打开 false 关闭
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月21日，新建函数
	///
	void OpenRecheckStatistics(const bool visible);

	///
	///  @brief 创建模拟数据（仅展示界面效果使用，非后台真实数据）
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月17日，新建函数
	///
	void CreateStimulateData();
//private slots:
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
private:
	Ui::QResultStatistics *					ui;
	QResultStatisticsRangeSetDlg*			m_pSetDlg;				// 结果分段设置
	std::map<int, QResultRangeCard* >		m_resultRangeCardsMap;	// 范围小卡片
	QResultStatisticsBarChart *				m_pBarChart;			// 直方图
	QVector<QVector<double>>				m_data;					// 统计数据
};
