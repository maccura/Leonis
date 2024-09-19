/***************************************************************************
*   This file is part of the BiochemistryDeviceUser project               *
*   Copyright (C) 2024 by Mike Medical Electronics Co., Ltd               *
*   xuxiaolong@maccura.com                                                *
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
/// @file     ChNodeDebugConfigManager.h
/// @brief    生化节点调试配置管理
///
/// @author   7997/XuXiaoLong
/// @date     2024年07月23日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7997/XuXiaoLong，2024年07月23日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <mutex>
#include <boost/noncopyable.hpp>
#include <boost/property_tree/ptree.hpp>
#include "ChDebugStruct.h"
#include "src/common/RapidjsonUtil/RapidjsonUtil.h"

using namespace std;

class ChNodeDebugConfigManager : public boost::noncopyable
{
public:

    /// @brief   获得单例对象
    ///
    /// @return 系统配置单例对象
    ///
    /// @par History:
    /// @li 7997/XuXiaoLong，2023年02月17日，新建函数
    ///
    static std::shared_ptr<ChNodeDebugConfigManager> GetInstance();

    ///
    /// @brief   加载配置文件
    ///
    /// @param[in]  jsonFile  配置文件全路径
    ///
    /// @return true表示加载成功
    ///
    /// @par History:
    /// @li 7997/XuXiaoLong，2023年02月17日，新建函数
    ///
    bool LoadConfig(const string& jsonFile);

    ///
    /// @brief  获取指令参数列表
    ///
    /// @return 指令参数列表
    ///
    /// @par History:
    /// @li 7997/XuXiaoLong，2023年03月10日，新建函数
    ///
	QMap<int, Cmd> GetCmdMaps() const;

    ///
    /// @brief  获得DCS的thrift服务ip
    ///
    /// @return ip
    ///
    /// @par History:
    /// @li 7997/XuXiaoLong，2023年02月17日，新建函数
    ///
	QMap<int, ChNodeDebug> GetChNodeDebugMaps() const;

protected:
    ///
    /// @brief  构造函数
    ///
    /// @par History:
    /// @li 7997/XuXiaoLong，2023年02月17日，新建函数
    ///
	ChNodeDebugConfigManager();

	///
	/// @brief  解析指令列表
	///
	/// @param[in]  vCmd  指令列表对应的json对象
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年7月23日，新建函数
	///
	void ParseCmdList(const Value& vCmd);

	///
	/// @brief  解析步进模块列表
	///
	/// @param[in]  vStepModule  步进模块列表对应的json对象
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年7月23日，新建函数
	///
	void ParseStepModuleList(const Value& vStepModule);

	///
	/// @brief  解析开关模块列表
	///
	/// @param[in]  vSwitchModule  开关模块列表对应的json对象
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年7月23日，新建函数
	///
	void ParseSwitchModuleList(const Value& vSwitchModule);

	///
	/// @brief  解析超声混匀模块列表
	///
	/// @param[in]  vUltraMixModule  超声混匀模块列表对应的json对象
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年7月23日，新建函数
	///
	void ParseUltraMixModuleList(const Value& vUltraMixModule);

	///
	/// @brief  根据指令id和参数组装Cmd
	///
	/// @param[in]  cmdId  指令id
	/// @param[in]  params 参数
	///
	/// @return (true表示成功，Cmd)
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年4月27日，新建函数
	///
	pair<bool, Cmd> PackageCmd(const string &cmdId, const string& params);

	///
	/// @brief  转换指令参数
	///
	/// @param[in]  cmds  字符串指令
	///
	/// @return QVector类型的指令参数
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年7月23日，新建函数
	///
	QVector<CanSendData> ConvertCmds(const string &cmds);

	///
	/// @brief  转换参数，将string类型参数转换到输入的tf::StCmdParam中
	///
	/// @param[in]  type  参数类型
	/// @param[in]  strValue  string参数值
	/// @param[in]  outData  输出的参数结构体
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年7月24日，新建函数
	///
	void ConvertParam(const int type, const string& strValue, tf::StCmdParam& outData);

	///
	/// @brief  替换指令参数
	///
	/// @param[in]  vecParams  输入的指令参数列表
	/// @param[in]  cmd		   指令
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年7月24日，新建函数
	///
	void ReplaceCmdParam(const std::vector<std::string> &vecParams, Cmd &cmd);

private:
    static std::shared_ptr<ChNodeDebugConfigManager> s_instance;///< 单例对象
    static std::recursive_mutex     s_mtx;                      ///< 保护单例对象的互斥量

	QMap<int, Cmd>					m_mapCmds;					///< 指令参数列表（指令索引，指令参数）
	QMap<int, ChNodeDebug>			m_mapChNodeDebugInfos;		///< 生化节点调试数据（模块id，节点调试数据）
};
