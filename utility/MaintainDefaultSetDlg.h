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
/// @file     MaintainDefaultSetDlg.h
/// @brief    维护保养-组合设置-默认设置
///
/// @author   8580/GongZhiQiang
/// @date     2023年6月26日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年6月26日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include "shared/basedlg.h"
#include "model/MaintainGroupCfgModel.h"
#include <src/thrift/gen-cpp/defs_types.h>

namespace Ui {
    class MaintainDefaultSetDlg;
};

class MaintainDefaultSetDlg : public BaseDlg
{
	Q_OBJECT

	// 参数设置控件的页码
	enum ParamPageNum
	{
		page_Normal = 0,				///< 常规页
		page_ChPipeFill,				///< 比色管路填充
		page_ProbeClean,				///< 针清洗
		page_IseDrainage,				///< ISE排液
		page_IsePipeFill				///< ISE管路填充
	};

public:
	MaintainDefaultSetDlg(QWidget *parent = Q_NULLPTR);
	~MaintainDefaultSetDlg();

	///
	/// @brief	加载维护项数据到控件
	///     
	/// @param[in]  item  修改的维护项
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年6月26日，新建函数
	///
	void LoadDataToCtrls(const MaintainGroupCfgModel::ItemData & item);

	///
	/// @brief	加载数据到控件
	///     
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年6月26日，新建函数
	///
	MaintainGroupCfgModel::ItemData GetModifiedMaintainItem();

	///
	/// @brief	是否包含该含参维护单项
	///     
	/// @param[in]  itemType  维护项类型
	///
	/// @return true:包含
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年6月26日，新建函数
	///
	static bool IsContainParamItem(int itemType);

signals:
	///
	/// @brief	用户修改了维护单项参数
	///     
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年6月26日，新建函数
	///
	void ModifiedMaintainItem();

private slots:
	///
	/// @brief	保存按钮被点击
	///     
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年6月26日，新建函数
	///
	void OnSaveBtnClicked();

	///
	/// @brief	为输入框设置正则表达式
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年6月21日，新建函数
	///
	void SetCtrlsRegExp();

private:
	
	///
	/// @brief	根据仪器类型初始化控件
	///
	/// @param[in]  itemType  维护项类型
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年6月21日，新建函数
	///
	void InitCtrls(const ::tf::MaintainItemType::type& itemType);

private:
	Ui::MaintainDefaultSetDlg* ui;

	MaintainGroupCfgModel::ItemData modifyItemTemp;				///< 用户修改的维护项数据暂存，确定后返回给上层界面保存
};
