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
/// @file     QHistorySampleAssayModel.h
/// @brief    历史数据模型
///
/// @author   7702/WangZhongXin
/// @date     2022年12月16日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7702/WangZhongXin，2022年12月16日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once
#include <thread>
#include <mutex>
#include <QAbstractTableModel>
#include "src/db/HistoryData.hxx"
#include "src/thrift/gen-cpp/defs_types.h"
#include "src/public/ConfigDefine.h"
#include "shared/CReadOnlyDelegate.h"

struct SampleShowSet;

#define  PAGE_LIMIT_ITEM                                (14)      // 分页查询（每一页的样本数）

// 项目结果结构体
struct stAssayResult
{
	QString m_result;
	QString m_resultFlag;
	QString m_backgroundColor;
	int		m_fontColorType;	// 在代理中实现，通过类型确定
	stAssayResult()
	{
		m_fontColorType = ColorType::NORMALCOLOR;
		m_backgroundColor = "#ffffff";	// 默认背景白色
	}
};

///
/// @brief 获取样本编号
///
/// @par History:
/// @li 7702/WangZhongXin，2023年1月7日，新建函数
///
QString GetSeqNo(::tf::HisSampleType::type sampleType, ::tf::TestMode::type testType, const std::string& seqNo);


class QHistorySampleAssayModel : public QAbstractTableModel
{
	Q_OBJECT

public:

	enum class COL
	{
		Check = 0,            ///< 选中列索引
		STATUS,				  ///< 状态列索引
		SEQNO,                ///< 样本号列索引
		BARCODE,              ///< 条码列索引
		TYPE,                 ///< 类型列索引
		POS,                  ///< 位置列索引
		ENDTIME,              ///< 结束检测列索引
		RECHECKENDTIME,       ///< 复查完成时间
		//AUDIT				  ///< 审核
	};

	enum class COLASSAY
	{
		Check = 0,            ///< 选中列索引
		STATUS ,             ///< 状态列索引
		SEQNO,                ///< 样本号列索引
		BARCODE,              ///< 条码列索引
		TYPE,                 ///< 类型列索引
		POS,                  ///< 位置列索引
		ENDTIME,              ///< 结束检测列索引
		RECHECKENDTIME,       ///< 复查完成时间
		ASSAY,				  ///< 项目
		RESULT,				  ///< 结果
		RESULTSTATUS,		  ///< 结果状态
		RECHECKRESULT,		  ///< 复查结果
		RECHECKSTATUS,		  ///< 结果状态（复查）
		UNIT,				  ///< 单位
		//AUDIT,				  ///< 审核
		MODULE				  ///< 模块
	};

	// 模式状态
	enum class MOUDLESTATUS
	{
		UNKONW = 0,            /// 未初始化
		PROCESS,			   /// 正在初始化
		FINISH,				   /// 初始化完成
	};

	// 模式类型
	enum class VIEWMOUDLE
	{
		SAMPLEBROWSE = 0,		/// 数据浏览-按样本展示
        DATABROWSE,
		ASSAYBROWSE,			/// 按项目展示
	};

    ///
    /// @brief
    ///     构建Sql语句,oder by 子句
    ///
    /// @param[in]  cond，筛选条件 
    /// @param[out]  sql，根据查询条件构建出的SQL语句 
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 7685/likai，2023年8月15日，新建函数
    static void BuildOrderByForHistoryBaseData(std::string & Sql, const ::tf::HistoryBaseDataQueryCond & cond);

    ///
    /// @brief
    ///     构建Sql语句,where子句除时间条件之外的语句
    ///
    /// @param[in]  bQuerySample，true表示用于构建按样本时查询样本信息的条件
    /// @param[in]  cond，筛选条件 
    /// @param[out]  sql，根据查询条件构建出的SQL语句 
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 7685/likai，2023年8月15日，新建函数
    static void BuildSQLForHistoryBaseData(std::string & Sql, const ::tf::HistoryBaseDataQueryCond & cond, bool bQuerySample = false);

    ///
    /// @brief
    ///     构建Sql语句,按样本查询时的样本相关信息，对应HistoryData.hxx的HistoryBaseDataSample视图
    ///
    /// @param[in]  cond，筛选条件 
    /// @param[out]  sql，根据查询条件构建出的SQL语句 
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 7685/likai，2023年8月15日，新建函数
    static bool CondToSql_Sample(const ::tf::HistoryBaseDataQueryCond& cond, std::string& sql);

