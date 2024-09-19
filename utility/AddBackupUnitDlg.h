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
/// @file     AddBackupUnitDlg.h
/// @brief    添加备选单位
///
/// @author   7951/LuoXin
/// @date     2023年11月22日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2023年11月22日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include "shared/basedlg.h"

namespace Ui {
    class AddBackupUnitDlg;
};

class AddBackupUnitDlg : public BaseDlg
{
    Q_OBJECT

public:
    AddBackupUnitDlg(QWidget *parent = Q_NULLPTR);
    ~AddBackupUnitDlg();

    ///
    /// @brief  显示新增界面
    ///     
    ///
    /// @param[in]  assayName  项目名称
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年11月22日，新建函数
    ///
    void ShowAddWidget(const QString& assayName);

    ///
    /// @brief  显示修改界面
    ///     
    ///
    /// @param[in]  assayName  项目名称
    /// @param[in]  unit       单位名称
    /// @param[in]  factor     单位倍率
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年11月22日，新建函数
    ///
    void ShowModifyWidget(const QString& assayName, const QString& unit, const QString& factor);

    protected Q_SLOTS:

    ///
    /// @brief
    ///     保存按钮被点击
    /// @par History:
    /// @li 7951/LuoXin，2022年8月19日，新建函数
    ///
    void OnSaveBtnClicked();

Q_SIGNALS:
    void ModifyComplete(QString unit, QString factor);
    void AddComplete(QString unit, QString factor);

private:
    Ui::AddBackupUnitDlg*								ui;
    QString                                             m_assayName;
    QString                                             m_unit;
    QString                                             m_factor;
};
