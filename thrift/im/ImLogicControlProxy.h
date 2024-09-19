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
/// @file     ImLogicControlProxy.h
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
#pragma once

#include "src/thrift/im/gen-cpp/ImLogicControl.h"
#include "src/thrift/im/gen-cpp/im_types.h"
#include <string>
#include <memory>
#include <boost/noncopyable.hpp>

namespace im
{
    ///
    /// @brief
    ///     质控图类型枚举
    ///
    enum QcGraphType{ QcGraphLJ = 0, QcGraphYouden};

///
/// @brief
///     发送免疫通用业务逻辑控制命令的代理
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
    static std::shared_ptr<im::tf::ImLogicControlClient> GetImLogicControlClient();

    ///
    /// @brief
    ///     添加免疫通用项目信息
    ///
    /// @param[out]  _return   执行结果
    /// @param[in]  gai        免疫通用项目信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年7月8日，新建函数
    ///
    static void AddGeneralAssayInfo(::tf::ResultLong& _return, const ::im::tf::GeneralAssayInfo& gai);

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
    static void QueryImGeneralAssayInfo(::im::tf::GeneralAssayInfoQueryResp& _return, const ::im::tf::GeneralAssayInfoQueryCond& gaiq);

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
    static bool DeleteGeneralAssayInfo(const ::im::tf::GeneralAssayInfoQueryCond& gaiq);

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
    static bool ModifyGeneralAssayInfo(const ::im::tf::GeneralAssayInfo& aci);

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
    static void AddReagentInfo(::tf::ResultLong& _return, const im::tf::ReagentInfoTable& rgntInfo);

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
    static bool QueryReagentInfo(::im::tf::ReagentInfoTableQueryResp& qryResp, const  ::im::tf::ReagentInfoTableQueryCond& qryCond);

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
	static bool QueryReagentInfoForUI(::im::tf::ReagentInfoTableQueryResp& qryResp, const  ::im::tf::ReagTableUIQueryCond& qryCond);

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
    static bool QueryReagentInfoForUIDirectly(::im::tf::ReagentInfoTableQueryResp& qryResp, const  ::im::tf::ReagTableUIQueryCond& qryCond);

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
    static bool ModifyReagentInfo(const ::im::tf::ReagentInfoTable& reagInfo);

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
    static void AddSuppliesInfo(::tf::ResultLong& _return, const im::tf::SuppliesInfoTable& splInfo);

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
    static bool QuerySuppliesInfo(::im::tf::SuppliesInfoTableQueryResp& qryResp, const  ::im::tf::SuppliesInfoTableQueryCond& qryCond);

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
    static bool QuerySuppliesInfoDirectly(::im::tf::SuppliesInfoTableQueryResp& qryResp, const  ::im::tf::SuppliesInfoTableQueryCond& qryCond);

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
    static bool ModifySuppliesInfo(const ::im::tf::SuppliesInfoTable& splInfo);

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
    static bool QueryGeneralAssayInfo(const ::im::tf::GeneralAssayInfoQueryCond& qryCond, ::im::tf::GeneralAssayInfoQueryResp& qryResp);

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
    static void AddCaliDocGroup(::tf::ResultLong& _return, const ::im::tf::CaliDocGroup& cdg);

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
    static bool ModifyCaliDocGroup(const ::im::tf::CaliDocGroup& cdg);

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
    static bool DeleteCaliDocGroup(const ::im::tf::CaliDocGroupQueryCond& cdgq);

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
    /// @li 1226/zhangjing，2022年5月9日，新建函数
    ///
    static bool QueryCaliDocGroup(::im::tf::CaliDocGroupQueryResp& _return, const ::im::tf::CaliDocGroupQueryCond& cdgq);


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
	static bool QueryCaliDoc(::im::tf::CaliDocQueryResp& _return, const ::im::tf::CaliDocQueryCond& cdgq);



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
	static bool DownloadAssayInfo(::tf::ResultLong& _return, const  im::tf::DownloadAssayInfo& dai);


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
	static bool ParseCaliGrpBarcodeInfo(tf::CaliDocGroup& _return, const std::string& strBarcode);
    
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
    static void AddDiluentInfo(::tf::ResultLong& _return, const ::im::tf::DiluentInfoTable& dlt);

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
    static bool QueryDiluentInfoTable(::im::tf::DiluentInfoTableQueryResp& dltr, const ::im::tf::DiluentInfoTableQueryCond& dltc);

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
	static bool QueryDiluentInfoForUI(::im::tf::DiluentInfoTableQueryResp& dltr, const  ::im::tf::ReagTableUIQueryCond& dltc);

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
    static bool QueryDiluentInfoForUIDirectly(::im::tf::DiluentInfoTableQueryResp& dltr, const  ::im::tf::ReagTableUIQueryCond& dltc);

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
    static bool ModifyDiluentInfo(const ::im::tf::DiluentInfoTable& dlt);

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
    static bool DeleteDiluentInfo(::tf::ResultLong& _return, const ::im::tf::DiluentInfoTableQueryCond& dltc);


