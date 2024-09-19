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
/// @file     CommonInformationManager.h
/// @brief    项目编号管理器
///
/// @author   4170/TangChuXian
/// @date     2020年10月29日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2020年10月29日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <string>
#include <mutex>
#include <map>
#include <set>
#include <vector>
#include <memory>
#include <boost/optional.hpp>
#include <QObject>
#include <QMap>
#include <QVector>
#include <QVariant>

#include "shared/uidef.h"
#include "src/public/ConfigDefine.h"
#include "src/thrift/ch/gen-cpp/ch_constants.h"
#include "src/thrift/ise/gen-cpp/ise_constants.h"
#include "src/thrift/im/gen-cpp/im_types.h"


// ------------------------------ 通用 -------------------
// 按名称排序项目
typedef std::map<std::string, std::shared_ptr<tf::GeneralAssayInfo>> AssayIndexNameMaps;
// 按编号排序项目
typedef std::map<int, std::shared_ptr<tf::GeneralAssayInfo>> AssayIndexCodeMaps;
// 组合项目信息地图(主键--组合项目信息)
typedef std::map<int64_t, std::shared_ptr<tf::ProfileInfo>> AssayProfileMap;
// 计算项目信息（项目编号--项目信息）
typedef std::map<int, std::shared_ptr<::tf::CalcAssayInfo>> CalcAssayInfoMap;
// 设备
typedef std::map<std::string, std::shared_ptr<tf::DeviceInfo>> DeviceMaps;

// ------------------------------ 生化 -------------------
// 按编号排序项目(所有机型)
typedef std::multimap<int, std::shared_ptr<ch::tf::GeneralAssayInfo>> ChAssayIndexCodeMaps;
// 按编号排序项目(唯一机型)
typedef std::map<std::string, std::shared_ptr<ch::tf::GeneralAssayInfo>> ChAssayIndexUniqueCodeMaps;
typedef std::map<int, std::shared_ptr<ch::tf::GeneralAssayInfo>> ChAssayIndexUniqueOrderCodeMaps;
// 耗材属性地图（耗材编号--属性信息）
typedef std::multimap<int, std::shared_ptr<ch::tf::SupplyAttribute>> SupplyAttributeMap;
// 生化特殊，按编号排序项目
typedef std::multimap<int, std::shared_ptr<ch::tf::SpecialAssayInfo>> ChSpecialAssayIndexMaps;
// 生化在线试剂加载器信息<设备序列号，加载器信息>
typedef QMap<QString, ::ch::tf::ReagentLoaderInfo> ChReagentLoaderMaps;
// 生化试剂信息（仓内试剂和耗材）
struct ChReagentInfo
{
	std::vector<::ch::tf::ReagentGroup> reagentInfos;	// 试剂信息
	::ch::tf::SuppliesInfo supplyInfo;					// 耗材信息（当此位置为试剂时，需从属性reagentInfos取值）

	ChReagentInfo() {}

	ChReagentInfo(const std::vector<::ch::tf::ReagentGroup>& ri, const ::ch::tf::SuppliesInfo &si)
	{
		reagentInfos = ri;
		supplyInfo = si;
	}
};
// 生化试剂盘信息<设备序列号, <位置，耗材信息>>
using ChReagentDiskInfoMaps = std::map<std::string, std::map<int, ChReagentInfo>>;
// 生化仓外耗材信息<设备序列号, <位置，耗材信息>>
using ChCabinetSuppliesInfoMaps = std::map<std::string, std::map<int, ::ch::tf::SuppliesInfo>>;

// ------------------------------ ISE ------------------ -
// 按编号排序项目(所有机型)
typedef std::map<int, std::shared_ptr<ise::tf::GeneralAssayInfo>> IseAssayIndexCodeMaps;
// 按编号排序(唯一机型)
typedef std::map<std::string, std::shared_ptr<ise::tf::GeneralAssayInfo>> IseAssayIndexUniqueCodeMaps;
typedef std::map<int, std::shared_ptr<ise::tf::GeneralAssayInfo>> IseAssayIndexUniqueOrderCodeMaps;
// 耗材属性地图(ise)<耗材编号, 属性信息>
typedef std::map<int, std::shared_ptr<ise::tf::SupplyAttribute>> ISESupplyAttributeMap;
// ISE特殊，按编号排序(所有机型)
typedef std::multimap<int, std::shared_ptr<ise::tf::SpecialAssayInfo>> IseSpecialAssayIndexMaps;
// ISE的所有模块信息地图
typedef std::map<int64_t, std::shared_ptr<ise::tf::IseModuleInfo>> IseModuleMaps;
// ISE仓外耗材信息<设备序列号, <模块号，位置>,耗材信息>>
using IseCabinetSuppliesInfoMaps = std::map<std::string, std::map<std::pair<int,int>, ::ise::tf::SuppliesInfo>>;

// ------------------------------ 免疫 ------------------ -
// 按编号排序(同一名称只配对一个)
typedef std::map<int, std::shared_ptr<tf::GeneralAssayInfo>> ImAssayIndexUniqueCodeMaps;

// 耗材属性地图(im)<耗材编号, 属性信息>
typedef std::map<int, std::shared_ptr<im::tf::SupplyAttribute>> ImSupplyAttributeMap;

// 按编号排序(所有机型)
typedef std::vector<std::pair<std::string, std::shared_ptr<tf::GeneralAssayInfo>>> ImAssayNameMaps;
typedef std::multimap<int, std::shared_ptr<im::tf::GeneralAssayInfo>> ImAssayIndexCodeMaps;
typedef std::map<int, std::pair<std::shared_ptr<tf::GeneralAssayInfo>, std::shared_ptr<im::tf::GeneralAssayInfo>>> ImAssayMaps;

// 双向同测项目类型
enum TwinsTestType
{
    FirstHalfAssay = 0,				///< 属于前段项目
    SecondHalfAssay,				///< 属于后段项目
    None							///< 不是双向同测项目
};

// 软件类型
enum SOFTWARE_TYPE
{
    UNKNOW,                         ///< 未知
    CHEMISTRY,                      ///< 仅生化
    IMMUNE,                         ///< 仅免疫
    CHEMISTRY_AND_IMMUNE            ///< 生免联机
};

// 指示灯颜色
enum EnumLightColor
{
	ELC_UNKNOWN = 0,	//< 未知颜色
	ELC_GRAY,			//< 灰色
	ELC_RED,			//< 红色
	ELC_YELLOW,			//< 黄色
	ELC_GREEN,			//< 绿色
};

// 前置声明
class KeyData;
// 项目状态
using spKeyData = std::shared_ptr<KeyData>;
using ReagentMap = std::map<int64_t, spKeyData>;
struct AssayStatus
{
	ReagentMap reagentMap;			///< 试剂地图
	int		   assayCode;			///< 项目名称
	bool	   maskAssay;			///< 项目遮蔽（true:遮蔽，flase：未遮蔽）
	bool	   abnormalReagent;		///< 试剂异常（true:异常，flase：未异常）
	bool	   abnormalCalibrate;	///< 校准异常（true:异常，flase：未异常）

	AssayStatus()
	{
		assayCode = -1;
		maskAssay = false;
		abnormalReagent = false;
		abnormalCalibrate = false;
	}
};

// 主要的数据结构
class KeyData
{
public:
    KeyData() = default;
    KeyData(QVariant reagent)
        :m_reagent(reagent)
    {}

     virtual bool IsCaliBrateAbnormal();
	 virtual bool IsReagentAbnormal();
	 virtual int64_t Getid();
	 virtual std::string GetDeviceSn();
	 virtual void SetStatus(AssayStatus& status) {Q_UNUSED(status) ; };
	 virtual int64_t GetCode();

protected:
    QVariant m_reagent;
};

class KeyIseDate : public KeyData
{

public:
	KeyIseDate() = default;
	KeyIseDate(QVariant reagent)
		:KeyData(reagent)
	{}

	bool IsCaliBrateAbnormal();
	bool IsReagentAbnormal();
	virtual std::string GetDeviceSn();
	int64_t GetCode() { return m_statusMap.assayCode;};
	void SetStatus(AssayStatus& status) { m_statusMap = status; };
private:
	const int GetOtherType(int pos) const
	{ 
		if (pos == 0)
		{
			return 1;
		}
		
		return (2 / pos);
	};

private:
	AssayStatus		m_statusMap;			///< 项目状态地图（项目遮蔽，试剂异常，校准异常）
};

class QSystemMonitor : public QObject
{
    Q_OBJECT
public:
    virtual ~QSystemMonitor();

    ///
    /// @brief 初始化项目状态地图
    ///
    /// @param[in]  spAssayInfo  项目信息
    ///
    /// @return true:初始化成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年8月21日，新建函数
    ///
    bool InitStatusMap(std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo);

    // 单例实现
    static QSystemMonitor& GetInstance();

    inline boost::optional<AssayStatus> GetAssayStatus(int assayCode)
    {
        auto iter = m_AssayStatusMap.find(assayCode);
        return (iter == m_AssayStatusMap.end()) ? boost::none : boost::make_optional(iter->second);
    }

