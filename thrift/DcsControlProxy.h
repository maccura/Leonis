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
/// @file     DcsControlProxy.h
/// @brief    发送DCS控制命令的代理
///
/// @author   3558/ZhouGuangMing
/// @date     2021年3月18日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 3558/ZhouGuangMing，2021年3月18日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include "src/thrift/gen-cpp/DcsControl.h"
#include <string>
#include <memory>
#include <functional>
#include <boost/noncopyable.hpp>
#include <thrift/transport/TBufferTransports.h>

class DcsControlHandler;

///
/// @brief
///     发送DCS控制命令的代理
///
class DcsControlProxy : public boost::noncopyable
{
public:
    ///
    /// @brief
    ///     获得单例对象
    ///
    /// @return 单例对象
    ///
    /// @par History:
    /// @li 3558/ZhouGuangMing，2021年3月18日，新建函数
    ///
    static std::shared_ptr<DcsControlProxy> GetInstance();

    ///
    /// @brief
    ///     析构函数
    ///
    /// @par History:
    /// @li 3558/ZhouGuangMing，2021年3月18日，新建函数
    ///
    virtual ~DcsControlProxy();

    ///
    /// @brief
    ///     初始化
    ///
    /// @param[in]  strIP   服务器IP
    /// @param[in]  usPort  服务器端口号
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 3558/ZhouGuangMing，2021年3月18日，新建函数
    ///
    bool Init(const std::string& strIP, unsigned short usPort);

    ///
    /// @brief
    ///     反初始化
    ///
    /// @par History:
    /// @li 3558/ZhouGuangMing，2021年3月18日，新建函数
    ///
    void UnInit();

    ///
    /// @brief
    ///     记录开机事件
    ///
    /// @param[in]   
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 7685/likai，2023年11月23日，新建函数
    bool RecordStartEvent();

    ///
    /// @brief
    ///     记录关机事件
    ///
    /// @param[in]   
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 7685/likai，2023年11月23日，新建函数
    bool RecordExitEvent();

    ///
    /// @brief 用户登录
    ///     
    /// @param[in]  userInfo  用户信息
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 3558/ZhouGuangMing，2021年3月18日，新建函数
    ///
    bool Login(const tf::UserInfo& userInfo);

    ///
    /// @brief
    ///     自动登录
    ///
    /// @return 自动登录成功的用户，空表示未能自动登录
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月8日，新建函数
    ///
    std::string AutoLogin();

    ///
    /// @brief
    ///     用户身份验证
    ///
    /// @param[in]  userName        用户名
    /// @param[in]  password        密码
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年5月9日，新建函数
    ///
    bool UserIdentityVerify(const std::string& userName, const std::string& password);

    ///
    /// @brief 开始测试
    ///
    /// @param[in]  params	开始测试的参数
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年10月13日，新建函数
    ///
    bool StartTest(const ::tf::StartTestParams& params);

    ///
    /// @brief
    ///     停止测试
    ///
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年10月13日，新建函数
    ///
    bool StopTest();

    ///
    /// @brief 暂停测试
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年10月13日，新建函数
    ///
    bool PauseTest();

	///
	///  @brief 执行维护
	///
	///  @param[in]   exeParams	    维护执行参数
	///
	///  @return true 表示发送维护任务成功
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2022年9月9日，新建函数
	///  @li 8580/GongZhiQiang，2024年5月13日，修改参数为结构体，并增加维护项参数
	///
    bool Maintain(const tf::MaintainExeParams& exeParams);

    ///
    ///  @brief 停止维护
    ///
    ///
    ///  @param[in]   lstDev  设备列表
    ///
    ///  @return	
    ///
    ///  @par History: 
    ///  @li 7656/zhang.changjiang，2022年9月19日，新建函数
    ///
    bool StopMaintain(const std::vector< ::tf::DevicekeyInfo> & lstDev);

    ///
    /// @brief 查询测试项目表
    ///
    /// @param[in]  qryCond  查询项目条件
    /// @param[in]  qryResp  查询结果
    ///
    /// @return true 返回成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年7月27日，新建函数
    ///
    bool QueryTestItems(const ::tf::TestItemQueryCond& qryCond, ::tf::TestItemQueryResp& qryResp);

    ///
    /// @brief新增样本信息
    ///     
    ///
    /// @param[in]  sampInfo  样本信息
    /// @param[in]  result  执行结果
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年4月9日，新建函数
    ///
    bool AddSampleInfo(::tf::ResultListInt64& _return, const ::tf::SampleInfo& sampleInfo, const int32_t count);

    ///
    /// @brief 修改样本信息
    ///     
    /// @param[in]  sampInfo  样本信息
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年4月9日，新建函数
    ///
    bool ModifySampleInfo(const ::tf::SampleInfo& sampInfo);

    ///
    /// @brief 修改项目信息
    ///
    /// @param[in]  vecTestItem  项目信息
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年6月16日，新建函数
    ///
    bool ModifyTestItemInfo(const ::tf::TestItem& ti);

    ///
    /// @brief
    ///     移除满足条件的样本信息
    ///
    /// @param[in]  queryCond  查询条件
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年4月9日，新建函数
    ///
    bool RemoveSampleInfo(const ::tf::SampleInfoQueryCond& queryCond);

    ///
    /// @brief
    ///     批量移除样本信息
    ///
    /// @param[in]  dbNos      移除样本信息的数据库主键
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年4月9日，新建函数
    ///
    bool RemoveSampleInfos(const std::vector<int64_t>& dbNos);

    ///
    /// @brief
    ///     样本序号是否已经被使用
    ///
    /// @param[in]  _return     返回结果
    /// @param[in]  startSeqNo  起始序号
    /// @param[in]  endSeqNo    终止序号
    ///
    /// @return true表示成功
    /// @par History:
    /// @li 5774/WuHongTao，2021年4月9日，新建函数
    ///
    bool WhetherSeqNoUsed(::tf::ResultBool& _return, const int32_t startSeqNo, const int32_t endSeqNo);

    ///
    /// @brief 查询样本信息
    ///     
    /// @param[in]   qryCond  查询条件
    /// @param[out]  qryResp  查询结果
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年4月9日，新建函数
    ///
    bool QuerySampleInfo(const ::tf::SampleInfoQueryCond& qryCond, ::tf::SampleInfoQueryResp& qryResp);

    ///
    /// @brief 传送样本到LIS
    ///
    /// @param[in]  vecSampleKeys  样本数据库主键列表
    /// @param[in]  transType      上传数据类型（参照 tf::LisTransferType ）
    /// @param[in]  history        是否传输历史数据(false: 数据浏览样本, true: 历史数据样本，两个页面的数据来源不一样)
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年12月2日，新建函数
    ///
    bool PushSampleToLIS(const std::vector<int64_t> & vecSampleKeys, const int32_t transType, const bool history);

