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
/// @file     ResetCountInfoDlg.h
/// @brief    应用-计数-重置
///
/// @author   7951/LuoXin
/// @date     2024年1月26日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2024年1月26日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "shared/basedlg.h"

namespace Ui {
    class ResetCountInfoDlg;
}

class QPushButton;

class ResetCountInfoDlg : public BaseDlg
{
    Q_OBJECT

public:
    ResetCountInfoDlg(QWidget *parent = Q_NULLPTR);
    ~ResetCountInfoDlg();

    ///
    /// @brief  显示界面
    ///     
    /// @param[in]  name  部件名称
    /// @param[in]  sn  部件序列号
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2024年1月26日，新建函数
    ///
    void showDlg(const QString& name, const QString& sn);

    ///
    /// @brief  显示界面
    ///     
    /// @param[in]  name  部件名称
    /// @param[in]  sn  部件序列号
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2024年1月26日，新建函数
    ///
    void GetData(QString& name, QString& sn);

    ///
    /// @brief  获取保存按钮的指针
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2024年1月26日，新建函数
    ///
    QPushButton* GetSaveBtnPtr();

private:
    Ui::ResetCountInfoDlg*							ui;
};