    ///
    /// @brief 更新项目状态
    ///
    /// @param[in]  reagentGroup  试剂组信息
    ///
    /// @return true:更新成功，反之不成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年8月18日，新建函数
    ///
    bool UpdateAssayStatus(spKeyData reagentGroup);

	///
	/// @brief 处理卸载试剂的流程
	///
	/// @param[in]  reagentGroup  卸载试剂
	///
	/// @return true 处理卸载试剂的流程成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年11月9日，新建函数
	///
	bool UnloadReagentProcess(spKeyData reagentGroup);

    ///
    /// @brief 更新项目的遮蔽状态
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年8月22日，新建函数
    ///
    void UpdateMaskDeviceInfo();

	///
	/// @brief 设置当前样本类型
	///
	/// @param[in]  sourceType  样本类型
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年8月31日，新建函数
	///
	void SetCurrentType(int sourceType);

	///
	/// @brief 获取当前界面选择的样本类型
	///
	///
	/// @return 返回当前界面选择的样本类型
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年8月31日，新建函数
	///
    inline ise::tf::SampleType::type GetCurrentType() { return m_sourceType; };

	///
	/// @brief 更新样本类型以后，重新计算ise的项目状态
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年8月31日，新建函数
	///
	void OnUpdateSampleType();

	///
	/// @brief 电解质模块校准状态的变化（ise的校准是针对模块，bug0014288）
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年9月2日，新建函数
	///
	void UpdateIseCaliModuleSatus();

    ///
    /// @brief
    ///     设置免疫试剂异常标志是否实时刷新（项目选择界面显示时置为true，否则置为false）
    ///
    /// @param[in]  updateRealTime  是否实时刷新
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年1月25日，新建函数
    ///
    void SetImRgntAbnFlagUpdateRealTime(bool updateRealTime);

protected:
    QSystemMonitor();

    ///
    /// @brief 根据项目编号和类型获取试剂信息
    ///
    /// @param[in]  assayCode  编号
    /// @param[in]  type	   类型（生化免疫等）
    ///
    /// @return 试剂信息的统一数据结构
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年8月22日，新建函数
    ///
    std::vector<std::shared_ptr<KeyData>> GetReagentGroup(int assayCode, int type);

    ///
    /// @brief 项目是否被屏蔽
    ///
    /// @param[in]  assayCode	  项目编号
	/// @param[in]  checkReagent  检查是否有试剂
    ///
    /// @return true:表明被屏蔽
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年8月22日，新建函数
    ///
	bool ISAssayShiled(AssayStatus& status, bool checkReagent = false);

	///
	/// @brief 更新项目屏蔽状态
	///
	/// @param[in]  assayCode  项目编号
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年1月15日，新建函数
	///
	bool UpdateAssayMaskStatus(int assayCode);

protected Q_SLOTS:

	///
	/// @brief 项目遮蔽处理
	///
	/// @param[in]  assyaCodes  项目列表
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年1月15日，新建函数
	///
	void OnShiledAssay(class std::set<int, struct std::less<int> , class std::allocator<int>> assyaCodes);

	///
	/// @brief 设备状态更新
	///
	/// @param[in]  groupName  设备组名称
	/// @param[in]  device	   设备名称
	/// @param[in]  isMask	   是否屏蔽
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年1月16日，新建函数
	///
	void OnUpdateDeviceAssayStatus(QString groupName, QString device, bool isMask);

    ///
    /// @brief 生化项目更新耗材的消息槽函数
    ///  
    /// @param[in]  supplyUpdates  更新耗材的位置编号
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年8月22日，新建函数
    ///
    void OnUpdateSupplyForReagent(std::vector<ch::tf::SupplyUpdate, std::allocator<ch::tf::SupplyUpdate>> supplyUpdates);

    ///
    /// @brief 更新免疫试剂的信息
    ///
    /// @param[in]  stuRgntInfo  免疫试剂信息
    /// @param[in]  type		 更新类型
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年8月22日，新建函数
    ///
    void OnUpdateIMSupplyForReagent(const im::tf::ReagentInfoTable& stuRgntInfo, const im::tf::ChangeType::type type);

	///
	/// @brief ISE项目更新耗材的消息槽函数
	///
	/// @param[in]  supplyUpdates  更新耗材的位置编号
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年8月30日，新建函数
	///
	void OnUpdateSupplyForISE(std::vector<ise::tf::SupplyUpdate, std::allocator<ise::tf::SupplyUpdate>> supplyUpdates);

    ///
    /// @brief
    ///     刷新免疫试剂异常校准异常标志
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年1月25日，新建函数
    ///
    void RefreshIMSupplyForReagent();

    ///
    /// @brief
    ///     检测模式更新处理
    ///
    /// @param[in]  mapUpdateInfo  检测模式更新信息（设备序列号-检测模式）
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年11月10日，新建函数
    ///
    void OnDetectModeChanged(QMap<QString, im::tf::DetectMode::type> mapUpdateInfo);

private:
    std::map<int, AssayStatus>			m_AssayStatusMap;			///< 项目状态地图（项目遮蔽，试剂异常，校准异常）
	ise::tf::SampleType::type			m_sourceType;				///< 样本类型（尿或者血清）
    bool                                m_bImRgntAbnUpdateRealTime; // 免疫试剂异常标志实时刷新
};

class CommonInformationManager : public QObject
{
    Q_OBJECT

public:
    ~CommonInformationManager();

    ///
    /// @brief
    ///     项目编号管理器初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年11月2日，新建函数
    ///
    bool Init();

    ///
    /// @brief
    ///     获取项目信息
    ///
    /// @return 项目信息
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年2月3日，新建函数
    ///
    inline AssayIndexCodeMaps GetAssayInfo() { return m_mapAssayCodeMaps; };

    ///
    /// @brief
    ///     获取项目名称
    ///
    /// @param[in]  iAssayCode  项目编号
    ///
    /// @return 项目名称
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年2月3日，新建函数
    ///
    inline std::string GetAssayNameByCode(int iAssayCode)
    {
        auto it = m_mapAssayCodeMaps.find(iAssayCode);
        return (it != m_mapAssayCodeMaps.end() ? it->second->assayName : "");
	};

	std::set<int> GetDifferentMap(const std::map<int, std::set<int>>& firstSet, const std::map<int, std::set<int>>& secondSet);

	///
	/// @brief 获取tips的内容
	///
	///
	/// @return tips的内容
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年1月2日，新建函数
	///
	QString GetTipsContent(const bool isAiEnable);

    ///
    /// @brief 获取项目名称(包括计算项目)
    ///
    /// @param[in]  iAssayCode  项目通道号
    ///
    /// @return 项目名称
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年8月3日，新建函数
    ///
    QString GetAssayNameByCodeEx(int iAssayCode);

	///
	/// @brief 根据状态码获取血清指数的定性判断
	///
	/// @param[in]  assayCode  血清指数的项目编号{L,H,I}
	/// @param[in]  statusCode  1-9
	///
	/// @return +++等
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月28日，新建函数
	///
	QString GetResultStatusLHIbyCode(int assayCode, int statusCode);

    ///
    /// @brief 获取项目打印名称
    ///
    /// @param[in]  assayCode  项目通道号
    ///
    /// @return 项目打印名称
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年7月18日，新建函数
    ///
    QString GetPrintAssayName(int iAssayCode);

    inline QString GetNameOfSampleOrAssay(int type)
    {
        auto iter = m_nameMap.find(type);
        return iter != m_nameMap.end() ? *iter : QString();
    }

    ///
    /// @brief 修改项目禁用的数据报警复查
    ///
    /// @param[in]  assayCode  项目通道号
    /// @param[in]  version    参数版本
    /// @param[in]  shieldStatusCodes  数据报警复查
    ///
    /// @return 项目打印名称
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年12月27日，新建函数
    ///
    bool ModifyAssayShieldStatusCodes(int assayCode, const std::string& version, const std::vector<std::string>& shieldStatusCodes);

    ///
    /// @brief
    ///     获取试剂信息表表头字符串
    ///
    /// @param[in]  type  类型
    ///
    /// @return 试剂表头对应名字
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月8日，新建函数
    ///
    inline QString GetHeaderNameOfRgntTbl(int type)
    {
        auto iter = m_rgntHeaderMap.find(type);
        return iter != m_rgntHeaderMap.end() ? *iter : QString();
    }

    ///
    /// @bref
    ///		通过项目名获取项目对应精度
    ///
    /// @param[in] assayName 项目名
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年8月2日，新建函数
    ///
    inline int GetPrecisionNum(const std::string& assayName)
    {
        auto it = m_mapAssayNameMaps.find(assayName);
        return (it == m_mapAssayNameMaps.end() ? UI_RESULT_PRECISION : (it->second->__isset.decimalPlace ? it->second->decimalPlace : UI_RESULT_PRECISION));
    }