    ///
    /// @brief 获取满足条件的最大样本号
    ///
    /// @param[out] _return     满足条件的最大样本号
    /// @param[out] stat        是否是急诊样本
    /// @param[out] mode        模式（0：序号 1：条码 2：样本架）
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年4月9日，新建函数
    ///
    bool GetMaxSampleSeqno(::tf::ResultStr& _return, bool stat, int mode);

    ///
    /// @brief
    ///     添加病人信息
    ///
    /// @param[out] _return     执行结果
    /// @param[in]  pi          病人信息
	/// @param[in]  sampleId    样本id
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年4月9日，新建函数
    ///
    bool AddPatientInfo(::tf::ResultLong& _return, const  ::tf::PatientInfo& pi, const int64_t sampleId);

    ///
    /// @brief
    ///     删除满足条件的病人信息
    ///
    /// @param[in]  piqc  查询条件
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年4月9日，新建函数
    ///
    bool DeletePatientInfo(const ::tf::PatientInfoQueryCond& piqc);

    ///
    /// @brief
    ///     修改病人信息
    ///
    /// @param[in]  pi  病人信息
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年4月9日，新建函数
    ///
    bool ModifyPatientInfo(const ::tf::PatientInfo& pi);

    ///
    /// @brief
    ///     查询满足条件的病人信息
    ///
    /// @param[out] _return 保存查询结果的容器
    /// @param[in]  piqc    查询条件
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年4月9日，新建函数
    ///
    bool QueryPatientInfo(::tf::PatientInfoQueryResp& _return, const ::tf::PatientInfoQueryCond& piqc);

	///
	/// @brief 查询满足条件的AI信息
	///
	/// @param[in]  _return  保存查询结果的容器
	/// @param[in]  piqc  查询条件
	///
	/// @return true表示成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年3月13日，新建函数
	///
	bool QueryAiRecognizeResult(::tf::AiRecognizeResultQueryResp& _return, const ::tf::AiRecognizeResultQueryCond& piqc);

    ///
    /// @brief
    ///     修改设备信息，UI不直接调用此接口，应该使用CommonInformationManager::ModifyDeviceInfo修改设备信息
    ///
    /// @param[in]  di      设备信息
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 7951/LuoXin，2022年10月8日，新建函数
    ///
    bool ModifyDeviceInfo(const ::tf::DeviceInfo& di);

    ///
    /// @brief
    ///     查询满足条件的设备信息
    ///
    /// @param[out]  _return  保存查询结果的容器
    /// @param[in]  diqc    查询条件
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年9月7日，新建函数
    ///
    bool QueryDeviceInfo(::tf::DeviceInfoQueryResp& _return, const ::tf::DeviceInfoQueryCond& diqc);

    ///
    /// @brief
    ///     获取下位机的子板的程序版本信息
    ///
    /// @param[out] _return 下位机的子板的程序版本信息,map<设备序列号, map<子板名字, 版本号信息>>
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年2月28日，新建函数
    ///
    bool QueryDeviceVersion(std::map<std::string, std::map<std::string, std::string> > & _return);

    ///
    /// @brief
    ///     获取设备类型-温度配置信息
    ///
    /// @param[out] _return 设备类型-温度配置信息
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年08月09日，新建函数
    ///
    virtual void GetTemperautureConfigInfo(
        std::map<::tf::DeviceType::type, std::vector< ::tf::TemperautureInfo>>& _return,
        const std::vector< ::tf::DeviceType::type>& types);

    ///
    /// @brief
    ///     获取设备类型-液路配置信息
    ///
    /// @param[out] _return 设备类型-液路配置信息
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年08月09日，新建函数
    ///
    virtual void GetLiquidPathConfigInfo(
        std::map<::tf::DeviceType::type, std::vector<std::string>>& _return,
        const std::vector< ::tf::DeviceType::type>& types);

    ///
    /// @brief
    ///     获取设备类型-单元配置信息
    ///
    /// @param[out] _return 设备类型-单元配置信息
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年08月09日，新建函数
    ///
    virtual void GetUnitConfigInfo(
        std::map<::tf::DeviceType::type, std::vector<std::string>>& _return
        , const std::vector< ::tf::DeviceType::type>& types);

    ///
    /// @brief
    ///     获取设备类型-计数配置信息
    ///
    /// @param[out] _return 设备类型-计数配置信息
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年08月09日，新建函数
    ///
    virtual void GetCountConfigInfo(
        std::map<::tf::DeviceType::type, std::vector< ::tf::CountInfo>>& _return
        , const std::vector< ::tf::DeviceType::type>& types);

    ///
    /// @brief
    ///     重置/取消重置部件的使用次数
    ///
    /// @param[in] isReset true:重置， false:取消重置
    /// @param[in] ci    取消重置的部件信息
    /// @param[in] devSn 设备序列号
    /// @param[in] moduleIndex 模块索引,ISE多连机是此参数才有使用
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年1月23日，新建函数
    ///
    bool SetLowerComponentCount(const bool isReset, const  ::tf::CountInfo& ci, const std::string& devSn, const int32_t moduleIndex = 1);

    ///
    /// @brief 设备管理-启用
    ///
    /// @param[in]  lstDeviceSN  设备编号列表
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 7702/WangZhongXin，2023年4月23日，新建函数
    ///
    bool DevicePowerOn(const std::vector<std::string> & lstDeviceSN);

    ///
    /// @brief 通知仪器下电
    ///
    /// @param[in]  lstDevSN  
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年4月19日，新建函数
    ///
    bool DevicePowerOff(const std::vector<std::string> & lstDevSN);

    ///
    /// @brief 更新上电计划表
    ///
    /// @param[in]  mapDevice2Pos   设备序列号----设备上电计划
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年3月24日，新建函数
    ///
    bool UpdetePowerOnSchedule(const std::map<std::string, ::tf::PowerOnSchedule>& mapDevice2Pos);

    ///
    /// @brief 通知下位机休眠
    ///
    ///
    /// @return  true表示成功
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年4月19日，新建函数
    ///
    bool DeviceSleep();

    ///
    /// @brief 唤醒下位机
    ///
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年4月19日，新建函数
    ///
    bool DeviceAwake();

    ///
    /// @brief
    ///     查询指定项目选择页面的元素
    ///
    /// @param[out] _return 保存查询结果的容器
    /// @param[in]  ac      项目分类
    /// @param[in]  pageIdx 页面索引，从0开始计数
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年4月13日，新建函数
    ///
    bool QueryGeneralAssayPage(::tf::GeneralAssayPageQueryResp& _return, ::tf::AssayClassify::type ac, int32_t pageIdx);

