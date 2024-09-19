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
/// @file     modifypassworddlg.h
/// @brief    质控登记对话框
///
/// @author   4170/TangChuXian
/// @date     2023年7月12日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月12日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once

#include "shared/basedlg.h"
namespace Ui { class ModifyPasswordDlg; };

// 前置声明
class QKeyEvent;        // 键盘事件

class ModifyPasswordDlg : public BaseDlg
{
    Q_OBJECT

public:
    ModifyPasswordDlg(QWidget *parent = Q_NULLPTR);
    ~ModifyPasswordDlg();

    ///
    /// @brief
    ///     清空内容
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月13日，新建函数
    ///
    void Clear();

protected:
    ///
    /// @brief
    ///     界面显示前初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月12日，新建函数
    ///
    void InitBeforeShow();

    ///
    /// @brief
    ///     界面显示后初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月12日，新建函数
    ///
    void InitAfterShow();

    ///
    /// @brief
    ///     初始化字符串资源
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年1月17日，新建函数
    ///
    void InitStrResource();

    ///
    /// @brief
    ///     初始化信号槽连接
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月12日，新建函数
    ///
    void InitConnect();

    ///
    /// @brief
    ///     初始化子控件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月12日，新建函数
    ///
    void InitChildCtrl();

    ///
    /// @brief
    ///     窗口显示事件
    ///
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月12日，新建函数
    ///
    void showEvent(QShowEvent *event);

    ///
    /// @brief
    ///     键盘按下事件
    ///
    /// @param[in]  event  键盘事件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年1月29日，新建函数
    ///
    void keyPressEvent(QKeyEvent *event);

protected Q_SLOTS:
    ///
    /// @brief
    ///     更新确定按钮是否使能
    ///
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年9月8日，新建函数
    ///
    void UpdateOkBtnEnabled();

private:
    Ui::ModifyPasswordDlg         *ui;                                 // ui对象指针
    bool                           m_bInit;                            // 是否已经初始化

    // 设置友元类
    friend class                   UserMngDlg;                         // 用户管理
};
