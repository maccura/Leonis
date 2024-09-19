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
/// @file     ChLogicControlProxy.h
/// @brief    发送生化通用业务逻辑控制命令的代理
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

#include "src/thrift/ch/gen-cpp/ChLogicControl.h"
#include "src/thrift/ch/gen-cpp/ch_types.h"
#include <string>
#include <memory>
#include <boost/noncopyable.hpp>

namespace ch
{

///
/// @brief
///     发送生化通用业务逻辑控制命令的代理
///
class LogicControlProxy : public boost::noncopyable
{
public:
    ///
    /// @brief
    ///     添加生化通用项目参数
    ///
    /// @param[out]  _return   执行结果
    /// @param[in]  gai        生化通用项目参数
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年7月8日，新建函数
    ///
    static bool AddGeneralAssayInfo(::tf::ResultLong& _return, const ::ch::tf::GeneralAssayInfo& gai);

    ///
    /// @brief
    ///     删除生化通用项目参数
    ///
    /// @param[in]  gaiq       查询条件
    ///
    /// @return true表示成功 
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年7月8日，新建函数
    ///
    static bool DeleteGeneralAssayInfo(const ::ch::tf::GeneralAssayInfoQueryCond& gaiq);

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
    static bool ModifyAssayConfigInfo(const ch::tf::GeneralAssayInfo& aci);

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
    static bool QueryAssayConfigInfo(const ::ch::tf::GeneralAssayInfoQueryCond& qryCond, ::ch::tf::GeneralAssayInfoQueryResp& qryResp);

	///
	/// @brief 添加耗材属性信息
	///
	/// @param[in]  _return  执行结果
	/// @param[in]  sa		 耗材属性信息
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年7月4日，新建函数
	///
	static void AddSupplyAttribute(::tf::ResultLong& _return, const  ::ch::tf::SupplyAttribute& sa);

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
	static bool DeleteSupplyAttribute(const  ::ch::tf::SupplyAttributeQueryCond& saq);

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
	static bool ModifySupplyAttribute(const  ::ch::tf::SupplyAttribute& sa);

	///
	/// @brief 查询条件指定的耗材属性
	///
	/// @param[in]  _return  执行结果
	/// @param[in]  saq		 耗材查询条件
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年7月4日，新建函数
	///
	static bool QuerySupplyAttribute(::ch::tf::SupplyAttributeQueryResp& _return, const  ::ch::tf::SupplyAttributeQueryCond& saq);

    ///
    /// @brief
    ///     添加生化特殊项目参数
    ///
    /// @param[out]  _return 执行结果
    /// @param[in]  sai      生化特殊项目参数
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年7月8日，新建函数
    ///
    static void AddSpecialAssayInfo(::tf::ResultLong& _return, const ::ch::tf::SpecialAssayInfo& sai);

    ///
    /// @brief
    ///     删除生化特殊项目参数
    ///
    /// @param[in]  saiq  查询条件
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年7月8日，新建函数
    ///
    static bool DeleteSpecialAssayInfo(const ::ch::tf::SpecialAssayInfoQueryCond& saiq);

    ///
    /// @brief
    ///     修改生化特殊项目参数
    ///
    /// @param[in]  sai   新的生化特殊项目参数
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年7月8日，新建函数
    ///
    static ::tf::ThriftResult::type ModifySpecialAssayInfo(const ::ch::tf::SpecialAssayInfo& sai);

    ///
    /// @brief  修改项目参数
    ///
    /// @param[in]  gai     通用项目参数(没有设置id就是新增项目)
    /// @param[in]  chGai   生化项目通用参数(没有设置id就是新增项目)
    /// @param[in]  chSai   生化特殊项目参数(没有设置id就是新增项目)
    ///
    /// @return 成功返回true
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年2月23日，新建函数
    ///
    static bool AddAssayInfo(const  ::tf::GeneralAssayInfo& gai,const  ::ch::tf::GeneralAssayInfo& chGai, const  ::ch::tf::SpecialAssayInfo& sai);

