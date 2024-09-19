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
#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <boost/noncopyable.hpp>
#include <boost/property_tree/ptree.hpp>
using namespace std;

///
/// @brief
///     系统配置管理
///
class SystemConfigManager: public boost::noncopyable
{
public:
    ///
    /// @brief
    ///     获得单例对象
    ///
    /// @return 系统配置单例对象
    ///
    /// @par History:
    /// @li 3558/ZhouGuangMing，2021年3月19日，新建函数
    ///
    static std::shared_ptr<SystemConfigManager> GetInstance();

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
    bool LoadConfig(const string& strFilePath);

    ///
    /// @brief
    ///     获得UI的thrift上报端口
    ///
    /// @return 端口号
    ///
    /// @par History:
    /// @li 3558/ZhouGuangMing，2021年3月19日，新建函数
    ///
    unsigned short GetUiReportPort() const;

    ///
    /// @brief
    ///     获得DCS的thrift服务端口
    ///
    /// @return 端口号
    ///
    /// @par History:
    /// @li 3558/ZhouGuangMing，2020年5月28日，新建函数
    ///
    unsigned short GetDcsControlPort() const;

	///
	/// @brief 是否全屏显示
	///
	///
	/// @return 成功与否
	///
	/// @par History:
	/// @li 6950/ChenFei，2022年6月14日，新建函数
	///
	bool IsShowFullScreen() const;

	///
	/// @brief 是否调试启动
	///
	/// @par History:
	/// @li 6950/ChenFei，2022年6月16日，新建函数
	///
	bool IsDebugMode() const;

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
    bool VerifyReg();

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
    bool GetNeedRegister();

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
    void SetNeedRegister(bool val);

protected:
    ///
    /// @brief
    ///     构造函数
    ///
    /// @par History:
    /// @li 3558/ZhouGuangMing，2021年3月19日，新建函数
    ///
    SystemConfigManager();

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
    bool LoadThriftConfig(const boost::property_tree::ptree& pt);

private:
    static std::shared_ptr<SystemConfigManager> s_instance;     ///< 单例对象
    static std::recursive_mutex     s_mtx;                      ///< 保护单例对象的互斥量

    unsigned short                  m_usUiReportPort;           ///< UI的thrift上报端口
    unsigned short                  m_usDcsControl;             ///< DCS的thrift服务端口
    
    bool                            m_bNeedRegister;            //< （为公司内部增加的判断）软件是否需要注册，true表示需要，false表示不需要
};