    ///
    /// @brief 查询指定项目选择页面的组合项目元素
    ///
    /// @param[in]  _return  保存查询结果的容器
    /// @param[in]  ac		 项目分类
    /// @param[in]  pageIdx  页面索引，从0开始计数
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年6月27日，新建函数
    ///
    bool QueryProfileAssayPage(::tf::ProfileAssayPageQueryResp& _return, const ::tf::AssayClassify::type ac, const int32_t pageIdx);

    ///
    /// @brief 查询通用项目信息
    ///
    /// @param[in]  _return 保存查询结果的容器
    /// @param[in]  gaiq    查询条件
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年7月7日，新建函数
    ///
    bool QueryGeneralAssayInfo(::tf::GeneralAssayInfoQueryResp& _return, const ::tf::GeneralAssayInfoQueryCond& gaiq);

    ///
    /// @brief 查询组合项目信息
    ///
    /// @param[in]  _return  保存查询结果的容器
    /// @param[in]  piqc     查询条件
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年6月27日，新建函数
    ///
    bool QueryProfileInfo(::tf::ProfileInfoQueryResp& _return, const ::tf::ProfileInfoQueryCond& piqc);

    ///
    /// @brief  修改组合项目信息
    ///
    /// @param[in]  pi      新的组合项目信息
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年6月27日，新建函数
    ///
    bool ModifyProfileInfo(const ::tf::ProfileInfo& pi);
    bool DeleteProfileInfo(const ::tf::ProfileInfoQueryCond& pi);
    bool AddProfileInfo(tf::ResultLong& _return, const ::tf::ProfileInfo& pi);

    ///
    /// @brief
    ///     修改通用项目信息
    ///
    /// @param[in]  gai     通用项目信息
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年7月7日，新建函数
    ///
    bool ModifyGeneralAssayInfo(const ::tf::GeneralAssayInfo& gai);

    ///
    /// @brief
    ///     删除满足条件的通用项目信息
    ///
    /// @param[in]  gaiq    查询条件
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年7月7日，新建函数
    ///
    bool DeleteGeneralAssayInfo(const ::tf::GeneralAssayInfoQueryCond& gaiq);

    ///
    /// @brief
    ///     添加通用项目信息
    ///
    /// @param[in]  _return 执行结果
    /// @param[in]  gai     通用项目信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年7月7日，新建函数
    ///
    void AddGeneralAssayInfo(::tf::ResultLong& _return, const ::tf::GeneralAssayInfo& gai);

	///
	/// @brief
	///     修改开放项目记录
	///
	/// @param[in]  oar     开放项目记录
	///
	/// @return true表示成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年12月7日，新建函数
	///
	bool ModifyOpenAssayRecord(const ::tf::OpenAssayRecord& oar);

	///
	/// @brief
	///     删除开放项目记录
	///
	/// @param[in]  oarqc    查询条件
	///
	/// @return true表示成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年12月7日，新建函数
	///
	bool DeleteOpenAssayRecord(const ::tf::OpenAssayRecordQueryCond& oarqc);

	///
	/// @brief
	///     添加开放项目记录
	///
	/// @param[in]  _return 执行结果
	/// @param[in]  oar     开放项目记录
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年12月7日，新建函数
	///
	void AddOpenAssayRecord(::tf::ResultLong& _return, const ::tf::OpenAssayRecord& oar);

	///
	/// @brief	查找开放项目信息
	///     
	///
	/// @param[in]  _return  保存查询结果的容器
	/// @param[in]  oarqc  查询条件
	///
	/// @return true表示成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年12月7日，新建函数
	///
	bool QueryOpenAssayRecord(::tf::OpenAssayRecordQueryResp&  _return, ::tf::OpenAssayRecordQueryCond& oarqc);

    ///
    /// @brief
    ///     删除字典信息
    ///
    /// @param[in]  dictionaryInfo  删除的字典信息
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年8月14日，新建函数
    ///
    bool DeleteDictionaryInfo(const ::tf::DictionaryInfoQueryCond& dictionaryInfo);

    ///
    /// @brief
    ///     修改字典信息
    ///
    /// @param[in]  dictionaryInfo  更新的字典信息
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年9月3日，新建函数
    ///
    bool ModifyDictionaryInfo(const ::tf::DictionaryInfo& dictionaryInfo);

    ///
    /// @brief
    ///     添加字典信息
    ///
    /// @param[in]  dictionaryInfo  添加的字典信息
    /// @param[out] lRet            成功返回对应的数据库主键
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年9月3日，新建函数
    ///
    bool AddDictionaryInfo(const ::tf::DictionaryInfo& dictionaryInfo, ::tf::ResultLong& lRet);

    ///
    /// @brief
    ///     
    ///
    /// @param[in]  _return  保存查询结果的容器
    /// @param[in]  gaiq  查询条件
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年9月3日，新建函数
    ///
    bool QueryDictionaryInfo(::tf::DictionaryInfoQueryResp&  _return, ::tf::DictionaryInfoQueryCond& gaiq);

    ///
    ///  @brief
    ///
    ///
    ///  @param[in]   _return   执行结果
    ///  @param[in]   mi		维护项目信息
    ///
    ///  @return	true 表示成功
    ///
    ///  @par History: 
    ///  @li 7656/zhang.changjiang，2022年9月1日，新建函数
    ///
    bool AddMaintainItem(::tf::ResultLong& _return, const ::tf::MaintainItem& mi);

    ///
    /// @brief
    ///     删除满足条件的维护项目
    ///
    /// @param[in]  miqc    查询条件
    ///
    /// @return true 表示成功
    ///
    /// @par History:
    /// @li 7656/zhang.changjiang，2022年9月1日，新建函数
    ///
    bool DeleteMaintainItem(const ::tf::MaintainItemQueryCond& miqc);

    ///
    /// @brief
    ///     修改维护项目
    ///
    /// @param[in]  mi          维护项目信息
    ///
    /// @return true 表示成功
    ///
    /// @par History:
    /// @li 7656/zhang.changjiang，2022年9月1日，新建函数
    ///
    bool ModifyMaintainItem(const ::tf::MaintainItem& mi);

    ///
    ///  @brief
    ///     查询满足条件的维护项目信息
    ///
    ///  @param[in]   _return     保存查询结果的容器  
    ///  @param[in]   miqc        查询条件  
    ///
    ///  @return	true 表示成功
    ///
    ///  @par History: 
    ///  @li 7656/zhang.changjiang，2022年9月1日，新建函数
    ///
    bool QueryMaintainItem(::tf::MaintainItemQueryResp& _return, const ::tf::MaintainItemQueryCond& miqc);