    ///
    /// @brief
    ///     构建Sql语句,按样本查询时的项目相关信息，对应HistoryData.hxx的HistoryBaseDataItem视图
    ///
    /// @param[in]  sampleID，查询指定样本的项目 
    /// @param[in]  cond，筛选条件 
    /// @param[out]  sql，根据查询条件构建出的SQL语句 
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 7685/likai，2023年8月15日，新建函数
    static bool CondToSql_Item(const ::tf::HistoryBaseDataQueryCond& cond, std::string& sql, const int64_t sampleID);

    ///
    /// @brief
    ///     构建Sql语句,按项目查询时的基本信息，对应HistoryData.hxx的HistoryBaseDataByItem视图
    ///
    /// @param[in]  cond，筛选条件 
    /// @param[out]  sql，根据查询条件构建出的SQL语句 
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 7685/likai，2023年8月15日，新建函数
    static bool CondToSqlByItem(const ::tf::HistoryBaseDataQueryCond& cond, std::string& sql);

	///
	/// @brief 更新病人信息
	///
	/// @param[in]  sampleId		样本ID
	/// @param[in]  patientId		病人数据库主键ID
	/// @param[in]  patientName		病人姓名
	/// @param[in]  caseNo			病例号
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年9月25日，新建函数
	///
	static void UpdatePatienInfo(const int64_t& sampleID, const int64_t& patientID, const std::string& patientName, const std::string& caseNo);

	virtual ~QHistorySampleAssayModel();

	///
	/// @brief 获取单例对象
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年6月30日，新建函数
	///
	static QHistorySampleAssayModel& Instance();

	///
	/// @brief 更新历史数据
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2022年12月29日，新建函数
	///
    void Update(const ::tf::HistoryBaseDataQueryCond& cond);
    void UpdatePrintFlag(const std::vector<int64_t>& sampleId, bool isPrinted);

    ///
    /// @bref
    ///		通过样本ID获取历史记录，用于按样本展示时获取样本对应的所有记录信息的导出
    ///
    /// @param[in] sampleId 样本ID
    /// @param[out] dataItem 历史记录
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年7月17日，新建函数
    ///
    static void GetHistoryDataBySampleId(int64_t sampleId, const ::tf::HistoryBaseDataQueryCond& cond, std::vector<std::shared_ptr<HistoryBaseDataByItem>>& dataItem);
    static QString GetHistoryAssayNameByBaseItem(const std::shared_ptr<HistoryBaseDataByItem>& dataItem);

	///
	/// @brief 获取当前查询条件
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年8月17日，新建函数
	///
	const ::tf::HistoryBaseDataQueryCond& GetQueryCond();

	///
	/// @brief 刷新
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年8月4日，新建函数
	///
	void Refush();

	///
	/// @brief 排序
	///
	/// @param[in]  col		排序列
	/// @param[in]  order	排序方式，0-无，1-降序，2-升序
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin锛?YEAR$骞?MONTH$鏈?DAY$鏃ワ紝鏂板缓鍑芥暟
	///
	void Sort(int col,int order);

	///
	/// @brief 返回是否为空
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年7月28日，新建函数
	///
	bool Empty() const;

	///
	/// @brief 更新历史数据Lis状态
	///
	/// @param[in]  vecSampleID		发生改变的样本ID
	/// @param[in]  vecStatus		发生改变的样本当前状态///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年5月24日，新建函数
	///
	void UpdateLisStatus(const QVector<int64_t>& vecSampleID);

	///
	/// @brief 更新复查结果
	///
	/// @param[in]  index	索引
	/// @param[in]  vol		复查结果，0-复查结果，1-复查状态
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年7月4日，新建函数
	///
	void UpdateRetestResult(const QModelIndex& index,std::tuple<std::string,std::string, int64_t> vol);

	///
	/// @brief 清除历史数据
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年3月16日，新建函数
	///
	void Clear();

    ///
	/// @brief 获取模式
	///
	///
	/// @return 获取当前模式
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年8月26日，新建函数
	///
	VIEWMOUDLE GetModule() const;

	///
	/// @brief 设置模式类型（项目选择，项目浏览-（按样本展示，按项目展示））
	///
	/// @param[in]  module  模式类型
	///
	/// @return 设置成功返回true
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年7月21日，新建函数
	///
	bool SetSampleModuleType(VIEWMOUDLE module);

	///
	/// @brief
	///     获取单元格属性(可编辑、可选择)
	///
	/// @param[in]    index      当前单元格索引
	///
	/// @return       QVariant   包装的数据
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年6月30日，新建函数
	///
	Qt::ItemFlags flags(const QModelIndex &index) const override;

	int rowCount(const QModelIndex &parent) const override;
	int columnCount(const QModelIndex &parent) const override;

