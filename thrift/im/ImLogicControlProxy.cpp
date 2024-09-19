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
/// @file     ImLogicControlProxy.cpp
/// @brief    发送免疫通用业务逻辑控制命令的代理
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
#include "ImLogicControlProxy.h"
#include "src/common/Mlog/mlog.h"
#include "src/dcs/interface/im/ImLogicControlHandler.h"
#include "src/thrift/im/gen-cpp/im_logic_control_constants.h"
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TCompactProtocol.h>
#include <thrift/protocol/TMultiplexedProtocol.h>
#include <stdexcept>
#include "src/leonis/reagent/rgntnoticedatamng.h"

///
/// @brief
///     端口号
///
unsigned short                      im::LogicControlProxy::m_usPort;

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
std::shared_ptr<im::tf::ImLogicControlClient> im::LogicControlProxy::GetImLogicControlClient()
{
	auto pTSocket = new apache::thrift::transport::TSocket("localhost", m_usPort);
	// 设置连接、发送，接收超时时间(单位ms) add by Chenjianlin 20240713
	pTSocket->setConnTimeout(20000);
	pTSocket->setRecvTimeout(20000);
	pTSocket->setSendTimeout(20000);
	std::shared_ptr<apache::thrift::transport::TTransport> spSocket(pTSocket);	
	std::shared_ptr<apache::thrift::transport::TTransport> spTransport(new apache::thrift::transport::TFramedTransport(spSocket));
    std::shared_ptr<apache::thrift::protocol::TProtocol> spProtocol(new apache::thrift::protocol::TCompactProtocol(spTransport));
    std::shared_ptr<apache::thrift::protocol::TMultiplexedProtocol> spMprotocol(new apache::thrift::protocol::TMultiplexedProtocol(spProtocol, im::tf::g_im_logic_control_constants.SERVICE_NAME));
    std::shared_ptr<im::tf::ImLogicControlClient> spClient(new im::tf::ImLogicControlClient(spMprotocol));

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
///     添加免疫通用项目信息
///
/// @param[in]  _return    执行结果
/// @param[in]  gai        免疫通用项目信息
///
/// @par History:
/// @li 5774/WuHongTao，2021年7月8日，新建函数
///
void im::LogicControlProxy::AddGeneralAssayInfo(::tf::ResultLong& _return, const ::im::tf::GeneralAssayInfo& gai)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	logicControl.AddGeneralAssayInfo(_return, gai);
}

///
/// @brief
///     删除满足条件的免疫通用项目信息
///
/// @param[in]  gaiq  删除条件
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2021年7月8日，新建函数
///
bool im::LogicControlProxy::DeleteGeneralAssayInfo(const ::im::tf::GeneralAssayInfoQueryCond& gaiq)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
    return logicControl.DeleteGeneralAssayInfo(gaiq);
}

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
bool im::LogicControlProxy::ModifyGeneralAssayInfo(const ::im::tf::GeneralAssayInfo& aci)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
    return logicControl.ModifyGeneralAssayInfo(aci);
}

///
/// @brief
///     查询免疫通用项目信息
///
/// @param[out]  _return   执行结果
/// @param[in]  gaiq        免疫通用项目信息查询条件
///
/// @par History:
/// @li 7685/likai，2023年10月27日，新建函数
///
void im::LogicControlProxy::QueryImGeneralAssayInfo(::im::tf::GeneralAssayInfoQueryResp& _return
	, const ::im::tf::GeneralAssayInfoQueryCond& gaiq)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    
	im::LogicControlHandler logicControl;
	_return.__set_result(::tf::ThriftResult::THRIFT_RESULT_UNKNOWN_ERROR);
	logicControl.QueryGeneralAssayInfo(_return, gaiq);
}

///
/// @brief
///     添加试剂信息
///
/// @param[in]  _return   添加结果
/// @param[in]  rgntInfo  试剂信息
///
/// @par History:
/// @li 4170/TangChuXian，2022年11月1日，新建函数
///
void im::LogicControlProxy::AddReagentInfo(::tf::ResultLong& _return, const im::tf::ReagentInfoTable& rgntInfo)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	logicControl.AddReagentInfo(_return, rgntInfo);
}

///
/// @brief
///     查询试剂
///
/// @param[in]  qryResp  查询结果
/// @param[in]  qryCond  查询条件
///
/// @return 
///
/// @par History:
/// @li 1226/zhangjing，2022年5月9日，新建函数
///
bool im::LogicControlProxy::QueryReagentInfo(::im::tf::ReagentInfoTableQueryResp& qryResp
	, const  ::im::tf::ReagentInfoTableQueryCond& qryCond)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	logicControl.QueryReagentInfoTable(qryResp, qryCond);
	if (qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

    return true;
}