    ///
    /// @brief
    ///     添加维护组
    ///
    /// @param[out] _return     执行结果
    /// @param[in]  mg          维护组信息
    ///
    ///  @return	true 表示成功
    ///
    /// @par History:
    /// @li 7656/zhang.changjiang，2022年9月1日，新建函数
    ///
    bool AddMaintainGroup(::tf::ResultLong& _return, const ::tf::MaintainGroup& mg);

    ///
    /// @brief
    ///     删除满足条件的维护组
    ///
    /// @param[in]  mgqc    查询条件
    ///
    /// @return true 表示成功
    ///
    /// @par History:
    /// @li 7656/zhang.changjiang，2022年9月1日，新建函数
    ///
    bool DeleteMaintainGroup(const ::tf::MaintainGroupQueryCond& mgqc);

	///
	///  @brief
	///		修改维护组
	///
	///  @param[in]   lstMg  维护组信息列表
	///
	///  @return	true 表示成功
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年6月16日，新建函数
	///
	bool ModifyMaintainGroups(const std::vector< ::tf::MaintainGroup> & lstMg);

    ///
    /// @brief
    ///     查询满足条件的维护组信息
    ///
    /// @param[out] _return     保存查询结果的容器
    /// @param[in]  mgqc        查询条件
    ///
    /// @return true 表示成功
    ///
    /// @par History:
    /// @li 7656/zhang.changjiang，2022年9月1日，新建函数
    ///
    bool QueryMaintainGroup(::tf::MaintainGroupQueryResp& _return, const ::tf::MaintainGroupQueryCond& mgqc);

    ///
    /// @brief
    ///     添加维护日志
    ///
    /// @param[out] _return     执行结果
    /// @param[in]  ml          维护日志信息
    ///
    /// @return true 表示成功
    ///
    /// @par History:
    /// @li 7656/zhang.changjiang，2022年9月1日，新建函数
    ///
    bool AddMaintainLog(::tf::ResultLong& _return, const ::tf::MaintainLog& ml);

    ///
    /// @brief
    ///     删除满足条件的维护日志
    ///
    /// @param[in]  mlqc     查询条件
    ///
    /// @return true 表示成功
    ///
    /// @par History:
    /// @li 7656/zhang.changjiang，2022年9月1日，新建函数
    ///
    bool DeleteMaintainLog(const ::tf::MaintainLogQueryCond& mlqc);

    ///
    /// @brief
    ///     查询满足条件的维护日志
    ///
    /// @param[out] _return     保存查询结果的容器
    /// @param[in]  mlqc        查询条件
    ///
    /// @return true 表示成功
    ///
    /// @par History:
    /// @li 7656/zhang.changjiang，2022年9月1日，新建函数
    ///
    bool QueryMaintainLog(::tf::MaintainLogQueryResp& _return, const ::tf::MaintainLogQueryCond& mlqc);

    ///
    /// @brief
    ///     记录维护失败原因字符串资源（用于维护失败报警）
    ///
    /// @param[in]  mapStrRc  字符串资源映射
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年9月6日，新建函数
    ///
    bool RecordMaintainFailReasonStringRc(const std::map<int, std::string>& mapStrRc);

    ///
    /// @brief
    ///     记录维护项名称字符串资源（用于维护失败报警）
    ///
    /// @param[in]  mapStrRc  字符串资源映射
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年9月9日，新建函数
    ///
    bool RecordMaintainItemNameStringRc(const std::map<int, std::string>& mapStrRc);

    ///
    ///  @brief
    ///
    ///
    ///  @param[in]   _return  执行结果
    ///  @param[in]   lmi      最近一次维护情况
    ///
    ///  @return	
    ///
    ///  @par History: 
    ///  @li 7656/zhang.changjiang，2022年9月24日，新建函数
    ///
    bool AddLatestMaintainInfo(::tf::ResultLong& _return, const ::tf::LatestMaintainInfo& lmi);

    ///
    ///  @brief 
    ///			查询满足条件的最近一次维护情况
    ///
    ///  @param[in]   _return  保存查询结果的容器
    ///  @param[in]   lmiqc    查询条件
    ///
    ///  @return	
    ///
    ///  @par History: 
    ///  @li 7656/zhang.changjiang，2022年9月23日，新建函数
    ///
    bool QueryLatestMaintainInfo(::tf::LatestMaintainInfoQueryResp& _return, const ::tf::LatestMaintainInfoQueryCond& lmiqc);

    ///
    /// @brief
    ///     修改维护详情
    ///
    /// @param[in]  lmi  维护详情
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年4月24日，新建函数
    ///
    bool ModifyLatestMaintainInfo(const ::tf::LatestMaintainInfo& lmi);

	///
	///  @brief 添加操作日志
	///
	///
	///  @param[in]   _return   执行结果
	///  @param[in]   ol		操作日志
	///
	///  @return	true 成功 false 失败
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年4月13日，新建函数
	///
	bool AddOperationLog(::tf::ResultLong& _return, const  ::tf::OperationLog& ol);

	///
	///  @brief 删除满足条件的操作日志（仅用于测试）
	///
	///
	///  @param[in]   olqc  操作日志查询条件
	///
	///  @return	true 成功 false 失败
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年4月13日，新建函数
	///
	bool DeleteOperationLog(const  ::tf::OperationLogQueryCond& olqc);

	///
	///  @brief 查询满足条件的操作日志
	///
	///
	///  @param[in]   _return  返回查询结果
	///  @param[in]   olqc     操作日志查询条件
	///
	///  @return	true 成功 false 失败
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年4月13日，新建函数
	///
	bool QueryOperationLog(::tf::OperationLogQueryResp& _return, const  ::tf::OperationLogQueryCond& olqc);

	///
	///  @brief 查询满足条件的操作日志数量
	///
	///
	///  @param[in]   olqc  操作日志查询条件
	///
	///  @return	满足条件的操作日志数量
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年4月17日，新建函数
	///
	int64_t QueryOperationLogCount(const  ::tf::OperationLogQueryCond& olqc);

	///
	///  @brief 添加试剂/耗材更换日志
	///
	///
	///  @param[in]   _return  执行结果
	///  @param[in]   ccl      试剂/耗材更换日志
	///
	///  @return	true 成功 false 失败
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年4月13日，新建函数
	///
	bool AddConsumableChangeLog(::tf::ResultLong& _return, const  ::tf::ConsumableChangeLog& ccl);

	///
	///  @brief 删除满足条件的试剂/耗材更换日志（仅用于测试）
	///
	///
	///  @param[in]   cclqc  试剂/耗材更换日志查询条件
	///
	///  @return	true 成功 false 失败
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年4月13日，新建函数
	///
	bool DeleteConsumableChangeLog(const  ::tf::ConsumableChangeLogQueryCond& cclqc);