    ///
    /// @brief 获取生化项目当前使用的项目的详细信息
    ///
    /// @param[in]  iAssayCode  项目编号
    /// @param[in]  type		机器类型
    /// @param[in]  version		参数版本
    ///
    /// @return 项目详细信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月17日，新建函数
    ///
    std::shared_ptr<ch::tf::GeneralAssayInfo> GetChemistryAssayInfo(int iAssayCode
		, ::tf::DeviceType::type type, const std::string& version = "");

	///
	/// @brief 获取全部生化项目的详细信息
	///
	/// @param[in]  iAssayCode  项目编号
	/// @param[in]  type		机器类型
	///
	/// @return 项目详细信息
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年2月24日，新建函数
	///
	std::vector<std::shared_ptr<ch::tf::GeneralAssayInfo>> GetAllChemistryAssayInfo(int iAssayCode, ::tf::DeviceType::type type);

    ///
    /// @brief 获取生化项目参数版本列表
    ///
    /// @param[in]  iAssayCode  项目编号
    /// @param[in]  type		机器类型
    ///
    /// @return 项目详细信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年12月18日，新建函数
    ///
    std::set<std::string> GetAssayVersionList(int iAssayCode, ::tf::DeviceType::type type);

    ///
    /// @brief 获取免疫项目当前使用的项目的详细信息
    ///
    /// @param[in]  iAssayCode  项目编号
    /// @param[in]  type        机器类型
    ///
    /// @return 项目详细信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年11月8日，新建函数
    ///
    std::shared_ptr<im::tf::GeneralAssayInfo> GetImmuneAssayInfo(int iAssayCode
		, ::tf::DeviceType::type devType, const std::string& ver = "");

    ///
    /// @brief 获取免疫项目当前使用的项目的详细信息队列
    ///
    /// @targetAssys[out]       匹配到的项目
    /// @param[in]  iAssayCode  项目编号
    /// @param[in]  type        机器类型
    ///
    /// @return 项目详细信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年11月8日，新建函数
    ///
    void GetImmuneAssayInfos(std::vector<std::shared_ptr<im::tf::GeneralAssayInfo>>& targetAssys
		, int iAssayCode, ::tf::DeviceType::type type);

    ///
    /// @brief 获取ISE项目当前使用的项目的详细信息
    ///
    /// @param[in]  iAssayCode  项目编号
    /// @param[in]  type		机器类型
	/// @param[in]  version		参数版本（空表示不限制）
    ///
    /// @return 项目详细信息
    ///
    /// @par History:
    /// @li 7951/LuoXin，2022年6月28日，新建函数
    ///
    std::shared_ptr<ise::tf::GeneralAssayInfo> GetIseAssayInfo(int iAssayCode
		, ::tf::DeviceType::type type, const std::string& version = "");

	///
	/// @brief 获取全部ISE项目的详细信息
	///
	/// @param[in]  iAssayCode  项目编号
	/// @param[in]  type		机器类型
	///
	/// @return 项目详细信息
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年2月24日，新建函数
	///
	std::vector<std::shared_ptr<ise::tf::GeneralAssayInfo>> GetAllIseAssayInfo(int iAssayCode, ::tf::DeviceType::type type);

    ///
    /// @brief 获取生化特殊项目列表
    ///
    /// @return 项目详细信息
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年1月9日，新建函数
    ///
    ChSpecialAssayIndexMaps GetChemistrySpecialAssayMap() { return m_mapChSpecialAssayCodeMaps; };

    ///
    /// @brief 根据生化项目关键信息获取生化项目特殊信息详细信息
    ///
    /// @param[in]  iAssayCode  项目编号
    /// @param[in]  type		机器类型
    /// @param[in]  sampSrcType 样本源类型（-1则不限制）
    /// @param[in]  version     参数版本（空表示获取当前启用的参数版本）
    ///
    /// @return 项目详细信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月17日，新建函数
    /// @li 6950/ChenFei，2022年5月18日，加入样本源类型参数
    ///
    std::vector<std::shared_ptr<ch::tf::SpecialAssayInfo>> GetChemistrySpecialAssayInfo(int iAssayCode
		, ::tf::DeviceType::type type, int sampSrcType = -1, const std::string& version = "");

    ///
    /// @brief  通过项目的编号获取他的双向同测的特殊项目信息
    ///
    /// @param[in]  iAssayCode  项目编号
    /// @param[in]  deviceType  设备类型
    ///
    /// @return 特殊项目配置参数
    ///
    /// @par History:
    /// @li 3558/ZhouGuangMing，2024年06月17日，新建函数
    ///
    std::shared_ptr<ch::tf::SpecialAssayInfo> GetChemistrySpecialTwinsAssayInfo(int iAssayCode, ::tf::DeviceType::type type);

    ///
    /// @brief  获得优先级最高的生化特殊项目配置参数
    ///
    /// @param[in]  iAssayCode  项目编号
    /// @param[in]  deviceType  设备类型
    /// @param[in]  version     参数版本号
    ///
    /// @return 特殊项目配置参数
    ///
    /// @par History:
    /// @li 3558/ZhouGuangMing，2023年5月10日，新建函数
    ///
    std::shared_ptr<ch::tf::SpecialAssayInfo> GetHighestPrioritySpecialAssay(int iAssayCode
        , ::tf::DeviceType::type deviceType, const std::string& version);

    ///
    /// @brief
    ///     根据ISE项目关键信息获取ISE项目特殊信息
    ///
    /// @param[in]  iAssayCode  项目编号
    /// @param[in]  type		机器类型
    ///
    /// @return     ISE项目特殊信息
    ///
    /// @par History:
    /// @li 6950/ChenFei，2022年05月18日，新建函数
    ///
    std::vector<std::shared_ptr<ise::tf::SpecialAssayInfo>> GetIseSpecialAssayInfo(int iAssayCode, ::tf::DeviceType::type type);

    ///
    /// @brief
    ///     根据ISE项目关键信息获取ISE项目特殊信息
    ///
    /// @param[in]  iAssayCode  项目编号
    /// @param[in]  SST			样本源类型
    /// @param[in]  type		机器类型
    ///
    /// @return     ISE项目特殊信息
    ///
    /// @par History:
    /// @li 7951/LuoXin，2022年12月12日，新建函数
    ///
    std::shared_ptr<ise::tf::SpecialAssayInfo> GetIseSpecialAssayInfo(int iAssayCode, ::tf::SampleSourceType::type SST, ::tf::DeviceType::type type);

    ///
    /// @brief	获取设备信息   
    ///
    /// @return 设备信息
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年1月11日，新建函数
    ///
    std::map<std::string, std::shared_ptr<const tf::DeviceInfo>> GetDeviceMaps();
    
    ///
    /// @brief  修改设备的信息
    ///     
    ///
    /// @param[in]  dev  设备信息
    ///
    /// @return 成功返回true
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年11月23日，新建函数
    ///
    bool ModifyDeviceInfo(const tf::DeviceInfo& dev);

    ///
    /// @brief  获取免疫设备单机版是否处于停机状态
    ///
    /// @param[in]    
    ///
    /// @return bool:true免疫设备处于停机状态 false免疫设备非停机状态
    ///
    /// @par History:
    /// @li 7915/LeiDingXiang，2023年8月29日，新建函数
    ///
    bool GetImDeviceIsHalt();

    ///
    /// @bref
    ///		是否存在运行中的设备（不包括轨道设备的状态）
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年5月24日，新建函数
    ///
    bool IsExistDeviceRuning(int assayClassfy = -1);

    ///
    /// @bref
    ///		是否所有设备都是待机状态
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年1月15日，新建函数
    ///
    bool IsAllDeviceStandby();

    ///
    /// @brief
    ///     根据设备sn号获取设备信息
    ///
    /// @param[in]  deviceSn  设备sn号
    ///
    /// @return 设备信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年1月11日，新建函数
    ///
    inline std::shared_ptr<const tf::DeviceInfo> GetDeviceInfo(const std::string& deviceSn)
    {
        auto it = m_mapDevices.find(deviceSn);
        return it != m_mapDevices.end() ? it->second : nullptr;
    }


    static std::string GetDeviceName(const std::string& deviceSn, const int moduleIndex);
	static std::string GetDeviceName(const std::string& deviceSn);

    ///
    /// @brief 根据设备名称获取设备信息
    ///
    /// @param[in]  deviceName  设备名
    ///
    /// @return 设备详细信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年6月23日，新建函数
    ///
    std::shared_ptr<const tf::DeviceInfo> GetDeviceInfoByDeviceName(const std::string& deviceName);

	///
	/// @brief  获取单个设备指示灯的颜色
	///
	/// @param[in]  status  设备状态
    /// @param[in]  rgtAlarmReaded 两台设备的试剂耗材余量不足的报警是否已读
	///
	/// @return 指示灯的颜色
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年8月29日，新建函数
	///
	EnumLightColor GetLightColor(const int status, const bool rgtAlarmReaded = true);

	///
	/// @brief  获取生化单机指示灯的颜色
	///
	/// @param[in]  statusCh  生化设备状态
	/// @param[in]  statusIse ISE设备状态
	/// @param[in]  rgtAlarmReaded 两台设备的试剂耗材余量不足的报警是否已读
	///
	/// @return 合成后的指示灯颜色
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年8月29日，新建函数
	///
	EnumLightColor GetChSingleLightColor(const int statusCh, const int statusIse, const bool rgtAlarmReaded);