    ///
    /// @brief  修改项目参数
    ///     
    ///
    /// @param[in]  gai     通用项目参数
    /// @param[in]  chGai   生化项目通用参数
    /// @param[in]  sai     生化特殊项目参数
    ///
    /// @return thrift错误码
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年12月7日，新建函数
    ///
    static  ::tf::ThriftResult::type ModifyAssayInfo(const  ::tf::GeneralAssayInfo& gai, const  ::ch::tf::GeneralAssayInfo& chGai, const  ::ch::tf::SpecialAssayInfo& sai);

    ///
    /// @brief
    ///     查询满足条件的生化特殊项目参数
    ///
    /// @param[out] _return 执行结果
    /// @param[in]  saiq    生化特殊项目参数
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年7月8日，新建函数
    ///
    static bool QuerySpecialAssayInfo(::ch::tf::SpecialAssayInfoQueryResp& _return, const ::ch::tf::SpecialAssayInfoQueryCond& saiq);

	///
	/// @brief
	///     生化项目信息编码
	///
	/// @param[in]  strAssayCode  编码后的字符串
	/// @param[in]  caiList		  需要编码的项目信息
    /// @param[in]  isEncryption  是否加密
	///
	/// @return true表示成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月18日，新建函数
	///
	static bool EncodeChAssayInfo(std::string& strAssayCode, const ::ch::tf::ChAssayInfoList& caiList, bool isEncryption);

	///
	/// @brief
	///     生化项目信息解码
	///
	/// @param[in]  strAssayCode  需要解码的字符串
	/// @param[in]  caiList		  解码后的项目信息
	///
	/// @return true表示成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月18日，新建函数
	///
	static bool DecodeChAssayInfo(const std::string& strAssayCode, ::ch::tf::ChAssayInfoList& caiList);

    ///
    /// @brief
    ///     查询试剂总览
    ///
    /// @param[out]  _return  执行结果
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月14日，新建函数
    ///
    static void QueryReagentOverviews(std::vector< ::ch::tf::ReagentOverview> & _return);

    ///
    /// @brief
    ///     查询指定设备的仓内仓外耗材总览
    ///
    /// @param[out]  _return  执行结果
    /// @param[in]  deviceSN  查询条件
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月14日，新建函数
    ///
    static void QuerySuppliesOverviews(std::vector< ::ch::tf::SuppliesOverview> & _return, const std::string& deviceSN);

    ///
    /// @brief 添加质控申请信息
    ///
    /// @param[in]  _return  执行结果
    /// @param[in]  qcApp    质控申请信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年8月30日，新建函数
    ///
	static void AddQcApply(::tf::ResultLong& _return, const ::ch::tf::QcApply& qcApp);

    ///
    /// @brief 删除满足条件的质控申请信息
    ///
    /// @param[in]  qcapcd  质控申请信息查询条件
    ///
    /// @return true表示删除成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年8月30日，新建函数
    ///
	static bool DeleteQcApply(const ::ch::tf::QcApplyQueryCond& qcapcd);

    ///
    /// @brief 修改质控申请信息
    ///
    /// @param[in]  qcApp  质控申请信息
    ///
    /// @return true修改成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年8月30日，新建函数
    ///
	static bool ModifyQcApply(const ::ch::tf::QcApply& qcApp);

    ///
    /// @brief 查询满足条件的质控申请信息
    ///
    /// @param[in]  _return  质控申请信息
    /// @param[in]  qcapcd   查询条件
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年8月30日，新建函数
    ///
	static void QueryQcApply(::ch::tf::QcApplyQueryResp& _return, const ::ch::tf::QcApplyQueryCond& qcapcd);

    ///
    /// @brief
    ///     查询质控计算结果
    ///
    /// @param[in]  _return   返回值
    /// @param[in]  qccalccd  查询条件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年1月14日，新建函数
    ///
    static void QueryQcCalculateResult(::ch::tf::QcCalculateResultQueryResp& _return, const ::ch::tf::QcCalculateResultQueryCond& qccalccd);

    ///
    /// @brief
    ///     查询质控结果
    ///
    /// @param[in]  _return  返回值
    /// @param[in]  qcrltcd  查询条件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年1月15日，新建函数
    ///
    static void QueryQcResult(::ch::tf::QcResultQueryResp& _return, const ::ch::tf::QcResultQueryCond& qcrltcd);