	///
	/// @brief 获取当前选中样本ID
	///
	/// @param[in]  index  
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2022年12月30日，新建函数
	///
	int64_t GetSampleID(const QModelIndex &index);
    bool IsRowChecked(int iRow) const;
    int64_t GetSelectedSampleID() const;

	///
	/// @brief 获取AI识别的ID
	///
	/// @param[in]  index  索引
	///
	/// @return ai的id
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年7月19日，新建函数
	///
	int64_t GetAiRecognizeResultId(const QModelIndex &index);

	///
	/// @brief 获取样本类型
	///
	/// @param[in]  index  
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年5月30日，新建函数
	///
	int GetSampleType(const QModelIndex &index);

    ///
    /// @brief 获取按项目展示模式下的选中数据（供结果详情使用）
    ///
    /// @param[in]  index  当前单元格索引
    ///
    /// @return 组装数据
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年2月8日，新建函数
    ///
	std::shared_ptr<HistoryBaseDataItem> GetAssayItemData(const QModelIndex &index) const;
    bool IsItemCalcAssay(const QModelIndex& idx) const;

	///
	/// @brief 审核样本
	///
	/// @param[in]  indexs	选中要审核的样本索引
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年1月5日，新建函数
	///
	void CheckSample(const QModelIndexList &indexs);

	///
	/// @brief 获取样本审核状态
	///
	/// @param[in]  index  
	///
	/// @return 1-已审核，0-未审核，-1-无法获取
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年2月17日，新建函数
	///
	int GetCheck(const QModelIndex &index) const;

	///
	/// @brief 是否可以审核
	///
	/// @param[in]  index  
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年4月19日，新建函数
	///
	bool CanCheck(const QModelIndex &index);

	///
	/// @brief 模糊查询
	///
	/// @param[in]  str			模糊查询匹配字符串
	/// @param[in]  type		模糊查询类型字符串
    /// @param[in]  direction	模糊查询方向(0-匹配第一个，1-向上，2-向下)
    /// @param[in]  beginPos	开始查找的位置
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年4月17日，新建函数
	///
	int LikeSearch(const QString& str,const QString& type,const int& direction = 0, int beginPos = 0);
	
	///
	///  @brief 获取选中的样本数据
	///
	///  @param[in]   selectedIndexs  选中的样本索引列表
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年4月25日，新建函数
	///
	void GetSelectedSamples(const QModelIndexList &selectedIndexs, std::vector<std::shared_ptr<HistoryBaseDataSample>>& selectedSamples) const;
	
	///
	///  @brief 获取选中的项目数据
	///
	///
	///  @param[in]   selectedIndexs  选中的项目索引列表
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年4月25日，新建函数
	///
	std::vector<std::shared_ptr<HistoryBaseDataByItem>> GetSelectedItems(const QModelIndexList &selectedIndexs);
	
	///
	///  @brief 获取选中样本数据包含的项目数据
	///
	///
	///  @param[in]   selectedSamples  选中的样本数据列表
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年4月25日，新建函数
	///
	void GetItemsInSelectedSamples(const std::vector<std::shared_ptr<HistoryBaseDataSample>>& selectedSamples, \
        std::vector<std::shared_ptr<HistoryBaseDataByItem>>& relatedAssays);
    std::shared_ptr<HistoryBaseDataByItem> GetItemInSelectedSample(int64_t sampleId, int64_t itemId);

    ///
    ///  @brief 获取选中项目数据的样本数据
    ///
    ///
    ///  @param[in]   pItem  选中的样本数据列表
    ///
    ///  @return	
    ///
    ///  @par History: 
    ///  @li 6889/ChenWei，2024年4月16日，新建函数
    ///
    void GetSamplesInSelectedItems(const std::shared_ptr<HistoryBaseDataByItem>& pItem, \
        std::shared_ptr<HistoryBaseDataSample>& pSample);
	
	///
	/// @brief 设置全勾选还是全不勾选
	///
	/// @param[in]  bCheck  
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年7月24日，新建函数
	///
	void SetOrClearAllCheck(bool bCheck);

	///
	/// @brief 设置全勾选还是全不勾选
	///
	/// @param[in]  index  
	/// @param[in]  IscontrolKeyPress  control键是否被按下（true:按下）
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年7月24日，新建函数
	///
	void SetCheckBoxStatus(const QModelIndex& index, bool IscontrolKeyPress = false);

	///
	/// @brief 获取勾选的索引
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年7月24日，新建函数
	///
	QModelIndexList GetCheckedModelIndex() const;

	bool IsSelectAll();

