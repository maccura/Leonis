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
/// @file     SystemConfigManager.h
/// @brief    系统配置管理
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
#include "SystemConfigManager.h"
#include "src/common/defs.h"
#include "src/common/Mlog/mlog.h"
#include <boost/filesystem.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/process/child.hpp>
#include <QMessageBox>
#include <QCoreApplication>
#include "src/common/common.h"
#include "third_party/regdll/include/regcom.hpp"

///
/// @brief 初始化单例对象
///
std::shared_ptr<SystemConfigManager>    SystemConfigManager::s_instance;
std::recursive_mutex                    SystemConfigManager::s_mtx;

///
/// @brief
///     获得单例对象
///
/// @return 系统配置单例对象
///
/// @par History:
/// @li 3558/ZhouGuangMing，2021年3月19日，新建函数
///
std::shared_ptr<SystemConfigManager> SystemConfigManager::GetInstance()
{
    // 检查是否存在单例对象
    if (NULL == s_instance)
    {
        std::unique_lock<std::recursive_mutex> autoLock(s_mtx);

        // 检查单例对象是否存在
        if (NULL == s_instance)
        {
            s_instance.reset(new SystemConfigManager);
        }
    }

    return s_instance;
}

///
/// @brief
///     加载配置文件
///
/// @param[in]  strFilePath  配置文件全路径
///
/// @return true表示加载成功
///
/// @par History:
/// @li 3558/ZhouGuangMing，2021年3月19日，新建函数
///
bool SystemConfigManager::LoadConfig(const string& strFilePath)
{
    ULOG(LOG_INFO, "%s(%s)", __FUNCTION__, strFilePath.c_str());

    // 检查文件是否存在
    if (!boost::filesystem::exists(strFilePath))
    {
        ULOG(LOG_ERROR, "config file [%s] not exist!", strFilePath.c_str());
        return false;
    }

    try
    {
        // 读取配置文件
        boost::property_tree::ptree pt;
        boost::property_tree::read_xml(strFilePath, pt);

        // 加载<thrift>子节点
        if (!LoadThriftConfig(pt))
        {
            ULOG(LOG_ERROR, "failed to execute LoadThriftConfig()");
            return false;
        }
    }
    catch (exception& ex)
    {
        ULOG(LOG_ERROR, "exception: %s", ex.what());
        return false;
    }
    catch (...)
    {
        ULOG(LOG_ERROR, "unknown exception");
        return false;
    }

    return true;
}

///
/// @brief
///     获得UI的thrift上报端口
///
/// @return 端口号
///
/// @par History:
/// @li 3558/ZhouGuangMing，2021年3月19日，新建函数
///
unsigned short SystemConfigManager::GetUiReportPort() const
{
    return m_usUiReportPort;
}

///
/// @brief
///     获得DCS的thrift服务端口
///
/// @return 端口号
///
/// @par History:
/// @li 3558/ZhouGuangMing，2020年5月28日，新建函数
///
unsigned short SystemConfigManager::GetDcsControlPort() const
{
    return m_usDcsControl;
}

///
/// @brief 是否全屏显示
///
///
/// @return 是则返回true
///
/// @par History:
/// @li 6950/ChenFei，2022年6月14日，新建函数
///
bool SystemConfigManager::IsShowFullScreen() const
{
	return !qApp->arguments().contains("--no-fullscreen");
}

///
/// @brief 是否调试启动
///
/// @par History:
/// @li 6950/ChenFei，2022年6月16日，新建函数
///
bool SystemConfigManager::IsDebugMode() const
{
	return qApp->arguments().contains("-debug");
}

///
/// @brief  判断软件是否注册
///
/// @param[in]    void
///
/// @return bool true:已注册 false:未注册
///
/// @par History:
/// @li 7915/LeiDingXiang，2024年4月8日，新建函数
///
bool SystemConfigManager::VerifyReg()
{
    // 内部使用的版本，判断根目录下有没有文件PlayAsGuest.txt，有该文件不需要注册
    string strFilePath = GetCurrentDir() + "/PlayAsGuest.txt";
    if (boost::filesystem::exists(strFilePath))
    {
        ULOG(LOG_INFO, "PlayAsGuest.");
        return true;
    }

    //判断是否需要注册
    if (!GetNeedRegister())
    {
        return true;
    }
    // 判断是否注册RegDll
    int iRet = 0;
    iRet = ::reg::regcom::verify_reg_code();
    if (iRet != 0)
    {
        QMessageBox::critical(nullptr, u8"错误", u8"软件未注册，请注册后重启软件");
        //获取错误码,提示软件需要注册
        ULOG(LOG_ERROR, "Device is not register,err type %d.", iRet);
        return false;
    }

    return true;
}

///
/// @brief  获取是否需要注册
///
/// @param[in]    void
///
/// @return bool
///
/// @par History:
/// @li 7915/LeiDingXiang，2024年4月8日，新建函数
///
bool SystemConfigManager::GetNeedRegister()
{
    return m_bNeedRegister;
}

///
/// @brief  设置是否需要注册
///
/// @param[in]    bool
///
/// @return void
///
/// @par History:
/// @li 7915/LeiDingXiang，2024年4月8日，新建函数
///
void SystemConfigManager::SetNeedRegister(bool val)
{
    m_bNeedRegister = val;
}

///
/// @brief
///     构造函数
///
/// @par History:
/// @li 3558/ZhouGuangMing，2021年3月19日，新建函数
///
SystemConfigManager::SystemConfigManager()
    : m_usUiReportPort(UI_REPORT_PORT)
    , m_usDcsControl(DCS_CONTROL_PORT)
    , m_bNeedRegister{true}
{
}

///
/// @brief
///     加载thrift相关的配置
///
/// @param[in]  pt  配置文件对象
///
/// @return true表示加载成功
///
/// @par History:
/// @li 3558/ZhouGuangMing，2021年3月19日，新建函数
///
bool SystemConfigManager::LoadThriftConfig(const boost::property_tree::ptree& pt)
{
    // UI的命令接收端口
    m_usUiReportPort = pt.get<unsigned short>("config.thrift.UiReport.port");
    // DCS的thrift服务端口
    m_usDcsControl = pt.get<unsigned short>("config.thrift.DcsControl.port");

    return true;
}
