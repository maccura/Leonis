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
/// @file     Circle.h
/// @brief    圆形的元素
///
/// @author   5774/WuHongTao
/// @date     2020年5月29日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2020年5月29日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "Circle.h"

///
/// @brief
///     构造圆的构造函数
///
/// @param[in]  width  长宽
/// @param[in]  height  高度
///
/// @par History:
/// @li 5774/WuHongTao，2020年5月29日，新建函数
///
CCircle::CCircle(qreal width, qreal height)
	:CSharp(width, height)
{
	updateScaleSharp(width, height);
}

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
void CCircle::updateScaleSharp(qreal width, qreal height)
{
	QPainterPath circle;
	m_width = width;
	m_height = height;

	//画圆的方法
	circle.moveTo(width / 2, height / 2);
	circle.arcTo(0, 0, width, height, 0, 360);
	m_sharp = circle;
}

///
/// @brief 析构函数
///     
/// @par History:
/// @li 5774/WuHongTao，2020年5月29日，新建函数
///
CCircle::~CCircle()
{
}
