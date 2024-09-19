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
/// @file     devmaintaindetailitemwgt.h
/// @brief    设备维护状态控件
///
/// @author   4170/TangChuXian
/// @date     2023年3月8日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月8日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <QWidget>
namespace Ui { class DevMaintainDetailItemWgt; };

// 前置声明
class QMovie;   // 动画

class DevMaintainDetailItemWgt : public QWidget
{
    Q_OBJECT

public:
    enum DevMaintainItemStatus
    {
        DEV_MAINTAIN_ITEM_STATUS_INIT = 0,                   // 初始状态
        DEV_MAINTAIN_ITEM_STATUS_WAIT = 1,                   // 等待
        DEV_MAINTAIN_ITEM_STATUS_SUCC = 2,                   // 维护成功
        DEV_MAINTAIN_ITEM_STATUS_FAILED = 3,                 // 维护失败
        DEV_MAINTAIN_ITEM_STATUS_EXCUTE = 4,                 // 正在维护
    };

public:
    DevMaintainDetailItemWgt(QWidget *parent = Q_NULLPTR);
    ~DevMaintainDetailItemWgt();

    ///
    /// @brief
    ///     获取维护项名称
    ///
    /// @return 维护项名称
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月8日，新建函数
    ///
    QString GetItemName();

    ///
    /// @brief
    ///     设置维护项名称
    ///
    /// @param[in]  strItemName  维护项名称
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月8日，新建函数
    ///
    void SetItemName(const QString& strItemName);

    ///
    /// @brief
    ///     获取设备名
    ///
    /// @return 设备名
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年5月22日，新建函数
    ///
    QString GetDevName();

    ///
    /// @brief
    ///     设置设备名
    ///
    /// @param[in]  strDevName  设备名
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年5月22日，新建函数
    ///
    void SetDevName(const QString& strDevName);

    ///
    /// @brief
    ///     获取维护时间
    ///
    /// @return 维护时间
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月13日，新建函数
    ///
    QString GetMaintainTime();

    ///
    /// @brief
    ///     设置维护时间
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月13日，新建函数
    ///
    void SetMaintainTime(const QString& strMaintainTime);

    ///
    /// @brief
    ///     获取设备维护状态
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月8日，新建函数
    ///
    DevMaintainItemStatus GetStatus();

    ///
    /// @brief
    ///     设置设备维护状态
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月8日，新建函数
    ///
    void SetStatus(DevMaintainItemStatus enStatus);

protected:
    ///
    /// @brief
    ///     显示之前初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月13日，新建函数
    ///
    void InitBeforeShow();

    ///
    /// @brief
    ///     更新状态
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月8日，新建函数
    ///
    void UpdateStatus();

    ///
    /// @brief
    ///     显示事件
    ///
    /// @param[in]  pEvt  事件对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年8月22日，新建函数
    ///
    void showEvent(QShowEvent* pEvt);

Q_SIGNALS:
    ///
    /// @brief
    ///     重新维护
    ///
    /// @param[in]  strDevNamw  设备名
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月13日，新建函数
    ///
    void SigReMaintain(QString strDevNamw);

protected Q_SLOTS:
    ///
    /// @brief
    ///     重新维护按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月13日，新建函数
    ///
    void OnReMaintainBtnClicked();

private:
    Ui::DevMaintainDetailItemWgt        *ui;                // UI对象指针
    QMovie*                              m_pMovie;          // 动画
    DevMaintainItemStatus                m_enStatus;        // 设备维护状态
};
