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
/// @file     usermngdlg.h
/// @brief    默认质控对话框
///
/// @author   4170/TangChuXian
/// @date     2023年3月17日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月17日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once

#include "shared/basedlg.h"
#include "src/thrift/gen-cpp/defs_types.h"
#include <QMap>
namespace Ui { class UserMngDlg; };

// 前置声明
class QTableWidgetItem;                     // 表格单元项
class QRadioButton;                         // 单选按钮
class AddUserDlg;                           // 添加用户对话框
class ModifyPasswordDlg;                    // 修改密码对话框

class UserMngDlg : public BaseDlg
{
    Q_OBJECT

public:
    UserMngDlg(QWidget *parent = Q_NULLPTR);
    ~UserMngDlg();

    enum OpUserMethod
    {
        OP_USER_METHOD_AUTO_LOGIN = 0,      // 设置自动登录
        OP_USER_METHOD_DELETE,              // 删除用户
        OP_USER_METHOD_RESET_PASSWORD,      // 重置密码
    };

protected:
    ///
    /// @brief
    ///     界面显示前初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月17日，新建函数
    ///
    void InitBeforeShow();

    ///
    /// @brief
    ///     界面显示后初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月17日，新建函数
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
    /// @li 4170/TangChuXian，2023年3月17日，新建函数
    ///
    void InitConnect();

    ///
    /// @brief
    ///     初始化子控件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月17日，新建函数
    ///
    void InitChildCtrl();

    ///
    /// @brief
    ///     获取下一个空白行
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月29日，新建函数
    ///
    int GetNextBlamkRow();

    ///
    /// @brief
    ///     窗口显示事件
    ///
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月17日，新建函数
    ///
    void showEvent(QShowEvent *event);

    ///
    /// @brief
    ///     是否允许去操作用户
    ///
    /// @param[in]  stuOpUser   被操作的用户
    /// @param[in]  enOpMd      操作方式
    ///
    /// @return true表示允许
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年9月8日，新建函数
    ///
    bool IsAllowedToOperateUser(const tf::UserInfo& stuOpUser, OpUserMethod enOpMd);

    ///
    /// @brief
    ///     更新表格行号显示
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年3月14日，新建函数
    ///
    void UpdateTblRowNum();

protected Q_SLOTS:
    ///
    /// @brief
    ///     更新用户信息表
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月29日，新建函数
    ///
    void UpdateUserInfoTbl();

    ///
    /// @brief
    ///     选中用户信息改变
    ///
    /// @param[in]  selIndex  选中索引
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月29日，新建函数
    ///
    void OnSelUserChanged(const QModelIndex& selIndex);

    ///
    /// @brief
    ///     新增按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月29日，新建函数
    ///
    void OnAddBtnClicked();

    ///
    /// @brief
    ///     修改按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月29日，新建函数
    ///
    void OnModifyBtnClicked();

    ///
    /// @brief
    ///     删除按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月29日，新建函数
    ///
    void OnDelBtnClicked();

    ///
    /// @brief
    ///     重置密码按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月2日，新建函数
    ///
    void OnResetPasswordBtnClicked();

    ///
    /// @brief
    ///     更新按钮使能状态
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年4月10日，新建函数
    ///
    void UpdateBtnEnableStatus();

    ///
    /// @brief
    ///     添加用户对话框确定按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月13日，新建函数
    ///
    void OnAddUserDlgOkBtnClicked();

    ///
    /// @brief
    ///     修改密码对话框确定按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月13日，新建函数
    ///
    void OnModifyPasswordDlgOkBtnClicked();

    ///
    /// @brief
    ///     自动登录按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月8日，新建函数
    ///
    void OnAutoLoginBtnClicked();

private:
    Ui::UserMngDlg                  *ui;                      // ui指针
    bool                             m_bInit;                 // 是否已经初始化

    QString                          m_strAutoLoginUser;      // 自动登录用户
    std::shared_ptr<tf::UserInfo>    m_spLoginUserInfo;       // 当前登录用户
    QMap<QString, tf::UserInfo>      m_mapUserInfo;           // 用户信息映射表

    AddUserDlg*                      m_pAddUserDlg;           // 添加用户对话框
    ModifyPasswordDlg*               m_pModifyPasswordDlg;    // 修改密码对话框
};
