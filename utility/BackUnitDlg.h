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
/// @file     BackUnitDlg.h
/// @brief    备选单位弹窗
///
/// @author   7951/LuoXin
/// @date     2022年8月26日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2022年8月26日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once
#include "shared/basedlg.h"

class QStandardItemModel;
class AddBackupUnitDlg;

namespace Ui {
    class BackUnitDlg;
};

class BackUnitDlg : public BaseDlg
{
    Q_OBJECT

public:
    BackUnitDlg(QWidget *parent = Q_NULLPTR);
    ~BackUnitDlg();

    ///
    /// @brief	加载界面显示信息
    ///     
    /// @param[in]  assayCode  项目编号
    /// @par History:
    /// @li 7951/LuoXin，2022年8月29日，新建函数
    ///
    void LoadDataToCtrls(int assayCode);

private:
    ///
    /// @brief	初始化控件
    /// @par History:
    /// @li 7951/LuoXin，2022年8月26日，新建函数
    ///
    void InitCtrls();

    protected Q_SLOTS:

    ///
    /// @brief	删除按钮被点击
    /// @par History:
    /// @li 7951/LuoXin，2022年8月26日，新建函数
    ///
    void OnDelBtnClicked();

    ///
    /// @brief	修改按钮被点击
    /// @par History:
    /// @li 7951/LuoXin，2023年11月22日，新建函数
    ///
    void OnModifyBtn();

private:
    ///
    /// @bref
    ///		判断是否是当前单位
    ///
    /// @param[in] strUnitName 被判定的单位名
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年1月8日，新建函数
    ///
    bool IsCurrentUnit(const QString& strUnitName) const;

private:
    Ui::BackUnitDlg*							ui;
    QStandardItemModel*							m_tabViewMode;			// 表格的model
    AddBackupUnitDlg*                           m_addBackupUnitDlg;     // 添加备选单位

    QString                                     m_currentAssayName;     // 当前项目名
};