	///
	/// @brief  获取质控状态
	///
	/// @param[in]  deviceSN  模块号
	/// @param[in]  id  质控组id
	/// @param[in]  assaycode  通道号
	///
	/// @return 质控状态
	///
	/// @par History:
	/// @li 1226/zhangjing，2022年10月31日，新建函数
	///
	static ::im::tf::QcReason::type GetQcApplyReason(const std::string& deviceSN, const int64_t id, const int32_t assaycode);

	///
	/// @brief  新增质控申请
	///
	/// @param[in]  qa 质控申请
	/// @param[in]  _return 是否成功
	///
	/// @par History:
	/// @li 1226/zhangjing，2022年10月31日，新建函数
	///
	static void AddQcApply(::tf::ResultLong& _return, const im::tf::QcApply& qa);

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
	static bool DeleteQcApply(const ::im::tf::QcApplyQueryCond& dltc);

	///
	/// @bref
	///		修改质控申请
	///
	/// @param[in] qa 存在变化的质控申请对象
	///
	/// @par History:
	/// @li 8276/huchunli, 2022年11月14日，新建函数
	///
	static bool ModifyQcApply(const im::tf::QcApply& qa);

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
	static void QueryQcApply(const ::im::tf::QcApplyQueryCond& dltc, std::vector<::im::tf::QcApply>& ret);

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
    static bool QueryQcApply(::im::tf::QcApplyQueryResp& qarp, const ::im::tf::QcApplyQueryCond& qacd);

    ///
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
    static bool QueryQcLjAssay(::im::tf::QcLjAssayResp& QcLjResp, const ::im::tf::QcResultQueryCond& QcRltCond);

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
    static bool QueryQcLjMaterial(::im::tf::QcLjMaterialResp& QcRlt, const ::im::tf::QcResultQueryCond& QcRltCond);

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
    static bool QueryQcYouden(::im::tf::QcYoudenResp& QcRlt, const ::im::tf::QcResultQueryCond& QcRltCond);

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
    static bool UpdateQcBreakReasonAndSolution(const int64_t qcResultId, const  ::im::tf::controlWay::type cway, const std::string& reason, const std::string& solution);

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
    static bool UpdateQcResultExclude(const int64_t qcResultId, const  ::im::tf::controlWay::type cway, bool exclude);

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
	static bool DetectModuleDebug(const  ::tf::MaintainItemType::type itemType, const std::vector<std::string> & lstDevSN, const std::vector<int32_t> & lstParam);

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
	static bool ProcessCheck(const  ::tf::MaintainItemType::type itemType, const std::vector<std::string> & lstDevSN, const std::vector<int32_t> & lstParam);

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
	static bool SingleMaintance(const ::tf::MaintainItemType::type itemType, const std::string& deviceSN, const std::vector<int32_t> & lstParam);

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
    static bool ParseSupplyCupInfo(::im::tf::ParseSupplyeCupResp& CupRlt, const std::string& cupBarcode, const std::string& deviceSN);

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
    static bool AddSupplyCup(::tf::ResultLong& _return, const im::tf::SuppliesInfoTable& si);

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
	static bool QueryReagentInfoForReCalc(::im::tf::ReagentInfoTableQueryResp& qryResp, const  ::im::tf::ReagentInfoTableQueryCond& qryCond);

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
    static bool QueryGeagentCoverStatus(const std::string& deviceSN);

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
	static void QueryReagentLoadUnloadExistReagent(::tf::ResultBool& _return, const std::string& deviceSN);

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
    static void QueryDeviceTemperatureErr(::tf::ResultBool& _return, const std::string& deviceSN);


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
    static bool UpdateUtilityUserChose(const ::im::tf::UserConsumablesState& utilityConsumablesState);

