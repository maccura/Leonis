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
/// @brief 	  样本量统计界面
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

#include <QStatisBaseWgt.h>
#include "shared/PrintExportDefine.h"
class QStandardItemModel;
class QSamplesDonutPie;
class QListView;
namespace Ui { class QSampleSizeStatistics; };

class QSampleSizeStatistics : public QStatisBaseWgt
{
	Q_OBJECT

public:

	enum StatisSampleCol
	{
		DEVICE_SN,                      // 设备序列号，即机身编号--模块
		REAGENT_LOT,                    // 试剂批号
		ASSAY_CODE,                     // 通道号--项目名称
		TOTAL_SAMPLES,                  // 样本量
		NOR_SAMPLES,                    // 常规样本量
		EMER_SAMPLES,                   // 急诊样本量
		QC_SAMPLES,                     // 质控
		CALI_SAMPLES                    // 校准
	};

	QSampleSizeStatistics(QWidget *parent = Q_NULLPTR);
	~QSampleSizeStatistics();
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
    bool GetStatisticsInfo(SampleStatisticsInfo& Info);

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
	/// @brief  初始化饼状图控件
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月7日，新建函数
	///
	void InitPie();
	
	///
	/// @brief  初始化表格
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月7日，新建函数
	///
	void InitTable();
	
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
private slots:
	
	///
	/// @brief  查询饼状图数据
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月7日，新建函数
	///
	void QueryPieData();
	
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

private:
	Ui::QSampleSizeStatistics *                             ui;
	QStandardItemModel *					                m_pTableModel;			// 表格模型
	QSamplesDonutPie *                                      m_pQSamplesDonutPie;    // 饼状图
	std::shared_ptr<::tf::SampleStatisPieData>              m_pieData;              // 饼状图数据
    QStringList                                             m_headerData;           // 表头数据
};
