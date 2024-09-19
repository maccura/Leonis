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
/// @file     messagebus.h
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

#pragma once

#include <QObject>
#include <QMap>
#include <QList>
#include <QReadWriteLock>

// 调用简化宏
#define REGISTER_HANDLER(msgId, obj, func)       MessageBus::Instance()->RegisterHandler(msgId, obj, #func)    // 注册消息处理例程
#define UNREGISTER_HANDLER(msgId, obj, func)     MessageBus::Instance()->UnRegisterHandler(msgId, obj, #func)  // 反注册消息处理例程
#define POST_MESSAGE(msgid, ...)                 MessageBus::Instance()->CustomPostMessage(msgid, ##__VA_ARGS__) // 投递消息(非阻塞)
#define SEND_MESSAGE(msgid, ...)                 MessageBus::Instance()->CustomSendMessage(msgid, ##__VA_ARGS__) // 发送消息(自动连接方式，可能阻塞)
#define QUEUE_SEND_MESSAGE(msgid, ...)           MessageBus::Instance()->CustomQueueSendMessage(msgid, ##__VA_ARGS__) // 发送消息(自动连接方式，可能阻塞)

// 槽信息
struct SlotInfo
{
    char*      slot;    // 槽函数名
    QObject*   obj;     // 目标对象

    bool operator==(const SlotInfo &v) const
    {
        return (v.obj == obj) && (strcmp(slot, v.slot) == 0);
    }
};

class MessageBus : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(MessageBus)

public:
    ~MessageBus();

    ///
    /// @brief 获取单例
    ///     
    /// @return 单实例
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月22日，新建函数
    ///
    static MessageBus* Instance();

    ///
    /// @brief 注册消息处理槽函数
    ///     
    /// @param[in]  iMsgId    // 消息id
    /// @param[in]  obj       // 目标对象
    /// @param[in]  slot      // 槽函数名
    ///
    /// @return ture表示成功,false表示失败
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月22日，新建函数
    ///
    bool RegisterHandler(int iMsgId, QObject* obj, const char* slot);

    ///
    /// @brief 反注册消息处理槽函数
    ///     
    /// @param[in]  iMsgId    // 消息id
    /// @param[in]  obj       // 目标对象
    /// @param[in]  slot      // 槽函数字符串
    ///
    /// @return ture表示成功,false表示失败
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月22日，新建函数
    ///
    bool UnRegisterHandler(int iMsgId, QObject* obj, const char* slot);

    ///
    /// @brief 投递消息
    ///     
    /// @param[in]  iMsgId   // 消息id
    /// @param[in]  args     // 处理函数参数包
    ///
    /// @return ture表示成功,false表示失败
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月23日，新建函数
    ///
    template<typename...ARGS>
    bool CustomPostMessage(int iMsgId, ARGS...args)
    {
        // 读锁
        QReadLocker lockGuard(&m_mapLock);

        // 注册所有类型
        RegisterType(args...);

        // 触发与之绑定同一事件的槽函数
        auto it_slot_find = m_EventIdAndSlotInfoMap.find(iMsgId);
        if (it_slot_find != m_EventIdAndSlotInfoMap.end())
        {
            for (const SlotInfo& slotInfo : it_slot_find.value())
            {
                QMetaObject::invokeMethod(slotInfo.obj, slotInfo.slot, Qt::QueuedConnection, QArgument<ARGS>(typeid(args).name(), args)...);
            }
        }
        return true;
    }

    ///
    /// @brief 发送消息
    ///     
    /// @param[in]  iMsgId   // 消息id
    /// @param[in]  args     // 处理函数参数包
    ///
    /// @return ture表示成功,false表示失败
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年6月19日，新建函数
    ///
    template<typename...ARGS>
    bool CustomSendMessage(int iMsgId, ARGS...args)
    {
        // 读锁
        QReadLocker lockGuard(&m_mapLock);

        // 注册所有类型
        RegisterType(args...);

        // 触发与之绑定同一事件的槽函数
        auto it_slot_find = m_EventIdAndSlotInfoMap.find(iMsgId);
        if (it_slot_find != m_EventIdAndSlotInfoMap.end())
        {
            for (const SlotInfo& slotInfo : it_slot_find.value())
            {
                QMetaObject::invokeMethod(slotInfo.obj, slotInfo.slot, Qt::AutoConnection, QArgument<ARGS>(typeid(args).name(), args)...);
            }
        }
        return true;
    }

    ///
    /// @brief 发送消息
    ///     
    /// @param[in]  iMsgId   // 消息id
    /// @param[in]  args     // 处理函数参数包
    ///
    /// @return ture表示成功,false表示失败
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年7月15日，新建函数
    ///
    template<typename...ARGS>
    bool CustomQueueSendMessage(int iMsgId, ARGS...args)
    {
        // 读锁
        QReadLocker lockGuard(&m_mapLock);

        // 注册所有类型
        RegisterType(args...);

        // 触发与之绑定同一事件的槽函数
        auto it_slot_find = m_EventIdAndSlotInfoMap.find(iMsgId);
        if (it_slot_find != m_EventIdAndSlotInfoMap.end())
        {
            for (const SlotInfo& slotInfo : it_slot_find.value())
            {
                QMetaObject::invokeMethod(slotInfo.obj, slotInfo.slot, Qt::BlockingQueuedConnection, QArgument<ARGS>(typeid(args).name(), args)...);
            }
        }
        return true;
    }

protected:
    MessageBus(QObject *parent = Q_NULLPTR);

    ///
    /// @brief qt元对象注册类型0参数定义
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月23日，新建函数
    ///
    void RegisterType()
    {
    }

    ///
    /// @brief qt元对象注册类型终止递归模板
    ///     
    /// @param[in]  arg  要注册类型的一个实例
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月23日，新建函数
    ///
    template<typename T>
    void RegisterType(T arg)
    {
        qRegisterMetaType<T>(typeid(arg).name());
    }

    ///
    /// @brief qt元对象注册类型变参模板
    ///     
    /// @param[in]  arg0  第一个参数
    /// @param[in]  args  参数包
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年4月23日，新建函数
    ///
    template<typename T, typename...ARGS>
    void RegisterType(T arg0, ARGS...args)
    {
        qRegisterMetaType<T>(typeid(arg0).name());
        RegisterType(args...);
    }

protected Q_SLOTS:
    ///
    /// @brief
    ///     当Qt对象被销毁
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年5月27日，新建函数
    ///
    void OnObjDestroyed();

private:
    QMap<int, QList<SlotInfo>>   m_EventIdAndSlotInfoMap;    // 事件Id与槽信息列表的映射
    QReadWriteLock               m_mapLock;                  // map容器读写锁
};
