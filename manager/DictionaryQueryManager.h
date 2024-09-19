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

#pragma once
///////////////////////////////////////////////////////////////////////////
/// @file     DictionaryQueryManager.h
/// @brief    用于提供字典查询的便捷接口，以及频繁访问的缓存处理
///
/// @author   8276/huchunli
/// @date     2023年11月28日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8276/huchunli，2023年11月28日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include <string>
#include <memory>
#include <map>
#include <set>
#include <QObject>
#include "src/public/ConfigDefine.h"
#include "src/public/im/ImConfigDefine.h"
#include "src/public/ise/IseConfigDefine.h"
#include "src/public/track/TrackConfigDefine.h"

struct SampleProbeWashConfig;
struct ImSampleProbeWashConfig;



class DictionaryQueryManager : public QObject
{
    Q_OBJECT
public:
    static DictionaryQueryManager* GetInstance();

    ~DictionaryQueryManager();


    ///
    /// @brief 获取日期时间格式基本信息
    ///
    /// @param[out]  baseSet  
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 1556/Chenjianlin，2023年8月15日，新建函数
    ///
    static bool GetUiBaseSet(BaseSet& baseSet);
    static bool SaveUiBaseSet(const BaseSet& baseSet);

    ///
    /// @bref
    ///		获取界面显示设置列表
    ///
    /// @param[out] vecDisplays 界面显示设置
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年12月20日，新建函数
    ///
    static bool GetUiDispalySet(std::vector<DisplaySet>& vecDisplays);
    static bool SaveUiDisplaySet(const std::vector<DisplaySet>& vecDisplays);

    ///
    /// @brief 获取当前的语言版本
    ///
    /// @param[out]		语言类型使用ISO 639-1语言代码(例：zh表示中文，en表示英文)
    ///
    /// @return 成功返回true
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年6月28日，新建函数
    ///
    static std::string GetCurrentLanuageType();

    ///
    /// @brief 获取时间显示方式
    ///
    /// @return 返回时间显示方式,-1:无效、0: 24小时制、1: 12小时制
    ///
    /// @par History:
    /// @li 7951/LuoXin，2022年9月29日，新建函数
    ///
    static int GetTimeDisplayMode();

    ///
    /// @brief 获取日期显示格式
    ///
    /// @return 格式字符串
    ///
    /// @par History:
    /// @li 7951/LuoXin，2022年9月29日，新建函数
    ///
    static QString GetUIDateFormat();

    ///
    /// @brief  获取生化被禁用的结果状态码
    ///     
    ///
    /// @return 被禁用的结果状态码(可能为空)
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年10月16日，新建函数
    ///
    static QStringList GetChResultStatusCodes();

    ///
    /// @bref
    ///		从数据库中获取项目备用订单
    ///
    /// @param[out] bkOrder 项目备用订单
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年7月25日，新建函数
    ///
    static bool TakeBackupOrderConfig(BackupOrderConfig& bkOrder);

    ///
    /// @bref
    ///		把备选项目订单设置保存到数据库
    ///
    /// @param[in] bkOrder 备选项目订单
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年7月25日，新建函数
    ///
    static bool SaveBackupOrderConfigAct(BackupOrderConfig& bkOrder);

    ///
    /// @brief  保存生化被禁用的结果状态码
    ///     
    /// @param[in]  codes  被禁用的结果状态码
    ///
    /// @return 成功返回true
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年10月16日，新建函数
    ///
    static bool SaveChResultStatusCodes(const QStringList& codes);

    ///
    /// @bref
    ///		获取界面的通讯设置
    ///
    /// @param[out] communicateParamSet 通讯设置
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年1月30日，新建函数
    ///
    static bool GetCommParamSet(CommParamSet& communicateParamSet);
    static bool SaveCommParamSet(const CommParamSet& communicateParamSet);

    ///
    /// @brief	自动复查勾选状态改变
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2023年2月23日，新建函数
    ///
    static bool SaveAutoRecheckConfig(bool bAutoRetest);

    ///
    /// @brief	保存维护配置
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2023年10月27日，新建函数
    ///
    static bool SaveStarttestMaintainConfig(bool bBMainte, const std::string& strData);
    static bool GetStarttestMaintainConfig(bool &bSMaintain, std::string& strdata);