	///
	/// @brief
	///     添加样本针特殊清洗信息
	///
	/// @param[in]  _return  执行结果
	/// @param[in]  swsp	 样本针特殊清洗信息
	///
	/// @par History:
	///  @li 7656/zhang.changjiang，2023年3月21日，新建函数
	///
	static void AddSpecialWashSampleProbe(::tf::ResultLong& _return, const  ::im::tf::SpecialWashSampleProbe& swsp);

	///
	/// @brief
	///     删除样本针特殊清洗信息
	///
	/// @param[in]  swspq  样本针特殊清洗查询条件
	///
	/// @return 成功返回true
	///
	/// @par History:
	///  @li 7656/zhang.changjiang，2023年3月21日，新建函数
	///
	static bool DeleteSpecialWashSampleProbe(const  ::im::tf::SpecialWashSampleProbeQueryCond& swspq);

	///
	/// @brief
	///     修改样本针特殊清洗信息
	///
	/// @param[in]  swsp  样本针特殊清洗信息
	///
	/// @return 成功返回true
	///
	/// @par History:
	///  @li 7656/zhang.changjiang，2023年3月21日，新建函数
	///
	static bool ModifySpecialWashSampleProbe(const  ::im::tf::SpecialWashSampleProbe& swsp);

	///
	/// @brief
	///     查询样本针特殊清洗信息
	///
	/// @param[in]  _return  查询结果
	/// @param[in]  swsp	 查询条件
	///
	/// @par History:
	///  @li 7656/zhang.changjiang，2023年3月21日，新建函数
	///
	static void QuerySpecialWashSampleProbe(::im::tf::SpecialWashSampleProbeQueryResp& _return, const  ::im::tf::SpecialWashSampleProbeQueryCond& swsp);

	///
	/// @brief
	///     查询满足条件的样本针特殊清洗总数
	///
	/// @param[in]  swsp  查询条件
	///
	/// @return 满足条件的样本针特殊清洗的记录总数，查询失败返回-1
	///
	/// @par History:
	///  @li 7656/zhang.changjiang，2023年3月21日，新建函数
	///
	static int32_t QuerySpecialWashSampleProbeCount(const  ::im::tf::SpecialWashSampleProbeQueryCond& swsp);

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
    static void AddSupplyAttribute(::tf::ResultLong& _return, const ::im::tf::SupplyAttribute& sa);

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
    static bool QuerySupplyAttribute(::im::tf::SupplyAttributeQueryResp& _return, const ::im::tf::SupplyAttributeQueryCond& saq);

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
    static bool ModifySupplyAttribute(const ::im::tf::SupplyAttribute& sa);

    ///
    /// @brief  
    ///     更新耗材、稀释液、清洗缓冲液报警值
    ///
    /// @param[in]    
    /// @return 
    ///
    /// @par History:
    /// @li 7915/LeiDingXiang，2023年3月29日，新建函数
    ///
    static bool UpdateLowerAlarmValue(const  ::im::tf::SuppliesType::type supType);

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
    static bool DeleteSupplyAttribute(const::im::tf::SupplyAttributeQueryCond& saq);

	///
	/// @brief
	///     查询校准组是否正在校准
	///
	/// @param[in]  cdg    校准组
	///
	/// @par History:
	/// @li 1226/zhangjing，2023年3月29日，新建函数
	///
	static bool IsCaliGrpCaling(const ::im::tf::CaliDocGroup& cdg, const bool pos = false);

