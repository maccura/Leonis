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
/// @file     menuwidget.h
/// @brief    菜单模块
///
/// @author   5774/WuHongTao
/// @date     2021年6月2日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2021年6月2日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <QWidget>

class QPushButton;


class MenuWidget : public QWidget
{
    Q_OBJECT

public:

    ///
    /// @brief
    ///     菜单模块的构造函数
    ///
    /// @param[in]  parent  父模块
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年6月2日，新建函数
    ///
    MenuWidget(QWidget *parent);

    ///
    /// @brief
    ///     析构函数
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年6月2日，新建函数
    ///
    ~MenuWidget();

	void SetExitBtnEnable(bool bEnable);

protected Q_SLOTS:

    ///
    /// @brief
    ///     退出按钮的响应函数
    ///
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年6月2日，新建函数
    ///
    void OnExitBtnClicked();

private:
    QPushButton*                    m_pExitBtn;                 // 退出按钮
};
