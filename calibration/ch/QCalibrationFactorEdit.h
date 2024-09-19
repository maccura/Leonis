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
/// @file     QCalibrationFactorEdit.h
/// @brief    校准系数编辑
///
/// @author   8580/GongZhiQiang
/// @date     2023年1月6日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年1月6日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QWidget>
#include "shared/basedlg.h"
#include "src/alg/ch/ch_alg/ch_alg.h"
#include "src/thrift/gen-cpp/defs_types.h"
#include "../thrift/ch/gen-cpp/ch_types.h"
#include <boost/optional.hpp>

namespace Ui {
    class QCalibrationFactorEdit;
};


class QCalibrationFactorEdit : public BaseDlg
{
	Q_OBJECT

public:
	QCalibrationFactorEdit(QWidget *parent = Q_NULLPTR);
	~QCalibrationFactorEdit();

	///
	/// @brief 初始化校准系数编辑对话框
	///
	/// @param[in]  rowIndex  当前操作校准申请的索引
	/// @param[in]  reagent   试剂详细信息
	///
	/// @return true:初始化成功 false：初始化不成功
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年1月6日，新建函数
	///
	bool initDialog(int rowIndex, ch::tf::ReagentGroup& reagent);

private:

	///
	/// @brief 检查曲线，如果校准方法为非线性，则不能进行校准编辑
	///                  
	///
	/// @param[in]  caliType  校准方法类型
	///
	/// @return true：线性  false：非线性
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年1月6日，新建函数
	///
	bool checkCurveIsLine(const ch::tf::CalibrationType::type& caliType);

	///
	/// @brief 增加一条编辑曲线，创建成功则返回曲线ID
	///                  
	///
	/// @param[in]  value_K       设置的k值
	/// @param[in]  value_S1labs  设置的b值
	/// @param[in]  caliCurveId   新增曲线的ID
	///
	/// @return true：增加成功  false：增加失败
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年1月6日，新建函数
	///
	bool addNewCurve(const double& value_K, const double& value_S1labs, int &caliCurveId);

	///
	/// @brief 修改旧曲线（暂时不用此函数）
	///                  
	///
	/// @param[in]  value_K       设置的k值
	/// @param[in]  value_S1labs  设置的b值
	///
	/// @return true：修改成功  false：修改失败
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年1月6日，新建函数
	///
	bool modifyOldCurve(const double& value_K, const double& value_S1labs);

	///
	/// @brief 初始化界面信息
	///                  
	/// @return true：初始化成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年1月6日，新建函数
	///
	bool initUI();

	///
	/// @brief 分析方法转换（ch::tf::AnalysisMethod的转换成ch::ANALYSIS_METHOD）
	///                  
	/// @return true：转换成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年1月6日，新建函数
	///
	bool analysisMethodExchange(const ch::tf::AnalysisMethod::type& chTfMethod, ::ch::ANALYSIS_METHOD& chMethod);

	///
	/// @brief 校准类型转换（ch::tf::CalibrationType的转换成ch::CALIBRATION_TYPE）
	///                  
	/// @return true：转换成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年1月6日，新建函数
	///
	bool calibrationTypeExchange(const ch::tf::CalibrationType::type& chTfType, ::ch::CALIBRATION_TYPE& chType);

	///
	/// @brief 获取试剂的当前校准曲线
	///
	/// @param[in]  reagent  试剂信息
	///
	/// @return 校准曲线
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年1月6日，新建函数
	///
	std::shared_ptr<ch::tf::CaliCurve> getCurrentCurve(const ch::tf::ReagentGroup& reagent);

	///
	/// @brief 获取当前试剂曲线句柄
	///
	/// @param[in]  reagent  试剂信息
	///
	/// @return 曲线句柄
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年1月6日，新建函数
	///
	std::shared_ptr<ch::ICalibrateCurve> getCurveHandel(const ch::tf::ReagentGroup& reagent);

    ///
    /// @brief  转换试剂阶段
    ///
    /// @param[in]  enType  试剂阶段
    ///
    /// @return 整形值
    ///
    /// @par History:
    /// @li 3558/ZhouGuangMing，2023年5月10日，新建函数
    ///
    int GetReagentStageIdx(::ch::tf::ReagentStage::type enType);

private slots:
	///
	/// @brief 执行保存
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年1月6日，新建函数
	///
	void saveCalibrationFactor();

signals:

	///
	/// @brief 校准系数更改成功信号
	///
	/// @param[in]  rowIndex    当前操作校准申请的索引
	/// @param[in]  caliCurveId 新增加编辑曲线的ID
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年1月11日，新建函数
	///
	void sigCalibrationFactorEdited(int rowIndex, int caliCurveId);

private:
	Ui::QCalibrationFactorEdit* ui;

	ch::tf::ReagentGroup currentReagent;						///< 当前需要编辑校准系数的试剂信息
	std::shared_ptr<ch::tf::GeneralAssayInfo> currentGAI;	    ///< 当前项目的通用信息接口
	int currentRow;                                             ///< 当前操作校准申请的索引
};