	///
	///  @brief 查询满足条件的试剂/耗材更换日志
	///
	///
	///  @param[in]   _return  返回查询结果
	///  @param[in]   cclqc    试剂/耗材更换日志查询条件
	///
	///  @return	true 成功 false 失败
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年4月13日，新建函数
	///
	bool QueryConsumableChangeLog(::tf::ConsumableChangeLogQueryResp& _return, const  ::tf::ConsumableChangeLogQueryCond& cclqc);

	///
	/// @brief 查询试剂/耗材更换日志名称
	///
	/// @param[in]  _return  
	///
	/// @return 
	///
	/// @par History:
	/// @li 1556/Chenjianlin，2024年2月1日，新建函数
	///
	void QueryConsumableChangeLogNames(std::vector<std::string> & _return);

	///
	///  @brief 查询满足条件的试剂/耗材更换日志的数量
	///
	///
	///  @param[in]   cclqc  试剂/耗材更换日志查询条件
	///
	///  @return	满足条件的试剂/耗材更换日志的数量
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年4月17日，新建函数
	///
	int64_t QueryConsumableChangeLogCount(const  ::tf::ConsumableChangeLogQueryCond& cclqc);

    ///
    /// @brief      增加被禁用的报警码
    ///     
    /// @param[in]  dac  被禁用的报警码列表
    ///
    /// @return true 表示成功
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年5月31日，新建函数
    ///
    bool AddDisableAlarmCodes(const std::vector< ::tf::DisableAlarmCode> & dacs);

    ///
    /// @brief      删除被禁用的报警码
    ///     
    ///
    /// @param[in]  dac  被禁用的报警码列表
    ///
    /// @return  true 表示成功
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年5月31日，新建函数
    ///
    bool DeleteDisableAlarmCodes(const std::vector< ::tf::DisableAlarmCode> & dacs);

    ///
    ///  @brief 通知其他模块报警信息已查看
    ///
    ///  @param[in]   deviceSns  设备序列号列表
    ///  @param[in]   fromLeonis  消息是否来自上位机（true：来自上位机， false：来自小屏幕）
    ///
    ///  @return true表示发送命令成功
    ///
    ///  @par History: 
    ///  @li 7951/LuoXin，2024年2月24日，新建函数
    ///
    bool DeviceAlarmReaded(const std::vector<std::string> & deviceSns, const bool fromLeonis);

    ///
    /// @brief      查询被禁用的报警码
    ///     
    /// @param[in]  _return  查询结果列表
    /// @param[in]  dac      查询条件
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年5月31日，新建函数
    ///
    bool QueryDisableAlarmCode(::tf::DisableAlarmCodeQueryResp& _return, const  ::tf::DisableAlarmCode& dac);

    ///
    /// @brief  获取所有的报警码信息
    ///     
    ///
    /// @param[out]  _return  报警码信息列表
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年5月31日，新建函数
    ///
    void GetAllAlarmPrompt(std::vector< ::tf::AlarmPrompt> & _return);

    ///
    /// @brief
    ///     删除指定的告警记录
    ///
    /// @param[in]  delAll  是否删除所有告警
    /// @param[in]  devSns  设备列表
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 7951/LuoXin，2022年7月26日，新建函数
    ///
    virtual bool DeleteCurAlarm(bool delAll, const std::vector<std::string> & devSns = {});


    ///
    /// @brief 查询当前告警记录
    ///
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年12月13日，新建函数
    ///
    virtual std::vector<::tf::AlarmDesc> QueryCurAlarm();

    ///
    /// @brief
    ///     查询指定的告警日志
    ///
    /// @param[in]  _return  查询结果
    /// @param[in]  aiq  查询条件
    ///
    /// @par History:
    /// @li 7951/LuoXin，2022年7月27日，新建函数
    ///
    virtual bool QueryAlarmInfo(::tf::AlarmInfoQueryResp& _return, const ::tf::AlarmInfoQueryCond& aiq);

    ///
    /// @brief	查询指定的告警日志总数
    ///     
    /// @param[in]  aiq  查询条件
    ///
    /// @return 指定的告警日志总数
    ///
    /// @par History:
    /// @li 7951/LuoXin，2022年8月25日，新建函数
    ///
    virtual int64_t QueryAlarmInfoCount(const  ::tf::AlarmInfoQueryCond& aiq);

    ///
    /// @brief	根据报警码获取其对应的报警标题
    /// 
    /// @param[in]  type   设备类型（::tf::DeviceType）
    /// @param[in]  codes  报警码列表
    ///
    /// @return 指定的告警日志总数
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年6月29日，新建函数
    ///
    virtual std::string GetAlarmTitleByCode(const  ::tf::DeviceType::type type, const std::string& code);
    
    ///
    /// @brief	清空所有被屏蔽的报警码
    /// 
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年9月19日，新建函数
    ///
    virtual void ClearAllAlarmShield();

    ///
    /// @brief	查询数据报警信息
    ///     
    /// @param[in]  code  状态码（ALL:获取所有的数据报警信息）
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年1月17日，新建函数
    ///
    virtual bool QueryDataAlarm(std::vector<::tf::DataAlarmItem>& _return, const std::string& code);

    ///
    /// @brief 添加质控文档
    ///
    /// @param[in]  _return  执行结果
    /// @param[in]  qcDoc	 质控文档
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年8月17日，新建函数
    ///
    bool AddQcDoc(::tf::ResultLong& _return, const ::tf::QcDoc& qcDoc);

    ///
    /// @bref
    ///		用户权限框中的新增删除
    ///
    /// @param[in] userInfo 用户信息
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年3月27日，新建函数
    ///
    bool AddUserInfo(tf::ResultLong& ret, const ::tf::UserInfo& userInfo);
    bool DeleteUserInfo(const std::string& userName);
    bool ModefyUserInfo(const ::tf::UserInfo& userInfo);
    bool QueryUserInfo(const std::string& userName, std::shared_ptr<tf::UserInfo>& userInfo);
    bool QueryAllUserInfo(tf::UserInfoQueryResp& _return, tf::UserInfoQueryCond& uiqc);

    ///
    /// @brief
    ///     添加耗材需求信息
    ///
    /// @param[in]  _return     执行结果
    /// @param[in]  splReqInfo  耗材需求信息
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年9月26日，新建函数
    ///
    bool AddSplReqInfo(tf::ResultLong& _return, const ::tf::SplRequireInfo& splReqInfo);

    ///
    /// @brief
    ///     删除耗材需求信息
    ///
    /// @param[in]  sriqc  查询条件
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年9月26日，新建函数
    ///
    bool DeleteSplReqInfo(const  ::tf::SplRequireInfoQueryCond& sriqc);

    ///
    /// @brief
    ///     修改耗材需求信息
    ///
    /// @param[in]  splReqInfo  耗材需求信息
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年9月26日，新建函数
    ///
    bool ModifySplReqInfo(const ::tf::SplRequireInfo& splReqInfo);

