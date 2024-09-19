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
/// @file     AssayAllocationDlg.h
/// @brief    应用--项目--项目分配
///
/// @author   7951/LuoXin
/// @date     2023年4月24日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2023年4月24日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <shared/basedlg.h>

namespace Ui {
    class AssayAllocationDlg;
};

class QStandardItemModel;
class QUtilitySortFilterProxyModel;

class AssayAllocationDlg : public BaseDlg
{
    Q_OBJECT

public:
    AssayAllocationDlg(QWidget *parent = Q_NULLPTR);
    ~AssayAllocationDlg();

    ///
    /// @brief	加载数据到控件
    ///     
    /// @return 
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年4月24日，新建函数
    ///
    void LoadDataToCtrls();

protected:
    ///
    /// @brief 窗口显示事件
    ///     
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年4月24日，新建函数
    ///
    void showEvent(QShowEvent *event);

private:
    ///
    /// @brief	初始化控件
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2023年4月24日，新建函数
    ///
    void InitCtrls();

    protected Q_SLOTS:
    ///
    /// @brief	确定按钮被点击
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2023年4月24日，新建函数
    ///
    void OnSaveBtnClicked();

private:
    Ui::AssayAllocationDlg*						ui;
    QStandardItemModel*                         m_pAssayModel;      // 项目列表的model
    QUtilitySortFilterProxyModel*               m_sortModel;        // 表格排序的model
};
