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
/// @file     SQuare.h
/// @brief    正方形元素
///
/// @author   5774/WuHongTao
/// @date     2020年6月10日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2020年6月10日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once
#include "Sharp.h"

class CSQuare :
	public CSharp
{
public:

	///
	/// @brief
	///     构造函数
	///
	/// @param[in]  width  长宽
	/// @param[in]  height  高度
	///
	/// @par History:
	/// @li 5774/WuHongTao，2020年5月29日，新建函数
	///
	CSQuare(qreal width, qreal height);

	///
	/// @brief
	///     更新sharp的大小
	///
	/// @param[in]  wscale  宽度大小
	/// @param[in]  hscale  高度大小
	///
	/// @par History:
	/// @li 5774/WuHongTao，2020年5月29日，新建函数
	///
	void updateScaleSharp(qreal wscale, qreal hscale);

	///
	/// @brief 析构函数
	///     
	/// @par History:
	/// @li 5774/WuHongTao，2020年5月29日，新建函数
	///
	~CSQuare();
};