    ///
    /// @brief  获取C2000指示灯的颜色
    ///
    /// @param[in]  statusA  设备A状态
    /// @param[in]  statusB  设备B状态
	/// @param[in]  rgtAlarmReaded 两台设备的试剂耗材余量不足的报警是否已读
    ///
    /// @return 合成后的指示灯颜色
    ///
    /// @par History:
    /// @li 7997/XuXiaoLong，2024年8月29日，新建函数
    ///
    EnumLightColor GetC2000LightColor(const int statusA, const int statusB, const bool rgtAlarmReaded);

	///
	/// @brief  通过设备关键信息获取设备的状态
	///     
	///
	/// @param[in]  keyInfo  设备的关键信息
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年12月11日，新建函数
	///
	tf::DeviceWorkState::type GetDevStateByKeyInfo(const tf::DevicekeyInfo& keyInfo);

    ///
    /// @brief  通过设备的名字或者组名获取设备的sn
    ///     
    ///
    /// @param[in]  name  设备名字或者设备的组名
    ///
    /// @return 设备序列号
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年6月7日，新建函数
    ///
    QStringList GetDeviceSnByName(const std::string& name);

    ///
    /// @brief 根据设备SN获取其绑定设备信息
    ///
    /// @param[in]  SN  设备序列号
    ///
    /// @return 设备信息
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年07月10日，新建函数
    ///
    inline std::shared_ptr<const tf::DeviceInfo> GetBindDeviceInfoBySn(const std::string& sn)
    {
        auto iter = m_mapDevSnAndBindSn.find(sn);
        return (iter == m_mapDevSnAndBindSn.end()) ? nullptr : GetDeviceInfo(iter->second);
    }

    ///
    /// @brief
    ///     设置项目信息
    ///
    /// @param[in]  spAI  项目信息
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年11月2日，新建函数
    ///
    bool SetAssayInfo(std::shared_ptr<tf::GeneralAssayInfo> spAI);

    ///
    /// @brief
    ///     获取对应类型的设备列表
    ///
    /// @param[in]  deviceType  设备类型
    ///
    /// @return 设备信息列表
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年1月12日，新建函数
    ///
    using DeviceType = tf::DeviceType::type;
    std::vector<std::shared_ptr<const tf::DeviceInfo>> GetDeviceFromType(std::vector<DeviceType> deviceTypes);

    ///
    /// @brief 是否存在某个类型的设备
    ///
    /// @param[in]  deviceClassify 设备类型
    ///
    /// @return true 存在
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2024年8月15日，新建函数
    ///
    bool IsDeviceExistedByAssayClassify(tf::AssayClassify::type deviceClassify);

    ///
    /// @brief
    ///     通过设备组名称获取设备
    ///
    /// @param[in]  deviceType  设备组名称
    ///
    /// @return 设备信息列表
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年4月25日，新建函数
    ///
    std::vector<std::shared_ptr<const tf::DeviceInfo>> GetDeviceByGroupName(const std::string& groupName);

    ///
    /// @brief
    ///     是否存在ISE设备
    ///
    /// @return 存在返回true
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年5月7日，新建函数
    ///
    bool HasIseDevice();

    ///
    /// @brief 获取当前项目对应的样本类型（此样本类型能够使用这个项目来做检测）
    ///
    /// @param[in]  assayCode  项目编号
    ///
    /// @return 样本类型列表
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年7月19日，新建函数
    ///
    void GetSampleTypeFromCode(int32_t assayCode, std::set<int32_t>& outSampleTypes);

    ///
    /// @brief
    ///     获取单实例
    ///
    /// @return 单实例
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年10月29日，新建函数
    ///
    static std::shared_ptr<CommonInformationManager> GetInstance();

    ///
    /// @brief 获取生化对应机型的生化通用参数
    ///
    /// @param[in]  type		机型
    /// @param[out]  indexMaps  参数列表
    ///
    /// @return true获取成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月17日，新建函数
    ///
    bool GetChAssayMaps(tf::DeviceType::type type, ChAssayIndexUniqueCodeMaps& indexMaps);

	///
	/// @brief 获取生化对应机型的生化通用参数(按照项目编号排序)
	///
	/// @param[in]  type		机型
	/// @param[out]  indexMaps  参数列表
	///
	/// @return true获取成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年3月22日，新建函数
	///
	bool GetChAssayMaps(tf::DeviceType::type type, ChAssayIndexUniqueOrderCodeMaps& indexOrderMaps);

	///
	/// @brief 获取耗材对于的模块地图信息（需要获取当前界面所有选择的样本类型，默认未血液）
	///
	/// @param[in]  supplyInfo  耗材信息
	///
	/// @return 模块信息
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年8月31日，新建函数
	///
	std::shared_ptr<ise::tf::IseModuleInfo> GetIseModuleMaps(ise::tf::SuppliesInfo& supplyInfo);

    ///
    /// @brief 获取ISE对应机型的免疫通用参数
    ///
    /// @param[in]   type       机型
    /// @param[out]  indexMaps  参数列表
    ///
    /// @return true获取成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月3日，新建函数
    ///
    bool GetIseAssayMaps(tf::DeviceType::type type, IseAssayIndexUniqueCodeMaps& indexMaps);

	///
	/// @brief 获取ISE对应机型的免疫通用参数(按编号排序)
	///
	/// @param[in]   type       机型
	/// @param[out]  indexMaps  参数列表
	///
	/// @return true获取成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年3月22日，新建函数
	///
	bool GetIseAssayMaps(tf::DeviceType::type type, IseAssayIndexUniqueOrderCodeMaps& indexMaps);

    ///
    /// @brief 获取免疫对应机型的免疫通用参数（按编号排序）
    ///
    /// @param[in]   type       机型
    /// @param[out]  indexMaps  参数列表
    ///
    /// @return true获取成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年11月8日，新建函数
    ///
    bool GetImAssayMaps(tf::DeviceType::type type, ImAssayIndexUniqueCodeMaps& indexMaps);

    ///
    /// @brief 获取免疫对应机型的通用项目参数（按编号排序）
    ///
    /// @param[in]   type       机型
    /// @param[out]  indexMaps  参数列表
    ///
    /// @return true获取成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年5月12日，新建函数
    ///
    bool GetImAssayMaps(tf::DeviceType::type type, ImAssayNameMaps& indexMaps);

    ///
    /// @brief
    ///     获取生化普通项目map
    ///
    /// @return 生化普通项目map常引用  
    ///
    /// @par History:
    /// @li 6950/ChenFei，2022年05月18日，新建函数
    ///
    inline const ChAssayIndexCodeMaps& GetChAssayIndexCodeMaps() { return m_mapChAssayCodeMaps; };

    ///
    /// @brief
    ///     获取免疫普通项目map
    ///
    /// @return 免疫普通项目map常引用    
    ///
    /// @par History:
    /// @li 8580/GongZhiQiang，2023年1月3日，新建函数
    ///
    inline const ImAssayIndexCodeMaps& GetImAssayIndexCodeMaps() { return m_mapImAssayCodeMaps; };

    ///
    /// @brief
    ///     获取ISE普通项目map
    ///
    /// @return ISE普通项目map常引用    
    ///
    /// @par History:
    /// @li 6950/ChenFei，2022年05月18日，新建函数
    ///
    inline const IseAssayIndexCodeMaps& GetIseAssayIndexCodeMaps() { return m_mapIseAssayCodeMaps; };

    ///
    /// @brief 获取所有的项目参数
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年6月1日，新建函数
    ///
    inline const AssayIndexNameMaps& GetGeneralAssayMap() { return m_mapAssayNameMaps; };

    ///
    /// @bref
    ///		将项目名列表转换成项目ID列表
    ///
    /// @param[in] assayNames 项目名列表
    /// @param[in] assayCode 项目ID列表
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年3月16日，新建函数
    ///
    void TranseAssayNameToAssayCode(const QStringList& assayNames, std::vector<int>& assayCode);

    ///
    /// @brief 获取所有的项目参数
    ///
    ///
    /// @par History:
    /// @li 7951/LuoXin，2022年9月15日，新建函数
    ///
    inline const AssayIndexCodeMaps& GetGeneralAssayCodeMap() { return m_mapAssayCodeMaps; };

    ///
    /// @brief
    ///     根据项目编号获取项目信息
    ///
    /// @param[in]  iAssayCode  项目编号
    ///
    /// @return 项目信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年11月2日，新建函数
    ///
    inline std::shared_ptr<::tf::GeneralAssayInfo> GetAssayInfo(int assayCode)
    {
        AssayIndexCodeMaps::iterator it = m_mapAssayCodeMaps.find(assayCode);
        return it != m_mapAssayCodeMaps.end() ? it->second : nullptr;
    }