	///
	///  @brief 获取审核字符串
	///
	///
	///  @param[in]   iCheck  审核枚举值
	///
	///  @return	审核字符串
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年5月19日，新建函数
	///
	static QString GetCheck(const int iCheck);

	///
	/// @brief   根据结果和设置显示结果范围
	///
	/// @param[in] isRe,cris, ref, reCris, reRef 历史数据（是否是重测，危机，参考，重测危机，重测参考）
	/// @param[in]		resultProm		设置参数
	/// @param[out]		firstResult		首查结果
	/// @param[out]		retestResult	复查结果
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年12月22日，新建函数
	///
	static void ResultRangeFlag(int isRe, int cris, int ref, int reCris, int reRef, int sampType,\
		stAssayResult& firstResult, stAssayResult& retestResult);

	///
	/// @brief 生成flag，根据结果状态
	///
	/// @param[in]  assayCode	 // 项目编号
	/// @param[in]  qualitative  // 定性结果,0阴性，1阳性，参见::tf::QualJudge
	/// @param[in]  cris		 // 参考范围标志，参见 tf::RangeJudge::type
	/// @param[in]  ref			 // 危急范围标志，参见 tf::RangeJudge::type
	/// @param[out]  result		 // 结果信息
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年7月10日，新建函数
	///
	static void ResultRangeFlag(int assayCode, int sampType, int qualitative, int cris, int ref, stAssayResult& result);

	///
	/// @brief 获取项目结果
	///
	/// @param[in]		data	历史数据
	/// @param[in]		bRetest	是否是复查列
	/// @param[out]		result  测试结果
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年12月22日，新建函数
	///
	static void GetAssayResult(const HistoryBaseDataItem& data, bool bRetest, stAssayResult& result);

	///
	/// @brief 获取项目结果
	///
	/// @param[in]		data	历史数据
	/// @param[in]		bRetest	是否是复查列
	/// @param[out]		result  测试结果
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年12月22日，新建函数
	///
	static void GetAssayResult(const HistoryBaseDataByItem& data, bool bRetest, stAssayResult& result);

    ///
    /// @bref
    ///		用结果完善组装的结果状态码
    ///
    /// @param[in] data 当前结果
    /// @param[in] bRetest 是否是复查
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年7月17日，新建函数
    ///
    static QString GetDisplayStatuCode(const HistoryBaseDataByItem& data, bool bRetest);

    ///
    /// @brief 为给定的项目结果设置标识
    ///
    /// @param[in]		dValue	结果
    /// @param[in]		isOriginal	是否是原始结果
    /// @param[in]		data	历史数据
    /// @param[in]		bRetest	是否是复查列
    /// @param[out]		result  测试结果
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 6889/ChenWei，2024年6月22日，新建函数
    ///
    static void SetAssayResultFlag(double dValue, bool isOriginal, const HistoryBaseDataByItem& data, bool bRetest, stAssayResult& result);