///
/// @brief
///     查询试剂(包含在备用关系，曲线有效期等信息)
///
/// @param[in]  qryResp  查询结果
/// @param[in]  qryCond  查询条件
///
///
/// @par History:
/// @li 1226/zhangjing，2022年10月19日，新建函数
///
bool im::LogicControlProxy::QueryReagentInfoForUI(::im::tf::ReagentInfoTableQueryResp& qryResp
	, const  ::im::tf::ReagTableUIQueryCond& qryCond)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
    return RgntNoticeDataMng::GetInstance()->QueryReagentInfoForUI(qryResp, qryCond);
}

///
/// @brief
///     查询试剂(包含在备用关系，曲线有效期等信息)
///
/// @param[in]  qryResp  查询结果
/// @param[in]  qryCond  查询条件
///
/// @par History:
/// @li 1226/zhangjing，2022年10月19日，新建函数
///
bool im::LogicControlProxy::QueryReagentInfoForUIDirectly(::im::tf::ReagentInfoTableQueryResp& qryResp
	, const ::im::tf::ReagTableUIQueryCond& qryCond)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	logicControl.QueryReagentInfoForUI(qryResp, qryCond);
	if (qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

    return true;
}

///
/// @brief
///     修改试剂信息
///
/// @param[in]  reagInfo  试剂信息
///
/// @return true表示成功
///
/// @par History:
/// @li 1226/zhangjing，2022年5月9日，新建函数
///
bool im::LogicControlProxy::ModifyReagentInfo(const ::im::tf::ReagentInfoTable& reagInfo)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
    return logicControl.ModifyReagentInfo(reagInfo);
}

///
/// @brief
///     添加试剂信息
///
/// @param[in]  _return   添加结果
/// @param[in]  splInfo   耗材信息
///
/// @par History:
/// @li 4170/TangChuXian，2022年11月1日，新建函数
///
void im::LogicControlProxy::AddSuppliesInfo(::tf::ResultLong& _return, const im::tf::SuppliesInfoTable& splInfo)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	logicControl.AddSuppliesInfo(_return, splInfo);
}

///
/// @brief
///     查询耗材信息
///
/// @param[in]  qryResp  查询结果
/// @param[in]  qryCond  查询条件
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月29日，新建函数
///
bool im::LogicControlProxy::QuerySuppliesInfo(::im::tf::SuppliesInfoTableQueryResp& qryResp
	, const ::im::tf::SuppliesInfoTableQueryCond& qryCond)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    return RgntNoticeDataMng::GetInstance()->QuerySuppliesInfo(qryResp, qryCond);
}

///
/// @brief
///     查询耗材信息
///
/// @param[in]  qryResp  查询结果
/// @param[in]  qryCond  查询条件
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月29日，新建函数
///
bool im::LogicControlProxy::QuerySuppliesInfoDirectly(::im::tf::SuppliesInfoTableQueryResp& qryResp
	, const ::im::tf::SuppliesInfoTableQueryCond& qryCond)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	logicControl.QuerySuppliesInfoTable(qryResp, qryCond);
	if (qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

    return true;
}

///
/// @brief
///     修改耗材信息
///
/// @param[in]  splInfo   耗材信息
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2022年5月9日，新建函数
///
bool im::LogicControlProxy::ModifySuppliesInfo(const ::im::tf::SuppliesInfoTable& splInfo)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
    return logicControl.ModifySuppliesInfo(splInfo);
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
bool im::LogicControlProxy::QueryGeneralAssayInfo(const ::im::tf::GeneralAssayInfoQueryCond& qryCond, ::im::tf::GeneralAssayInfoQueryResp& qryResp)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	logicControl.QueryGeneralAssayInfo(qryResp, qryCond);
	if (qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

    return true;
}

///
/// @brief
///     添加校准品组文档
///
/// @param[in]  _return  执行结果
/// @param[in]  cdg  校准品组文档
///
/// @return 
///
/// @par History:
/// @li 1226/zhangjing，2022年5月9日，新建函数
///
void im::LogicControlProxy::AddCaliDocGroup(::tf::ResultLong& _return, const ::im::tf::CaliDocGroup& cdg)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	logicControl.AddCaliDocGroup(_return, cdg);
}

///
/// @brief
///     修改校准品组文档
///
/// @param[in]  cdg  校准品组文档
///
/// @return true表示成功
///
/// @par History:
/// @li 1226/zhangjing，2022年5月9日，新建函数
///
bool im::LogicControlProxy::ModifyCaliDocGroup(const ::im::tf::CaliDocGroup& cdg)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
    return logicControl.ModifyCaliDocGroup(cdg);
}


///
/// @brief
///     删除校准品组文档
///
/// @param[in]  cdgq  查询条件
///
/// @return true表示成功
///
/// @par History:
/// @li 1226/zhangjing，2022年5月9日，新建函数
///
bool im::LogicControlProxy::DeleteCaliDocGroup(const ::im::tf::CaliDocGroupQueryCond& cdgq)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
    return logicControl.DeleteCaliDocGroup(cdgq);
}