    inline std::shared_ptr<::tf::GeneralAssayInfo> GetAssayInfo(const std::string& assayName)
    {
        AssayIndexNameMaps::iterator it = m_mapAssayNameMaps.find(assayName);
        return it == m_mapAssayNameMaps.end() ? nullptr : it->second;
    };

    ///
    /// @brief
    ///     更新通用项目信息
    ///
    /// @param[in]  spAssayInfo  项目信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年8月6日，新建函数
    ///
    void UpdateGnrAssayInfo(std::shared_ptr<::tf::GeneralAssayInfo> spAssayInfo);

    ///
    /// @bref
    ///		判断是否存在位置相同的项目
    ///
    /// @param[in] pageIdx 页码
    /// @param[in] positionIdx 位置序号
    /// @param[out] 获取到的位置相同的项目信息
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年6月15日，新建函数
    ///
    void GetSamePositiongAssayInfo(int pageIdx, int positionIdx, std::vector<std::shared_ptr<tf::GeneralAssayInfo>>& outAssays);

    ///
    /// @bref
    ///		更新指定项目的位置信息
    ///
    /// @param[in] assayCode 项目编号
    /// @param[in] pageIdx 页位置
    /// @param[in] positionIdx 序号位置
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年6月15日，新建函数
    ///
    bool UpdateAssayPosition(int assayCode, int pageIdx, int positionIdx);

    ///
    /// @brief
    ///     获取所有的设备分类
    ///
    /// @par History:
    /// @li 7951/LuoXin，2022年6月23日，新建函数
    ///
    std::vector<::tf::AssayClassify::type> GetAllDeviceClassify();

	///
	/// @brief 是否包含目标设备分类
	///
	/// @param[in]  classifyType  目标设备类型
	///
	/// @return true:包含
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年1月8日，新建函数
	///
	bool IsContainsAimClassify(::tf::AssayClassify::type classifyType);

    ///
    /// @brief 获取组合通用项目地图
    ///
    ///
    /// @return 项目的地图
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年6月27日，新建函数
    ///
    inline boost::optional<AssayProfileMap> GetCommonProfileMap() { return boost::make_optional(m_mapProfileAssay); };

    inline std::shared_ptr<tf::ProfileInfo> GetCommonProfile(int64_t profileId)
    {
        auto prof = m_mapProfileAssay.find(profileId);
        return prof != m_mapProfileAssay.end() ? prof->second : nullptr;
    }

    ///
    /// @brief 根据名称获取组合项目的数据库主键
    ///
    /// @param[in]  profileName  项目名称
    ///
    /// @return 组合项目数据库主键
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年6月29日，新建函数
    ///
    boost::optional<std::int64_t> GetProfileInfoFromName(const std::string& profileName);
    void GetProfileSubNames(const std::string& profileName, QStringList& subNames);

    ///
    /// @bref
    ///		获取组合项目的名字与子项目名，子项目名以顿号（、）分割
    ///
    /// @param[out] allProfileNames 输出组合项目名， <profilename, subassaynames>
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年3月15日，新建函数
    ///
    void GetProfileAssaysList(std::vector<std::pair<QString, QString>>& pfNamesCh, \
        std::vector<std::pair<QString, QString>>& pfNamesIm);
    bool DeleteProfileInfo(const std::string& profileName);
    bool AddProfileInfo(const tf::ProfileInfo& profileInfo);
    bool ModifyProfileInfo(const tf::ProfileInfo& profileInfo);

    ///
    /// @bref
    ///		更新组合项目的位置信息
    ///
    /// @param[in] assayCode 组合项目编号
    /// @param[in] pageIdx 页码
    /// @param[in] positionIdx 项目序号
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年6月15日，新建函数
    ///
    bool ModifyProfilePosition(int assayCode, int pageIdx, int positionIdx);

    ///
    /// @bref
    ///		获取相同位置的组合项目
    ///
    /// @param[in] pageIdx 页位置
    /// @param[in] positionIdx 项位置
    /// @param[out] sameProf 获取到的组合项目列表
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年6月15日，新建函数
    ///
    void GetSamePositionAssayInfoProf(int pageIdx, int positionIdx, std::vector<std::shared_ptr<tf::ProfileInfo>>& sameProf);

    ///
    /// @bref
    ///		从通用项目结构中获取区分了免疫与生化的项目名称
    ///
    /// @param[out] immyData 免疫项目名
    /// @param[out] chemiData 生化项目名
    /// @param[in] onlyShowed 是否在选择界面上显示的
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年9月4日，新建函数
    ///
    void GetDepartedAssayNames(QStringList& immyData, QStringList& chemiData, bool onlyShowed = false);

    ///
    /// @brief 根据类型码获取属性信息
    ///
    /// @param[in]  typeCode	   类型码
    /// @param[in]  IsSampleProbe  是否是样本针使用的耗材
    ///
    /// @return 耗材属性信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年7月5日，新建函数
    ///
    boost::optional<std::shared_ptr<ch::tf::SupplyAttribute>> GetSupplyAttributeByTypeCode(int typeCode, bool IsSampleProbe = false);

    ///
    /// @brief 根据类型码获取属性信息
    ///
    /// @param[in]  typeCode  类型码
    ///
    /// @return 耗材属性信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年7月13日，新建函数
    ///
    boost::optional<std::shared_ptr<im::tf::SupplyAttribute>> GetImSupplyAttributeByTypeCode(int typeCode);


    ///
    /// @brief 根据类型码获取属性信息
    ///
    /// @param[in]  typeCode  类型码
    ///
    /// @return 耗材属性信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年7月13日，新建函数
    ///
    boost::optional<std::shared_ptr<ise::tf::SupplyAttribute>> GetISESupplyAttributeByTypeCode(int typeCode);

    ///
    /// @brief 将当前单位与主单位进行换算
    ///
    /// @param[in]  assayCode  项目编号
    /// @param[in]  value  转换前的值
    ///
    /// @return 换算后的结果
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年9月5日，新建函数
    ///
    double ToMainUnitValue(int assayCode, double value);

    ///
    /// @brief 将主单位和当前单位进行换算
    ///
    /// @param[in]  assayCode  项目编号
    /// @param[in]  value  转换前的值
    ///
    /// @return 换算后的结果
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年9月5日，新建函数
    ///
    double ToCurrentUnitValue(int assayCode, double value);

    ///
    /// @brief	通过计算项目编号获取计算项目信息
    ///     
    /// @param[in]  assayCode  计算项目编号
    ///
    /// @return 计算项目信息
    ///
    /// @par History:
    /// @li 7951/LuoXin，2022年12月19日，新建函数
    ///
    inline std::shared_ptr<::tf::CalcAssayInfo> GetCalcAssayInfo(int assayCode)
    {
        auto it = m_calcAssayInfoMaps.find(assayCode);
        return (it != m_calcAssayInfoMaps.end() ? it->second : nullptr);
    }

    ///
    /// @brief	通过计算项目编号,病人信息获取计算项目结果参考区间
    ///     
    /// @param[in]  assayCode  计算项目编号
    ///
    /// @return 计算项目信息
    ///
    /// @par History:
    /// @li 6889/ChenWei，2023年11月22日，新建函数
    ///
    bool GetCalcAssayResultReference(int assayCode, int sourceType, std::shared_ptr<tf::PatientInfo> pPatientInfo, tf::AssayReferenceItem& matchRef);

    ///
    /// @brief  转换年龄单位
    ///
    /// @param[in]  iAge        现有单位对应年龄
    /// @param[in]  eAgeUnit    当前单位，参见 ::tf::AgeUnit::type 类型
    /// @param[in]  eExpectedAgeUnit  目标单位，参见 ::tf::AgeUnit::type 类型
    ///
    /// @return 转换后的年龄，小于0表示转换失败
    ///
    /// @par History:
    /// @li 6889/ChenWei，2023年11月22日，新建函数
    ///
    double ConvertAge(int iAge, int eAgeUnit, int eExpectedAgeUnit);

    ///
    /// @bref
    ///		返回计算项目缓存容器
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年10月20日，新建函数
    ///
    inline CalcAssayInfoMap GetCalcAssayInfoMap() { return m_calcAssayInfoMaps; }

    void GetCalcAssayInfo(std::vector<std::shared_ptr<tf::CalcAssayInfo>>& calcAssayInfo);
    std::shared_ptr<tf::CalcAssayInfo> GetCalcAssayInfoById(int calcItemId);

    bool ModifyCalcAssayInfo(const tf::CalcAssayInfo& calcItem);
    bool AddCalcAssayInfo(const tf::CalcAssayInfo& calcItem);
    bool DeleteCalcAssayInfoById(int calcId, int calcCode);

    ///
    /// @brief  检查当前项目的名字是否已经存在
    ///     
    /// @param[in]  assayCode  项目编号
    /// @param[in]  assayName  项目的名字
    ///
    /// @return 已经存在返回true
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年4月27日，新建函数
    ///
    bool CheckAssayNameExisted(int assayCode, const std::string& assayName) const;

