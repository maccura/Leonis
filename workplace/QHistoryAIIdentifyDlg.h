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

#pragma once

#include <QWidget>
#include "ui_QHistoryAIIdentifyDlg.h"
#include "shared/basedlg.h"

namespace Ui { class QHistoryAIIdentifyDlg; };

class QHistoryAIIdentifyDlg : public BaseDlg
{
	Q_OBJECT

public:
	QHistoryAIIdentifyDlg(QWidget *parent = Q_NULLPTR);
	~QHistoryAIIdentifyDlg();

private:
	///
	/// @brief 初始化
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年7月24日，新建函数
	///
	void Init();

private slots:
	///
	/// @brief 确定按钮点击
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年7月24日，新建函数
	///
	void OnOkBtnClicked();

	///
	/// @brief 取消按钮点击
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年7月24日，新建函数
	///
	void OnCancelBtnClicked();

private:
	Ui::QHistoryAIIdentifyDlg* ui;
};
