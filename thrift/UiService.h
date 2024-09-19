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
/// @file     UiService.h
/// @brief    界面层thrift服务类
///
/// @author   3558/ZhouGuangMing
/// @date     2021年3月19日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 3558/ZhouGuangMing，2021年3月19日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <memory>
#include <boost/noncopyable.hpp>
#include <thrift/server/TServer.h>
#include <thrift/server/TNonblockingServer.h>
#include <thrift/concurrency/ThreadManager.h>
using namespace std;

///
/// @brief
///     界面层thrift服务类
///
class UiService : public boost::noncopyable
{
public:
    ///
    /// @brief
    ///     析构函数
    ///
	~UiService();

    ///
    /// @brief
    ///     获得单例对象
    ///
    /// @return 界面层thrift服务单例
    ///
    /// @par History:
    /// @li 3558/ZhouGuangMing，2021年3月18日，新建函数
    ///
    static std::shared_ptr<UiService> GetInstance();

    ///
    /// @brief
    ///     开启服务
    ///
    /// @param[in]  usPort  thrift服务监听端口
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 3558/ZhouGuangMing，2021年3月18日，新建函数
    ///
    bool Start(unsigned short usPort);

    ///
    /// @brief
    ///     停止服务
    ///
    /// @par History:
    /// @li 3558/ZhouGuangMing，2021年3月18日，新建函数
    ///
    void Stop();

private:
    static std::shared_ptr<UiService>                           s_us;                   ///< 单例对象
    std::shared_ptr<std::thread>                                m_spServiceThread;      ///< DCS服务线程
    std::shared_ptr<apache::thrift::server::TNonblockingServer> m_spServer;             ///< thrift服务器
    std::shared_ptr<apache::thrift::concurrency::ThreadManager> m_spThreadManager;      ///< 线程管理器
};
