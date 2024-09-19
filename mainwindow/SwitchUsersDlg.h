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
/// @file     SwitchUsersDlg.h
/// @brief 	  切换用户对话框
///
/// @author   8580/GongZhiQiang
/// @date      2023年7月10日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年7月10日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include "shared/basedlg.h"

namespace Ui {
    class SwitchUsersDlg;
};

class SwitchUsersDlg : public BaseDlg
{
	Q_OBJECT

public:
	SwitchUsersDlg(QWidget *parent = Q_NULLPTR);
	~SwitchUsersDlg();

protected:
	///
	///  @brief 初始化
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 8580/GongZhiQiang，2023年7月10日，新建函数
	///
	void Init();

private slots:
	///
	///  @brief 确定按钮
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 8580/GongZhiQiang，2023年7月10日，新建函数
	///
	void OnOkBtnClicked();

	///
	///  @brief 取消按钮
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 8580/GongZhiQiang，2023年7月10日，新建函数
	///
	void OnCancerBtnClicked();

    ///
    /// @brief  用户名称回车
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年2月1日，新建函数
    ///
    void OnUserNameEntern();

private:
	Ui::SwitchUsersDlg* ui;
};
