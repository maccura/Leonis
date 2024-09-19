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
/// @file     customlabel.h
/// @brief    自定义标签
///
/// @author   4170/TangChuXian
/// @date     2023年8月4日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2023年8月4日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <QLabel>

class CustomLabel : public QLabel
{
    Q_OBJECT

public:
    CustomLabel(QWidget *parent);

public Q_SLOTS:
    ///
    /// @brief
    ///     设置文本
    ///
    /// @param[in]  strText  文本
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月4日，新建函数
    ///
    void setText(const QString& strText);

Q_SIGNALS:
    ///
    /// @brief
    ///     文本改变
    ///
    /// @param[in]  strText  
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月4日，新建函数
    ///
    void textChanged(const QString& strText);
};