    ///
    /// @brief 加载电子报警按钮状态
    ///     
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年11月16日，新建函数
    ///
    static bool GetShowLowerAlarmConfig(bool& isShowLowerAlarm);
    static bool UpdateLowerAlarmStatus(bool bCheck);

    ///
    /// @brief 加载报警音乐配置
    ///     
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年11月16日，新建函数
    ///
    static bool GetAlarmMusicConfig(AlarmMusic& alarmMus);
    static bool SaveAlarmMusicConfig(const AlarmMusic& alarmMus);

    ///
    /// @brief 加载校准提示的当前有效期报警提示
    ///     
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年2月4日，新建函数
    ///
    static bool GetCaliLineExpire(bool& caliLineExpire);
    static bool SaveCaliLineExpire(bool caliLineExpire);

    ///
    /// @bref
    ///		获取应用界面设置
    ///
    /// @param[out] pgSet 应用界面设置
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年1月30日，新建函数
    ///
    static bool GetPageset(PageSet& pgSet);
    static bool SavePageset(const PageSet& pgSet);

    ///
    /// @bref
    ///		保存报警屏蔽设置到字典表
    ///
    /// @param[in] shiledCfg 报警屏蔽设置信息
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年1月30日，新建函数
    ///
    static bool SaveAlarmShieldConfig(const AlarmShieldInfo& shiledCfg);

    ///
    /// @brief 获取报警屏蔽配置
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年1月13日，新建函数
    ///
    static bool GetAlarmShieldConfig(AlarmShieldInfo& alarmShiledCfg);

    ///
    /// @brief 查询自动复查配置是否启用
    ///
    /// @return 启用返回true
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年2月23日，新建函数
    ///
    static bool QueryAutoRecheckConfig();

    ///
    /// @brief 获取当前的进样模式
    ///     
    /// @return 进样模式
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年2月23日，新建函数
    ///
    static int GetInjectionModel();

    ///
    /// @bref
    ///		获取动态计算的配置
    ///
    /// @param[out] dynCalcCfg 动态计算配置
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年1月31日，新建函数
    ///
    static bool GetDynCalcConfig(RgntReqCalcCfg& dynCalcCfg);
    static bool SaveDynCalcConfig(const RgntReqCalcCfg& dynCalcCfg);

    ///
    /// @bref
    ///		获取导出配置
    ///
    /// @param[out] exportCfg 导出配置
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年1月31日，新建函数
    ///
    static bool GetExportConfig(std::map<ExportType, std::set<ExportInfoEn>>& exportCfg);
    static bool SaveExportConfig(const std::map<ExportType, std::set<ExportInfoEn>>& exportCfg);

    ///
    /// @bref
    ///		获取开放项目使用的UUID
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年1月31日，新建函数
    ///
    static bool GetSystemUUID(std::string& strUUID);

    ///
    /// @brief
    ///     获取分析参数界面系数A~F
    ///
    /// @par History:
    /// @li 7951/LuoXin，2022年6月20日，新建函数
    ///
    static bool GetChSerumIndexFactor(std::vector<int>& seriFactor);
    static bool SaveChSerumIndexFactor(const std::vector<int>& seriFactor);

    ///
    /// @bref
    ///		获取项目加样顺序设置
    ///
    /// @param[in] showOrder 显示设置
    /// @param[in] isCh 是否是生化的
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年1月31日，新建函数s
    ///
    static bool GetItemTestOrder(OrderAssay& assayOrder, bool isCh);
    static bool SaveItemTestOrder(const OrderAssay& assyaOrder, bool isCh);

    ///
    /// @bref
    ///		获取项目显示顺序
    ///
    /// @param[in] assayOrder 显示顺序
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年1月31日，新建函数
    ///
    static bool GetAssayShowOrder(OrderAssay& assayOrder);
    static bool SaveAssayShowOrder(const OrderAssay& assayOrder);

    ///
    /// @bref
    ///		从数据库字典表中获取报警设置
    ///
    /// @param[out] warnSet 报警设置
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年12月18日，新建函数
    ///
    static bool GetWarnsSet(SystemOverviewWarnSet& warnSet);
    static bool SaveWarnsSet(const SystemOverviewWarnSet& warnSet);

