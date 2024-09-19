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
/// @file     AddWashImSampleProbeDlg.cpp
/// @brief 	 添加/编辑样本针清洗配置
///
/// @author   7656/zhang.changjiang
/// @date      2023年3月22日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年3月22日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include "shared/basedlg.h"
#include "src/thrift/im/gen-cpp/im_types.h"

namespace Ui {
    class AddWashImSampleProbeDlg;
};

namespace wsp
{
	// 界面显示信息
	struct ImCtrlsData
	{
		int				id;					// 数据库主键
		QString			num;				// 序号
		QString			assayName;			// 项目名称
        QString			volume;				// 用量
		QString			sampleSourceType;	// 样本源
	};
}

class AddWashImSampleProbeDlg : public BaseDlg
{
	Q_OBJECT

public:
	AddWashImSampleProbeDlg(QWidget *parent = Q_NULLPTR);
	~AddWashImSampleProbeDlg();

	///
	/// @brief	加载样本针配置信息到控件
	///     
	/// @param[in]  data  样本针配置信息
	///
	/// @par History:
	///  @li 7656/zhang.changjiang，2023年3月22日，新建函数
	///
	void LoadDataToCtrls(wsp::ImCtrlsData data);

	///
	/// @brief	清空控件中的数据
	///     
	/// @par History:
	///  @li 7656/zhang.changjiang，2023年3月22日，新建函数
	///
	void ClearCtrls();

	///
	/// @brief	加载序号到控件中
	///     
	/// @param[in]  num  序号
	/// @par History:
	///  @li 7656/zhang.changjiang，2023年3月22日，新建函数
	///
    void LoadNumToCtrls(const QString& num);

private:
	///
	/// @brief	初始化控件
	///     
	/// @par History:
	///  @li 7656/zhang.changjiang，2023年3月22日，新建函数
	///
	void InitCtrls();

	///
	/// @brief	检查用户输入的数据是否合法
	///     
	/// @return 成功返回true
	///
	/// @par History:
	///  @li 7656/zhang.changjiang，2023年3月22日，新建函数
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
	///  @li 7656/zhang.changjiang，2023年3月22日，新建函数
	///
	bool CheckDataNoRepeat(const im::tf::SpecialWashSampleProbe& swsp);

signals:
	///
	/// @brief
	///     样本针配置已更新
	///
	/// @par History:
	///  @li 7656/zhang.changjiang，2023年3月22日，新建函数
	///
	void WashImSampleProbeChanged(const bool pageChanged = true, const bool isAdd = true);

protected Q_SLOTS:
	///
	/// @brief	确定按钮被点击
	///     
	/// @par History:
	///  @li 7656/zhang.changjiang，2023年3月22日，新建函数
	///
	void OnSaveBtnClicked();

private:
	Ui::AddWashImSampleProbeDlg*					ui;
    std::map<int, std::shared_ptr<tf::GeneralAssayInfo>> m_mapAssayInfo;			// 通用项目的信息
	int											m_id;					// 数据库主键
};