    ///
    /// @brief
    ///     修改质控结果信息
    ///
    /// @param[in]  qcRlt  质控结果信息
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年1月16日，新建函数
    ///
    static bool ModifyQcResult(const ::ch::tf::QcResult& qcRlt);

    ///
    /// @brief
    ///     查询联合质控结果
    ///
    /// @param[in]  _return  返回值
    /// @param[in]  qcrltcd  查询条件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年1月16日，新建函数
    ///
    static void QueryUnionQcResult(::ch::tf::UnionQcResultQueryResp& _return, const ::ch::tf::UnionQcResultQueryCond& qcrltcd);

    ///
    /// @brief
    ///     修改联合质控结果信息
    ///
    /// @param[in]  qcRlt  质控结果信息
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年1月16日，新建函数
    ///
    static bool ModifyUnionQcResult(const ::ch::tf::UnionQcResult& qcRlt);

    ///
    /// @brief
    ///     添加试剂需求信息
    ///
    /// @param[out]  _return  执行结果
    /// @param[in]   rn       添加数据
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月14日，新建函数
    ///
    static void AddReagentNeed(::tf::ResultLong& _return, const ::ch::tf::ReagentNeed& rn);

    ///
    /// @brief
    ///     删除满足条件的试剂需求信息
    ///
    /// @param[in]  rnqc  需求信息的条件
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月14日，新建函数
    ///
    static bool DeleteReagentNeed(const  ::ch::tf::ReagentNeedQueryCond& rnqc);

    ///
    /// @brief
    ///     修改试剂需求信息
    ///
    /// @param[in]  rn  需求信息
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月14日，新建函数
    ///
    static bool ModifyReagentNeed(const std::vector< ::ch::tf::ReagentNeed> & rn);

    ///
    /// @brief
    ///     查询满足条件的试剂需求信息
    ///
    /// @param[out]  _return  执行结果
    /// @param[in]   rnqc     需求信息的条件
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月14日，新建函数
    ///
    static void QueryReagentNeed(::ch::tf::ReagentNeedQueryResp& _return, const  ::ch::tf::ReagentNeedQueryCond& rnqc);

    ///
    /// @brief
    ///     查询满足条件的校准品文档
    ///
    /// @param[in]  _return  保存查询结果的容器
    /// @param[in]  cdqc  查询条件
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月8日，新建函数
    ///
    static bool QueryCaliDoc(::ch::tf::CaliDocQueryResp& _return, const ::ch::tf::CaliDocQueryCond& cdqc);

    ///
    /// @brief
    ///     
    ///
    /// @param[in]  cali  
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月9日，新建函数
    ///
    static bool ModifyCaliDoc(const ::ch::tf::CaliDoc& cali);

    ///
    /// @brief
    ///     添加校准品文档
    ///
    /// @param[in]  _return  执行结果
    /// @param[in]  cali  校准品文档
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月9日，新建函数
    ///
	static void AddCaliDoc(::tf::ResultLong& _return, const ::ch::tf::CaliDoc& cali);

	///
	/// @brief
	///     删除校准品文档
	///
	/// @param[in]  cdgq  查询条件
	///
	/// @return true表示成功
	///
	/// @par History:
	/// @li 8090/YeHuaning，2022年8月26日，新建函数
	///
	static bool DeleteCaliDoc(const ::ch::tf::CaliDocQueryCond& cdgq);

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
    /// @li 5774/WuHongTao，2021年11月8日，新建函数
    ///
    static void AddCaliDocGroup(::tf::ResultLong& _return, const ::ch::tf::CaliDocGroup& cdg);

    ///
    /// @brief
    ///     修改校准品组文档
    ///
    /// @param[in]  cdg  校准品组文档
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月9日，新建函数
    ///
    static bool ModifyCaliDocGroup(const ::ch::tf::CaliDocGroup& cdg);

    ///
    /// @brief
    ///     删除校准品组文档
    ///
    /// @param[in]  cdgq  查询条件
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月8日，新建函数
    ///
    static bool DeleteCaliDocGroup(const ::ch::tf::CaliDocGroupQueryCond& cdgq);

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
    static bool QueryCaliDocGroup(::ch::tf::CaliDocGroupQueryResp& _return, const ::ch::tf::CaliDocGroupQueryCond& cdgq);