    ///
    /// @bref
    ///		获取样本针清洗配置
    ///
    /// @param[out] washConfig 清洗配置
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年1月31日，新建函数
    ///
    static bool GetSampleProbeWashConfigCh(SampleProbeWashConfig& washConfig);
    static bool GetSampleProbeWashConfigIm(ImSampleProbeWashConfig& washConfig);

    static bool SaveSampleProbeWashConfigCh(const SampleProbeWashConfig& washConfig);
    static bool SaveSampleProbeWashConfigIm(const ImSampleProbeWashConfig& washConfig);

    ///
    /// @bref
    ///		获取检测设置
    ///
    /// @param[out] detecConfg 检测设置
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年1月31日，新建函数
    ///
    static bool GetDetectionConfig(DetectionSetting& detecConfg);
    static bool SaveDetectionConfig(const DetectionSetting& detecConfg);

    ///
    /// @bref
    ///		获取样本架分配的设置
    ///
    /// @param[out] rackConfig 样本架分配设置
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年1月31日，新建函数
    ///
    static bool GetSampleRackAllocationConfig(SampleRackAllocation& rackConfig);
    static bool SaveSampleRackAllocationConfig(const SampleRackAllocation& rackConfig);

    ///
    /// @bref
    ///		获取样本接收模式设置
    ///
    /// @param[out] sampleRecv 样本接收模式
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年1月31日，新建函数
    ///
    static bool GetSampleRecvModelConfig(ConfigSampleRecvMode& sampleRecv);
    static bool SaveSampleRecvModelConfig(const ConfigSampleRecvMode& sampleRecv);

    ///
    /// @brief
    ///     获取维护显示模式
    ///
    /// @return 维护显示模式
    ///
    /// @par History:
    /// @li 8580/GongZhiQiang，2023年7月18日，新建函数
    ///
    static bool GetMaintainShowConfig(MaintainShowSet& maintShowConfig);
    static bool SaveMaintainShowConfig(const MaintainShowSet& maintShowConfig);

	///
    /// @brief
    ///     获取电极清洗设置
    ///
    /// @return 电极清洗设置
    ///
    /// @par History:
    /// @li 8580/GongZhiQiang，2024年3月14日，新建函数
    ///
    static bool GetElectrodeCleanSet(ElectrodeCleanSet& electrodeCleanSet);
    static bool SaveElectrodeCleanSet(const ElectrodeCleanSet& electrodeCleanSet);

    ///
    /// @bref
    ///		获取功能管理数据的配置
    ///
    /// @param[out] functionManage 功能管理数据
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年1月31日，新建函数
    ///
    static bool GetFunctionManageConfig(std::map<std::string, std::vector<FunctionManageItem>>& functionManage);
    static bool SaveFunctionManageConfig(const std::map<std::string, std::vector<FunctionManageItem>>& functionManage);

    ///
    /// @bref
    ///		获取生化的废液桶报警值设置
    ///
    /// @param[in] iChWasteWarnValue 报警值
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年1月31日，新建函数
    ///
    static bool GetWasteBucketWarningValueCh(int& iChWasteWarnValue);
    bool SaveWasteBucketWarningValueCh(int iChWasteWarnValue);

    ///
    /// @bref
    ///		获取AI分析的配置
    ///
    /// @param[out] aiAnaysis AI分析的配置
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年1月31日，新建函数
    ///
    static bool GetAiAnalysisConfig(AiAnalysisCfg& aiAnaysis);
    static bool SaveAiAnalysisConfig(const AiAnalysisCfg& aiAnaysis);

    ///
    /// @bref
    ///		获取配置工具免疫参数配置
    ///
    /// @param[out] cfgToolImParam AI分析的配置
    ///
    /// @par History:
    /// @li 4170/TangChuXian, 2024年8月28日，新建函数
    ///
    static bool GetCfgToolImParamConfig(CfgToolImParam& cfgToolImParam);
    static bool SaveCfgToolImParamConfig(const CfgToolImParam& cfgToolImParam);

    ///
    /// @bref
    ///		获取情况设置
    ///
    /// @param[out] clearConfig 情况设置
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年1月31日，新建函数
    ///
    static bool GetClearSet(ClearSetData& clearConfig);
    static bool SaveClearSet(const ClearSetData& clearConfig);

