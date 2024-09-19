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

#pragma once
#include <qwt_scale_draw.h>
#include <functional>
#include <future>
#include "mccustomscaledraw.h"

// 自画坐标轴(用于质控Y轴)
class QCustomScaleDraw: public McCustomIntegerScaleDraw
{
public:
    explicit QCustomScaleDraw(const std::function<QString(double)>& f);

    // 重写函数:设置坐标轴的刻度值
    virtual QwtText label(double value) const;


    std::function<QString(double)> m_func;

    std::future<std::string> m_task;
};