	///
	/// @brief
	///     解析校准品二维码
	///
	/// @param[in]  _return   解析出的二维码校准信息
	/// @param[in]  strQRcode  需要解析的字符串
	///
	/// @return true表示成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年5月16日，新建函数
	///
	static bool ParseCaliQRcode(::ch::tf::CaliGroupQRcode& _return, const std::string& strQRcode);

	///
	/// @brief 下载校准品
	///     
	///
	/// @param[out] _return	 下载的校准品文档组信息
	/// @param[in]  qqcdgc   查询条件
	///
	/// @return true表示成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年3月17日，新建函数
	///
	static bool DownloadCaliDocGroup(::adapter::chCloudDef::tf::CloudCaliDocGroupResp& _return, const  ::adapter::chCloudDef::tf::QueryCloudCaliDocGroupCond& qqcdgc);

	///
	/// @brief 新增校准文档组
	///     
	///
	/// @param[out] cdg		 校准品文档组信息
	/// @param[in]  lstdoc   对应的文档列表
	///
	/// @return true表示成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年3月17日，新建函数
	///
	static bool AddNewCaliDocGroup(const ::ch::tf::CaliDocGroup& cdg, const  std::vector< ::ch::tf::CaliDoc> & lstdoc);

	///
	/// @brief
	///     添加试剂针特殊清洗信息
	///
	/// @param[in]  _return  执行结果
	/// @param[in]  swrp	 试剂针特殊清洗信息
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年9月9日，新建函数
	///
	static void AddSpecialWashReagentProbe(::tf::ResultLong&  _return, const  ::ch::tf::SpecialWashReagentProbe&  swrp);

	///
	/// @brief
	///     删除试剂针特殊清洗信息
	///
	/// @param[in]  swrpq  试剂针特殊清洗查询条件
	///
	/// @return 成功返回true
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年9月9日，新建函数
	///
	static bool DeleteSpecialWashReagentProbe(const  ::ch::tf::SpecialWashReagentProbeQueryCond& swrpq);

	///
	/// @brief
	///     修改试剂针特殊清洗信息
	///
	/// @param[in]  swrp  试剂针特殊清洗信息
	///
	/// @return 成功返回true
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年9月9日，新建函数
	///
	static bool ModifySpecialWashReagentProbe(const  ::ch::tf::SpecialWashReagentProbe& swrp);

	///
	/// @brief
	///     查询试剂针特殊清洗信息
	///
	/// @param[in]  _return	保存查询结果的容器
	/// @param[in]  swrpq	查询条件
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年9月9日，新建函数
	///
	static void QuerySpecialWashReagentProbe(::ch::tf::SpecialWashReagentProbeQueryResp& _return, const  ::ch::tf::SpecialWashReagentProbeQueryCond& swrpq);

	///
	/// @brief
	///     查询满足条件的记录数
	///
	/// @param[in]  swrpq  查询条件
	///
	/// @return 满足查询条件的记录数，查询失败返回-1
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年9月9日，新建函数
	///
	static int32_t QuerySpecialWashReagentProbeCount(const  ::ch::tf::SpecialWashReagentProbeQueryCond& swrpq);

	///
	/// @brief
	///     添加样本针特殊清洗信息
	///
	/// @param[in]  _return  执行结果
	/// @param[in]  swsp	 样本针特殊清洗信息
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年9月9日，新建函数
	///
	static void AddSpecialWashSampleProbe(::tf::ResultLong& _return, const  ::ch::tf::SpecialWashSampleProbe& swsp);

	///
	/// @brief
	///     删除样本针特殊清洗信息
	///
	/// @param[in]  swspq  样本针特殊清洗查询条件
	///
	/// @return 成功返回true
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年9月9日，新建函数
	///
	static bool DeleteSpecialWashSampleProbe(const  ::ch::tf::SpecialWashSampleProbeQueryCond& swspq);

	///
	/// @brief
	///     修改样本针特殊清洗信息
	///
	/// @param[in]  swsp  样本针特殊清洗信息
	///
	/// @return 成功返回true
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年9月9日，新建函数
	///
	static bool ModifySpecialWashSampleProbe(const  ::ch::tf::SpecialWashSampleProbe& swsp);

