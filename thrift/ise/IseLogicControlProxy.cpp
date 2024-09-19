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
/// @file     IseLogicControlProxy.cpp
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
#include "IseLogicControlProxy.h"
#include "src/common/Mlog/mlog.h"
#include "src/dcs/interface/ise/IseLogicControlHandler.h"
#include "src/thrift/Ise/gen-cpp/Ise_logic_control_constants.h"
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TCompactProtocol.h>
#include <thrift/protocol/TMultiplexedProtocol.h>
#include <stdexcept>

///
/// @brief
///     端口号
///
unsigned short                      ise::LogicControlProxy::m_usPort;

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
std::shared_ptr<ise::tf::IseLogicControlClient> ise::LogicControlProxy::GetIseLogicControlClient()
{
    std::shared_ptr<apache::thrift::transport::TTransport> spSocket(new apache::thrift::transport::TSocket("localhost", m_usPort));
    std::shared_ptr<apache::thrift::transport::TTransport> spTransport(new apache::thrift::transport::TFramedTransport(spSocket));
    std::shared_ptr<apache::thrift::protocol::TProtocol> spProtocol(new apache::thrift::protocol::TCompactProtocol(spTransport));
    std::shared_ptr<apache::thrift::protocol::TMultiplexedProtocol> spMprotocol(new apache::thrift::protocol::TMultiplexedProtocol(spProtocol, ise::tf::g_ise_logic_control_constants.SERVICE_NAME));
    std::shared_ptr<ise::tf::IseLogicControlClient> spClient(new ise::tf::IseLogicControlClient(spMprotocol));

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
/// @brief
///     添加ISE通用项目参数
///
/// @param[out]  _return    执行结果
/// @param[in]  gai         ISE通用项目参数
///
/// @par History:
/// @li 5774/WuHongTao，2021年7月8日，新建函数
///
void ise::LogicControlProxy::AddGeneralAssayInfo(::tf::ResultLong& _return, const ::ise::tf::GeneralAssayInfo& gai)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ise::LogicControlHandler logicControl;
	logicControl.AddGeneralAssayInfo(_return, gai);
}

///
/// @brief
///     删除ISE通用项目参数
///
/// @param[in]  gaiq       查询条件
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2021年7月8日，新建函数
///
bool ise::LogicControlProxy::DeleteGeneralAssayInfo(const ::ise::tf::GeneralAssayInfoQueryCond& gaiq)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ise::LogicControlHandler logicControl;
	return logicControl.DeleteGeneralAssayInfo(gaiq);
}

///
/// @brief
///     修改ISE通用项目参数
///
/// @param[in]  aci     新的ISE通用项目参数
///
/// @return true表示成功
///
/// @par History:
/// @li 3558/ZhouGuangMing，2021年4月7日，新建函数
///
bool ise::LogicControlProxy::ModifyAssayConfigInfo(const ise::tf::GeneralAssayInfo& aci)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ise::LogicControlHandler logicControl;
    return logicControl.ModifyGeneralAssayInfo(aci);
}

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
bool ise::LogicControlProxy::QueryAssayConfigInfo(const ::ise::tf::GeneralAssayInfoQueryCond& qryCond, ::ise::tf::GeneralAssayInfoQueryResp& qryResp)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ise::LogicControlHandler logicControl;
	logicControl.QueryGeneralAssayInfo(qryResp, qryCond);
	if (qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

	return true;
}

///
/// @brief
///     添加ISE特殊项目参数
///
/// @param[out]  _return 执行结果
/// @param[in]  sai      ISE特殊项目参数
///
/// @par History:
/// @li 5774/WuHongTao，2021年7月8日，新建函数
///
void ise::LogicControlProxy::AddSpecialAssayInfo(::tf::ResultLong& _return, const ::ise::tf::SpecialAssayInfo& sai)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ise::LogicControlHandler logicControl;
	logicControl.AddSpecialAssayInfo(_return, sai);
}

///
/// @brief
///     删除ISE特殊项目参数
///
/// @param[in]  saiq  查询条件
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2021年7月8日，新建函数
///
bool ise::LogicControlProxy::DeleteSpecialAssayInfo(const ::ise::tf::SpecialAssayInfoQueryCond& saiq)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ise::LogicControlHandler logicControl;
    return logicControl.DeleteSpecialAssayInfo(saiq);
}

