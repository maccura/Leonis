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
/// @file     messagebus.cpp
/// @brief    界面消息总线
///
/// @author   4170/TangChuXian
/// @date     2020年4月24日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2020年4月24日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "messagebus.h"

MessageBus::MessageBus(QObject *parent)
    : QObject(parent)
{
}

MessageBus::~MessageBus()
{
}

///
/// @brief 获取单例
///     
/// @return 单实例
///
/// @par History:
/// @li 4170/TangChuXian，2020年4月22日，新建函数
///
MessageBus* MessageBus::Instance()
{
    static MessageBus s_obj;
    return &s_obj;
}

///
/// @brief 绑定事件和Qt槽函数
///     
/// @param[in]  iMsgId    // 消息id
/// @param[in]  obj       // 目标对象
/// @param[in]  slot      // 槽函数
///
/// @return ture表示成功,false表示失败
///
/// @par History:
/// @li 4170/TangChuXian，2020年4月22日，新建函数
///
bool MessageBus::RegisterHandler(int iMsgId, QObject* obj, const char* slot)
{
    // 写锁
    QWriteLocker lockGuard(&m_mapLock);

    // 将信息添加到槽map中
    auto it_slot_find = m_EventIdAndSlotInfoMap.find(iMsgId);
    if (it_slot_find == m_EventIdAndSlotInfoMap.end())
    {
        // 没有该事件id则添加
        m_EventIdAndSlotInfoMap.insert(iMsgId, QList<SlotInfo>());
    }

    // 构造信息并添加
    SlotInfo slotInfo;
    slotInfo.obj  = obj;
    slotInfo.slot = (char*)slot;
    m_EventIdAndSlotInfoMap[iMsgId].push_back(slotInfo);

    // 监听obj销毁信号
    connect(obj, SIGNAL(destroyed()), this, SLOT(OnObjDestroyed()));

    return true;
}

///
/// @brief 解除绑定事件和Qt槽函数
///     
/// @param[in]  iMsgId    // 消息id
/// @param[in]  obj       // 目标对象
/// @param[in]  slot      // 槽函数
///
/// @return ture表示成功,false表示失败
///
/// @par History:
/// @li 4170/TangChuXian，2020年4月22日，新建函数
///
bool MessageBus::UnRegisterHandler(int iMsgId, QObject* obj, const char* slot)
{
    // 写锁
    QWriteLocker lockGuard(&m_mapLock);

    // 将信息从槽map中移除
    auto it_slot_find = m_EventIdAndSlotInfoMap.find(iMsgId);
    if (it_slot_find != m_EventIdAndSlotInfoMap.end())
    {
        // 构造信息并移除
        SlotInfo slotInfo;
        slotInfo.obj = obj;
        slotInfo.slot = (char*)slot;
        m_EventIdAndSlotInfoMap[iMsgId].removeAll(slotInfo);
    }

    return true;
}


///
/// @brief
///     当Qt对象被销毁
///
/// @par History:
/// @li 4170/TangChuXian，2020年5月27日，新建函数
///
void MessageBus::OnObjDestroyed()
{
    // 写锁
    QWriteLocker lockGuard(&m_mapLock);

    // 移除发送者关注信息
    QObject* pSender = sender();
    for (auto it = m_EventIdAndSlotInfoMap.begin(); it != m_EventIdAndSlotInfoMap.end();)
    {
        for (auto itSlot = it.value().begin(); itSlot != it.value().end();)
        {
            if (itSlot->obj == pSender)
            {
                itSlot = it.value().erase(itSlot);
                continue;
            }

            ++itSlot;
        }

        if (it.value().isEmpty())
        {
            it = m_EventIdAndSlotInfoMap.erase(it);
            continue;
        }

        ++it;
    }
}
