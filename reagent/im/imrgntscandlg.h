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
/// @file     imrgntscandlg.h
/// @brief    免疫试剂扫描对话框
///
/// @author   4170/TangChuXian
/// @date     2022年11月7日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2022年11月7日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once

#include "shared/BaseDlg.h"
namespace Ui { class ImRgntScanDlg; };

class ImRgntScanDlg : public BaseDlg
{
    Q_OBJECT

public:
    ImRgntScanDlg(QWidget *parent = Q_NULLPTR);
    ~ImRgntScanDlg();

protected:
    ///
    /// @brief
    ///     显示之前初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年11月7日，新建函数
    ///
    void InitBeforeShow();

    ///
    /// @brief
    ///     显示之后初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年11月7日，新建函数
    ///
    void InitAfterShow();

    ///
    /// @brief
    ///     重写显示事件
    ///
    /// @param[in]  event  事件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年11月7日，新建函数
    ///
    virtual void showEvent(QShowEvent* event) override;

Q_SIGNALS:
    ///
    /// @brief
    ///     确认按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年11月7日，新建函数
    ///
    void SigOkBtnClicked();

protected Q_SLOTS:
    ///
    /// @brief
    ///     扫描方式改变
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年11月7日，新建函数
    ///
    void OnScanMethodChanged();

public:
    Ui::ImRgntScanDlg                 *ui;                  // UI对象指针
    bool                               m_bInit;             // 是否初始化

    // 友元类
    friend class                       ImRgntPlateWidget;   // 免疫试剂盘界面
    friend class                       ImReagentShowList;   // 免疫试剂信息界面
};
