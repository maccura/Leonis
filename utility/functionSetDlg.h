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
/// @file     functionSetDlg.h
/// @brief    应用--仪器--功能设置
///
/// @author   7951/LuoXin
/// @date     2023年1月4日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2024年1月4日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <shared/basedlg.h>

namespace Ui {
    class functionSetDlg;
};

class functionSetDlg : public BaseDlg
{
    Q_OBJECT

public:
    functionSetDlg(QWidget *parent = Q_NULLPTR);
    ~functionSetDlg();

    ///
    /// @brief	加载数据到控件
    ///     
    /// @return 
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年1月4日，新建函数
    ///
    void ShowDataToCtrls(QString name, int index);

    protected Q_SLOTS:
    ///
    /// @brief	确定按钮被点击
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2024年1月4日，新建函数
    ///
    void OnSaveBtnClicked();

private:
    Ui::functionSetDlg*		ui;
};
