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
/// @file     C1005UiControlHandler.h
/// @brief    处理C1005设备UI控制命令的类
///
/// @author   3558/ZhouGuangMing
/// @date     2021年12月17日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 3558/ZhouGuangMing，2021年12月17日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include "src/thrift/ch/c1005/gen-cpp/C1005UiControl.h"

namespace ch
{
namespace c1005
{

///
/// @brief
///     处理UI控制命令的类
///
class UiControlHandler : public ch::c1005::tf::C1005UiControlIf
{
public:
    ///
    /// @brief 上报耗材更新信息
    ///
    /// @param[in]  deviceSN    设备序列号
    /// @param[in]  area        耗材分区
    /// @param[in]  positions   位置列表（耗材位置----更新类型，参见 tf::UpdateType）
    ///
    /// @par History:
    /// @li 3558/ZhouGuangMing，2021年12月17日，新建函数
    ///
    virtual void NotifySupplyInfoUpdate(const std::vector<ch::tf::SupplyUpdate>& sus) override;

    ///
    /// @brief 通知试剂加载状态变化
    ///
    /// @param[in]  deviceSN    设备编号
    /// @param[in]  loaderInfo  试剂加载状态
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年10月31日，新建函数
    ///
    virtual void NotifyReagentLoaderStatusUpdate(const std::string& deviceSN, const ::ch::tf::ReagentLoaderInfo& rli) override;

	///
	/// @brief 通知试剂盘信息更新
	///
	/// @param[in]  deviceSN    设备序列号
	/// @param[in]  rpi			试剂盘信息
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年06月05日，新建函数
	///
	virtual void NotifyReagentPlateInfoUpdate(const std::string& deviceSN, const ::ch::tf::ReagentPlateInfo& rpi) override;
};

}
}
