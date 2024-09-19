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
#include "UiChControlHandler.h"
#include "src/common/defs.h"
#include "src/common/Mlog/mlog.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"


///
/// @brief 上报质控申请信息更新消息
///
/// @param[in]  sus  质控申请更新消息
///
/// @par History:
/// @li 5774/WuHongTao，2022年9月7日，新建函数
///
void UiChControlHandler::NotifyQcAppUpdate(const std::vector< ::ch::tf::QcApply> & sus)
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);
    // 发送质控申请更新信息到UI消息总线
    POST_MESSAGE(MSG_ID_QCAPP_INFO_UPDATE, sus);
}

///
/// @brief 校准文档组信息变更通知
///
/// @param[in]  infos  更新的校准品组信息
///
/// @par History:
/// @li 8090/YeHuaNing，2022年10月20日，新建函数
///
void UiChControlHandler::NotifyCaliDocGroupInfoUpdate(const std::vector<ch::tf::CaliGroupUpdate>& infos)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 将校准品组信息更新消息发送到UI消息总线
    POST_MESSAGE(MSG_ID_CALI_GRP_INFO_UPDATE, const_cast<std::vector<ch::tf::CaliGroupUpdate>&>(infos));
    
}

void UiChControlHandler::NotifyReactionCupInfoUpdate(const std::string& deviceSN,
    const std::vector<::ch::tf::ReactionCupHistoryInfo> & lrcs)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    POST_MESSAGE(MSG_ID_REACTION_CUP_INFO_UPDATE, QString::fromStdString(deviceSN),
        const_cast<std::vector<::ch::tf::ReactionCupHistoryInfo>&>(lrcs));
}