    ///
    /// @brief
    ///     查询耗材需求信息
    ///
    /// @param[in]  _return     查询结构
    /// @param[in]  sriqc       查询条件
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年9月26日，新建函数
    ///
    bool QuerySplReqInfo(::tf::SplRequireInfoQueryResp& _return, const ::tf::SplRequireInfoQueryCond& sriqc);

    ///
    /// @brief 修改质控文档
    ///
    /// @param[in]  qcDoc  质控文档
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年8月17日，新建函数
    ///
    bool ModifyQcDoc(tf::ResultLong& ret, const ::tf::QcDoc& qcDoc);

    ///
    /// @brief 查询满足条件的质控文档
    ///
    /// @param[in]  _return  质控文档列表
    /// @param[in]  cdqc	 查询条件
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年8月17日，新建函数
    ///
    bool QueryQcDoc(::tf::QcDocQueryResp& _return, const ::tf::QcDocQueryCond& cdqc);

    ///
    /// @bref
    ///		查询获取所有质控品
    ///
    /// @param[in] _return 反馈的质控品列表
    ///
    /// @par History:
    /// @li 8276/huchunli, 2022年11月11日，新建函数
    ///
    bool QueryQcDocAll(::tf::QcDocQueryResp& _return);

    ///
    /// @brief 查询包含指定项目的质控文档
    ///
    /// @param[in]  _return  质控文档列表
    /// @param[in]  assayCode 项目编号
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月15日，新建函数
    ///
    bool QueryQcDocByAssayCode(::tf::QcDocQueryResp& _return, int32_t assayCode);

    ///
    /// @brief 删除满足条件的质控文档
    ///
    /// @param[in]  cdgq  删除条件
    /// @param[out]  failedDelete  删除失败的质控品ID
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年8月17日，新建函数
    ///
    bool DeleteQcDoc(const ::tf::QcDocQueryCond& cdgq, std::vector<int64_t>& failedDelete);

	///
	/// @brief  解析质控品条码
	///
	/// @param[in]  _return  质控品信息
	/// @param[in]  strBarcode  条码字符串
	///
	/// @return 是否成功解析
	///
	/// @par History:
	/// @li 1226/zhangjing，2022年8月2日，新建函数
	///
	bool ParseQCBarcodeInfo(::tf::QcBarCodeInfo& _return, const std::string& strBarcode);

    ///
    /// @brief 添加仪器系数
    ///
    /// @param[in]  _return  执行结果
    /// @param[in]  mf  项目
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年9月14日，新建函数
    ///
    bool AddMachineFactor(::tf::ResultLong& _return, const ::tf::MachineFactor& mf);

    ///
    /// @brief 修改仪器系数
    ///
    /// @param[in]  mf  需要修改的项目
    ///
    /// @return 是否成功
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年9月14日，新建函数
    ///
    bool ModifyMachineFactor(const std::vector< ::tf::MachineFactor> & mf);

    ///
    /// @brief 查询仪器系数
    ///
    /// @param[in]  _return  返回结果
    /// @param[in]  mfqc  查询条件
    ///
    /// @return 是否成功
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年9月14日，新建函数
    ///
    bool QueryMachineFactors(::tf::MachineFactorQueryResp& _return, const ::tf::MachineFactorQueryCond mfqc);

    ///
    /// @brief	添加计算项目记录
    ///     
    /// @param[in]  _return		返回结果
    /// @param[in]  cai			计算项目信息
    ///
    /// @par History:
    /// @li 7951/LuoXin，2022年11月1日，新建函数
    ///
    bool AddCalcAssayInfo(::tf::ResultLong& _return, const ::tf::CalcAssayInfo& cai);

    ///
    /// @brief	删除满足条件的计算项目
    ///     
    /// @param[in]  caiqc  删除条件
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 7951/LuoXin，2022年11月1日，新建函数
    ///
    bool DeleteCalcAssayInfo(const ::tf::CalcAssayInfoQueryCond& caiqc);

    ///
    /// @brief	修改计算项目
    ///     
    /// @param[in]  cai  计算项目
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 7951/LuoXin，2022年11月1日，新建函数
    ///
    bool ModifyCalcAssayInfo(const ::tf::CalcAssayInfo& cais);

    ///
    /// @brief	查询满足条件的计算项目
    ///     
    /// @param[in]  _return  返回结果
    /// @param[in]  caiqc	 查询条件
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 7951/LuoXin，2022年10月31日，新建函数
    ///
    bool QueryCalcAssayInfo(::tf::CalcAssayInfoQueryResp& _return, const ::tf::CalcAssayInfoQueryCond& caiqc);

    ///
    /// @brief	添加计算项目结果
    ///     
    ///
    /// @param[in]  _return  返回结果
    /// @param[in]  catr	 计算项目结果
    ///
    /// @par History:
    /// @li 7951/LuoXin，2022年11月1日，新建函数
    ///
    bool AddCalcAssayTestResult(::tf::ResultLong& _return, const ::tf::CalcAssayTestResult& catr);

    ///
    /// @brief	删除满足条件的计算项目结果
    ///     
    ///
    /// @param[in]  catrqc  删除条件
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 7951/LuoXin，2022年11月1日，新建函数
    ///
    bool DeleteCalcAssayTestResult(const ::tf::CalcAssayTestResultQueryCond& catrqc);

    ///
    /// @brief	修改计算项目结果
    ///     
    /// @param[in]  catrs  计算项目结果
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 7951/LuoXin，2022年11月1日，新建函数
    ///
    bool ModifyCalcAssayTestResult(const ::tf::CalcAssayTestResult& catr);

    ///
    /// @brief	查询满足条件的计算项目结果
    ///     
    ///
    /// @param[in]  _return  返回结果
    /// @param[in]  catrqc	 查询条件
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 7951/LuoXin，2022年11月1日，新建函数
    ///
    bool QueryCalcAssayTestResult(::tf::CalcAssayTestResultQueryResp& _return, const ::tf::CalcAssayTestResultQueryCond& catrqc);

    ///
    /// @bref
    ///		更新通用项目信息中的质控规则设定
    ///
    /// @param[in] assayCode 指定的项目编号
    /// @param[in] param 质控规则设定
    ///
    /// @par History:
    /// @li 8276/huchunli, 2022年12月27日，新建函数
    ///
    bool SetOutQcRules(int assayCode, const tf::QcJudgeParam& param);

	///
	/// @brief 
	///     更新HistoryBaseData表的用户设定复查信息
	///
	/// @param[in]  _return  
	/// @param[in]  ri  
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年7月3日，新建函数
	///
	bool UpdateHistoryBaseDataUserUseRetestInfo(::tf::ResultLong& _return, const ::tf::HistoryBaseDataUserUseRetestUpdate& ri);

