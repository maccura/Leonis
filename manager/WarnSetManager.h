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
/// @file     WarnSetManager.h
/// @brief    系统总览-报警设置管理器头文件
///
/// @author   7997/XuXiaoLong
/// @date     2023年8月16日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7997/XuXiaoLong，2023年8月16日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <mutex>
#include <set>
#include <QObject>
#include <boost/noncopyable.hpp>
#include <boost/optional/optional.hpp>
#include "src/thrift/im/gen-cpp/im_types.h"
#include "src/thrift/ch/gen-cpp/ch_types.h"
#include "src/thrift/ise/gen-cpp/ise_types.h"
using namespace std;

struct ChReagentInfo;
class QTimer;

//< 报警设置类型
typedef enum EnumWarnType
{
	EWT_ISE_SUPPLY = 0,		// ISE耗材
	EWT_CH_REAGENT,			// 生化试剂
	EWT_CH_SUPPLY,			// 生化耗材
	EWT_CH_LIQUID_WASTE,	// 生化废液
	EWT_IM_REAGENT,			// 免疫试剂
	EWT_IM_SUPPLY,			// 免疫耗材
	EWT_IM_LIQUID_WASTE,	// 免疫废液
	EWT_IM_TRASH_CAN,		// 免疫废桶
}EM_WT;

//< 报警等级
typedef enum EnumWarnLevel
{
	EWL_NOTICE = 0,			// 注意（黄色）
	EWL_WARNING,			// 警告（红色）
}EM_WL;

//< 报警瓶子位置
typedef enum EnumBottlePos
{
	EBP_ISE_SUPPLY = 0,		// ISE耗材
	EBP_CH_REAGENT,			// 试剂（生化）
	EBP_CH_SUPPLY,			// 耗材（生化）
	EBP_CH_LIQUID_WASTE,	// 废液桶（生化）
	EBP_IM_REAGENT,			// 试剂（免疫）
	EBP_IM_SUPPLY,			// 耗材（免疫）
	EBP_IM_TRASH_CAN,		// 垃圾桶（免疫）
	EBP_IM_LIQUID_WASTE		// 废液桶（免疫）
}EM_BP;

struct SystemOverviewWarnSet;

class CacheImSupplyattributes
{
public:
    void ReloadAttributes();
    void QueryAttributes(std::vector<::im::tf::SupplyAttribute>& spplyAttributes);
    void QueryAttributes(std::vector<::im::tf::SupplyAttribute>& spplyAttributes, im::tf::SuppliesType::type attrType);

private:
    std::vector<::im::tf::SupplyAttribute> allAttributes;
};

///
/// @brief   系统总览-报警设置管理
///
class WarnSetManager: public QObject, public boost::noncopyable
{
	Q_OBJECT

protected:

	///
	/// @brief  构造函数
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年8月17日，新建函数
	///
	WarnSetManager();

public:
    ///
    /// @brief  获得单例对象
    ///
    /// @return 报警设置管理单例对象
    ///
    /// @par History:
    /// @li 7997/XuXiaoLong，2023年8月16日，新建函数
    ///
    static std::shared_ptr<WarnSetManager> GetInstance();

	///
	/// @brief  初始化
	///
	/// @return true 表示初始化成功
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年8月18日，新建函数
	///
	bool init();

	///
	/// @brief  过滤并生成报警列表结果
	///
	/// @return  过滤后的报警列表结果
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年8月17日，新建函数
	///
	QMap<QString, QMap<EM_BP, EM_WL>> filterGenerateWarns();

    ///
    /// @brief
    ///     忽略免疫耗材更新
    ///
    /// @param[in]  bIgnore  忽略
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年6月21日，新建函数
    ///
    void SetIgnoreImSplUpdate(bool bIgnore);