///
/// @brief
///     查询满足条件的校准品组文档
///
/// @param[in]  _return  保存查询结果的容器
/// @param[in]  cdgq  查询条件
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月8日，新建函数
///
/// @par History:
/// @li 1226/zhangjing，2022年5月9日，新建函数
///
bool im::LogicControlProxy::QueryCaliDocGroup(::im::tf::CaliDocGroupQueryResp& _return
	, const ::im::tf::CaliDocGroupQueryCond& cdgq)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	logicControl.QueryCaliDocGroup(_return, cdgq);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

    return true;
}


///
/// @brief
///     查询满足条件的校准品组文档
///
/// @param[in]  _return  保存查询结果的容器
/// @param[in]  cdgq  查询条件
///
/// @return true表示成功
///
/// @par History:
/// @li 1226/zhangjing，2022年12月13日，新建函数
///
bool im::LogicControlProxy::QueryCaliDoc(::im::tf::CaliDocQueryResp& _return, const ::im::tf::CaliDocQueryCond& cdgq)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	logicControl.QueryCaliDoc(_return, cdgq);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

	return true;
}


///
/// @brief 下载项目参数
///
/// @param[out]	_return  
/// @param[in]  dai		项目参数
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2022年5月12日，新建函数
///
bool im::LogicControlProxy::DownloadAssayInfo(::tf::ResultLong& _return, const im::tf::DownloadAssayInfo& dai)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	logicControl.DownloadAssayInfo(_return, dai);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

	return true;
}

///
/// @brief  解析校准品条码
///
/// @param[in]  _return  校准品信息
/// @param[in]  strBarcode  条码字符串
///
/// @return 是否成功解析
///
/// @par History:
/// @li 1226/zhangjing，2022年10月8日，新建函数
///
bool im::LogicControlProxy::ParseCaliGrpBarcodeInfo(tf::CaliDocGroup& _return, const std::string& strBarcode)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	logicControl.ParseCaliGrpBarcodeInfo(_return, strBarcode);
	if (!_return.__isset.assayCode || !_return.__isset.lot)
	{
		return false;
	}

	return true;
}

///
/// @brief
///     添加稀释液信息
///
/// @param[in]  _return  返回添加结果
/// @param[in]  dlt      稀释液信息
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月18日，新建函数
///
void im::LogicControlProxy::AddDiluentInfo(::tf::ResultLong& _return, const ::im::tf::DiluentInfoTable& dlt)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	logicControl.AddDiluentInfo(_return, dlt);
}

///
/// @brief
///     查询指定条件的稀释液信息
///
/// @param[in]  dltr  查询结果
/// @param[in]  dltc  查询条件
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月18日，新建函数
///
bool im::LogicControlProxy::QueryDiluentInfoTable(::im::tf::DiluentInfoTableQueryResp& dltr
	, const ::im::tf::DiluentInfoTableQueryCond& dltc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	logicControl.QueryDiluentInfoTable(dltr, dltc);
	if (dltr.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

    return true;
}

///
/// @brief
///     查询指定条件的稀释液信息
///
/// @param[in]  dltr  查询结果
/// @param[in]  dltc  查询条件
///
///
/// @par History:
/// @li 1226/zhangjing，2022年11月14日，新建函数
///
bool im::LogicControlProxy::QueryDiluentInfoForUI(::im::tf::DiluentInfoTableQueryResp& dltr
	, const  ::im::tf::ReagTableUIQueryCond& dltc)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
    return RgntNoticeDataMng::GetInstance()->QueryDiluentInfoForUI(dltr, dltc);
}

///
/// @brief
///     查询指定条件的稀释液信息
///
/// @param[in]  dltr  查询结果
/// @param[in]  dltc  查询条件
///
/// @par History:
/// @li 1226/zhangjing，2022年11月14日，新建函数
///
bool im::LogicControlProxy::QueryDiluentInfoForUIDirectly(::im::tf::DiluentInfoTableQueryResp& dltr
	, const ::im::tf::ReagTableUIQueryCond& dltc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	logicControl.QueryDiluentInfoForUI(dltr, dltc);
	if (dltr.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

    return true;
}

///
/// @brief
///     更新稀释液信息
///
/// @param[in]  dlt  更新的稀释液信息
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月18日，新建函数
///
bool im::LogicControlProxy::ModifyDiluentInfo(const ::im::tf::DiluentInfoTable& dlt)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
    return logicControl.ModifyDiluentInfo(dlt);
}

///
/// @brief
///     删除指定条件的稀释液信息
///
/// @param[in]  dltc  删除条件
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月18日，新建函数
///
bool im::LogicControlProxy::DeleteDiluentInfo(::tf::ResultLong& _return, const ::im::tf::DiluentInfoTableQueryCond& dltc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	logicControl.DeleteDiluentInfo(_return, dltc);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

	return true;
}