    ///
    /// @bref
    ///		获取语言设置
    ///
    /// @param[out] softLanguages 语言设置
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年1月31日，新建函数
    ///
    static bool GetSoftLanguageConfig(std::vector<SoftLanguage>& softLanguages);

    ///
    /// @bref
    ///		获取是否使能审核设置
    ///
    /// @param[out] bEnableAudit 是否是能审核
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年1月31日，新建函数
    ///
    static bool GetAuditConfig(bool& bEnableAudit);
    static bool SaveAuditConfig(bool bEnableAudit);

    ///
    /// @bref
    ///		获取自动登陆用户
    ///
    /// @param[out] strUser 用户名
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年1月31日，新建函数
    ///
    static bool GetAutoLoginUser(std::string& strUser);
    static bool SaveAutoLoginUser(const std::string& strUser);

    ///
    /// @bref
    ///		获取批量添加样本配置
    ///
    /// @param[out] batchConfig 批量添加样本配置
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年1月31日，新建函数
    ///
    static bool GetBatchAddSampleConfig(AddSampleAction& batchConfig);
    static bool SaveBatchAddSampleConfig(const AddSampleAction& batchConfig);

    ///
    /// @bref
    ///		获取用户自定义字典数据
    ///
    /// @param[in] dataType 数据标识
    /// @param[out] cutomData 自定义数据
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年2月1日，新建函数
    ///
    static bool GetCustomSetData(int dataType, std::vector<CustomSetRowData>& cutomData);

public:
    ///
    /// @brief 获取架号和位置号的范围
    ///
    ///
    /// @return 架号和位置号的范围
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年6月16日，新建函数
    ///
    inline std::shared_ptr<RackNumRange> GetRackRange() { return m_rnr; };

    ///
    /// @brief
    ///     查找单位对应的类型
    ///
    /// @param[in]  unitString  单位名称
    ///
    /// @return 对应的类型代号
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年9月6日，新建函数
    ///
    inline int FindUnitType(const std::string& unitString)
    {
        std::map<std::string, std::string>::iterator it = m_mapUnitType.find(unitString);
        return it == m_mapUnitType.end() ? -1 : atoi(it->first.c_str());
    }

    ///
    /// @brief
    ///     查找样本对应的类型
    ///
    /// @param[in]  souceString  类型名称
    ///
    /// @return 对应的类型代号
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年9月6日，新建函数
    ///
    inline int FindSouceType(const std::string& souceString)
    {
        std::map<std::string, std::string>::iterator it = m_mapSouceType.find(souceString);
        return it == m_mapSouceType.end() ? -1 : atoi(it->first.c_str());
    }

    ///
    /// @brief 获取数据报警的屏蔽码
    ///
    /// @return 数据报警的屏蔽码
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年12月13日，新建函数
    ///
    inline std::set<QString> GetShileDataAlarm() { return m_dataAlarmShiledMap; };

    ///
    /// @brief
    ///     获取单位对应表
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年9月6日，新建函数
    ///
    inline std::map<std::string, std::string>& GetMapUnitType() { return m_mapUnitType; };

    ///
    /// @brief
    ///     获取样本源对应表
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年9月6日，新建函数
    ///
    inline std::map<std::string, std::string>& GetMapSouceType() { return m_mapSouceType; };

    ///
    /// @brief 获取样本显示的默认设置
    ///
    ///
    /// @return 默认设置信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年4月10日，新建函数
    ///
    inline std::vector<int>& GetSampleShowSet() { return m_sampleShowSet; }

	///
	/// @brief 获取生化的最大吸光度
	///
	///
	/// @return 最大吸光度
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年7月29日，新建函数
	///
	inline int GetChMaxAbs() { return m_chMaxAbs; };

    static bool SaveSampleDefaultShowType(const std::vector<int>& defautShow);

    ///
    /// @brief 获取结果显示的设置信息
    ///
    ///
    /// @return 结果显示的设置信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年4月10日，新建函数
    ///
    inline SampleShowSet GetSampleResultSet() { return m_sampleResultShowSet; };
    static bool SaveSampleShowSet(const SampleShowSet& showSet);


    ///
    /// @brief 获取结果提示设置信息
    ///
    ///
    /// @return 获取结果提示设置信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年4月10日，新建函数
    ///
    inline ResultPrompt GetResultTips() { return m_resultTips; };
    static bool SaveResultTips(const ResultPrompt& resTip);