    ///
    /// @brief
    ///     更新HistoryBaseData表的项目审核信息
    ///
    /// @param[in]   
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 7685/likai，2023年1月3日，新建函数
    ///
    bool UpdateHistoryBaseDataCheckStatus(::tf::ResultLong& _return, const ::tf::HistoryBaseDataCheckUpdate & cs);

    ///
    /// @bref
    ///		更新样本数据的打印标记
    ///
    /// @param[in] samples 样本ID集合
    /// @param[in] isPrinted 是否已被打印
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年5月7日，新建函数
    ///
    bool UpdateHistoryBaseDataPrint(const std::vector<int64_t>& samples, bool isPrinted);

    ///
    /// @bref
    ///		获取质控LJ界面的是否显示不计算点的设置
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年3月9日，新建函数
    ///
    bool GetQcLjNoCalculateFlag();
    void SetQcLjNoCalculateFlag(bool showNoCalculateFlag);

    ///
    /// @bref
    ///		获取质控LJ界面的是否显示子点
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年3月9日，新建函数
    ///
    bool GetQcLjSubPointFlag();
    void SetQcLjSubPointFlag(bool showSubPoint);

    ///
    /// @brief
    ///     获取质控Tp图不计算点显示标志
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年10月8日，新建函数
    ///
    bool GetQcTpNoCalculateFlag();

    ///
    /// @brief
    ///     设置质控Tp图不计算点显示标志
    ///
    /// @param[in]  showNoCalculateFlag  显示不计算点标志
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年10月8日，新建函数
    ///
    void SetQcTpNoCalculateFlag(bool showNoCalculateFlag);

    ///
    /// @bref
    ///		获取应用项目位置设置界面的搜索框是否显示
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年8月23日，新建函数
    ///
    bool GetAppAssayPositionSearchEditShow();

    ///
    /// @bref
    ///		获取质控瓶号、批号长度限制
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年3月18日，新建函数
    ///
    int  GetQcConfigLengthSn();
    int  GetQcConfigLengthLot();

    ///
    /// @bref
    ///		获取质控的Level最大值
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年4月8日，新建函数
    ///
    int GetQcLevelLimit();

    ///
    /// @bref
    ///		获取质控品二维码长度
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年10月9日，新建函数
    ///
    int GetQcBarcodeLength();

    ///
    /// @bref
    ///		打印设置接口
    ///
    /// @par History:
    /// @li 6889/ChenWei, 2023年4月12日，新建函数
    ///
    bool SavePrintSetInfo(const std::vector< ::tf::PrintSetInfo> & pi);
    void GetPrintSetInfo(std::vector< ::tf::PrintSetInfo> & _return);
    
    ///
    /// @bref
    ///		设置/获取 项目位置或项目选择界面的单项目行数
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年6月14日，新建函数
    ///
    void SetAssayTabSignleRowAmount(int rowAmount);
    int  GetAssayTabSignleRowAmount();

    ///
    /// @bref
    ///		序号模式下获取开始测试的默认样本号
    ///
    /// @param[in]   lastSeqNo 上一次开始测试的起始样本号
    ///
    /// @par History:
    /// @li 7951/LuoXin, 2023年6月20日，新建函数
    ///
    std::string GetSeqStartSampleNumber(const std::string& lastSeqNo);

    ///
    /// @bref
    ///		通过user type 获取对应权限
    ///
    /// @param[in] userType 角色类型
    /// @param[out] vecPermission 角色权限表
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年6月26日，新建函数
    ///
    bool GetRolePermision(tf::UserType::type userType, tf::PermissionType::type perType, std::set<int>& vecPermission);

	///
	/// @brief  查询lis连接状态
	///
	/// @param[in]  status  true表示已连接
	///
	/// @return true表示成功
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年8月22日，新建函数
	///
	bool QueryLisConnectionStatus(bool& status);

    ///
    /// @bref
    ///		获取免疫的稀释倍数列表
    ///
    /// @param[out] lstDuli 返回的稀释倍数列表
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年8月31日，新建函数
    ///
    bool GetDilutionRatioListIm(std::vector<int>& lstDuli);

	///
	/// @brief  查询满足条件的样本量统计表格
	///
	/// @param[out]  _return  查询结果
	/// @param[in]   sstdqc   查询条件
	///
	/// @return true 成功 false 失败
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月1日，新建函数
	///
	bool QuerySampleStatisTab(::tf::SampleStatisTabQueryResp& _return, const  ::tf::SampleStatisTabDataQueryCond& sstdqc);

	///
	/// @brief  查询样本量统计饼状图数据
	///
	/// @param[out]  _return  查询结果
	/// @param[in]   sspqc    查询条件
	///
	/// @return true 成功 false 失败
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月1日，新建函数
	///
	bool QuerySampleStatisPie(::tf::SampleStatisPieQueryResp& _return, const  ::tf::SampleStatisPieQueryCond& sspqc);

	///
	/// @brief  查询结果统计数据
	///
	/// @param[out]  _return  返回值
	/// @param[in]  rscqc     查询条件
	///
	/// @return  true 成功 false 失败
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月4日，新建函数
	///
	bool QueryResultStatisCurve(::tf::ResultStatisCurveQueryResp& _return, const  ::tf::ResultStatisCurveQueryCond& rscqc);

	///
	/// @brief  查询校准统计列表
	///
	/// @param[in]  _return  查询结果集
	/// @param[in]  sctqc    查询条件
	///
	/// @return  true 成功 false 失败 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月8日，新建函数
	///
	bool QueryStatisCaliTab(::tf::StatisCaliTabQueryResp& _return, const  ::tf::StatisCaliTabQueryCond& sctqc);

	///
	/// @brief  查询校准失败原因统计表
	///
	/// @param[in]  _return  查询结果集
	/// @param[in]  scfctqc  查询条件
	///
	/// @return  true 成功 false 失败 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月8日，新建函数
	///
	bool QueryStatisCaliFailCauseTab(::tf::StatisCaliFailCauseTabQueryResp& _return, const  ::tf::StatisCaliFailCauseTabQueryCond& scfctqc);

	///
	/// @brief  新增一条校准统计信息
	///
	/// @param[in]  _return  返回值
	/// @param[in]  sc       校准统计信息
	///
	/// @return  true 成功 false 失败 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月8日，新建函数
	///
	bool AddStatisCaliInfo(::tf::ResultLong& _return, const  ::tf::StatisCali& sc);

	///
	/// @brief  查询试剂统计曲线
	///
	/// @param[in]  _return  返回值
	/// @param[in]  srcqc    查询条件
	///
	/// @return  true 成功 false 失败  
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月10日，新建函数
	///
	bool QueryStatisReagentCurve(::tf::StatisReagentCurveQueryResp& _return, const  ::tf::StatisReagentCurveQueryCond& srcqc);