	///
	/// @brief  设置系统总览显隐
	///
	/// @param[in]  bShow true表示显示  
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年8月15日，新建函数
	///
	void SetSysVisble(const bool bShow);

private:
	///
	/// @brief  更新报警信息
	///
	/// @param[in]  devSn		设备序列号
	/// @param[in]  type		报警类型
	/// @param[in]  bottlePos	报警瓶子位置
	/// @param[in]  index		同类型报警索引
	///							CH、IM试剂耗材:位置
	///							CH、IM校准:位置
	///							ISE耗材:模块索引 + "-" + 位置
	///							ISE校准:设备序列号
	/// @param[in]  codeLev		报警代码和等级
	/// @param[in]  resolve		是否解决（默认false-未解决）
	///
	/// @return 
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年8月17日，新建函数
	///
	void updateWarn(const string &devSn, const EM_WT type, const EM_BP bottlePos,
		const string& index, const pair<int, EM_WL>& codeLev, const bool resolve = false);

	///
	/// @brief  删除匹配的报警
	///
	/// @param[in]  devSn  设备序列号
	/// @param[in]  type   报警类型
	/// @param[in]  index  同类型报警索引
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年8月31日，新建函数
	///
	void deleteWarn(const string &devSn, const EM_WT type, const string& index);

    ///
    /// @brief
    ///     清空某个设备某个类型的告警
    ///
    /// @param[in]  type    告警类型
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年4月22日，新建函数
    ///
    void clearWarn(const EM_WT type);

	///
	/// @brief  初始化关注的报警设置列表
	///
	/// @return true 表示成功
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年8月23日，新建函数
	///
	bool initCareWarns();	

	///
	/// @brief  初始化缓存报警列表
	///
	/// @return true 表示成功
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年8月24日，新建函数
	///
	bool initTempWarns();

	///
	/// @brief  初始化生化设备报警
	///
	/// @param[in]  devices  生化设备序列号列表
	///
	/// @return		true 表示成功
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年8月24日，新建函数
	///
	bool initChWarns(const std::vector<std::string>& devices);

	///
	/// @brief  初始化免疫设备报警
	///
	/// @param[in]  devices  免疫设备序列号列表
	///
	/// @return		true 表示成功
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年8月24日，新建函数
	///
	bool initImWarns(const std::vector<std::string>& devices);

	///
	/// @brief  初始化ISE设备报警
	///
	/// @param[in]  devices		  ISE设备序列号列表
	///
	/// @return		true 表示成功
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年8月24日，新建函数
	///
	bool initIseWarns(const std::vector<std::string>& devices);

	///
	/// @brief  更新ise模块信息
	///
	/// @param[in]  devSn  设备序列号
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年9月1日，新建函数
	///
	void updateIseModule(const string& devSn);

	///
	/// @brief  更新单个生化试剂报警
	///
	/// @param[in]  cri  试剂信息
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年8月24日，新建函数
	///
	void updateChReagent(const ChReagentInfo& cri);

	///
	/// @brief  更新单个生化耗材报警(仓内仓外的耗材)
	///
	/// @param[in]  type  EWT_CH_REAGENT:仓内耗材  EWT_CH_SUPPLY:仓外耗材
	/// @param[in]  si    耗材信息
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年8月24日，新建函数
	///
	void updateChSupply(const EM_WT type,const ch::tf::SuppliesInfo& si);

	///
	/// @brief  更新生化废液
	///
	/// @param[in]  devSN 设备序列号
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年8月24日，新建函数
	///
	void updateChWasteLiquid(const std::string& devSN);

	///
	/// @brief  更新单个免疫试剂报警
	///
	/// @param[in]  rit  试剂信息
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年8月24日，新建函数
	///
	void updateImReagent(const im::tf::ReagentInfoTable& rit);

	///
	/// @brief  更新单个免疫稀释液报警
	///
	/// @param[in]  dit  稀释液信息
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年8月24日，新建函数
	///
	void updateImDiluent(const im::tf::DiluentInfoTable& dit);

	///
	/// @brief  更新单个免疫耗材报警
	///
	/// @param[in]  sit  耗材信息
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年8月24日，新建函数
	///
	void updateImSupply(const im::tf::SuppliesInfoTable& sit);

	///
	/// @brief  更新单个ISE耗材报警
	///
	/// @param[in]  si  耗材信息
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年8月31日，新建函数
	///
	void updateIseSupply(const ise::tf::SuppliesInfo& si);

