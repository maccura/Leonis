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
/// @file     I6000LogicControlProxy.cpp
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
#include "Ise1005LogicControlProxy.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/defs.h"
#include "src/dcs/interface/ise/ise1005/Ise1005LogicControlHandler.h"
#include "src/thrift/ise/ise1005/gen-cpp/ise1005_logic_control_constants.h"
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TCompactProtocol.h>
#include <thrift/protocol/TMultiplexedProtocol.h>
#include <stdexcept>

///
/// @brief
///     端口号
///
unsigned short                      ise::ise1005::LogicControlProxy::m_usPort = DCS_CONTROL_PORT;

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
std::shared_ptr<ise::ise1005::tf::Ise1005LogicControlClient> ise::ise1005::LogicControlProxy::GetIse1005LogicControlClient()
{
    std::shared_ptr<apache::thrift::transport::TTransport> spSocket(new apache::thrift::transport::TSocket("localhost", m_usPort));
    std::shared_ptr<apache::thrift::transport::TTransport> spTransport(new apache::thrift::transport::TFramedTransport(spSocket));
    std::shared_ptr<apache::thrift::protocol::TProtocol> spProtocol(new apache::thrift::protocol::TCompactProtocol(spTransport));
    std::shared_ptr<apache::thrift::protocol::TMultiplexedProtocol> spMprotocol(new apache::thrift::protocol::TMultiplexedProtocol(spProtocol, ise::ise1005::tf::g_ise1005_logic_control_constants.SERVICE_NAME));
    std::shared_ptr<ise::ise1005::tf::Ise1005LogicControlClient> spClient(new ise::ise1005::tf::Ise1005LogicControlClient(spMprotocol));

    // 连接服务器
    try
    {
        spTransport->open();
    }
    catch (std::exception& ex)
    {
        ULOG(LOG_ERROR, "thrift error: %s", ex.what());
        return nullptr;
    }
    catch (...)
    {
        ULOG(LOG_ERROR, "thrift error: catch unknown exception");
        return nullptr;
    }

    return spClient;
}

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
bool ise::ise1005::LogicControlProxy::ModifyAssayTestResult(const  ::ise::tf::AssayTestResult& tr)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ise::ise1005::LogicControlHandler logicControl;
    return logicControl.ModifyAssayTestResult(tr);
}

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
bool ise::ise1005::LogicControlProxy::QueryAssayTestResult(ise::tf::AssayTestResultQueryResp& qryResp
	, ise::tf::AssayTestResultQueryCond& qryCond)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ise::ise1005::LogicControlHandler logicControl;
	logicControl.QueryAssayTestResult(qryResp, qryCond);
	if (qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

	return true;
}

///
/// @brief 查询满足条件的耗材
///
/// @param[in]  _return  执行结果
/// @param[in]  siqc	 查询条件
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月7日，新建函数
///
void ise::ise1005::LogicControlProxy::QuerySuppliesInfo(::ise::tf::SuppliesInfoQueryResp& _return
	, const ::ise::tf::SuppliesInfoQueryCond& siqc)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ise::ise1005::LogicControlHandler logicControl;
	logicControl.QuerySuppliesInfo(_return, siqc);
}

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
::ise::tf::ManualRegisterResult::type ise::ise1005::LogicControlProxy::ManualRegisterSupplies(
	const ::ise::tf::SuppliesInfo& si)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ise::ise1005::LogicControlHandler logicControl;
	return logicControl.ManualRegisterSupplies(si);
}

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
bool ise::ise1005::LogicControlProxy::AskForLoadSupply(const std::string& deviceSN)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ise::ise1005::LogicControlHandler logicControl;
	return logicControl.AskForLoadSupply(deviceSN);
}

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
bool ise::ise1005::LogicControlProxy::LoadSupply(const ::ise::tf::SupplyPosInfo& supplyPos
	, const int32_t fillPipeTime, const bool switchRunAfterMaintain)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	
	ise::ise1005::LogicControlHandler logicControl;
	return logicControl.LoadSupply(supplyPos, fillPipeTime, switchRunAfterMaintain);
}

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
void ise::ise1005::LogicControlProxy::AddIseCaliCurve(::tf::ResultLong & _return, const::ise::tf::CaliCurve & cc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ise::ise1005::LogicControlHandler logicControl;
	logicControl.AddIseCaliCurve(_return, cc);
}

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
bool ise::ise1005::LogicControlProxy::DeleteIseCaliCurve(const::ise::tf::IseCaliCurveQueryCond & iccq)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ise::ise1005::LogicControlHandler logicControl;
    return logicControl.DeleteIseCaliCurve(iccq);
}

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
bool ise::ise1005::LogicControlProxy::ModifyIseCaliCurve(const::ise::tf::CaliCurve & cc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ise::ise1005::LogicControlHandler logicControl;
    return logicControl.ModifyIseCaliCurve(cc);
}

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
void ise::ise1005::LogicControlProxy::QueryIseCaliCurve(::ise::tf::IseCaliCurveQueryResp & _return
	, const::ise::tf::IseCaliCurveQueryCond & iccq)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ise::ise1005::LogicControlHandler logicControl;
	logicControl.QueryIseCaliCurve(_return, iccq);
}
