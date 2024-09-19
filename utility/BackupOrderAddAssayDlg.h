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
/// @file     BackupOrderAddAssayDlg.h
/// @brief    应用--检测--备用订单--增加项目
///
/// @author   7951/LuoXin
/// @date     2022年9月20日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2022年9月20日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include "shared/basedlg.h"
#include "src/public/ConfigDefine.h"
#include <set>
#include <utility>

namespace Ui {
    class BackupOrderAddAssayDlg;
};


class BackupOrderAddAssayDlg : public BaseDlg
{
	Q_OBJECT

public:
	BackupOrderAddAssayDlg(QWidget *parent = Q_NULLPTR);
	~BackupOrderAddAssayDlg();

	///
	/// @brief	获取默认测试项目
	///     
	/// @param[out]  selectAssayCodes  选中的项目编号
    /// @param[out]  profiles          组合项目ID
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年9月21日，新建函数
	///
	void GetUserConfigData(std::vector<int32_t>& selectAssayCodes, std::vector<int64_t>& profiles);

	///
	/// @brief	获取保存按钮指针
	///     
	/// @par History:
	/// @li 7951/LuoXin，2022年9月20日，新建函数
	///
    QPushButton* GetSaveBtnPtr();

    ///
    /// @brief	显示项目添加界面
    ///   
    /// @param[int]  assayCodes  已选中的项目
    /// @param[int]  profiles    组合项目id
    /// @param[int]  sampleSource  样本源类型
    ///   
    /// @par History:
    /// @li 7951/LuoXin，2023年9月18日，新建函数
    ///
    void ShowAddAssayDlg(const std::vector<int32_t>& assayCodes, std::vector<int64_t>& profiles, int sampleSource);

protected:

	///
	/// @brief	初始化控件
	///     
	/// @par History:
	/// @li 7951/LuoXin，2022年9月20日，新建函数
	///
	void InitCtrls();

private:
	Ui::BackupOrderAddAssayDlg*					ui;
};