	///
	/// @brief  更新系统总览报警瓶子
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年8月17日，新建函数
	///
	void updateSysWarnBottles();

	///
	/// @brief  判断是否过期
	///
	/// @param[in]  exprieTime  过期时间
	///
	/// @return true 表示正常未过期
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年8月25日，新建函数
	///
	bool judgeExprie(const std::string& exprieTime);

	///
	/// @brief  判断项目模块余量
	///
	/// @param[in]  devSn		设备序列号
	/// @param[in]  pos			位置
	/// @param[in]  code		项目编号
	/// @param[in]  canUseTimes	可用测试数
	/// @param[in]  isCh		是生化试剂还是免疫试剂，默认生化
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年8月25日，新建函数
	///
	void judgeProModuleMargin(const std::string& devSn,const int pos,const int code,const int canUseTimes, const bool isCh = true);

	///
	/// @brief  判断免疫模块余量
	///
	/// @param[in]  devSn		设备序列号
	/// @param[in]  pos			位置
	/// @param[in]  type		耗材类型
	/// @param[in]  tType		EWT_IM_REAGENT:仓内耗材  EWT_IM_SUPPLY:仓外耗材
	/// @param[in]  bPos		报警位置
	/// @param[in]  remainCount	余量
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年8月25日，新建函数
	///
	void judgeImModuleMargin(const std::string& devSn, const string& pos, const im::tf::SuppliesType::type type,
		const EM_WT tType, const EM_BP bPos, const int remainCount);

	///
	/// @brief  判断生化校准曲线过期
	///
	/// @param[in]  rg  试剂组信息
	///
	/// @return (瓶校准曲线过期标志、批校准曲线过期标志)
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年8月28日，新建函数
	///
	std::pair<bool, bool> judgeChCaliCurveExpire(const ch::tf::ReagentGroup& rg);

	///
	/// @brief  判断ISE校准曲线过期
	///
	/// @param[in]  caliCurveId  校准曲线id
	///
	/// @return true 表示正常未过期
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年9月1日，新建函数
	///
	bool judgeISeCaliCurveExpire(const int caliCurveId);

	///
	/// @brief  获取ise校准信息列表
	///
	/// @param[in]  devSn  设备序列号
	///
	/// @return ise校准信息列表
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年9月1日，新建函数
	///
	std::vector<::ise::tf::IseModuleInfo> getIseModuleInfos(const std::string& devSn);

protected Q_SLOTS:
	///
	/// @brief  更新关注的报警设置列表
	///
	/// @param[in]  sows  关注的报警设置列表
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年8月16日，新建函数
	///
	void slotUpdateCareWarns(const SystemOverviewWarnSet &sows);

	///
	/// @brief  生化试剂/耗材更新
	///   
	/// @param[in]  supplyUpdates  更新的信息
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年8月24日，新建函数
	///
	void slotUpdateChReagentSupply(std::vector<ch::tf::SupplyUpdate, std::allocator<ch::tf::SupplyUpdate>> supplyUpdates);

	///
	/// @brief  免疫试剂更新
	///
	/// @param[in]  ri    更新的试剂信息
	/// @param[in]  type  更新类型
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年8月24日，新建函数
	///
	void slotUpdateImReagent(const im::tf::ReagentInfoTable& ri, im::tf::ChangeType::type type);

	///
	/// @brief  免疫耗材更新
	///
	/// @param[in]  si    更新的耗材信息
	/// @param[in]  type  更新类型
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年8月24日，新建函数
	///
	void slotUpdateImSupply(const im::tf::SuppliesInfoTable& si, im::tf::ChangeType::type type);

	///
	/// @brief  免疫稀释液更新
	///
	/// @param[in]  di	  更新的稀释液信息
	/// @param[in]  type  更新类型
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年8月24日，新建函数
	///
	void slotUpdateImDiluent(const im::tf::DiluentInfoTable& di, im::tf::ChangeType::type type);

