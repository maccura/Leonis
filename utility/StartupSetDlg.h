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
/// @file     StartupSetDlg.h
/// @brief    应用--系统--自动开机设置
///
/// @author   7951/LuoXin
/// @date     2022年10月13日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2022年10月13日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <vector>
#include <map>
#include "src/public/ConfigDefine.h"
#include "shared/basedlg.h"


namespace Ui {
    class StartupSetDlg;
};
class QStandardItemModel;
class QTime;
class QTimeEdit;
class QCheckBox;


class StartupSetDlg : public BaseDlg
{
    Q_OBJECT

public:
    StartupSetDlg(QWidget *parent = Q_NULLPTR);
    ~StartupSetDlg();

private:
    ///
    /// @brief	初始化控件
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2022年10月13日，新建函数
    ///
    void InitCtrls();

    protected Q_SLOTS:
    ///
    /// @brief	保存按钮被点击
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2022年10月13日，新建函数
    ///
    void OnSaveBtnClicked();

    // 时间变化
    void OnTimeChanged(const QTime& time);

    // 设备框被勾选
    void OnDeviceChecked(int stat);

    ///
    /// @bref
    ///		权限变化响应
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年4月10日，新建函数
    ///
    void OnPermisionChanged();

private:
    Ui::StartupSetDlg*  ui;
    QStandardItemModel* m_model;

    struct CellItem
    {
        CellItem(QCheckBox* pCheckBox, QTimeEdit* pTimeEdit)
            : itemCheck(pCheckBox), itemTimeE(pTimeEdit)
        {}
        QCheckBox *itemCheck;
        QTimeEdit *itemTimeE;
    };
    std::map<std::string, std::vector<CellItem>> m_rowItems; // <deviceSN, <CellWidgets>>
};
