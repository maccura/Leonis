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
/// @file     adduserdlg.h
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
namespace Ui { class AddUserDlg; };

class AddUserDlg : public BaseDlg
{
    Q_OBJECT

public:
    AddUserDlg(QWidget *parent = Q_NULLPTR);
    ~AddUserDlg();

    ///
    /// @brief
    ///     清空界面
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
    ///     获取文本的字符数
    ///
    /// @param[in]  strText  文本
    ///
    /// @return 文本字符数
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年1月31日，新建函数
    ///
    int GetCharCnt(const QString& strText);

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

    ///
    /// @brief
    ///     用户名文本被编辑
    ///
    /// @param[in]  strText  新的文本
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年1月31日，新建函数
    ///
    void OnUserNameTextEditted(const QString& strText);

    ///
    /// @brief
    ///     密码文本被编辑
    ///
    /// @param[in]  strText  新的文本
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年2月22日，新建函数
    ///
    void OnPasswordTextEditted(const QString& strText);

private:
    Ui::AddUserDlg         *ui;                                 // ui对象指针
    bool                    m_bInit;                            // 是否已经初始化

    // 限制输入
    const int               m_ciUserNameMaxCharCnt;             // 用户名最大字符数
    const int               m_ciPasswordMaxCharCnt;             // 密码最大字符数

    // 设置友元类
    friend class            UserMngDlg;                         // 用户管理
};