    ///
    /// @brief 获取结果单位
    ///
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 6889/ChenWei，2024年5月29日，新建函数
    ///
    static QString GetResultUnit(const HistoryBaseDataByItem& data);

protected:
	QVariant data(const QModelIndex &index, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	QHistorySampleAssayModel();

Q_SIGNALS:
    void sampleChanged();
	///
	/// @brief 高亮信号
	///
	/// @param[in]  index  索引
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年4月17日，新建函数
	///
	void HightLightChanged(int index);

private:

	///
	/// @brief 获取样本的显示数据(按样本显示)
	///
	/// @param[in]  index  位置
	/// @param[in]  role   角色
	///
	/// @return 显示数据
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年8月3日，新建函数
	///
	QVariant DataSample(const QModelIndex &index, int role) const;

	///
	/// @brief 获取样本的显示数据(按项目显示)
	///
	/// @param[in]  index  位置
	/// @param[in]  role   角色
	///
	/// @return 显示数据
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年8月3日，新建函数
	///
	QVariant DataAssay(const QModelIndex &index, int role) const;
public:
	///
	/// @brief 通过样本ID审核样本
	///
	/// @param[in]  vecSampleID	样本ID
	/// @param[in]  bCheck		审核状态	
	/// @param[in]  bUpdate		是否上传结果	
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年5月31日，新建函数
	///
	void CheckSampleBySampleID(const std::vector<int64_t> &vecSampleID, bool bCheck, const bool bUpdate = true);	

    int GetHistorySample(int64_t sampleId) const
    {
        for (const std::shared_ptr<HistoryBaseDataSample>& pS : m_vecHistoryBaseDataSamples)
        {
            if (pS->m_sampleId == sampleId)
            {
                return pS->m_testMode;
            }
        }
        return -1;
    }

private slots:
	///
private:
	// QStringList                     m_assaySelectHeader;        ///< 项目选择的表头
	QStringList                     m_sampleBrowseHeader;       ///< 数据浏览的表头
	QStringList                     m_assayBrowseHeader;        ///< 数据浏览中按项目展示
	VIEWMOUDLE						m_moudleType;				///< 模式类型
	MOUDLESTATUS					m_moduleStatus;				///< 模式的状态
	::tf::HistoryBaseDataQueryCond	m_cond;						///< 查询条件

	std::vector<std::shared_ptr<HistoryBaseDataSample>>	m_vecHistoryBaseDataSamples;
	std::set<int>	m_setHistoryBaseDataSamplesCheck;	        ///< 样本展示时勾选的行号
	bool							m_bCheckAll;				///< 全选
	std::vector<std::shared_ptr<HistoryBaseDataByItem>>	m_vecHistoryBaseDataItems;
	std::set<int>	m_setHistoryBaseDataItemsCheck;		        ///< 项目展示时勾选的行号

	std::map<int64_t, int64_t>		m_mapSampleIndex;			///< 样本按样本展示的映射表，key-样本ID，value-vector索引
	std::multimap<int64_t, int64_t>	m_mapSampleAssayIndex;		///< 样本按项目展示的映射表，key-样本ID，value-vector索引 按项目展示时样本ID可能重复
	std::shared_ptr<std::thread>	m_spThread;					///< 组织映射表的子线程
	std::atomic_bool				m_bThreadStopFlag;			///< 子线程停止标志
	std::mutex						m_mutex;					///< 映射表锁
};

// 历史数据按样本展示-项目数据
class QHistorySampleModel_Assay : public QAbstractTableModel
{
	Q_OBJECT
public:
	enum class COL
	{
		ASSAY = 0,			  ///< 项目
		RESULT,				  ///< 结果
		RESULTSTATUS,		  ///< 结果状态
		RECHECKRESULT,		  ///< 复查结果
		RECHECKSTATUS,		  ///< 结果状态（复查）
		UNIT,				  ///< 单位
		MODULE				  ///< 模块
	};

	virtual ~QHistorySampleModel_Assay();

	///
	/// @brief 获取单例对象
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年6月30日，新建函数
	///
	static QHistorySampleModel_Assay& Instance();

	///
	/// @brief 更新历史数据
	///
	/// @param[in]  sampleID	样本ID
	/// @param[in]  cond		查询条件
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2022年12月29日，新建函数
	///
	void Update(const int64_t& sampleID, const ::tf::HistoryBaseDataQueryCond& cond);
    void GetItems(int64_t sampleId, const ::tf::HistoryBaseDataQueryCond& cond, std::vector<std::shared_ptr<HistoryBaseDataItem>>& vecItems);

	///
	/// @brief 刷新
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年7月17日，新建函数
	///
	void Refush();

	///
	/// @brief 排序规则变化时刷新数据
    ///
    /// @param[out]  vecItems 被排序的Item
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年10月12日，新建函数
	///
	void RefushBySort(std::vector<std::shared_ptr<HistoryBaseDataItem>>& vecItems);

	///
	/// @brief 清除历史数据
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年3月16日，新建函数
	///
	void Clear();

	///
	/// @brief
	///     获取单元格属性(可编辑、可选择)
	///
	/// @param[in]    index      当前单元格索引
	///
	/// @return       QVariant   包装的数据
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年6月30日，新建函数
	///
	Qt::ItemFlags flags(const QModelIndex &index) const override;

	int rowCount(const QModelIndex &parent) const override;
	int columnCount(const QModelIndex &parent) const override;

	inline const std::vector<std::shared_ptr<HistoryBaseDataItem>>& GetAssay()
	{
		return m_vecHistoryBaseDataItems;
	}
	inline const int64_t& GetSampleID()
	{
		return m_iSampleID;
	}

private slots:

	///
	/// @brief 更新索引顺序
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年10月8日，新建函数
	///
	bool UpdateIndex();

protected:
	QVariant data(const QModelIndex &index, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	QHistorySampleModel_Assay();

private:
	QStringList                     m_assayBrowseHeader;        ///< 数据浏览中按项目展示
	std::vector<std::shared_ptr<HistoryBaseDataItem>> m_vecHistoryBaseDataItems;
	int64_t							m_iSampleID;			///< 样本ID
	::tf::HistoryBaseDataQueryCond	m_cond;					///< 查询条件
	std::map<int, int>              m_showOrder;                ///< 显示顺序(项目编号--位置)
};

