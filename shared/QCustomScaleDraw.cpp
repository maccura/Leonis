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
/// @file     QCustomScaleDraw.cpp
/// @brief    用于Qwt图形的自定义坐标轴
///
/// @author   8090/YeHuaNing
/// @date     2023年3月27日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8090/YeHuaNing，2023年3月27日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "QCustomScaleDraw.h"

QCustomScaleDraw::QCustomScaleDraw(const std::function<QString(double)>& f)
    : m_func(f)
{
}

QwtText QCustomScaleDraw::label(double value) const
{
    if (m_func)
    {
        // 构造返回值
        QwtText qwtText(m_func(value));
        if (m_color.isValid() && (int(value) == m_iSelVal))
        {
            qwtText.setColor(m_color);
        }

        // 显示刻度值
        return qwtText;
    }

    return QwtScaleDraw::label(value);
}
