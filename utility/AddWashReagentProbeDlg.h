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
/// @file     AddWashReagentProbeDlg.h
/// @brief    添加/编辑试剂针清洗配置
///
/// @author   7951/LuoXin
/// @date     2022年9月14日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2022年9月14日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include "shared/basedlg.h"
#include "src/thrift/ch/gen-cpp/ch_types.h"

namespace Ui {
    class AddWashReagentProbeDlg;
};


namespace wrp 
{
	// 界面显示信息
	struct CtrlsData
	{
		int				id;				// 数据库主键
		QString			num;			// 序号
		QString			probe;			// 试剂针
		QString			fromAssay;		// 污染物
		QString			toAssay;		// 受污染物
		QString			detergentType;	// 清洗液
		QString			volume;			// 用量
	};
}

class AddWashReagentProbeDlg : public BaseDlg
{
	Q_OBJECT

public:
	AddWashReagentProbeDlg(QWidget *parent = Q_NULLPTR);
	~AddWashReagentProbeDlg();

	///
	/// @brief	加载试剂针配置信息到控件
	///     
	///
	/// @param[in]  data  试剂针配置信息
	///
	/// @return 
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年9月14日，新建函数
	///
	void LoadDataToCtrls(wrp::CtrlsData data);

	///
	/// @brief	清空控件中的数据
	///     
	/// @par History:
	/// @li 7951/LuoXin，2022年9月14日，新建函数
	///
	void ClearCtrls();

	///
	/// @brief	加载序号到控件中
	///     
	/// @param[in]  num  序号
	/// @par History:
	/// @li 7951/LuoXin，2022年9月14日，新建函数
	///
    void LoadNumToCtrls(const QString& num);

private:
	///
	/// @brief	初始化控件
	///     
	/// @par History:
	/// @li 7951/LuoXin，2022年9月14日，新建函数
	///
	void InitCtrls();

	///
	/// @brief	初始化项目下拉框
	///     
	/// @par History:
	/// @li 7951/LuoXin，2023年4月17日，新建函数
	///
	void InitAssayCodeCombox();

	///
	/// @brief	检查用户输入的数据是否合法
	///     
	/// @return 成功返回true
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年9月14日，新建函数
	///
	bool CheckUserData();

	///
	/// @brief	检查当新增/修改的记录是否不重复
	///     
	///
	/// @param[in]  swrp  新增/修改的数据
	///
	/// @return 不重复返回true
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年9月16日，新建函数
	///
	bool CheckDataNoRepeat(const ch::tf::SpecialWashReagentProbe& swrp);

signals:
	///
	/// @brief
	///     试剂针配置已更新
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年9月15日，新建函数
	///
	void WashReagentProbeChanged();

protected Q_SLOTS:
	///
	/// @brief	确定按钮被点击
	///     
	/// @par History:
	/// @li 7951/LuoXin，2022年9月14日，新建函数
	///
	void OnSaveBtnClicked();

private:
	Ui::AddWashReagentProbeDlg*					ui;
    std::map<int, std::shared_ptr<tf::GeneralAssayInfo>> m_mapAssayInfo;			// 通用项目的信息
	int											m_id;					// 数据库主键
};