///
/// @brief
///     修改ISE特殊项目参数
///
/// @param[in]  sai   新的ISE特殊项目参数
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2021年7月8日，新建函数
///
bool ise::LogicControlProxy::ModifySpecialAssayInfo(const ::ise::tf::SpecialAssayInfo& sai)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ise::LogicControlHandler logicControl;
    return logicControl.ModifySpecialAssayInfo(sai);
}

///
/// @brief
///     查询满足条件的ISE特殊项目参数
///
/// @param[out] _return 执行结果
/// @param[in]  saiq    ISE特殊项目参数
///
/// @par History:
/// @li 5774/WuHongTao，2021年7月8日，新建函数
///
bool ise::LogicControlProxy::QuerySpecialAssayInfo(::ise::tf::SpecialAssayInfoQueryResp& _return, const ::ise::tf::SpecialAssayInfoQueryCond& saiq)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ise::LogicControlHandler logicControl;
	logicControl.QuerySpecialAssayInfo(_return, saiq);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

    return true;
}

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
bool ise::LogicControlProxy::EncodeIseAssayInfo(std::string& strAssayCode, const ::ise::tf::IseAssayInfoList& iaiList)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ise::LogicControlHandler logicControl;
	logicControl.EncodeIseAssayInfo(strAssayCode, iaiList);
	
	return true;
}

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
bool ise::LogicControlProxy::DecodeIseAssayInfo(const std::string& strAssayCode, ::ise::tf::IseAssayInfoList& iaiList)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ise::LogicControlHandler logicControl;
	logicControl.DecodeIseAssayInfo(iaiList, strAssayCode);

	return true;
}

///
/// @brief 添加耗材属性信息
///
/// @param[in]  _return  执行结果
/// @param[in]  sa		 耗材属性信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月4日，新建函数
///
void ise::LogicControlProxy::AddSupplyAttribute(::tf::ResultLong& _return, const ::ise::tf::SupplyAttribute& sa)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ise::LogicControlHandler logicControl;
	logicControl.AddSupplyAttribute(_return, sa);
}

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
bool ise::LogicControlProxy::DeleteSupplyAttribute(const ::ise::tf::SupplyAttributeQueryCond& saq)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ise::LogicControlHandler logicControl;
	return logicControl.DeleteSupplyAttribute(saq);
}

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
bool ise::LogicControlProxy::ModifySupplyAttribute(const ::ise::tf::SupplyAttribute& sa)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ise::LogicControlHandler logicControl;
	return logicControl.ModifySupplyAttribute(sa);
}

///
/// @brief 查询条件指定的耗材属性
///
/// @param[in]  _return  执行结果
/// @param[in]  saq		 耗材查询条件
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月4日，新建函数
///
bool ise::LogicControlProxy::QuerySupplyAttribute(::ise::tf::SupplyAttributeQueryResp& _return, const ::ise::tf::SupplyAttributeQueryCond& saq)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ise::LogicControlHandler logicControl;
	logicControl.QuerySupplyAttribute(_return, saq);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

    return true;
}

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
void ise::LogicControlProxy::AddIseModuleInfo(::tf::ResultLong & _return, const::ise::tf::IseModuleInfo & imi)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ise::LogicControlHandler logicControl;
	logicControl.AddIseModuleInfo(_return, imi);
}

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
bool ise::LogicControlProxy::DeleteIseModuleInfo(const::ise::tf::IseModuleInfoQueryCond & imiq)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ise::LogicControlHandler logicControl;
    return logicControl.DeleteIseModuleInfo(imiq);
}

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
bool ise::LogicControlProxy::ModifyIseModuleInfo(const::ise::tf::IseModuleInfo & imi)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ise::LogicControlHandler logicControl;
    return logicControl.ModifyIseModuleInfo(imi);
}

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
void ise::LogicControlProxy::QueryIseModuleInfo(::ise::tf::IseModuleInfoQueryResp & _return, const::ise::tf::IseModuleInfoQueryCond & imiq)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	ise::LogicControlHandler logicControl;
    logicControl.QueryIseModuleInfo(_return, imiq);
}
