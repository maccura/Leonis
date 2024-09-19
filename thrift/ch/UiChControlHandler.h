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
/// @file     UiChControlHandler.h
/// @brief    处理UI控制命令的类（生化）
///
/// @author   5774/WuHongTao
/// @date     2022年9月7日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年9月7日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include "src/thrift/ch/gen-cpp/ChUiControl.h"

///
/// @brief
///     处理UI控制命令的类（生化）
///
class UiChControlHandler : public ch::tf::ChUiControlIf
{
public:
    ///
    /// @brief 上报质控申请信息更新消息
    ///
    /// @param[in]  sus  质控申请更新消息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年9月7日，新建函数
    ///
    virtual void NotifyQcAppUpdate(const std::vector< ::ch::tf::QcApply> & sus) override;

    ///
    /// @brief 校准文档组信息变更通知
    ///
    /// @param[in]  infos  更新的校准品组信息
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年10月20日，新建函数
    ///
    virtual void NotifyCaliDocGroupInfoUpdate(const std::vector<ch::tf::CaliGroupUpdate>& infos) override;

    ///
    /// @brief  通知Ui反应杯信息更新
    ///     
    ///
    /// @param[in]  deviceSN  设备序列号
    /// @param[in]  lrcs      反应杯信息
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年8月15日，新建函数
    ///
    virtual void NotifyReactionCupInfoUpdate(const std::string& deviceSN,
        const std::vector<::ch::tf::ReactionCupHistoryInfo> & lrcs) override;
};