///
/// @brief  获取质控状态
///
/// @param[in]  deviceSN  模块号
/// @param[in]  id  质控组id
/// @param[in]  assaycode  通道号
/// @param[in]  rgntLot  试剂批号
/// @param[in]  rgntSn  试剂瓶号
///
/// @return 质控状态
///
/// @par History:
/// @li 1226/zhangjing，2022年10月31日，新建函数
///
im::tf::QcReason::type im::LogicControlProxy::GetQcApplyReason(const std::string& deviceSN, const int64_t id, const int32_t assaycode)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	return logicControl.GetQcApplyReason(deviceSN, id, assaycode);
}

///
/// @brief  新增质控申请
///
/// @param[in]  qa 质控申请
/// @param[in]  _return 是否成功
///
/// @par History:
/// @li 1226/zhangjing，2022年10月31日，新建函数
///
void im::LogicControlProxy::AddQcApply(::tf::ResultLong& _return, const im::tf::QcApply& qa)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	logicControl.AddQcApply(_return, qa);
}


///
/// @brief
///     删除指定条件的质控申请
///
/// @param[in]  dltc  删除条件
///
/// @return true表示成功
///
/// @par History:
/// @li 1226/zhangjing，2022年11月1日，新建函数
///
bool im::LogicControlProxy::DeleteQcApply(const ::im::tf::QcApplyQueryCond& dltc)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	return logicControl.DeleteQcApply(dltc);
}

///
/// @bref
///		修改质控申请
///
/// @param[in] qa 存在变化的质控申请对象
///
/// @par History:
/// @li 8276/huchunli, 2022年11月14日，新建函数
///
bool im::LogicControlProxy::ModifyQcApply(const im::tf::QcApply& qa)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	return logicControl.ModifyQcApply(qa);
}

///
/// @bref
///		查询质控申请
///
/// @param[in] dltc 查询条件
/// @param[out] ret 查询到的质控申请列表
///
/// @par History:
/// @li 8276/huchunli, 2022年11月16日，新建函数
///
void im::LogicControlProxy::QueryQcApply(const ::im::tf::QcApplyQueryCond& dltc, std::vector<::im::tf::QcApply>& ret)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	::im::tf::QcApplyQueryResp resp;
	logicControl.QueryQcApply(resp, dltc);
	if (resp.result == ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ret = std::move(resp.lstQcApplys);
	}
}

///
/// @brief
///     查询质控申请信息
///
/// @param[out] qarp  质控申请结果
/// @param[in]  qacd  质控申请查询条件
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月31日，新建函数
///
bool im::LogicControlProxy::QueryQcApply(::im::tf::QcApplyQueryResp& qarp, const ::im::tf::QcApplyQueryCond& qacd)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	logicControl.QueryQcApply(qarp, qacd);
	if (qarp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

	return true;
}

/// @brief
///     查询质控LJ图质控品信息
///
/// @param[in]  QcLjResp   质控LJ图质控新信息
/// @param[in]  QcRltCond  查询条件
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2022年11月15日，新建函数
///
bool im::LogicControlProxy::QueryQcLjAssay(::im::tf::QcLjAssayResp& QcLjResp, const ::im::tf::QcResultQueryCond& QcRltCond)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	logicControl.QueryQcLjAssay(QcLjResp, QcRltCond);
    if (QcLjResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        return false;
    }

    return true;
}

///
/// @brief
///     查询质控LJ图质控结果信息
///
/// @param[in]  QcRlt      质控结果信息
/// @param[in]  QcRltCond  查询条件
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2022年11月15日，新建函数
///
bool im::LogicControlProxy::QueryQcLjMaterial(::im::tf::QcLjMaterialResp& QcRlt, const ::im::tf::QcResultQueryCond& QcRltCond)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	logicControl.QueryQcLjMaterial(QcRlt, QcRltCond);
	if (QcRlt.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

    return true;
}

///
/// @brief
///     查询质控Youden图结果信息
///
/// @param[in]  QcRlt       查询到的质控结果
/// @param[in]  QcRltCond   查询条件
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月30日，新建函数
///
bool im::LogicControlProxy::QueryQcYouden(::im::tf::QcYoudenResp& QcRlt, const ::im::tf::QcResultQueryCond& QcRltCond)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	logicControl.QueryQcYouden(QcRlt, QcRltCond);
	if (QcRlt.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

    return true;
}

///
/// @brief
///     更新质控结果失控原因和处理措施
///
/// @param[in]  qcResultId  质控结果ID
/// @param[in]  cway        质控方式
/// @param[in]  reason      失控原因
/// @param[in]  solution    处理措施
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2022年11月15日，新建函数
///
bool im::LogicControlProxy::UpdateQcBreakReasonAndSolution(const int64_t qcResultId, const ::im::tf::controlWay::type cway, const std::string& reason, const std::string& solution)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
    return logicControl.UpdateQcBreakReasonAndSolution(qcResultId, cway, reason, solution);
}

