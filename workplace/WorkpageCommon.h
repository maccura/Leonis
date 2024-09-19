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
/// @file     WorkpageCommon.h
/// @brief    工作界面公共工具函数
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
#pragma once
#include <QVariant>
#include <QVariantList>
#include <QMap>
#include "src/thrift/gen-cpp/defs_types.h"
#include "thrift/ch/c1005/C1005LogicControlProxy.h"
#include "thrift/ise/ise1005/Ise1005LogicControlProxy.h"
#include "thrift/im/i6000/I6000LogicControlProxy.h"
#include "thrift/DcsControlProxy.h"
#include "src/thrift/gen-cpp/defs_types.h"
#include "src/public/ConfigSerialize.h"
#include <boost/optional/optional.hpp>
#include "PrintExportDefine.h"
#include "src/db/HistoryData.hxx"

struct SampleShowSet;
class QPushButton;
class QGridLayout;

namespace boost {
    namespace posix_time {
        class ptime;
    };
};

class ConstructExportDataInfo
{
public:
    inline void SetDataKey(int64_t key) { m_key = key; };
	virtual bool BuildExportDataInfo(SampleExportInfo& exportData) = 0;
protected:
	int64_t m_key;
};

class ConstructExportSampleInfo : public ConstructExportDataInfo
{
public:
	virtual bool BuildExportDataInfo(SampleExportInfo& exportData);
private:
	bool BuildExportSampleInfo(const std::shared_ptr<tf::SampleInfo>& spSample, SampleExportInfo& exportData);
};

class ConstructExportItemInfo : public ConstructExportDataInfo
{
public:
	std::vector<SampleExportInfo>& GetExportData() { return m_exportDatas; };
	virtual bool BuildExportDataInfo(SampleExportInfo& exportData);
private:
	bool BuildExportItemInfo(const std::shared_ptr<tf::TestItem>& spTestIetm, SampleExportInfo& exportData);
private:
	int64_t							m_sampleKey;
	static SampleExportInfo			m_exportSampleBase;
	std::vector<SampleExportInfo>	m_exportDatas;
};

class ConstructExportResultInfo : public ConstructExportDataInfo
{
public:
	virtual bool BuildExportDataInfo(SampleExportInfo& exportData);
private:
	int64_t					m_itemKey;
};

class ConstructHistoryExportItemInfo : public ConstructExportDataInfo
{
public:
	static void ClearCaliMap() { m_caliSampleIdMap.clear(); };
	void SetData(const std::shared_ptr<HistoryBaseDataByItem>& spHisTestIetm);
	std::vector<SampleExportInfo>& GetExportData() { return m_exportDatas; };
	QVariantList& GetExtendTitle() { return m_titleExtend; };
	virtual bool BuildExportDataInfo(SampleExportInfo& exportData);
private:
	bool BuildExportItemInfo(const std::shared_ptr<HistoryBaseDataByItem>& spHisTestIetm, SampleExportInfo& exportData);
	bool BuildExportSampleInfo(const std::shared_ptr<HistoryBaseDataByItem>& spHisTestIetm, SampleExportInfo& exportData);
private:
	QVariantList									m_titleExtend;
	std::vector<SampleExportInfo>					m_exportDatas;
	std::shared_ptr<HistoryBaseDataByItem>			m_spHisTestIetm;
	static std::map<int64_t, std::map<int, int>>	m_caliSampleIdMap;
};

class ConstructExportInfo
{
	using  SampleExportVec = std::vector<SampleExportInfo>;
public:
	bool ProcessOnebyOne(const int64_t testKey);
private:
	const ExportType GetExportType(const int64_t testKey);
	std::map<ExportType, SampleExportVec> m_sampleExportDataVec;
};

using  SampleExportVec = std::vector<SampleExportInfo>;
class ConstructHisExportInfo
{
public:
	std::map<ExportType, SampleExportVec>& GetExportMap() { return m_sampleExportDataVec; };
	void ProcessData(std::vector<std::shared_ptr<HistoryBaseDataByItem>>& dataItemVec);
	std::map<ExportType, QVariantList>& GetExtendTitleMap() { return m_titleExtendMap; };
private:
	bool ProcessOnebyOne(const std::shared_ptr<HistoryBaseDataByItem>& hisTestItem);
	const ExportType GetExportType(const std::shared_ptr<HistoryBaseDataByItem>& hisTestItem);
private:
	std::map<ExportType, SampleExportVec> m_sampleExportDataVec;
	std::map<ExportType, QVariantList>	  m_titleExtendMap;
};

