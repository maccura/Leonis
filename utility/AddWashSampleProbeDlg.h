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
/// @file     AddWashSampleProbeDlg.h
/// @brief    添加/编辑样本针清洗配置
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
    class AddWashSampleProbeDlg;
};


namespace wsp
{
	// 界面显示信息
	struct CtrlsData
	{
		int				id;					// 数据库主键
		QString			num;				// 序号
		QString			assayName;			// 项目名称
		QString			detergentType;		// 清洗液
		QString			volume;				// 用量
		QString			sampleSourceType;	// 样本源
	};
}

class AddWashSampleProbeDlg : public BaseDlg
{
	Q_OBJECT

public:
	AddWashSampleProbeDlg(QWidget *parent = Q_NULLPTR);
	~AddWashSampleProbeDlg();

	///
	/// @brief	加载样本针配置信息到控件
	///     
	/// @param[in]  data  样本针配置信息
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年9月14日，新建函数
	///
	void LoadDataToCtrls(wsp::CtrlsData data);

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
	/// @param[in]  swsp  新增/修改的数据
	///
	/// @return 不重复返回true
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年9月16日，新建函数
	///
	bool CheckDataNoRepeat(const ch::tf::SpecialWashSampleProbe& swsp);

signals:
	///
	/// @brief
	///     样本针配置已更新
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年9月15日，新建函数
	///
	void WashSampleProbeChanged();

protected Q_SLOTS:
	///
	/// @brief	确定按钮被点击
	///     
	/// @par History:
	/// @li 7951/LuoXin，2022年9月14日，新建函数
	///
	void OnSaveBtnClicked();

private:
	Ui::AddWashSampleProbeDlg*					ui;
	int											m_id;					// 数据库主键
};
