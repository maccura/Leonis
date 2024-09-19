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
/// @file     IseLogicControlProxy.h
/// @brief    发送ISE通用业务逻辑控制命令的代理
///
/// @author   3558/ZhouGuangMing
/// @date     2021年4月7日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 3558/ZhouGuangMing，2021年4月7日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include "src/thrift/ise/gen-cpp/IseLogicControl.h"
#include "src/thrift/Ise/gen-cpp/Ise_types.h"
#include <string>
#include <memory>
#include <boost/noncopyable.hpp>

namespace ise
{

///
/// @brief
///     发送Ise通用业务逻辑控制命令的代理
///
class LogicControlProxy : public boost::noncopyable
{
public:

    ///
    /// @brief
    ///     获取thift客户端
    ///
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年9月29日，新建函数
    ///
    static std::shared_ptr<ise::tf::IseLogicControlClient> GetIseLogicControlClient();

    ///
    /// @brief
    ///     添加Ise通用项目参数
    ///
    /// @param[out]  _return   执行结果
    /// @param[in]  gai        Ise通用项目参数
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年7月8日，新建函数
    ///
    static void AddGeneralAssayInfo(::tf::ResultLong& _return, const ::ise::tf::GeneralAssayInfo& gai);

    ///
    /// @brief
    ///     删除Ise通用项目参数
    ///
    /// @param[in]  gaiq       查询条件
    ///
    /// @return true表示成功 
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年7月8日，新建函数
    ///
    static bool DeleteGeneralAssayInfo(const ::ise::tf::GeneralAssayInfoQueryCond& gaiq);

    ///
    /// @brief
    ///     修改项目信息
    ///
    /// @param[in]  aci     具体的修改信息
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 3558/ZhouGuangMing，2021年4月7日，新建函数
    ///
    static bool ModifyAssayConfigInfo(const ise::tf::GeneralAssayInfo& aci);

    ///
    /// @brief    
    ///     查询项目信息
    ///
    /// @param[in]  qryCond  查询条件
    /// @param[out] qryResp  查询结果
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 3558/ZhouGuangMing，2021年4月7日，新建函数
    ///
    static bool QueryAssayConfigInfo(const ::ise::tf::GeneralAssayInfoQueryCond& qryCond, ::ise::tf::GeneralAssayInfoQueryResp& qryResp);

    ///
    /// @brief
    ///     添加Ise特殊项目参数
    ///
    /// @param[out]  _return 执行结果
    /// @param[in]  sai      Ise特殊项目参数
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年7月8日，新建函数
    ///
    static void AddSpecialAssayInfo(::tf::ResultLong& _return, const ::ise::tf::SpecialAssayInfo& sai);

    ///
    /// @brief
    ///     删除Ise特殊项目参数
    ///
    /// @param[in]  saiq  查询条件
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年7月8日，新建函数
    ///
    static bool DeleteSpecialAssayInfo(const ::ise::tf::SpecialAssayInfoQueryCond& saiq);

    ///
    /// @brief
    ///     修改Ise特殊项目参数
    ///
    /// @param[in]  sai   新的Ise特殊项目参数
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年7月8日，新建函数
    ///
    static bool ModifySpecialAssayInfo(const ::ise::tf::SpecialAssayInfo& sai);

    ///
    /// @brief
    ///     查询满足条件的Ise特殊项目参数
    ///
    /// @param[out] _return 执行结果
    /// @param[in]  saiq    Ise特殊项目参数
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年7月8日，新建函数
    ///
    static bool QuerySpecialAssayInfo(::ise::tf::SpecialAssayInfoQueryResp& _return, const ::ise::tf::SpecialAssayInfoQueryCond& saiq);

	///
	/// @brief
	///     电解质项目信息编码
	///
	/// @param[in]  strAssayCode  编码后的字符串
	/// @param[in]  iaiList		  需要编码的项目信息
	///
	/// @return true表示成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月18日，新建函数
	///
	static bool EncodeIseAssayInfo(std::string& strAssayCode, const ::ise::tf::IseAssayInfoList& iaiList);

	///
	/// @brief
	///     电解质项目信息解码
	///
	/// @param[in]  strAssayCode  需要解码的字符串
	/// @param[in]  iaiList		  解码后的项目信息
	///
	/// @return true表示成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月18日，新建函数
	///
	static bool DecodeIseAssayInfo(const std::string& strAssayCode, ::ise::tf::IseAssayInfoList& iaiList);

	///
	/// @brief 添加耗材属性信息
	///
	/// @param[in]  _return  执行结果
	/// @param[in]  sa		 耗材属性信息
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年7月4日，新建函数
	///
	static void AddSupplyAttribute(::tf::ResultLong& _return, const  ::ise::tf::SupplyAttribute& sa);

	///
	/// @brief 删除条件指定的耗材属性
	///
	/// @param[in]  saq		指定条件
	///
	/// @return true表示成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年7月4日，新建函数
	///
	static bool DeleteSupplyAttribute(const  ::ise::tf::SupplyAttributeQueryCond& saq);

	///
	/// @brief 修改指定耗材属性信息
	///
	/// @param[in]  sa		耗材属性
	///
	/// @return true表示成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年7月4日，新建函数
	///
	static bool ModifySupplyAttribute(const  ::ise::tf::SupplyAttribute& sa);

	///
	/// @brief 查询条件指定的耗材属性
	///
	/// @param[in]  _return  执行结果
	/// @param[in]  saq		 耗材查询条件
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年7月4日，新建函数
	///
    static bool QuerySupplyAttribute(::ise::tf::SupplyAttributeQueryResp& _return, const  ::ise::tf::SupplyAttributeQueryCond& saq);

    ///
    /// @brief 添加Ise模块信息
    ///
    /// @param[in]  _return  执行结果
    /// @param[in]  imi  Ise模块信息
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年12月26日，新建函数
    ///
    static void AddIseModuleInfo(::tf::ResultLong& _return, const  ::ise::tf::IseModuleInfo& imi);

    ///
    /// @brief 删除满足条件的Ise模块信息
    ///
    /// @param[in]  imiq  查询条件
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年12月26日，新建函数
    ///
    static bool DeleteIseModuleInfo(const  ::ise::tf::IseModuleInfoQueryCond& imiq);

    ///
    /// @brief 修改Ise模块信息
    ///
    /// @param[in]  imi  ise模块信息
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年12月26日，新建函数
    ///
    static bool ModifyIseModuleInfo(const  ::ise::tf::IseModuleInfo& imi);

    ///
    /// @brief 查询满足条件的Ise模块信息
    ///
    /// @param[in]  _return  查询结果
    /// @param[in]  imiq  查询条件
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年12月26日，新建函数
    ///
    static void QueryIseModuleInfo(::ise::tf::IseModuleInfoQueryResp& _return, const  ::ise::tf::IseModuleInfoQueryCond& imiq);

public:
    static unsigned short           m_usPort;                   ///< 端口号
};

}
