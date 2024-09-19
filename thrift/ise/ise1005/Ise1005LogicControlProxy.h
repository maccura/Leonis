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
/// @file     I6000LogicControlProxy.h
/// @brief    发送I6000业务逻辑控制命令的代理
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

#include "src/thrift/ise/ise1005/gen-cpp/Ise1005LogicControl.h"
#include <string>
#include <memory>
#include <boost/noncopyable.hpp>

namespace ise
{
namespace ise1005
{

///
/// @brief
///     发送I6000业务逻辑控制命令的代理
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
    static std::shared_ptr<ise::ise1005::tf::Ise1005LogicControlClient> GetIse1005LogicControlClient();

    ///
    /// @brief 修改ise的测试结果信息
    ///
    /// @param[in]  tr  测试结果
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年4月3日，新建函数
    ///
    static bool ModifyAssayTestResult(const  ::ise::tf::AssayTestResult& tr);

    ///
    /// @brief
    ///     查询项目测试结果
    ///
    /// @param[in]  qryResp  项目测试结果
    /// @param[in]  qryCond  样本查找条件
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年8月5日，新建函数
    ///
    static bool QueryAssayTestResult(ise::tf::AssayTestResultQueryResp& qryResp, ise::tf::AssayTestResultQueryCond& qryCond);

	///
	/// @brief 查询满足条件的耗材
	///
	/// @param[in]  _return  执行结果
	/// @param[in]  siqc	 查询条件
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年7月7日，新建函数
	///
	static void QuerySuppliesInfo(::ise::tf::SuppliesInfoQueryResp& _return, const  ::ise::tf::SuppliesInfoQueryCond& siqc);

	///
	/// @brief 手动录入耗材信息
	///
	/// @param[in]  si  耗材信息
	///
	/// @return 执行结果（成功，失败原因，见数据结构）
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年7月7日，新建函数
	///
	static  ::ise::tf::ManualRegisterResult::type ManualRegisterSupplies(const  ::ise::tf::SuppliesInfo& si);

	///
	/// @brief 下位机准备执行耗材加载
	///
	/// @param[in]  deviceSN  设备sn
	///
	/// @return true:加载准备成功，反之失败
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年9月5日，新建函数
	///
	static bool AskForLoadSupply(const std::string& deviceSN);

	///
	/// @brief 下位机执行耗材加载
	///
	/// @param[in]  supplyPos			耗材位置
	/// @param[in]  fillPipeTime		管路填充次数
	/// @param[in]  switchRunAfterMaintain  是否运行态
	///
	/// @return true:加载成功，反之失败
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年9月5日，新建函数
	///
	static bool LoadSupply(const  ::ise::tf::SupplyPosInfo& supplyPos, const int32_t fillPipeTime, const bool switchRunAfterMaintain);

    ///
    /// @brief 添加Ise校准曲线
    ///
    /// @param[in]  _return  执行结果
    /// @param[in]  cc  校准曲线
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年12月26日，新建函数
    ///
    static void AddIseCaliCurve(::tf::ResultLong& _return, const  ::ise::tf::CaliCurve& cc);

    ///
    /// @brief 删除满足条件的Ise校准曲线
    ///
    /// @param[in]  iccq  查询条件
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年12月26日，新建函数
    ///
    static bool DeleteIseCaliCurve(const  ::ise::tf::IseCaliCurveQueryCond& iccq);

    ///
    /// @brief 修改Ise校准曲线
    ///
    /// @param[in]  cc  校准曲线
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年12月26日，新建函数
    ///
    static bool ModifyIseCaliCurve(const  ::ise::tf::CaliCurve& cc);

    ///
    /// @brief 查询满足条件的Ise校准曲线
    ///
    /// @param[in]  _return  执行结果
    /// @param[in]  iccq  查询条件
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年12月26日，新建函数
    ///
    static void QueryIseCaliCurve(::ise::tf::IseCaliCurveQueryResp& _return, const  ::ise::tf::IseCaliCurveQueryCond& iccq);

public:
    static unsigned short           m_usPort;                   ///< 端口号
};

}
}