class ConvertDataToExportString
{
	enum DATATYPE
	{
		INTTYPE,
		DOUBLETYPE
	};

public:
	ConvertDataToExportString();
	bool GenData(const bool isXlsx, const std::map<ExportType, SampleExportVec>& sourceData, const std::map<ExportType, QVariantList>& titleMap);
	QVariantList GetTitle(const ExportType& titletype);
	QVariantList GetContent(const ExportType& titletype);
private:
	bool CreatTitle(const tf::UserType::type UType, const std::map<ExportType, std::set<ExportInfoEn>>& savedata, const std::map<ExportType, QVariantList>& titleMap);
	QVariantList CreatContent(const bool isXlsx, const tf::UserType::type UType, const SampleExportInfo& srcData, const std::set<ExportInfoEn>& configData);
	bool ConvertData(const DATATYPE dstType, const QString& input, QVariant& dst);
	bool GenTitleList(const tf::UserType::type UType, QVariantList& updateTitle, const std::set<ExportInfoEn>& configData);
private:
	std::map<ExportType, QVariantList>			 m_exportTitle;
	std::map<ExportType, QVariantList>			 m_exportContent;
	std::map<ExportInfoEn, DATATYPE>			 m_exportDataMap;			///< 需要导出特殊类型的列
	std::map<ExportType, std::set<ExportInfoEn>> m_saveConfigdata;
};

class SampleColumn : public QObject
{
	Q_OBJECT

public:
	enum class COL
	{
		INVALID = 0,          ///< 无效列
		STATUS,               ///< 状态列索引
		SEQNO,                ///< 样本号列索引
		BARCODE,              ///< 条码列索引
		TYPE,                 ///< 类型列索引
		POS,                  ///< 位置列索引
		ENDTIME,              ///< 结束检测列索引
		RECHECKENDTIME,       ///< 复查完成时间
		AUDIT,				  ///< 审核
		PRINT				  ///< 打印
	};

	enum class COLASSAY
	{
		INVALID = 0,          ///< 无效列
		STATUS,               ///< 状态列索引
		SEQNO,                ///< 样本号列索引
		BARCODE,              ///< 条码列索引
		TYPE,                 ///< 类型列索引
		POS,                  ///< 位置列索引
		ENDTIME,              ///< 结束检测列索引
		RECHECKENDTIME,       ///< 复查完成时间
		AUDIT,				  ///< 审核
		PRINT,				  ///< 打印
		ASSAY,				  ///< 项目
		RESULT,				  ///< 结果
		RESULTSTATUS,		  ///< 结果状态
		RECHECKRESULT,		  ///< 复查结果
		RECHECKSTATUS,		  ///< 结果状态（复查）
		UNIT,				  ///< 单位
		MODULE,				  ///< 模块
		FIRSTABSORB,		  ///< 检测信号值
		REABSORB			  ///< 复查信号值
	};

	enum class SAMPLEASSAY
	{
		INVALID = 0,          ///< 无效列
		RECHECKOPTION,	      ///< 复查选项
		ASSAY,				  ///< 项目
		RESULT,				  ///< 结果
		RESULTSTATUS,		  ///< 结果状态
		RECHECKRESULT,		  ///< 复查结果
		RECHECKSTATUS,		  ///< 结果状态（复查）
		UNIT,				  ///< 单位
		MODULE,				  ///< 模块
		FIRSTABSORB,		  ///< 检测信号值
		REABSORB			  ///< 复查信号值
	};

	enum class SAMPLEMODE
	{
		SAMPLEAPP = 0,        ///< 无效列
		SAMPLE,				  ///< 按样本
		SAMPLEASSAY,	      ///< 按样本的项目
		ASSAYLIST			  ///< 按项目
	};

	template<typename IndexType>
	struct ItemInfo
	{
		QString     name;      ///< 标题名称
		int         index;     ///< 当前位置
		int			width;	   ///< 当前宽度
		IndexType	type;      ///< 当前项目类型
	};

	~SampleColumn() {};
	static SampleColumn& Instance();
    inline COLASSAY IndexToStatus(int column) { return (m_assayKeymap.contains(column)) ? m_assayKeymap[column].type : COLASSAY::INVALID; };
	SAMPLEASSAY IndexAssayToStatus(int column);
	COL IndexSampleToStatus(SAMPLEMODE mode, int column);
	QStringList GetHeaders(SAMPLEMODE mode, bool IsHistory = false);
	bool SetAttributeOfType(SAMPLEMODE mode, int index, int width);
	boost::optional<std::pair<int, int>> GetAttributeByType(SAMPLEMODE mode, int type);
Q_SIGNALS:
	void ColumnChanged();
private:
	SampleColumn();
	void AssignDisplaySet(DisplayType displayType, int colType, int width);
	private slots:
	void OnUpdateColumn();
private:
	QMap<int, ItemInfo<COL>>        m_sampleKeymap;             ///< 样本信息数据地图
	QMap<int, ItemInfo<COLASSAY>>   m_assayKeymap;              ///< 项目数据地图 
	QMap<int, ItemInfo<SAMPLEASSAY>>m_sampleAssayKeymap;        ///< 样本项目数据地图
	std::vector<DisplaySet>			m_displaySet;				///< 设置数据（主要用于保存宽度）
};