///
/// @brief
///     更新质控数据的LJ排除点状态
///
/// @param[in]  qcResultId  质控结果数据库主键
/// @param[in]  cway        质控方式
/// @param[in]  exclude     是否排除计算
///
/// @par History:
/// @li 4170/TangChuXian，2022年11月16日，新建函数
///
bool im::LogicControlProxy::UpdateQcResultExclude(const int64_t qcResultId, const ::im::tf::controlWay::type cway, bool exclude)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
    return logicControl.UpdateQcResultExclude(qcResultId, cway, exclude);
}

///
///  @brief 发光剂流程
///    
///
///  @param[in]   itemType  发光剂流程维护项
///  @param[in]   lstParam  发光剂参数
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年12月9日，新建函数
///
bool im::LogicControlProxy::DetectModuleDebug(const  ::tf::MaintainItemType::type itemType, const std::vector<std::string> & lstDevSN, const std::vector<int32_t> & lstParam)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	return logicControl.DetectModuleDebug(itemType, lstDevSN, lstParam);
}


///
/// @brief
///     流程检查
///
/// @param[in]  itemType 流程检查维护项
/// @param[in]  lstDevSN 设备序号
/// @param[in]  lstParam 参数
///
/// @return 
///
/// @par History:
/// @li 4058/WangZhiNang，2024年1月15日，新建函数
///
bool im::LogicControlProxy::ProcessCheck(const  ::tf::MaintainItemType::type itemType, const std::vector<std::string> & lstDevSN, const std::vector<int32_t> & lstParam)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	return logicControl.ProcessCheck(itemType, lstDevSN, lstParam);
}

///
///  @brief 单项维护（指定位置试剂扫描）
///
///
///  @param[in]   itemType  维护项类型
///  @param[in]   deviceSN  设备序列号
///  @param[in]   lstParam  参数列表
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年2月22日，新建函数
///
bool im::LogicControlProxy::SingleMaintance(const ::tf::MaintainItemType::type itemType, const std::string & deviceSN, const std::vector<int32_t>& lstParam)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	return logicControl.SingleMaintance(itemType, deviceSN, lstParam);
}

///
/// @brief
///     解析反应杯耗材信息
///
/// @param[in]  CupRlt      解析结果
/// @param[in]  cupBarcode  反应杯条码
/// @param[in]  deviceSN    设备序列号
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月23日，新建函数
///
bool im::LogicControlProxy::ParseSupplyCupInfo(::im::tf::ParseSupplyeCupResp& CupRlt, const std::string& cupBarcode, const std::string& deviceSN)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	logicControl.ParseSupplyCupInfo(CupRlt, cupBarcode, deviceSN);
	if (CupRlt.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

	return true;
}

///
/// @brief
///     添加反应杯耗材
///
/// @param[out]  _return  返回结果
/// @param[in]    si      耗材信息
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月23日，新建函数
///
bool im::LogicControlProxy::AddSupplyCup(::tf::ResultLong& _return, const im::tf::SuppliesInfoTable& si)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	logicControl.AddSupplyCup(_return, si);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

    return true;
}

///
/// @brief  更新用户应用界面下耗材管理选择
///
/// @param[in]    UserConsumablesState:用户设置的耗材状态
///
/// @return bool
///
/// @par History:
/// @li 7915/LeiDingXiang，2023年2月4日，新建函数
///
bool im::LogicControlProxy::UpdateUtilityUserChose(const ::im::tf::UserConsumablesState& utilityConsumablesState)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    
	im::LogicControlHandler logicControl;
    return logicControl.UpdateUtilityUserChose(utilityConsumablesState);
}

///
/// @brief  查询试剂仓盖打开信息
///
/// @param[in]    deviceSN:设备序列号
///
/// @return bool
///
/// @par History:
/// @li 7915/LeiDingXiang，2023年1月3日，新建函数
///
bool im::LogicControlProxy::QueryGeagentCoverStatus(const std::string& deviceSN)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	return logicControl.QueryGeagentCoverStatus(deviceSN);
}

///
/// @brief 查询试剂加载机构试剂槽上是否有试剂
///
/// @param[in]  DeviceSn  
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年8月29日，新建函数
///
void im::LogicControlProxy::QueryReagentLoadUnloadExistReagent(::tf::ResultBool& _return, const std::string& deviceSN)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	logicControl.QueryReagentLoadUnloadExistReagent(_return, deviceSN);
}

///
/// @brief 获取当前仪器设备是否存在温度异常
///
/// @param[in]  DeviceSn  
///
/// @return bool:true
///
/// @par History:
/// @li 7702/LeiDingXiang，2023年8月29日，新建函数
///
void im::LogicControlProxy::QueryDeviceTemperatureErr(::tf::ResultBool& _return, const std::string& deviceSN)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	logicControl.QueryDeviceTemperatureErr(_return, deviceSN);
}

///
/// @brief
///     查询试剂用于重新计算
///
/// @param[in]  qryResp  查询结果
/// @param[in]  qryCond  查询条件
///
/// @par History:
/// @li 1226/zhangjing，2023年1月17日，新建函数
///
bool im::LogicControlProxy::QueryReagentInfoForReCalc(::im::tf::ReagentInfoTableQueryResp& qryResp
	, const  ::im::tf::ReagentInfoTableQueryCond& qryCond)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	logicControl.QueryReagentInfoForReCalc(qryResp, qryCond);
	if (qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

	return true;
}

