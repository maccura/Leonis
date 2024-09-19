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
#include "shared/QSwitchWidget.h"

class QAssayTabWidgets;

class QSwitchSample : public QSwitchWidget
{
	Q_OBJECT

public:
	QSwitchSample(QWidget *parent);
	~QSwitchSample();

protected:
	///
	/// @brief
	///     注册切换后的模块主页面
	///
	/// @param[in]  index  模块序号
	/// @param[in]  page   页面句柄
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年2月18日，新建函数
	///
	void RegisterPage(int index, const QString& name, QAssayTabWidgets* page);

	///
	/// @brief 初始化页面信息
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月9日，新建函数
	///
	void Init();

private:
	std::vector<QAssayTabWidgets*>	m_pages;					///< 切换页面的集合
};
