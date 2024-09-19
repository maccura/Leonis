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
/// @file     imbeadmixdlg.h
/// @brief    免疫磁珠混匀对话框
///
/// @author   4170/TangChuXian
/// @date     2022年9月27日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月27日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once

#include "shared/BaseDlg.h"
#include <string>
#include <map>
#include "src/thrift/gen-cpp/defs_types.h"

class QCheckBox;
namespace Ui { class ImBeadMixDlg; };

class ImBeadMixDlg : public BaseDlg
{
    Q_OBJECT

public:
    ImBeadMixDlg(QWidget *parent = Q_NULLPTR);
    ~ImBeadMixDlg();

    ///
    /// @brief
    ///     获取选中的设备序列号
    ///
    /// @param[in]  deviceSnVec  设备序列号数组
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年11月1日，新建函数
    ///
    void GetSelectDeviceSn(std::vector<std::string>& deviceSnVec);

    ///
    /// @brief
    ///     获取混匀次数
    ///
    /// @return 混匀次数
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年11月1日，新建函数
    ///
    int GetMixTimes();

protected:
    ///
    /// @brief
    ///     显示之前初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年9月27日，新建函数
    ///
    void InitBeforeShow();

    ///
    /// @brief
    ///     显示之后初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年9月27日，新建函数
    ///
    void InitAfterShow();

    ///
    /// @brief
    ///     重写显示事件
    ///
    /// @param[in]  event  事件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年9月27日，新建函数
    ///
    virtual void showEvent(QShowEvent* event) override;

protected Q_SLOTS:
    ///
    /// @brief
    ///     【全部】复选框选中状态改变
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年11月7日，新建函数
    ///
    void OnAllCBCheckChanged();

    ///
    /// @brief
    ///     模块复选框选中状态改变
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年11月7日，新建函数
    ///
    void OnModuleCBCheckChanged();

    ///
    /// @brief
    ///     设备状态更新
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年6月19日，新建函数
    ///
    void OnDeviceStatusUpdate(tf::DeviceInfo deviceInfo);

    ///
    /// @brief
    ///     混匀次数编辑框文本改变
    ///
    /// @param[in]  strText  文本
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年12月19日，新建函数
    ///
    void OnMixCntEditTextChanged(const QString& strText);

public:
    Ui::ImBeadMixDlg                 *ui;                   // UI对象指针
    bool                              m_bInit;              // 是否初始化


    std::map<std::string, std::string>  m_mapNameToSn;
    // 管理所有模块复选框
    QVector<QCheckBox*> m_vCB;

    // 友元类
    friend class                      ImReagentShowList;    // 免疫试剂信息界面
};