	///
	/// @brief
	///     查询样本针特殊清洗信息
	///
	/// @param[in]  _return  查询结果
	/// @param[in]  swsp	 查询条件
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年9月9日，新建函数
	///
	static void QuerySpecialWashSampleProbe(::ch::tf::SpecialWashSampleProbeQueryResp& _return, const  ::ch::tf::SpecialWashSampleProbeQueryCond& swsp);

	///
	/// @brief
	///     查询满足条件的样本针特殊清洗总数
	///
	/// @param[in]  swsp  查询条件
	///
	/// @return 满足条件的样本针特殊清洗的记录总数，查询失败返回-1
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年9月9日，新建函数
	///
	static int32_t QuerySpecialWashSampleProbeCount(const  ::ch::tf::SpecialWashSampleProbeQueryCond& swsp);

	///
	/// @brief
	///     添加反应杯特殊清洗信息
	///
	/// @param[in]  _return  执行结果
	/// @param[in]  swc		 反应杯特殊清洗信息
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年9月9日，新建函数
	///
	static void AddSpecialWashCup(::tf::ResultLong& _return, const  ::ch::tf::SpecialWashCup& swc);

	///
	/// @brief
	///     删除反应杯特殊清洗信息
	///
	/// @param[in]  swcq  反应杯特殊清洗查询条件
	///
	/// @return 成功返回true
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年9月9日，新建函数
	///
	static bool DeleteSpecialWashCup(const  ::ch::tf::SpecialWashCupQueryCond& swcq);

	///
	/// @brief
	///     修改反应杯特殊清洗信息
	///
	/// @param[in]  swc  反应杯特殊清洗信息
	///
	/// @return 成功返回true
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年9月9日，新建函数
	///
	static bool ModifySpecialWashCup(const  ::ch::tf::SpecialWashCup& swc);

	///
	/// @brief
	///     查询反应杯特殊清洗信息
	///
	/// @param[in]  _return  查询结果
	/// @param[in]  swcq	 查询条件
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年9月9日，新建函数
	///
	static void QuerySpecialWashCup(::ch::tf::SpecialWashCupQueryResp& _return, const  ::ch::tf::SpecialWashCupQueryCond& swcq);

	///
	/// @brief
	///     查询满足条件的反应杯特殊清洗的记录总数
	///
	/// @param[in]  swcq  查询条件
	///
	/// @return 反应杯特殊清洗的记录总数，查询失败返回-1
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年9月9日，新建函数
	///
	static int32_t QuerySpecialWashCupCount(const  ::ch::tf::SpecialWashCupQueryCond& swcq);

	///
	/// @brief
	///     添加维护项结果信息
	///
	/// @param[in]  _return  执行结果
	/// @param[in]  miri	 维护项结果信息
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
	///
	static void AddMaintainItemResultInfo(::tf::ResultLong& _return, const  ::ch::tf::MaintainItemResultInfo& miri);

	///
	/// @brief
	///     查找维护项结果信息
	///
	/// @param[in]  _return  执行结果
	/// @param[in]  miriqc	 查询条件
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
	///
	static void QueryMaintainItemResultInfo(::ch::tf::MaintainItemResultInfoQueryResp& _return, const  ::ch::tf::MaintainItemResultInfoQueryCond& miriqc);

    ///
    /// @brief  查询所有的反应杯历史信息
    ///     
    /// @param[out]  _return  反应杯历史信息
    /// @param[in]   sn       设备序列号
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年8月16日，新建函数
    ///
    static void QueryReactionCupHistoryInfo(std::vector< ::ch::tf::ReactionCupHistoryInfo> & _return, const std::string& sn);

	///
	/// @brief 重置反应杯历史信息
	///
	/// @param[in]  sn  设备序列号
	/// @param[in]  cupNums  需要重置的杯号
	///
	/// @return true:重置成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年8月6日，新建函数
	///
	static bool ResetReactionCupHistoryInfo(const std::string& sn, const std::vector<int32_t> & cupNums);

protected:
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
	static std::shared_ptr<ch::tf::ChLogicControlClient> GetChLogicControlClient();

public:
    static unsigned short           m_usPort;                   ///< 端口号
};

}