namespace WorkpageCommon
{
    ///
    /// @brief 复查状态
    ///
    /// @param[in]  item  
    ///
    /// @return 复查状态按钮的Pic路径
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年5月30日，新建函数
    ///
    QVariant GetItemReCheckPixmap(const tf::TestItem& item);
    QVariant GetItemReCheckPixmap(const std::vector<std::shared_ptr<tf::TestItem>>& vecItem);

    QVariant GetRecheckPixmap(const boost::posix_time::ptime& retestEndtime, const boost::posix_time::ptime& endTestTime);

    QVariant GetSendLisPixmap();
	QVariant GetDefaultPixmap();
    QVariant GetTestStatusPixmap(int iStatus);

    // useless.
    QVariant GetItemAduitPrintPixmap(const std::shared_ptr<tf::SampleInfo>& pSample);

    ///
    /// @brief 样本是否被上传
    ///
    /// @param[in]  sample  样本
    ///
    /// @return 显示上传的pic
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年10月11日，新建函数
    ///
    QVariant GetUploadPixmap(const tf::SampleInfo& sample, bool bAiRec);

    ///
    /// @brief 获取样本的智能状态
    ///
    /// @param[in]  sample  样本
    ///
    /// @return 显示上传的pic
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年10月11日，新建函数
    ///
    QVariant GetSampleAiStatus(const tf::SampleInfo& sample);
    QVariant GetSampleAiStatus();
	std::vector<std::shared_ptr<ch::tf::AssayTestResult>> GetChAssayTestResult(int64_t itemId);
	std::vector<std::shared_ptr<ise::tf::AssayTestResult>> GetIseAssayTestResult(int64_t itemId);
	std::vector<std::shared_ptr<tf::CalcAssayTestResult>> GetCalcAssayTestResult(int64_t itemId);

	///
	/// @brief 根据id获取结果详情
	///
	/// @param[in]  id  结果Id
	///
	/// @return 结果详情
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年4月23日，新建函数
	///
	std::shared_ptr<ch::tf::AssayTestResult> GetAssayResultByid(int64_t id);

	///
	/// @brief 获取校准次数
	///   
	/// @param[in]  caliMap  校准地图
	/// @param[in]  sampleId 校准样本id
	/// @param[in]  assayCode校准项目编号  
	///
	/// @return 校准次数
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年4月26日，新建函数
	///
	int GetcaliTimes(std::map<int64_t, std::map<int, int>>& caliMap, const int64_t sampleId, const int assayCode);

	///
	/// @brief 获取生化测试结果的导出数据
	///
	/// @param[in]  assayTestResultId	生化结果的id
	/// @param[in]  exportData			导出数据
	///
	/// @return true:导出成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年4月23日，新建函数
	///
	bool GetChResultExport(const std::shared_ptr<ch::tf::AssayTestResult>& spAssayTestResult, SampleExportInfo& exportData);

	///
	/// @brief 获取Ise测试结果导出数据
	///
	/// @param[in]  spAssayTestResult  ise测试结果
	/// @param[in]  exportData		   导出数据
	///
	/// @return true:导出成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年4月23日，新建函数
	///
	bool GetIseResultExport(const std::shared_ptr<ise::tf::AssayTestResult>& spAssayTestResult, SampleExportInfo& exportData);

	///
	/// @brief 根据结果信息获取吸光度信息（生化）
	///
	/// @param[in]  spAssayTestResult  结果信息
	/// @param[in]  exportData		   吸光度导出信息
	/// @param[in]  dynamicTitle	   吸光度标题
	///
	/// @return true:导出成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年4月24日，新建函数
	///
	bool GetResultAbs(const std::shared_ptr<ch::tf::AssayTestResult> spAssayTestResult, SampleExportInfo& exportData, QVariantList& dynamicTitle);

	///
	/// @brief 获取吸光度的导出标题
	///
	/// @param[in]  rawtitle	原始标题
	/// @param[in]  outputTitle 导出标题 
	/// @param[in]  configData  设置信息
	///
	/// @return true:导出成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年4月24日，新建函数
	///
	bool GetExportAbsTitle(const QVariantList& rawtitle, QVariantList& outputTitle, const std::set<ExportInfoEn>& configData);

