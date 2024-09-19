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
/// @file     UiIseControlHandler.cpp
/// @brief    处理UI控制命令的类（电解质）
///
/// @author   8090/YeHuaNing
/// @date     2022年12月29日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8090/YeHuaNing，2022年12月29日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "UiIseControlHandler.h"
#include "src/common/defs.h"
#include "src/common/Mlog/mlog.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"


///
/// @brief 上报电解质校准申请信息更新消息
///
/// @param[in]  sus  电解质校准申请更新消息
///
/// @par History:
/// @li 8090/YeHuaNing，2022年12月29日，新建函数
///
void ise::UiIseControlHandler::NotifyIseCaliAppUpdate(const std::vector< ::ise::tf::IseModuleInfo> & imis)
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);
    // 发送更新信息到UI消息总线
    POST_MESSAGE(MSG_ID_ISE_CALI_APP_UPDATE, imis);
}

void ise::UiIseControlHandler::NotifySupplyInfoUpdate(const std::vector<::ise::tf::SupplyUpdate> & supplyUpdates)
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
	POST_MESSAGE(MSG_ID_ISE_INFO_UPDATE, supplyUpdates);
}