	///
	/// @brief  获取结果状态码
	///
	/// @par History:
	/// @li 1226/zhangjing，2023年4月17日，新建函数
	///
	static bool GetResultCodeSet(std::vector< ::im::tf::ResultCodeSet> & _return);

	///
	/// @brief  保存结果状态设置(最好将有变化的进行更新)
	///
	/// @param[in]  rcs  结果状态列
	///
	/// @return 
	///
	/// @par History:
	/// @li 1226/zhangjing，2023年4月17日，新建函数
	///
	static bool SaveResultCode(const std::vector< ::im::tf::ResultCodeSet> & rcs);

    ///
    /// @bref
    ///		判断质控品是否已使用
    ///
    /// @param[in] qcDocId 质控品ID
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年5月15日，新建函数
    ///
    static bool IsQcUsed(int64_t qcDocId);

    ///
    /// @bref
    ///		查询质控结果
    ///
    /// @param[in] cond 查询条件
    /// @param[out] qcResults 返回的质控结果集
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年5月29日，新建函数
    ///
    static void QueryQcResult(const ::im::tf::QcResultQueryCond& cond, ::im::tf::QcResultQueryResp& resp);

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
    static bool IsReagentControling(const ::im::tf::ReagentInfoTable& di);

    ///
    /// @brief  更新结果
    ///
    /// @param[in]  _return  修改后的结果
    /// @param[in]  ri  已修改浓度的结果
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年9月14日，新建函数
    ///
    static bool UpdateCurResult(::im::tf::AssayTestResult& _return, const ::tf::CurResultUpdate & cr);

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
    static bool TestModeUpdateByUI(const std::vector<std::string>& vecDeviceSn);

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
    static void ParaseAssayBarCode(const std::vector<std::string>& vecBar, std::vector<im::tf::ImportAssayParamInfo>& assayInfo);
    static bool ParaseAssayBarCodeToDB(const std::string& strBarCode);

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
	static bool IsCanLoadReagent(const std::string& deviceSN);

	///
	/// @brief UI通知的进样模式改变消息
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2024年2月22日，新建函数
	///
	static bool DetectModeUpdateByUI();

    ///
    /// @brief 执行调试流程
    ///
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 6889/ChenWei，2024年3月13日，新建函数
    ///
    static bool ExecuteDebugProcess(const im::tf::DebugProcess& Process);

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
    static bool GetDebugModules(const std::string& DeviceSn, im::tf::DebugModuleQueryResp& stuDbgMdResp);

    ///
    /// @brief
    ///     移动电机
    ///
    /// @param[in]  DeviceSn    设备序列号
    /// @param[in]  forward     方向
    /// @param[in]  DebugPart   调试部件
    /// @param[in]  step        步数
    /// @param[in]  retl        返回值
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年4月8日，新建函数
    ///
    static bool MoveMotor(::tf::ResultLong& retl, const std::string& DeviceSn, bool forward, const im::tf::DebugPart& DebugPart, int step);

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
    static bool SaveDebugParameter(::tf::ResultLong& retl, const std::string& DeviceSn, const im::tf::DebugPart& DebugPart);

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
    static bool ExcuteDebugUnit(::tf::ResultLong& retl, const std::string& DeviceSn, const im::tf::DebugUnit& DebugUnit);

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
    static bool Switch(::tf::ResultLong& retl, const std::string& DeviceSn, bool bOn, const im::tf::DebugPart& DebugPart);

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
    static bool ExportParamFile(::tf::ResultLong& retl, const std::string& DeviceSn, const std::string& path);

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
    static bool ImportParamFile(::tf::ResultLong& retl, const std::string& DeviceSn, const std::string& path);

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
    static bool ReadDebugParameter(::tf::ResultLong& retl, const std::string& DeviceSn, const im::tf::DebugPart& DebugPart);

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
    static bool UpdateResultToItemHist(::tf::ResultLong& _return, const std::vector<int64_t> & lstItemId);

public:
    static unsigned short           m_usPort;                   ///< 端口号
};

}
