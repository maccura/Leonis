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
/// @brief 	  校准统计界面
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
#include "shared/PrintExportDefine.h"

class QStandardItemModel;

namespace Ui { class QCalibrationStatistics; };

class QCalibrationStatistics : public QStatisBaseWgt
{
	Q_OBJECT

public:

	enum StatisCaliCol
	{
		DEVICE_SN,                      // 设备序列号，即机身编号--模块
		ASSAY_CODE,                     // 通道号--项目名称
		REAGENT_LOT,                    // 试剂批号
		CALI_STEPS,                     // 校准次数
		CALI_SUCCEED_STEPS,             // 校准通过次数
		CALI_FAIL_STEPS,                // 校准失败次数
		CALI_SUCCEED_RATIO              // 校准通过率
	};

	enum CaliFailCauseCol
	{
		CALI_FAIL_CAUSE,                // 校准失败原因
		CALI_FAIL_NUM,                  // 次数
		FAIL_CAUSE_RATIO                // 原因占比
	};

	QCalibrationStatistics(QWidget *parent = Q_NULLPTR);
	~QCalibrationStatistics();
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
    bool GetStatisticsInfo(CaliStatisticsInfo& Info);
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
	/// @brief  初始化校准统计表
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月9日，新建函数
	///
	void InitCaliStatisTab();

	///
	/// @brief  初始化失败原因统计表
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月9日，新建函数
	///
	void InitFailCauseTab();

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
	/// @brief  点击选中校准统计表的一行
	///
	/// @param[in]  index  选中行所在的索引
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月9日，新建函数
	///
	void OnCaliStatisTabCliked(const QModelIndex &index);
private:
	Ui::QCalibrationStatistics *			ui;
	QStandardItemModel *					m_pStatisticsModel;			// 统计表格模型
	QStandardItemModel *					m_pReasonModel;				// 失败原因表格模型
};
