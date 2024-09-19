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
/// @file     C1005UiControlHandler.cpp
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
#include "C1005UiControlHandler.h"
#include "src/common/Mlog/mlog.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"

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
void ch::c1005::UiControlHandler::NotifySupplyInfoUpdate(const std::vector<ch::tf::SupplyUpdate>& sus)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 将试剂信息更新消息发送到UI消息总线
    POST_MESSAGE(MSG_ID_REAGENT_INFO_UPDATE, const_cast<std::vector<ch::tf::SupplyUpdate>&>(sus));
}

///
/// @brief 通知试剂加载状态变化
///
/// @param[in]  deviceSN    设备编号
/// @param[in]  loaderInfo  试剂加载状态
///
/// @par History:
/// @li 5774/WuHongTao，2022年10月31日，新建函数
///
void ch::c1005::UiControlHandler::NotifyReagentLoaderStatusUpdate(const std::string& deviceSN, const ::ch::tf::ReagentLoaderInfo& rli)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    POST_MESSAGE(MSG_ID_REAGENT_LOADER_UPDATE, QString::fromStdString(deviceSN), const_cast<::ch::tf::ReagentLoaderInfo&>(rli));
}

void ch::c1005::UiControlHandler::NotifyReagentPlateInfoUpdate(const std::string & deviceSN, const::ch::tf::ReagentPlateInfo& rpi)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	// 将试剂盘信息更新消息发送到UI消息总线
	POST_MESSAGE(MSG_ID_REAGENT_PLATE_INFO_UPDATE, QString::fromStdString(deviceSN), const_cast<::ch::tf::ReagentPlateInfo&>(rpi));
}

