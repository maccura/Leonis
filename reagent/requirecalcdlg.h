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
/// @file     requirecalcdlg.h
/// @brief    需求计算对话框
///
/// @author   4170/TangChuXian
/// @date     2023年6月26日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2023年6月26日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once

#include "shared/basedlg.h"

// 前置声明
class QRadioButton;

namespace Ui { class RequireCalcDlg; };

class RequireCalcDlg : public BaseDlg
{
    Q_OBJECT

public:
    enum WeekDayTabIdx
    {
        TabIdxWeekDay1 = 0,
        TabIdxWeekDay2,
        TabIdxWeekDay3,
        TabIdxWeekDay4,
        TabIdxWeekDay5,
        TabIdxWeekDay6,
        TabIdxWeekDay7
    };

public:
    RequireCalcDlg(QWidget *parent = Q_NULLPTR);
    ~RequireCalcDlg();

protected:
    ///
    /// @brief
    ///     界面显示前初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年6月22日，新建函数
    ///
    void InitBeforeShow();

    ///
    /// @brief
    ///     界面显示后初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年6月22日，新建函数
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
    /// @li 4170/TangChuXian，2021年6月22日，新建函数
    ///
    void InitConnect();

    ///
    /// @brief
    ///     初始化子控件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年6月22日，新建函数
    ///
    void InitChildCtrl();

    ///
    /// @brief
    ///     窗口显示事件
    ///
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年6月22日，新建函数
    ///
    void showEvent(QShowEvent *event);

Q_SIGNALS:
    ///
    /// @brief
    ///     保存数据
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年9月26日，新建函数
    ///
    void SigSaveData();

protected Q_SLOTS:
    ///
    /// @brief
    ///     需求计算设置按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年6月27日，新建函数
    ///
    void OnReqCalcCfgBtnClicked();

    ///
    /// @brief
    ///     确定按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年9月26日，新建函数
    ///
    void OnOkBtnClicked();

    ///
    /// @brief
    ///     加载数据
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年9月26日，新建函数
    ///
    void LoadData();

    ///
    /// @brief 响应打印按钮
    ///
    /// @par History:
    /// @li 6889/ChenWei，2024年3月25日，新建函数
    ///
    void OnPrintBtnClicked();

private:
    Ui::RequireCalcDlg     *ui;             // ui指针对象
    bool                    m_bInit;        // 是否已经初始化

    // 显示设备类型单元框
    QRadioButton*           m_pAllDevRBtn;  // 全部单选框
    QRadioButton*           m_pAllChRBtn;   // 生化单选框
    QRadioButton*           m_pAllImRBtn;   // 免疫单选框
};