void im::LogicControlProxy::AddSpecialWashSampleProbe(::tf::ResultLong& _return
	, const ::im::tf::SpecialWashSampleProbe& swsp)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	logicControl.AddSpecialWashSampleProbe(_return, swsp);
}

bool im::LogicControlProxy::DeleteSpecialWashSampleProbe(const ::im::tf::SpecialWashSampleProbeQueryCond& swspq)
{
	im::LogicControlHandler logicControl;
	return logicControl.DeleteSpecialWashSampleProbe(swspq);
}

bool im::LogicControlProxy::ModifySpecialWashSampleProbe(const ::im::tf::SpecialWashSampleProbe& swsp)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	return logicControl.ModifySpecialWashSampleProbe(swsp);
}

void im::LogicControlProxy::QuerySpecialWashSampleProbe(::im::tf::SpecialWashSampleProbeQueryResp& _return
	, const ::im::tf::SpecialWashSampleProbeQueryCond& swsp)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	logicControl.QuerySpecialWashSampleProbe(_return, swsp);
}

int32_t im::LogicControlProxy::QuerySpecialWashSampleProbeCount(const ::im::tf::SpecialWashSampleProbeQueryCond& swsp)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	return logicControl.QuerySpecialWashSampleProbeCount(swsp);
}

///
/// @brief  
///         添加耗材属性信息
/// @param[in]    
///
/// @return 
///
/// @par History:
/// @li 7915/LeiDingXiang，2023年3月29日，新建函数
///
void im::LogicControlProxy::AddSupplyAttribute(::tf::ResultLong& _return, const ::im::tf::SupplyAttribute& sa)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	logicControl.AddSupplyAttribute(_return, sa);
}

///
/// @brief  
///     查询耗材属性信息
///
/// @param[in]  qryResp  查询结果
/// @param[in]  qryCond  查询条件
/// @return 
///
/// @par History:
/// @li 7915/LeiDingXiang，2023年3月29日，新建函数
///
bool im::LogicControlProxy::QuerySupplyAttribute(::im::tf::SupplyAttributeQueryResp& _return, const ::im::tf::SupplyAttributeQueryCond& saq)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	logicControl.QuerySupplyAttribute(_return, saq);
	if (_return.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		return false;
	}

	return true;
}

///
/// @brief  
///     修改耗材属性信息
///
/// @param[in]  qryResp  查询结果
/// @param[in]  qryCond  查询条件
/// @return 
///
/// @par History:
/// @li 7915/LeiDingXiang，2023年3月29日，新建函数
///
bool im::LogicControlProxy::ModifySupplyAttribute(const ::im::tf::SupplyAttribute& sa)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    im::LogicControlHandler logicControl;
    bool bRet = logicControl.ModifySupplyAttribute(sa);

    if (bRet)
    {
        //通知下位机、小屏幕更新报警值
        if (!im::LogicControlProxy::UpdateLowerAlarmValue(sa.type))
        {
            ULOG(LOG_ERROR, "%s()", __FUNCTION__);
        }
    }
	
    return bRet;
}

///
/// @brief  
///     更新耗材、稀释液、清洗缓冲液报警值
///
/// @param[in]    void
/// @return bool : true 更新成功 false:更新失败
///
/// @par History:
/// @li 7915/LeiDingXiang，2023年3月29日，新建函数
///
bool im::LogicControlProxy::UpdateLowerAlarmValue(const  ::im::tf::SuppliesType::type supType)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
    return logicControl.UpdateLowerAlarmValue(supType);
}

///
/// @brief  
///     删除耗材属性信息
///
/// @param[in]  qryResp  查询结果
/// @param[in]  qryCond  查询条件
/// @return 
///
/// @par History:
/// @li 7915/LeiDingXiang，2023年3月29日，新建函数
///
bool im::LogicControlProxy::DeleteSupplyAttribute(const::im::tf::SupplyAttributeQueryCond& saq)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
    return logicControl.DeleteSupplyAttribute(saq);
}

///
/// @brief
///     查询校准组是否正在校准
///
/// @param[in]  cdg    校准组
///
/// @par History:
/// @li 1226/zhangjing，2023年3月29日，新建函数
///
bool im::LogicControlProxy::IsCaliGrpCaling(const ::im::tf::CaliDocGroup& cdg,const bool pos)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	return logicControl.IsCaliGrpCaling(cdg, pos);
}

///
/// @brief  获取结果状态码
///
/// @par History:
/// @li 1226/zhangjing，2023年4月17日，新建函数
///
bool im::LogicControlProxy::GetResultCodeSet(std::vector< ::im::tf::ResultCodeSet> & _return)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	logicControl.GetResultCodeSet(_return);

	return true;
}

