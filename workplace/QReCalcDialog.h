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
/// @file     QReCalcDialog.h
/// @brief    
///
/// @author   1226/ZhangJing
/// @date     2023年1月17日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 1226/ZhangJing，2023年1月17日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QWidget>
#include "shared/basedlg.h"
#include "../thrift/im/gen-cpp/im_types.h"
#include "src/thrift/gen-cpp/defs_types.h"
#include <QList>
#include <boost/optional.hpp>

namespace Ui {
    class QReCalcDialog;
};
class QSerialModel;


class QReCalcDialog : public BaseDlg
{
    Q_OBJECT

public:
	QReCalcDialog(QWidget *parent = Q_NULLPTR);
    ~QReCalcDialog();	

public slots:

	///
	/// @brief 执行保存
	///
	/// @par History:
	/// @li 1226/zhangjing，2023年1月17日，新建函数
	///
	void SaveReCalcResults();

	///
	/// @brief 当某一行选择后
	///
	/// @param[in]  index  被点击的项目
	///
    /// @par History:
    /// @li 1226/zhangjing，2023年1月17日，新建函数
    ///
    void onSelectRow(const QModelIndex &index);

	///
	/// @brief  重新计算该试剂全部未计算结果
	///
	/// @par History:
	/// @li 1226/zhangjing，2023年2月1日，新建函数
	///
	void onReCalcAllResults();


	///
	/// @brief  重新计算该试剂选中未计算结果
	///
	/// @par History:
	/// @li 1226/zhangjing，2023年2月1日，新建函数
	///
	void onReCalcSelResults();

	///
	/// @bref
	///		项目下拉框事件
	///
	/// @param[in] sel 选择的sel
	///
	/// @par History:
	/// @li 1226/zhangjing，2023年2月1日，新建函数
	///
	void OnItemComboxChange(int sel);

private:
    ///
    /// @brief 初始化表标题
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年1月17日，新建函数
    ///
    void InitTableTitle();

    ///
    /// @brief 初始化设备
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年1月17日，新建函数
    ///
    void InitMachines();

    ///
    /// @brief  当选择设备后，进行刷新
    ///
    /// @param[in]  devClassify  
    /// @param[in]  deviceName  设备名称
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年1月17日，新建函数
    ///
    void onSelectMachine(tf::AssayClassify::type  devClassify, const std::string& deviceName);

    ///
    /// @brief 将当前表格数据设置为免疫信息
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年1月17日，新建函数
    ///
    void SetImReagInfoToTable();

	///
	/// @brief  更新右侧未计算结果信息
	///
	/// @par History:
	/// @li 1226/zhangjing，2023年1月17日，新建函数
	///
	void UpdateResultsInfoToTable();

	///
	/// @brief  重新计算未计算结果
	///
	/// @param[in]  bAll  是否全部
	///
	/// @par History:
	/// @li 1226/zhangjing，2023年2月1日，新建函数
	///
	void reCalcResults(bool bAll);

	///
	/// @brief  筛选试剂
	///
	/// @par History:
	/// @li 1226/zhangjing，2023年2月1日，新建函数
	///
	void onQueryReagInfos();

	///
	/// @brief  筛选试剂
	///
	/// @par History:
	/// @li 1226/zhangjing，2023年2月1日，新建函数
	///
	void onResetReagInfos();

	enum ImReagHeader {
		rcr_ReagLot= 0,
		rcr_ReagSN,
		rcr_CaliLot,
		rcr_CaliDate,
		rcr_CaliCurExpDate,
	};

	enum ImResultHeader {
        rcr_Select= 0,
		rcr_SampleNo ,
		rcr_SampBarcode,
		rcr_RLU,
		rcr_ReCalcConc,
		rcr_ResultStatus,
	};

private:
	Ui::QReCalcDialog*				ui;
	std::string                     m_curDevSN;                 ///当前设备序号
	QSerialModel*					m_tableModelReag;	        ///< 试剂表数据
	QSerialModel*					m_tableModelResults;	    ///< 结果表数据
};