    ///
    /// @brief 判断项目是否是ISE
    ///
    /// @param[in]  iAssayCode  项目编号
    ///
    /// @return true:是ISE
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年8月15日，新建函数
    ///
    inline bool IsAssayISE(int iAssayCode)
    {
        return (iAssayCode == ::ise::tf::g_ise_constants.ASSAY_CODE_NA
            || iAssayCode == ::ise::tf::g_ise_constants.ASSAY_CODE_K
            || iAssayCode == ::ise::tf::g_ise_constants.ASSAY_CODE_CL);
    }

    ///
    /// @brief 判断项目是否是LHI
    ///
    /// @param[in]  iAssayCode  项目编号
    ///
    /// @return true:是ILHI
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年8月15日，新建函数
    ///
    inline bool IsAssaySIND(int iAssayCode)
    {
        return (iAssayCode > ::ch::tf::g_ch_constants.ASSAY_CODE_SIND
            && iAssayCode <= ::ch::tf::g_ch_constants.ASSAY_CODE_I);
    }

    ///
    /// @bref
    ///		更新远端通用项目的质控规则设置，以及本对象缓存的质控规则设置
    ///
    /// @param[in] assayCode 指定的项目编号
    /// @param[in] qcRule 质控规则
    ///
    /// @par History:
    /// @li 8276/huchunli, 2022年12月27日，新建函数
    ///
    void UpdateQcRules(int64_t assayCode, const tf::QcJudgeParam& qcRule);

    ///
    /// @brief	获取当前单位转换倍率
    ///
    /// @param[in]  assayCode   项目编号
    /// @param[in]  factor		单位转换倍率
    ///
    /// @return		成功返回true
    ///
    /// @par History:
    /// @li 7951/LuoXin，2022年9月5日，新建函数
    ///
    bool GetUnitFactor(int assayCode, double& factor);

    ///
    /// @bref
    ///		获取指定设备类型的项目通道号
    ///
    /// @param[in] deviceType 项目类型
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年2月25日，新建函数
    ///
    void GetAssayCodeListCh(tf::DeviceType::type deviceType, std::vector<int32_t>& assayCodeList);
    void GetAssayCodeListIse(tf::DeviceType::type deviceType, std::vector<int32_t>& assayCodeList);
    void GetAssayCodeListIm(tf::DeviceType::type deviceType, std::vector<int32_t>& assayCodeList);

    ///
    /// @brief 根据设备类型和页面获取项目位置地图
    ///
    /// @param[in]  typeInfo  设备类型信息
    /// @param[in]  pageIdx   第几页
    /// @param[out]  outPage   返回项集（函数中没有清空outPage）
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年6月15日，新建函数
    ///
    void GetAssayItems(::tf::AssayClassify::type typeInfo, int32_t pageIdx, std::vector<tf::GeneralAssayPageItem>& outPage);

    ///
    /// @brief 获取组合项目信息，通过类型和页面
    ///
    /// @param[in]  typeInfo   设备类型信息
    /// @param[in]  pageIdx    第几页
    /// @param[out]  outPage   返回项集（函数中没有清空outPage）
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年6月15日，新建函数
    ///
    void GetAssayProfiles(::tf::AssayClassify::type typeInfo, int32_t pageIdx, std::vector<::tf::ProfileAssayPageItem>& outPage);

    ///
    /// @brief 获取当前单位
    ///
    /// @param[in]  assayCode  项目编号
    ///
    /// @return 单位
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年1月13日，新建函数
    ///
    QString GetCurrentUnit(int assayCode);

    ///
    /// @brief 获取项目主单位
    ///
    /// @param[in]  assayCode 项目编号
    ///
    /// @return 项目的主单位
    ///
    /// @par History:
    /// @li 8580/GongZhiQiang，2023年7月6日，新建函数
    ///
    QString GetAssayMainUnit(const int& assayCode);

    ///
    /// @bref
    ///		获取当前单位相对于主单位的变化率（所有数据基于主单位）
    ///
    /// @param[in] assayCode 项目编号
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年5月15日，新建函数
    ///
    double GetUnitChangeRate(int assayCode);

    ///
    /// @brief 判断是否计算项目
    ///
    /// @param[in]  assayCode  项目编号
    ///
    /// @return true:表示是计算项目否则非计算项目
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年11月28日，新建函数
    ///
    static inline bool IsCalcAssay(int assayCode)
    {
        return (assayCode <= tf::AssayCodeRange::CALC_RANGE_MAX && assayCode >= tf::AssayCodeRange::CALC_RANGE_MIN);
    }


	///
	/// @brief 判断计算项目中是否包含生化项目或者免疫项目
	///
	/// @param[in]  assayCode  计算项目编号
	/// @param[in]  isCh	   是否包含生化：true：生化，false:免疫(默认生化)
	///
	/// @return true:包含生化或者免疫
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年8月12日，新建函数
	///
	bool IsCaClHaveChorIM(int assayCode, bool isCh = true);

    ///
    /// @brief  根据项目编号，获得项目分类
    ///
    /// @param[in]  assayCode  项目编号
    ///
    /// @return 项目分类，参见 tf::AssayClassify
    ///
    /// @par History:
    /// @li 3558/ZhouGuangMing，2023年1月5日，新建函数
    ///
    static tf::AssayClassify::type GetAssayClassify(int assayCode);

    ///
    /// @brief  获取免疫显示字符串
    ///
    /// @param[in]  assayCode  通道号
    /// @param[in]  result  结果
    ///
    /// @return 显示字符
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年3月18日，新建函数
    ///
    QString GetImDisplyResultStr(int assayCode, const im::tf::AssayTestResult& result);

    ///
    /// @brief 架号位置号是否在范围之内
    ///
    /// @param[in]  type      样本架类型
    /// @param[in]  rack      架号
    ///
    /// @return true：在范围之外，false：范围内
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年4月17日，新建函数
    ///
    bool IsRackRangOut(::tf::RackType::type rackType, int rack);

	///
	/// @brief 获取架号范围
	///
	/// @param[in]  rackType  样本架类型
	///
	/// @return 某种类型的架号范围
	///
	/// @par History:
	/// @li 1556/Chenjianlin，2024年5月9日，新建函数
	///
	RackRangeOneType GetRackRange(::tf::RackType::type rackType);

	tf::RackType::type GetRackTypeFromRack(int rack);

    ///
    /// @brief  获取是否免疫单机模式
    ///
    /// @param[in]    void
    ///
    /// @return bool
    ///
    /// @par History:
    /// @li 7915/LeiDingXiang，2023年3月31日，新建函数
    ///
    bool GetImisSingle();

    ///
    /// @brief 判断是否免疫电机设备，若是则返回此设备，其他情况返回空
    ///
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 1556/Chenjianlin，2023年8月2日，新建函数
    ///
    std::shared_ptr<const tf::DeviceInfo> GetImSingleDevice();

    ///
    /// @brief	获取该单位转换倍率
    ///
    /// @param[in]  assayCode   项目编号
    /// @param[in]  unti        单位名称
    /// @param[in]  factor		单位转换倍率
    ///
    /// @return		成功返回true
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年3月28日，新建函数
    ///
    bool GetNewUnitFactor(int assayCode, const QString& unit, double& factor);

    // 查询项目简称是否已经存在
    bool IsExistGeneralAssayName(int assayCode, const std::string& assayName);

    // 删除指定的备选单位
    bool DeleteAssayUnit(const QString& assayName, const QString& unitName, QString& errInfo);
    bool CanDeleteAssayUnit(const QString& assayName, const QString& unitName, QString& errInfo);

    /// @bref
    ///		判断项目号是否在计算项目中存在
    ///
    /// @param[in] assayCode 待查询的项目号
    /// @param[out] calAssayName 如果存在，则返回的计算项目名
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年5月17日，新建函数
    ///
    bool IsExistAssayCodeInCalcAssayInfo(int assayCode, std::vector<std::string>& calAssayName);

    ///
    /// @brief  获取软件类型
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年5月12日，新建函数
    ///
    SOFTWARE_TYPE GetSoftWareType();

    ///
    /// @brief  判断是否有设备存在指定状态
    ///
    /// @param[in] status 状态
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年8月21日，新建函数
    ///
    bool HasDeviceEqualStatus(::tf::DeviceWorkState::type status);

    ///
    /// @bref
    ///		清理被删除的联合质控设置
    ///
    /// @param[in] qcDocId 涉及的质控品ID
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年7月3日，新建函数
    ///
    void DeleteUnionQcSettings(int64_t qcDocId);

    ///
    /// @bref
    ///		获取项目位置表的单项目行数
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年8月17日，新建函数
    ///
    inline int  GetSingleAssayRowAmount() { return m_singleAssayRowAmount; }

	///
	/// @brief 获取项目的自动显示顺序
	///
	///
	/// @return 显示顺序map
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年11月15日，新建函数
	///
	inline std::map<int, int> GetAssayDefaultMap() { return m_defaultPostionMap; }

    ///
    /// @bref
    ///		设置项目位置表的单项目行数
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年8月17日，新建函数
    ///
    void SetSingleAssayRowAmount(int assayRowAmount);