    ///
    /// @brief 获取备用订单信息
    ///
    ///
    /// @return 备用订单信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年9月15日，新建函数
    ///
    inline BackupOrderConfig GetBackupOrder() { return m_backupOrder; };

	///
	/// @brief 获取轨道配置信息
	///
	///
	/// @return 轨道配置信息
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年5月27日，新建函数
	///
	inline track::bcyime::stTrackConfig GetTrackConfig() { return m_trackConfig; };

    ///
    /// @brief 获取生化的废液桶报警量
    ///
    ///
    /// @return 报警量
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年3月29日，新建函数
    ///
    inline int GetChWasteAlarmVolumn() { return m_chWasteWarning; };

    ///
    /// @brief 获取联机与否
    ///
    ///
    /// @return 是否联机（true代表联机）
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年3月30日，新建函数
    ///
    inline bool GetPipeLine() { return m_isPipeLine; };

    ///
    /// @bref
    ///		获取质控品SN的长度配置
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年3月15日，新建函数
    ///
    int GetQcSnLengthConfig();

    ///
    /// @bref
    ///		获取质控品批号的长度配置
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年3月15日，新建函数
    ///
    int GetQcLotLengthConfig();

    ///
    /// @bref
    ///		获取质控品二维码的长度配置
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年3月15日，新建函数
    ///
    int GetQcBarcodeLengthConfig();

    ///
    /// @bref
    ///		获取质控品Level的最大值配置
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年3月15日，新建函数
    ///
    int GetQcLevelConfig();

    ///
    /// @brief 获取软件版本
    ///
    /// @param[in]  
    ///
    /// @return 例如“V1.0.0”, 获取失败返回空字符串
    ///
    /// @par History:
    /// @li 8580/GongZhiQiang，2023年9月12日，新建函数
    ///
    std::string GetSoftwareVersion();

    ///
    /// @brief
    ///     是否提示校准过期
    ///
    /// @return true表示要提示
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年3月12日，新建函数
    ///
    inline bool IsCaliLineExpire() { return m_bCalExpireNotice; }
    inline void SetCaliLineExpire(bool bCalExpireNotice) { m_bCalExpireNotice = bCalExpireNotice; }

    ///
    /// @bref
    ///		获取用于排除的免疫非复查数据报警码
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年7月17日，新建函数
    ///
    const std::set<std::string>& GetImAlaramDataCodeExcept();

    protected Q_SLOTS:
    ///
    /// @brief
    ///     更新数据字典映射信息
    ///
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年9月3日，新建函数
    ///
    bool UpdateDictionaryReflection();

private:
    DictionaryQueryManager();

    int m_chWasteWarning;                   ///< 废液桶的告警量（0.1μl）
    bool m_isPipeLine;                      ///< 是否联机    
    bool m_bCalExpireNotice;                //   校准过期是否要提示
    std::vector<int> m_sampleShowSet;       ///< 样本默认信息显示（样本管类型和样本类型）
    std::set<QString> m_dataAlarmShiledMap; ///< 数据报警屏蔽地图
    std::map<std::string, std::string> m_mapUnitType;       ///< 单位映射表
    std::map<std::string, std::string> m_mapSouceType;      ///< 样本源映射表

    std::shared_ptr<RackNumRange> m_rnr;                     ///< 架号范围
    SampleShowSet m_sampleResultShowSet;    ///< 校准/质控/样本结果的显示设置
    ResultPrompt m_resultTips;			    ///< 结果提示的设置
    BackupOrderConfig m_backupOrder;        ///< 备用订单
    track::bcyime::stTrackConfig m_trackConfig;	///< 轨道配置信息

    int m_qcDocSnLength;                    ///< 缓存质控品SN的长度配置
    int m_qcDocLotLength;                   ///< 缓存质控品Lot的长度配置
    int m_qcDocBarcodeLength;               ///< 缓存质控品二维码长度配置
    int m_qcDocLevel;                       ///< 缓存质控品的Level最大值
    int m_chMaxAbs;							///< 缓存生化最大吸光度的值

    std::set<std::string> m_imResultAlarmCodeExcept; ///< 缓存免疫数据报警码非用于复查的码（用于排除在复查设置中的显示
};
