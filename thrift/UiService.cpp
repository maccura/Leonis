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
/// @file     UiService.cpp
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
#include "UiService.h"
#include "UiControlHandler.h"
#include "thrift/ch/UiChControlHandler.h"
#include "thrift/ch/c1005/C1005UiControlHandler.h"
#include "thrift/im/i6000/I6000UiControlHandler.h"
#include "thrift/ise/UiIseControlHandler.h"
#include "src/common/Mlog/mlog.h"
#include "src/thrift/gen-cpp/ui_control_constants.h"
#include "src/thrift/ch/gen-cpp/ch_ui_control_constants.h"
#include "src/thrift/ch/c1005/gen-cpp/c1005_ui_control_constants.h"
#include "src/thrift/im/i6000/gen-cpp/i6000_ui_control_constants.h"
#include "src/thrift/ise/gen-cpp/ise_ui_control_constants.h"
#include <thrift/concurrency/ThreadFactory.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/server/TNonblockingServer.h>
#include <thrift/protocol/TCompactProtocol.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TNonblockingServerSocket.h>
#include <thrift/processor/TMultiplexedProcessor.h>

///
/// @brief 初始化单例对象
///
std::shared_ptr<UiService>    UiService::s_us(new UiService);

UiService::~UiService()
{
    Stop();
}

///
/// @brief
///     获得单例对象
///
/// @return 界面层thrift服务单例
///
/// @par History:
/// @li 3558/ZhouGuangMing，2021年3月18日，新建函数
///
std::shared_ptr<UiService> UiService::GetInstance()
{
    return s_us;
}

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
bool UiService::Start(unsigned short usPort)
{
    ULOG(LOG_INFO, "%s(%d)", __FUNCTION__, usPort);

    // 获得CUP核数
    unsigned int hw_threads = std::thread::hardware_concurrency();
    int io_threads = hw_threads / 2 + 1;
    int worker_threads = hw_threads * 2 + 2;

    // 创建多路复用的事务处理器
    std::shared_ptr<apache::thrift::TMultiplexedProcessor> mprocessor(new apache::thrift::TMultiplexedProcessor);
    // ui控制接口
    std::shared_ptr<UiControlHandler> uiHandler(new UiControlHandler());
    std::shared_ptr<apache::thrift::TProcessor> uiProcessor(new tf::UiControlProcessor(uiHandler));
    mprocessor->registerProcessor(::tf::g_ui_control_constants.SERVICE_NAME, uiProcessor);

    // 生化ui控制接口
    std::shared_ptr<UiChControlHandler> uiChHandler(new UiChControlHandler());
    std::shared_ptr<apache::thrift::TProcessor> chUiProcessor(new ch::tf::ChUiControlProcessor(uiChHandler));
    mprocessor->registerProcessor(::ch::tf::g_ch_ui_control_constants.SERVICE_NAME, chUiProcessor);

    // c1005的ui控制接口
    std::shared_ptr<ch::c1005::UiControlHandler> c1005UiHandler(new ch::c1005::UiControlHandler());
    std::shared_ptr<apache::thrift::TProcessor> c1005UiProcessor(new ch::c1005::tf::C1005UiControlProcessor(c1005UiHandler));
    mprocessor->registerProcessor(::ch::c1005::tf::g_c1005_ui_control_constants.SERVICE_NAME, c1005UiProcessor);

	// i6000的ui控制接口
	std::shared_ptr<im::i6000::I6000UiControlHandler> i6000UiHandler(new im::i6000::I6000UiControlHandler());
	std::shared_ptr<apache::thrift::TProcessor> i6000UiProcessor(new im::i6000::tf::i6000UiControlProcessor(i6000UiHandler));
	mprocessor->registerProcessor(::im::i6000::tf::g_i6000_ui_control_constants.SERVICE_NAME, i6000UiProcessor);

    // ise的ui控制接口
    std::shared_ptr<ise::UiIseControlHandler> iseUiHandler(new ise::UiIseControlHandler());
    std::shared_ptr<apache::thrift::TProcessor> iseUiProcessor(new ise::tf::IseUiControlProcessor(iseUiHandler));
    mprocessor->registerProcessor(::ise::tf::g_ise_ui_control_constants.SERVICE_NAME, iseUiProcessor);

    // 创建多线程管理器
    std::shared_ptr<apache::thrift::concurrency::ThreadFactory> thread_fac(new apache::thrift::concurrency::ThreadFactory());
    m_spThreadManager = apache::thrift::concurrency::ThreadManager::newSimpleThreadManager(worker_threads);
    m_spThreadManager->threadFactory(thread_fac);
    m_spThreadManager->start();

    // 创建非阻塞模式的服务器
    std::shared_ptr<apache::thrift::transport::TNonblockingServerSocket> tran_ep(new apache::thrift::transport::TNonblockingServerSocket(usPort));
    std::shared_ptr<apache::thrift::protocol::TProtocolFactory> proto_fac(
        new apache::thrift::protocol::TCompactProtocolFactoryT<apache::thrift::transport::TFramedTransport>());
    m_spServer.reset(new apache::thrift::server::TNonblockingServer(mprocessor, proto_fac, tran_ep, m_spThreadManager));

    // 开始服务
    ULOG(LOG_INFO, "Starting the server...");
    m_spServer->setNumIOThreads(io_threads);
    m_spServiceThread.reset(new std::thread([this]()
    {
        try
        {
            m_spServer->serve();
            ULOG(LOG_INFO, "Service complete exit");
        }
        catch (exception& ex)
        {
            ULOG(LOG_ERROR, "exception: %s", ex.what());
        }
        catch (...)
        {
            ULOG(LOG_ERROR, "exception: unknown thrift exception");
        }

		ULOG(LOG_INFO, "Service complete exit");
    }));

    return true;
}

///
/// @brief 停止服务
///
/// @attention 由于此函数会被单例对象的析构函数调用，所以不要调用MLOG打印任何日志
///
/// @par History:
/// @li 3558/ZhouGuangMing，2021年3月18日，新建函数
///
void UiService::Stop()
{
    // 检查服务是否开启
    if (nullptr == m_spServer)
    {
        return;
    }

    // 停止服务
    m_spServer->stop();
    m_spThreadManager->stop();

    // 等待thrift服务线程结束
    if ((nullptr != m_spServiceThread) && m_spServiceThread->joinable())
    {
        m_spServiceThread->join();
        m_spServiceThread = nullptr;
    }

    m_spThreadManager = nullptr;
    m_spServer = nullptr;
}