    ///
    /// @brief 样本量/稀释倍数(导出数据用，未匹配时，返回一个空格)
    ///
    /// @param[in]  spAssayTestResult  测试结果指针
    ///
    /// @return 组合后的倍数
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年8月28日，新建函数
    ///
    QString GetDilutionFactor(std::shared_ptr<ch::tf::AssayTestResult> spAssayTestResult);

    ///
    /// @brief 获取是否存在默认数据项目
    ///
    /// @param[in]  type  病人信息类型（参考tf::PatientFields::type）
    /// @param[in]  csrData  出参
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年10月20日，新建函数
    ///
    bool GetDefaultDictData(int type, std::vector<CustomSetRowData>& csrData);

	///
	/// @brief 根据告警码获取解释
	///
	/// @param[in]  codes  告警码
	///
	/// @return 告警详细解释
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年11月2日，新建函数
	///
	QString GetResultStatusDetail(QString code);

    ///
    /// @brief
    ///     设备是否温度异常
    ///
    /// @param[in]  strDevSn  设备序列号
    ///
    /// @return true表示温度异常
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年10月30日，新建函数
    ///
    bool IsDevTemperatureErr(const QString& strDevSn);

    ///
    /// @brief  设置双项同测后更新生化特殊项目信息
    ///     
    ///
    /// @param[in]  firstCode   前项目编号
    /// @param[in]  secondCode  后项目编号
    /// @param[in]  editBeforeSecondCode  修改的后项目编号（-1表示修改前没有后项目）
    ///
    /// @return 
    ///
    /// @li 7951/LuoXin，2023年12月1日，新建函数
    ///
    void ReloadAssayInfoChSpecialAfterEditTwins(int firstCode, int secondCode, int editBeforeSecondCode = -1);

    ///
    /// @bref
    ///		更新设备的自动开机计划
    ///
    /// @param[in] mapDevice2Pos 自动开机计划<deviceSn, schedule>
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年11月22日，新建函数
    ///
    bool UpdateDevicePoweronSchedul(const std::map<std::string, ::tf::PowerOnSchedule>& mapDevice2Pos);

	///
	/// @brief  根据设备序列号获取在线试剂加载信息
	///
	/// @param[in]  strDevSn  设备序列号
	///
	/// @return 在线试剂加载信息
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年11月24日，新建函数
	///
    inline ::ch::tf::ReagentLoaderInfo getReagentLoaderByDevSn(const QString& strDevSn) 
    {
        return (m_reagentLoaderMap.contains(strDevSn)) ? m_reagentLoaderMap[strDevSn] : ::ch::tf::ReagentLoaderInfo();
    };

	///
	/// @brief  项目未分配
	///
	/// @param[in]  deviceSn   设备序列号
	/// @param[in]  assayCode  项目编号
	///
	/// @return true表示未分配
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年12月25日，新建函数
	///
	bool IsUnAllocatedAssay(const std::string &deviceSn, const int assayCode);

	///
	/// @brief 当前系统是否支持目标设备类型
	///
	/// @param[in]  devType  目标设备类型
	///
	/// @return true:支持该设备类型
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年2月22日，新建函数
	///
	bool IsSupportAimDevType(const tf::DeviceType::type& devType);

    ///
    /// @brief 根据设备列表，获取所有的试剂组
    ///
    /// @param[in]  devices  设备列表
    ///
    /// @return 设备的试剂组
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2024年3月28日，新建函数
    ///
    std::vector<ch::tf::ReagentGroup> GetAllChReagentGroupsByDevice(const std::vector<std::string>& devices);

	///
	/// @brief  根据设备序列号和耗材编号获取生化仓内同组的试剂信息
	///
	/// @param[in]  devSn  设备序列号
	/// @param[in]  suppliesCode 耗材编号
	///
	/// @return  生化仓内同组的试剂信息
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年3月19日，新建函数
	///
	std::vector<ch::tf::ReagentGroup> GetChGroupReagents(const std::string& devSn, const int suppliesCode);

	///
	/// @brief  根据设备序列号和耗材编号获取生化仓内同组的耗材信息
	///
	/// @param[in]  devSn  设备序列号
	/// @param[in]  supplyCode 耗材编号
	///
	/// @return  生化仓内同组的耗材信息
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年3月19日，新建函数
	///
	std::vector<ch::tf::SuppliesInfo> GetChGroupSupplies(const std::string& devSn, const int supplyCode);

	///
	/// @brief  根据设备序列号和耗材编号获取生化仓内试剂&耗材信息
	///
	/// @param[in]  devSn  设备序列号
	/// @param[in]  setPos 查询的位置 空表示查询所有
	///
	/// @return  生化仓内试剂&耗材信息
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年3月19日，新建函数
	///
	std::map<int, ChReagentInfo> GetChDiskReagentSupplies(const std::string& devSn, const std::set<int> &setPos = {});

    ///
    /// @brief  指定项目是否有在机试剂
    ///
    /// @param[in]  code  项目编号
    ///
    /// @return  存在返回true
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年5月24日，新建函数
    ///
    bool IsExistOnUsedReagentByCode(int code);

	///
	/// @brief  根据设备序列号获取生化仓外耗材信息
	///
	/// @param[in]  devSn  设备序列号
	/// @param[in]  setPos 查询的位置 空表示查询所有
	///
	/// @return  生化仓外耗材信息
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年3月19日，新建函数
	///
	std::map<int, ::ch::tf::SuppliesInfo> GetChCabinetSupplies(const std::string& devSn, const std::set<int> &setPos = {});

	///
	/// @brief  根据设备序列号获取ise耗材信息
	///
	/// @param[in]  devSn  设备序列号
	/// @param[in]  setPos 查询的位置 空表示查询所有
	///
	/// @return ise耗材信息
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年3月19日，新建函数
	///
	std::map<std::pair<int, int>, ::ise::tf::SuppliesInfo> GetIseSupplies(const std::string& devSn,const std::set< std::pair<int, int>> &setPos = {});

    ///
    /// @brief	检查当前模式下的试剂可以次数
    ///  
    /// @param[in]  assayAndTimes 待测项目的编号和次数
    ///  
    /// @return true：当前试剂次数够用
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年4月8日，新建函数
    ///
    bool CheckReagentRemainCount(const std::map<int, int>& assayAndTimes);

	///
	/// @brief  获取指定设备序列号的废液量
	///
	/// @param[in]  devSN  设备序列号
	///
	/// @return 废液量
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年5月18日，新建函数
	///
    inline int GetChWasteCnt(const std::string& devSN) { return (m_chWasteCntMap.count(devSN)) ? m_chWasteCntMap[devSN] : 0; };

    ///
    /// @brief  更新项目的参考范围
    ///     
    ///
    /// @param[in]  code  项目编号
    /// @param[in]  sampleSourceType  样本源类型
    /// @param[in]  version  参数版本
    /// @param[in]  referrence  参考范围
    ///
    /// @return 成功返回true
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年6月4日，新建函数
    ///
    bool updateReferenceRanges(int code, int sampleSourceType, const std::string& version, const ::tf::AssayReference& referrence);

protected:
    CommonInformationManager();

    ///
    /// @brief
    ///     初始化项目映射
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年11月2日，新建函数
    ///
    bool InitAssayReflection();

    ///
    /// @brief
    ///     初始化设备信息映射表
    ///
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年1月11日，新建函数
    ///
    bool InitDeviceReflection();

public Q_SLOTS :

    ///
    /// @brief
    ///     更新项目配置信息
    ///
    /// @param[in]  assayCode  项目编号
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年11月4日，新建函数
    ///
	void UpdateAssayCfgInfo(const QVector<int32_t>& assayCodes);

protected Q_SLOTS:

    ///
    /// @brief
    ///     更新组合项目信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年11月4日，新建函数
    ///
    void UpdateProfileInfo();

    ///
    /// @brief	设备信息更新
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2022年10月8日，新建函数
    ///
    void UpdateDeviceInfo();

    ///
    /// @brief	设备信息更新
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2023年6月5日，新建函数
    ///
    void UpdateDeviceStatus(tf::DeviceInfo deviceInfo);

    ///
    /// @brief	更新设备是否可追加测试样本的状态
    ///
    /// @param[in]  devSN   设备序列号
    /// @param[in]  faats   影响开始（追加）测试样本的异常
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年3月22日，新建函数
    ///
    void UpdateDeviceCanAppendTest(QString devSN, std::set<enum tf::FaultAffectAppendTest::type, 
		std::less<enum tf::FaultAffectAppendTest::type>, std::allocator<enum tf::FaultAffectAppendTest::type>> faats);

    ///
    /// @brief	计算项目信息更新
    /// 
    /// @param[in]  calcAssayCode   计算项目编号
    /// 
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 7951/LuoXin，2022年12月19日，新建函数
    ///
    bool UpdateCalcAssayInfo(int calcAssayCode = -1);

    ///
    /// @brief	更新耗材属性
    /// 
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年9月13日，新建函数
    ///
    void UpdateSupplyAttribute();

	///
	/// @brief 更新数据报警ｍａｐ
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年11月2日，新建函数
	///
	void UpdateDataAlarmMap();

