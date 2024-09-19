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
/// @file     sysmaintainstatuswgt.h
/// @brief    系统维护状态控件
///
/// @author   4170/TangChuXian
/// @date     2023年2月15日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月15日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <QWidget>
namespace Ui { class SysMaintainStatusWgt; };

class SysMaintainStatusWgt : public QWidget
{
    Q_OBJECT

public:
    // 系统维护状态
    enum SysMaintainStatus
    {
        SYS_MAINTAIN_STATUS_NOT_MAINTAIN = 0,           // 未维护
        SYS_MAINTAIN_STATUS_MAINTAINING,                // 正在维护
        SYS_MAINTAIN_STATUS_FAILED,                     // 维护失败
        SYS_MAINTAIN_STATUS_MAINTAINING_FAILED,         // 正在维护并存在失败
        SYS_MAINTAIN_STATUS_MAINTAIN_FINISHED,          // 维护完成
    };

public:
    SysMaintainStatusWgt(QWidget *parent = Q_NULLPTR);
    ~SysMaintainStatusWgt();

    ///
    /// @brief
    ///     获取进度
    ///
    /// @return 进度（分数表示）
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月15日，新建函数
    ///
    QString GetProgress();

    ///
    /// @brief
    ///     设置进度
    ///
    /// @param[in]  strProgress  进度（分数表示）
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月15日，新建函数
    ///
    void SetProgress(QString strProgress);

    ///
    /// @brief
    ///     获取系统维护状态
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月15日，新建函数
    ///
    SysMaintainStatus GetStatus();

    ///
    /// @brief
    ///     设置系统维护状态
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月15日，新建函数
    ///
    void SetStatus(SysMaintainStatus enStatus);

    ///
    /// @brief
    ///     更新状态显示
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月15日，新建函数
    ///
    void UpdateStatusDisplay();

protected:
    ///
    /// @brief
    ///     显示前初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月15日，新建函数
    ///
    void InitBeforeShow();

private:
    Ui::SysMaintainStatusWgt        *ui;                // UI对象指针

    SysMaintainStatus                m_enStatus;        // 系统维护状态
};