///
/// @brief  保存结果状态设置
///
/// @param[in]  rcs  结果状态列
///
/// @return 
///
/// @par History:
/// @li 1226/zhangjing，2023年4月17日，新建函数
///
bool im::LogicControlProxy::SaveResultCode(const std::vector< ::im::tf::ResultCodeSet> & rcs)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	return logicControl.SaveResultCode(rcs);
}

///
/// @bref
///		判断质控品是否已使用
///
/// @param[in] qcDocId 质控品ID
///
/// @par History:
/// @li 8276/huchunli, 2023年5月15日，新建函数
///
bool im::LogicControlProxy::IsQcUsed(int64_t qcDocId)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	return logicControl.IsQcUsed(qcDocId);
}

void im::LogicControlProxy::QueryQcResult(const ::im::tf::QcResultQueryCond& cond, ::im::tf::QcResultQueryResp& resp)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	logicControl.QueryQcResult(resp, cond);
}

///
/// @brief  该试剂是否正在质控
///
/// @param[in]  iReagInfoId  试剂id
///
/// @return 是为正在质控
///
/// @par History:
/// @li 1226/zhangjing，2023年7月12日，新建函数
///
bool im::LogicControlProxy::IsReagentControling(const ::im::tf::ReagentInfoTable& di)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	return logicControl.IsReagentControling(di);
}

///
/// @brief  更新结果
///
/// @param[in]  _return  修改后的结果
/// @param[in]  ri  已修改浓度的结果
///
/// @par History:
/// @li 1226/zhangjing，2023年9月14日，新建函数
///
bool im::LogicControlProxy::UpdateCurResult(::im::tf::AssayTestResult& _return, const ::tf::CurResultUpdate & cr)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	logicControl.UpdateCurResult(_return, cr);
	if (_return.id <= 0)
	{
		return false;
	}

	return true;
}

///
/// @brief
///     UI更新测试模式后通知DCS
///
/// @param[in]  vecDeviceSn  更新测试模式的设备的设备序列号
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2023年11月4日，新建函数
///
bool im::LogicControlProxy::TestModeUpdateByUI(const std::vector<std::string>& vecDeviceSn)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
    return logicControl.TestModeUpdateByUI(vecDeviceSn);
}

///
/// @bref
///		解析导入的项目信息接口
///
/// @param[in] vecBar 导入的项目barCode
/// @param[out] assayInfo 解析出来的概要信息
///
/// @par History:
/// @li 8276/huchunli, 2023年12月12日，新建函数
///
void im::LogicControlProxy::ParaseAssayBarCode(const std::vector<std::string>& vecBar, std::vector<im::tf::ImportAssayParamInfo>& assayInfo)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	logicControl.ParaseAssayBarCode(assayInfo, vecBar);
}

bool im::LogicControlProxy::ParaseAssayBarCodeToDB(const std::string& strBarCode)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	return logicControl.ImportAssayInfo(strBarCode);
}

///
/// @brief  当前设备是否能加载试剂
///
/// @param[in]  deviceSN  设备序列号
///
/// @return true 能加载试剂 false 不能加载试剂
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年12月26日，新建函数
///
bool im::LogicControlProxy::IsCanLoadReagent(const std::string & deviceSN)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	return logicControl.IsCanLoadReagent(deviceSN);
}

///
/// @brief UI通知的进样模式改变消息
///
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2024年2月22日，新建函数
///
bool im::LogicControlProxy::DetectModeUpdateByUI()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	im::LogicControlHandler logicControl;
	return logicControl.DetectModeUpdateByUI();
}

///
/// @brief 执行调试流程
///
///
/// @return 
///
/// @par History:
/// @li 6889/ChenWei，2024年3月13日，新建函数
///
bool im::LogicControlProxy::ExecuteDebugProcess(const im::tf::DebugProcess& Process)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    
	im::LogicControlHandler logicControl;
    return logicControl.ExecuteDebugProcess(Process);
}

///
/// @brief
///     获取设备调试模块列表
///
/// @param[in]  DeviceSn        设备序列号
/// @param[in]  stuDbgMdResp    调试模块列表
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2024年4月2日，新建函数
///
bool im::LogicControlProxy::GetDebugModules(const std::string& DeviceSn, im::tf::DebugModuleQueryResp& stuDbgMdResp)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    im::LogicControlHandler logicControl;
    logicControl.GetDebugModules(stuDbgMdResp, DeviceSn);
    return true;
}

///
/// @brief
///     移动电机
///
/// @param[in]  retl        返回值
/// @param[in]  DeviceSn    设备序列号
/// @param[in]  forward     方向
/// @param[in]  DebugPart   调试部件
/// @param[in]  step        步数
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2024年4月8日，新建函数
///
bool im::LogicControlProxy::MoveMotor(::tf::ResultLong& retl, const std::string& DeviceSn, bool forward, const im::tf::DebugPart& DebugPart, int step)
{
    ULOG(LOG_INFO, "%s(%s, %d, %d, %d)", __FUNCTION__, DeviceSn, DebugPart.MotorInd, int(forward), step);
    im::LogicControlHandler logicControl;
    logicControl.MoveMotor(retl, DeviceSn, forward, DebugPart, step);
    return true;
}