    ///
    /// @brief
    ///     查询动态计算值
    ///
    /// @param[in]  _return     查询结果
    /// @param[in]  isReagent   是否是试剂
    /// @param[in]  dcqc        查询条件
    ///
    /// @return true表示查询成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年12月8日，新建函数
    ///
    bool QueryDynamicCalVal(::tf::DynamicCalcQueryResp& _return, const bool isReagent, const ::tf::DynamicCalcQueryCond& dcqc);

	///
	/// @brief  查询耗材统计曲线
	///
	/// @param[in]  _return  返回值
	/// @param[in]  sscqc    查询条件
	///
	/// @return  true 成功 false 失败  
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月10日，新建函数
	///
	bool QueryStatisSupCurve(::tf::StatisSupCurveQueryResp& _return, const  ::tf::StatisSupCurveQueryCond& sscqc);

	///
	/// @brief  查询对应统计表中所有的设备序列号（去重）
	///
	/// @param[out]  _return     返回值
	/// @param[in]   scbqc       查询条件
	///
	/// @return  true 成功 false 失败 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月6日，新建函数
	///
	bool QueryDeviceSN(::tf::DeviceSNQueryResp& _return, const ::tf::StatisComboBoxQueryCond &scbqc);

	///
	/// @brief  查询对应统计表中所有的试剂批号（去重）
	///
	/// @param[out]  _return     返回值
	/// @param[in]   scbqc       查询条件
	///
	/// @return  true 成功 false 失败 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月6日，新建函数
	///
	bool QueryReagentLot(::tf::ReagentLotQueryResp& _return, const ::tf::StatisComboBoxQueryCond &scbqc);

	///
	/// @brief  查询对应统计表中所有的项目通道号（去重）
	///
	/// @param[out]  _return     返回值
	/// @param[in]   scbqc       查询条件
	///
	/// @return  true 成功 false 失败 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月6日，新建函数
	///
	bool QueryAssayCode(::tf::AssayCodeQueryResp& _return, const ::tf::StatisComboBoxQueryCond &scbqc);

	///
	/// @brief  查询对应统计表中所有的耗材（去重）
	///
	/// @param[in]  _return     返回值
	/// @param[in]   scbqc       查询条件
	///
	/// @return  true 成功 false 失败  
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月14日，新建函数
	///
	bool QueryConsumables(::tf::ConsumablesQueryResp& _return, const ::tf::StatisComboBoxQueryCond &scbqc);

    ///
    /// @brief  解锁翻盖
    ///
    /// @param[in]  deviceSN  设备序列号
    /// @param[in]  type	  前或后翻盖
    ///
    /// @return 解锁失败的设备序列号  
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年11月3日，新建函数
    ///
    bool UnlockFlap(const std::vector<std::string> & deviceSNs, const  ::tf::EnumFlapType::type type);

    ///
    /// @brief  下载质控数据
    ///
    /// @param[in]  _return  返回值
    /// @param[in]  qqc  筛选条件
    ///
    /// @return  true 成功 false 失败  
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年11月28日，新建函数
    ///
    bool DownloadQcDocs(::adapter::tf::CloudQcDocResp& _return, const  ::adapter::tf::QueryCloudQcCond& qqc);

	///
	/// @brief 
	///     获取所有的子设备信息（包含轨道模块）
	/// @param[in]  vecDevice 子设备信息
	///
	/// @return true 成功 false 失败 
	///
	/// @par History:
	/// @li 5220/SunChangYan，2024年7月26日，新建函数
	///
	bool LoadAllSubDeviceInfo(std::vector<::tf::SubDeviceInfo>& vecDevice);

	///
	/// @brief 
	///     按机型获取传感器配置信息
	/// @param[in]  _return 传感器配置信息
	/// @param[in]  deviceInfo 设备信息
	///
	/// @return true 成功 false 失败 
	///
	/// @par History:
	/// @li 5220/SunChangYan，2024年7月26日，新建函数
	///
	bool LoadSensorInfo(std::vector< ::tf::ModuleSensorInfo> & _return, const ::tf::SubDeviceInfo& deviceInfo);

	///
	/// @brief 
	///     开启传感器监控
	/// @param[in]  deviceInfo 设备信息
	///
	/// @return true 成功 false 失败 
	///
	/// @par History:
	/// @li 5220/SunChangYan，2024年7月26日，新建函数
	///
	bool StartSensorMonitor(const ::tf::SubDeviceInfo& deviceInfo);

	///
	/// @brief 
	///     停止传感器监控
	/// @param[in]  deviceInfo 设备信息
	///
	/// @return true 成功 false 失败 
	///
	/// @par History:
	/// @li 5220/SunChangYan，2024年7月26日，新建函数
	///
	bool StopSensorMonitor(const ::tf::SubDeviceInfo& deviceInfo);

	///
	/// @brief  项目参数URL下载
	///
	/// @param[in]  _return  返回值
	/// @param[in]  qcapc  筛选条件
	///
	/// @return  true 成功 false 失败  
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年2月26日，新建函数
	///
	bool GetCloudAssayParamUrls(::adapter::tf::CloudAssayParamResp& _return, const  ::adapter::tf::QueryCloudAssayParamCond& qcapc);

	///
	/// @brief  项目参数信息下载
	///
	/// @param[in]  _return  返回值
	/// @param[in]  lscp  通过原来的URL获取
	///
	/// @return  true 成功 false 失败  
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年2月26日，新建函数
	///
	bool GetCloudAssayParamInfos(::adapter::tf::CloudAssayParamResp& _return, const std::vector< ::adapter::tf::CloudAssayParam> & lscp);

	///
	/// @brief  查询试剂耗材余量不足报警已读标志
	///
	/// @param[in]  devSn  设备序列号
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年2月29日，新建函数
	///
	::tf::ResultBool QueryReagentAlarmReaded(const std::string& devSn);

private:

    bool QueryDictionary(const std::string& keyName, tf::DictionaryInfo& outVaule);

    int  GetDictionaryValueInt(const std::string& keyName, int defaultValue = 0);
    bool SetDictionaryValueInt(const std::string& keyName, int iValue);

    ///
    /// @brief 获取thrift句柄
    ///
    ///
    /// @return 句柄
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年5月9日，新建函数
    ///
    std::shared_ptr<tf::DcsControlConcurrentClient> GetClient() { return m_spClient; };

private:
    static std::shared_ptr<DcsControlProxy>                 s_dcp;              ///< 单例对象
    std::shared_ptr<tf::DcsControlConcurrentClient>         m_spClient;         ///< 连接对象
	std::shared_ptr<DcsControlHandler>						m_spDcs;			///< dcs对象
};