	///
	/// @brief  ISE耗材更新
	///
	/// @param[in]  supplyUpdates  更新的耗材信息
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年8月31日，新建函数
	///
	void slotUpdateIseSupply(std::vector<ise::tf::SupplyUpdate, std::allocator<ise::tf::SupplyUpdate>> supplyUpdates);

	///
	/// @brief  ise校准申请信息更新
	///  
	/// @param[in]  iseAppUpdates  校准申请信息
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年9月1日，新建函数
	///
	void slotUpdateCaliAppSatus(std::vector<ise::tf::IseModuleInfo, std::allocator<ise::tf::IseModuleInfo>> iseAppUpdates);

    ///
    /// @brief
    ///     试剂耗材的提醒值更新
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年4月22日，新建函数
    ///
    void slotUpdateAlarmVol();

    ///
    /// @brief  重新加载告警(可优化)
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年4月22日，新建函数
    ///
    void ReloadWarns();

	///
	/// @brief  更新废液桶
	///
	/// @param[in]  devSN  设备序列号
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年3月19日，新建函数
	///
	void slotUpdateBucketStatus(const QString& devSN);

	///
	/// @brief 校准提示发生变化
	///
	/// @param[in]  bOn  开关状态
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年7月11日，新建函数
	///
	void OnCaliExpireNoticeChanged(bool bOn);

	///
	/// @brief  质控结果更新
	///
	/// @param[in]  devSn		设备序列号
	/// @param[in]  qcDocId		质控文档id
	/// @param[in]  changedId	质控品结果主键
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年8月12日，新建函数
	///
	void slotQcResultUpdate(QString devSn, int64_t qcDocId, QVector<int64_t> changedId);

	///
	/// @brief  项目信息更新（可能是校准周期改变，刷新校准相关的报警）
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年9月5日，新建函数
	///
	void OnAssayInfoUpdate();

public Q_SLOTS:

	///
	/// @brief  更新时间相关的参数
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年8月15日，新建函数
	///
	void OnUpdateTimeDependParam();

private:
	//< 报警详细信息
	typedef struct WarnDetailInfo								
	{
		string	m_devSn;			//< 设备序列号
		EM_WT	m_type;				//< 报警类型
		EM_BP	m_bottlePos;		//< 报警瓶子位置
		string	m_index;			//< 同类型报警索引
		pair<int, EM_WL> m_codeLev;	//< 报警代码和等级
	
		// 构造函数
		WarnDetailInfo(const string &devSn, const EM_WT type, const EM_BP bottlePos, const string& index,const pair<int, EM_WL>& codeLev)
			: m_devSn(devSn), m_type(type), m_bottlePos(bottlePos), m_index(index), m_codeLev(codeLev)
		{
		}

		// 重载== 比较相等
        bool operator==(const WarnDetailInfo& other) const;

		// 重载< 用于放入set中
        bool operator<(const WarnDetailInfo& other) const;

	}ST_WDI;

private:
    static std::shared_ptr<WarnSetManager> s_instance;			///< 单例对象
    static std::recursive_mutex		s_mtx;						///< 保护单例对象的互斥量

	map<EM_WT, set<int>>			m_mapCareWarns;				///< 关注的报警列表
	std::recursive_mutex			m_careWarnsMtx;				///< 关注的报警列表锁

	set<ST_WDI>						m_setTempWarns;				///< 缓存上一次的报警列表
	std::recursive_mutex			m_setWarnsMtx;				///< 上一次的报警列表锁

	std::vector<std::string>		m_deviceSnListCh;			///< 缓存生化设备列表
	std::vector<std::string>		m_deviceSnListIm;			///< 缓存免疫设备列表
	std::vector<std::string>		m_deviceSnListIse;			///< 缓存ISE设备列表
    bool                            m_bIgnoreImSplUpdate = true;///< 是否忽略免疫耗材更新（性能优化）
	QTimer*							m_timer = nullptr;			///< 定时器，用于定时检测开瓶有效期和失效日期
    CacheImSupplyattributes         m_cacheImSupplyAttri;       ///< 缓存免疫耗材属性，以减少频繁查询
};