///
/// @brief
///     保存调试参数
///
/// @param[in]  retl        返回值
/// @param[in]  DeviceSn    设备序列号
/// @param[in]  DebugPart   调试部件信息
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2024年4月8日，新建函数
///
bool im::LogicControlProxy::SaveDebugParameter(::tf::ResultLong& retl, const std::string& DeviceSn, const im::tf::DebugPart& DebugPart)
{
    ULOG(LOG_INFO, "%s(%s, %d, %d)", __FUNCTION__, DeviceSn, DebugPart.MotorInd, DebugPart.tempParam);
    im::LogicControlHandler logicControl;
    logicControl.SaveDebugParameter(retl, DeviceSn, DebugPart);
    return true;
}

///
/// @brief
///     保存调试参数
///
/// @param[in]  retl        返回值
/// @param[in]  DeviceSn    设备序列号
/// @param[in]  DebugUnit   执行调试单元
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2024年4月8日，新建函数
///
bool im::LogicControlProxy::ExcuteDebugUnit(::tf::ResultLong& retl, const std::string& DeviceSn, const im::tf::DebugUnit& DebugUnit)
{
    ULOG(LOG_INFO, "%s(%s, %s)", __FUNCTION__, DeviceSn, DebugUnit.UnitName);
    im::LogicControlHandler logicControl;
    logicControl.ExcuteDebugUnit(retl, DeviceSn, DebugUnit);
    return true;
}

///
/// @brief
///     开关调试部件
///
/// @param[in]  retl        返回值
/// @param[in]  bOn         是否打开
/// @param[in]  DeviceSn    设备序列号
/// @param[in]  DebugPart   调试部件信息
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2024年4月8日，新建函数
///
bool im::LogicControlProxy::Switch(::tf::ResultLong& retl, const std::string& DeviceSn, bool bOn, const im::tf::DebugPart& DebugPart)
{
    ULOG(LOG_INFO, "%s(%s, %d, %d)", __FUNCTION__, DeviceSn, DebugPart.GetCmdCode, int(bOn));
    im::LogicControlHandler logicControl;
    logicControl.Switch(retl, DeviceSn, bOn, DebugPart);
    return true;
}

///
/// @brief
///     导出参数文件（节点调试）
///
/// @param[in]  retl        返回值
/// @param[in]  DeviceSn    设备序列号
/// @param[in]  path        路径
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2024年4月10日，新建函数
///
bool im::LogicControlProxy::ExportParamFile(::tf::ResultLong& retl, const std::string& DeviceSn, const std::string& path)
{
    ULOG(LOG_INFO, "%s(%s, %s)", __FUNCTION__, DeviceSn, path);
    im::LogicControlHandler logicControl;
    logicControl.ExportParamFile(retl, path, DeviceSn);
    return true;
}

///
/// @brief
///     导入参数文件（节点调试）
///
/// @param[in]  retl        返回结果
/// @param[in]  DeviceSn    设备序列号
/// @param[in]  path        路径
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2024年4月10日，新建函数
///
bool im::LogicControlProxy::ImportParamFile(::tf::ResultLong& retl, const std::string& DeviceSn, const std::string& path)
{
    ULOG(LOG_INFO, "%s(%s, %s)", __FUNCTION__, DeviceSn, path);
    im::LogicControlHandler logicControl;
    logicControl.ImportParamFile(retl, path, DeviceSn);
    return true;
}

///
/// @brief
///     读取调试参数
///
/// @param[in]  retl        返回值
/// @param[in]  DeviceSn    设备序列号
/// @param[in]  DebugPart   调试部件信息
///
/// @return true表示成功
///
/// @par History:
/// @li 1226/zhangjing，2024年8月5日，新建函数
///
bool im::LogicControlProxy::ReadDebugParameter(::tf::ResultLong& retl, const std::string& DeviceSn, const im::tf::DebugPart& DebugPart)
{
    ULOG(LOG_INFO, "%s(%s, %d, %d)", __FUNCTION__, DeviceSn, DebugPart.MotorInd, DebugPart.tempParam);
    im::LogicControlHandler logicControl;
    logicControl.ReadDebugParameter(retl, DeviceSn, DebugPart);
    return true;
}

///
/// @brief  结果更新写入item和历史表
///
/// @param[in]  _return  返回
/// @param[in]  lstItemId  itemid列表
///
/// @return true成功
///
/// @par History:
/// @li 1226/zhangjing，2024年8月6日，新建函数
///
bool  im::LogicControlProxy::UpdateResultToItemHist(::tf::ResultLong& _return, const std::vector<int64_t> & lstItemId)
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);
    im::LogicControlHandler logicControl;
    logicControl.UpdateResultToItemHist(_return, lstItemId);
    return true;
}