	///
	/// @brief 获取计算项目的结果导出数据
	///
	/// @param[in]  spAssayTestResult  计算项目结果
	/// @param[in]  exportData		   导出数据
	///
	/// @return true:导出成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年4月24日，新建函数
	///
	bool GetCalcResultExport(const std::shared_ptr<tf::CalcAssayTestResult>& spAssayTestResult, SampleExportInfo& exportData);

	QString GetSampleTypeStat(const int sampleType);

	///
	/// @brief 获取测试项目对应
	///
	/// @param[in]  db		   测试项目的数据库主键
	/// @param[in]  assayCode  项目编号
	///
	/// @return （初测，复查）
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年2月26日，新建函数
	///
	std::pair<std::string, std::string> GetItemTestTime(int64_t db, int assayCode);

    ///
    /// @brief 审核打印状态
    ///
    /// @param[in]  data
    ///
    /// @return 返回是打印还是审核状态
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年5月30日，新建函数
    ///
    QVariant GetItemAduitPrint(int iCheck);

	///
	/// @brief 记录删除样本的工作日志
	///
	/// @param[in]  sample  样本信息
	///
	/// @return true:删除成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月14日，新建函数
	///
	bool DeleteSampleOptLog(const tf::SampleInfo& sample);

    ///
    /// @brief 是否显示浓度
    ///
    /// @param[in]  sampleSet  设置信息
    /// @param[in]  sampleType 样本类型
    ///
    /// @return true:显示浓度，反之显示吸光度
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年9月12日，新建函数
    ///
    bool IsShowConc(const SampleShowSet& sampleSet, tf::SampleType::type sampleType);
    bool IsShowConc(const SampleShowSet& sampleSet, tf::HisSampleType::type sampleType);

	///
	/// @brief 项目是否处于复查状态
	///
	/// @param[in]  testItem  项目信息
	///
	/// @return true:复查状态
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年1月25日，新建函数
	///
    inline bool IsItemReCheckStatus(const tf::TestItem& testItem) 
    { 
        return (testItem.status == tf::TestItemStatus::type::TEST_ITEM_STATUS_PENDING && testItem.rerun == true); 
    };

    ///
    /// @brief 根据项目通道号获取项目的小数位、和主单位倍率、单位名称
    ///
    /// @param[in]		assayCode	项目通道号
    /// @param[out]		unitName	单位名称
    /// @param[out]		iPrecision	小数位
    /// @param[out]		factor		和主单位倍率
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 7702/WangZhongXin，2023年9月21日，新建函数
    ///
    void GetPrecFactorUnit(int assayCode, int& iPrecision, double& factor, std::string& unitName);

    ///
    /// @bref
    ///		审核确认弹框
    ///
    /// @param[in] verfiyBtn 审核按钮
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年1月23日，新建函数
    ///
    bool ConfirmVerify(QPushButton* verfiyBtn);

    ///
    /// @bref
    ///		设置Grid中的样本类型按钮的显示与隐藏
    ///
    /// @param[in] sampleTypeBtns 按钮列表
    /// @param[in] gridLay Grid容器
    /// @param[in] iTargPos 目标按钮位置
    /// @param[in] bShow 显示获取隐藏
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年2月23日，新建函数
    ///
    void SetSampleTypeGridBtnsVisble(std::vector<QPushButton*>& sampleTypeBtns, QGridLayout* gridLay, int iTargPos, bool bShow);

	///
	/// @brief 更新患者信息中的审核者信息
	///
	/// @param[in]  patientId  病人信息数据库主键
	/// @param[in]  sampleId   样本信息数据库主键
	/// @param[in]  auditName  审核者信息
	///
	/// @return true:修改成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年6月11日，新建函数
	///
	bool UpdateAduitInfo(const int64_t patientId, const int64_t sampleId, const std::string& auditName);

	///
	/// @brief 判断结果是否符合浓度显示要求
	///
	/// @param[in]  isCalc		是否计算项目
	/// @param[in]  statusCode  结果中的状态码
	/// @param[in]  conc		浓度
	///
	/// @return true:符合显示标准
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年7月29日，新建函数
	///
	bool IsConcValid(const bool isCalc, const std::string& statusCode, const double conc);
	bool IsConcValid(const bool isCalc, const std::string& statusCode, const std::string& conc);

	///
	/// @brief 判断是否符合吸光度显示标准
	///
	/// @param[in]  absValue  吸光度值
	///
	/// @return true:符合显示标准
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年7月29日，新建函数
	///
	bool IsRLUValid(const int absValue);

	///
	/// @brief 判断是否符合吸光度显示标准
	///
	/// @param[in]  rluString  吸光度值(字符串形式)
	///
	/// @return true:符合显示标准
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年7月29日，新建函数
	///
	bool IsRLUValid(const string& rluString);
};