    ///
    /// @brief
    ///     设备温度异常状态改变
    ///
    /// @param[in]  strDevSn    设备序列号
    /// @param[in]  bErr        是否温度异常
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年10月30日，新建函数
    ///
    void OnDevTemperatureErrChanged(QString strDevSn, bool bErr);

	///
	/// @brief 电解质模块校准状态的变化（ise的校准是针对模块，bug0014288）
	///
	/// @param[in]  iseAppUpdates  更新信息
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年9月2日，新建函数
	///
	void OnUpdateCaliAppSatus(class std::vector<class ise::tf::IseModuleInfo, class std::allocator<class ise::tf::IseModuleInfo>> iseAppUpdates);

	///
	/// @brief  试剂报警值更新
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年11月24日，新建函数
	///
	void OnReagentAlarmValueChanged();

	///
	/// @brief  在线试剂加载器信息更新
	///
	/// @param[in]  deviceSN  
	/// @param[in]  rliloaderUpdate  
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年11月24日，新建函数
	///
	void OnChReagentLoaderInfoUpdate(QString deviceSN, class ch::tf::ReagentLoaderInfo loaderUpdate);

	///
	/// @brief   生化试剂&耗材更新消息
	///
	/// @param[in]  supplyUpdates  更新位置信息
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年3月19日，新建函数
	///
	void OnUpdateChReagentSupply(std::vector<ch::tf::SupplyUpdate, std::allocator<ch::tf::SupplyUpdate>> supplyUpdates);

	///
	/// @brief   ISE耗材更新消息
	///
	/// @param[in]  supplyUpdates  更新位置信息
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年3月19日，新建函数
	///
	void OnUpdateISeSupply(std::vector<ise::tf::SupplyUpdate, std::allocator<ise::tf::SupplyUpdate>> supplyUpdates);

	///
	/// @brief  更新废液桶
	///
	/// @param[in]  statusInfo  设备信息 
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年3月19日，新建函数
	///
	void OnUpdateBucketStatus(class tf::UnitStatusInfo statusInfo);
	
private:

    ///
    /// @brief 获取生化指定项目编号的样本类型
    /// 
    void GetSampleTypeCh(int assayCode, std::set<int>& sampleType);

    ///
    /// @bref
    ///		获取免疫指定项目编号的样本类型（tf::DeviceType::DEVICE_TYPE_I6000）
    ///
    /// @param[in] assayCode 项目编号
    /// @param[out] sampleType 项目类型
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年8月25日，新建函数
    ///
    void GetSampleTypeIm(int assayCode, std::set<int>& sampleType);

private:

    ///
    /// @brief  重新加载通用项目信息
    ///     
    ///
    /// @param[in]  assayCode  项目编号（-1：刷新所有的项目信息）
    ///
    /// @return 
    ///
    /// @par History:由原void ReloadGeneralAssayInfo()函数改造而来
    /// @li 7951/LuoXin，2023年9月13日，新建函数
    ///
    void ReloadGeneralAssayInfo(int assayCode = -1);

    ///
    /// @brief  重新加载生化通用项目信息
    ///     
    ///
    /// @param[in]  assayCode  项目编号（-1：刷新所有的项目信息）
    ///
    /// @return 
    ///
    /// @par History:由原void ReloadAssayInfoCh()函数改造而来
    /// @li 7951/LuoXin，2023年9月13日，新建函数
    ///
    void ReloadAssayInfoCh(int assayCode = -1);

    ///
    /// @brief  重新加载生化特殊项目信息
    ///     
    ///
    /// @param[in]  assayCode  项目编号（-1：刷新所有的项目信息）
    ///
    /// @return 
    ///
    /// @par History:由原void ReloadAssayInfoChSpecial()函数改造而来
    /// @li 7951/LuoXin，2023年9月13日，新建函数
    ///
    void ReloadAssayInfoChSpecial(int assayCode = -1);

    ///
    /// @brief  重新加载免疫项目信息
    ///     
    ///
    /// @param[in]  assayCode  项目编号（-1：刷新所有的项目信息）
    ///
    /// @return 
    ///
    /// @par History:由原void ReloadAssayInfoIm()函数改造而来
    /// @li 7951/LuoXin，2023年9月13日，新建函数
    ///
    void ReloadAssayInfoIm(int assayCode = -1);

    ///
    /// @brief  重新加载ISE通用项目信息
    ///
    /// @return 
    ///
    /// @par History:由原void ReloadAssayInfoIse()函数改造而来
    /// @li 7951/LuoXin，2023年9月13日，新建函数
    ///
    void ReloadAssayInfoIse();

    ///
    /// @brief  重新加载ISE特殊项目信息
    ///     
    /// @return 
    ///
    /// @par History:由原void ReloadAssayInfoIseSpecial()函数改造而来
    /// @li 7951/LuoXin，2023年9月13日，新建函数
    ///
    void ReloadAssayInfoIseSpecial();

    // 重新加载通用项目组合信息
    void ReloadGeneralProfile();

    // 重新加载生化耗材属性信息
    void ReloadSupplyAttributeCh();
    // 重新加载ISE耗材属性信息
    void ReloadSupplyAttributeIse();
    // 重新加载Im耗材属性信息
    void ReloadSupplyAttributeIm();

    ///
    /// @bref
    ///		用设备列表更新缓存的设备列表
    ///
    /// @param[in] vecDev 新的设备列表
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年11月25日，新建函数
    ///
    void UpdateDeviceBuff(const std::vector<tf::DeviceInfo>& vecDev);

	///
	/// @brief  初始化生化设备信息(包括试剂、耗材信息，加载器信息等)
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年3月19日，新建函数
	///
	void InitChDeviceInfo();

	///
	/// @brief  初始化ISE设备信息(包括耗材信息等)
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年3月19日，新建函数
	///
	void InitIseDeviceInfo();

private:
    static std::shared_ptr<CommonInformationManager> sm_instance; ///< 单实例对象

    AssayIndexCodeMaps              m_mapAssayCodeMaps;			///< 项目映射表
	std::map<int, int>				m_defaultPostionMap;		///< 项目默认显示顺序（项目编号-位置号）
    AssayIndexNameMaps              m_mapAssayNameMaps;			///< 项目名称映射表
    ChAssayIndexCodeMaps            m_mapChAssayCodeMaps;	    ///< 生化项目映射表
    ChSpecialAssayIndexMaps			m_mapChSpecialAssayCodeMaps;///< 生化项目专用映射表
    IseAssayIndexCodeMaps			m_mapIseAssayCodeMaps;		///< ISE项目映射表
    IseSpecialAssayIndexMaps        m_mapIseSpecialAssayCodeMaps;///< ISE特殊项目映射表
    ImAssayIndexCodeMaps            m_mapImAssayCodeMaps;	    ///< 免疫项目映射表
    AssayProfileMap					m_mapProfileAssay;			///< 组合项目的映射表
    std::mutex						m_devMapMtx;				///< 设备信息表的互斥量
    DeviceMaps                      m_mapDevices;               ///< 设备信息映射表(设备sn--设备信息)
    SupplyAttributeMap				m_supplyAttributeMap;		///< 耗材的属性地图
    ISESupplyAttributeMap			m_iseSupplyAttributeMap;	///< 耗材的属性地图(ise)
    ImSupplyAttributeMap			m_ImSupplyAttributeMap;	    ///< 耗材的属性地图(im)

    CalcAssayInfoMap				m_calcAssayInfoMaps;		///< 计算项目映射表
    QMap<int, QString>              m_nameMap;                  ///< 工作页面标题栏名称映射
    QMap<int, QString>              m_rgntHeaderMap;            ///< 工作页面标题栏名称映射
    QMap<QString, bool>             m_deviceTemperatureErrMap;  ///< 设备温度异常映射
	IseModuleMaps					m_iseModuleMaps;			///< ise的模块地图
	std::map<std::string, tf::DataAlarmItem> m_dataAlarmMap;	///< 数据报警地图
    int                             m_singleAssayRowAmount = 5; ///< 缓存项目位置列表的单项目所占行数
	ChReagentLoaderMaps				m_reagentLoaderMap;			///< 在线试剂加载器信息

	ChReagentDiskInfoMaps			m_chReagentDiskInfoMap;		///< 生化试剂盘信息 (设备序列号, (位置，试剂信息))
	ChCabinetSuppliesInfoMaps		m_chCabinetSuppliesInfoMap;	///< 生化仓外耗材信息 (设备序列号, (位置，耗材信息))
	IseCabinetSuppliesInfoMaps		m_iseCabinetSuppliesInfoMap;///< ise仓外耗材信息 (设备序列号, (模块号，位置)，耗材信息))
	std::map<std::string, int64_t>	m_chWasteCntMap;			///< 缓存生化废液桶量(设备序列号，缓存废液桶量)
    std::map<std::string, std::string> m_mapDevSnAndBindSn;     ///< 缓存设备序列号和其绑定设备的序列号  (设备序列号，与其绑定设备的序列号)  
};

#define CIM_INSTANCE	CommonInformationManager::GetInstance() // 公共数据管理器实例简写宏
