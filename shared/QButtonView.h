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
/// @file     QButtonBasicView.h
/// @brief    项目选择容器的基础控件（项目的view）
///
/// @author   5774/WuHongTao
/// @date     2023年12月11日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2023年12月11日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <boost/optional.hpp>
#include <boost/bimap.hpp>
#include <QWidget>
#include "QAssaySelectButton.h"
#include "src/thrift/gen-cpp/defs_types.h"
#include "shared/CommonInformationManager.h"

class QPushButton;
class QGridLayout;
class QAssayCardPage;

namespace Ui 
{ 
	class QButtonViewPage;
	class QAssayCardPage;
};

enum class AssayDeviceType
{
	CHEMISTRYTYPE = 0,
	IMMUNETYPE,
	COMBINTYPE
};

enum class AssayTypeNew
{
	ASSAYTYPE1_WORKPAGE = 0,      // 项目选择器类型1, 工作页面，有属性的按钮
	ASSAYTYPE2,                   // 项目选择器类型2, 应用--项目位置设置框、备用订单，增加项目的项目选择
	ASSAYTYPE3_POSITION_SET       // 项目选择器类型3, 应用--项目位置设置框中的按钮
};

// 影响项目申请的重要参数状态
struct KeyDataIn
{
	AssayTypeNew				assayType;					///< 表格类型
	bool						isAudit = false;			///< 是否审核
	int							sourceType;					///< 样本源类型
	SampleSize					dulitionType;				///< 稀释方式
	int							dulitions;					///< 稀释倍数（当增量，减量，常量的时候不使用）
};

class QAssaySelectProcess : public QObject
{
	Q_OBJECT
public:
	QAssaySelectProcess() {};

	///
	/// @brief 初始化数据结构
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月15日，新建函数
	///
	virtual void Init() {};

	///
	/// @brief 设置句柄
	///
	/// @param[in]  page  句柄
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年6月25日，新建函数
	///
	void SetCardPage(QAssayCardPage* page) { m_cardPage = page; };

	///
	/// @brief 设置关键属性
	///
	/// @param[in]  keyStatus  属性
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月12日，新建函数
	///
	void SetKeyStatus(KeyDataIn& keyStatus);
	void SetCurrentSampleType(int currentType);
	void SetAudit(bool audit);
	void SetDilution(SampleSize absortSize, int dulitions);
	KeyDataIn GetKeyStatus() { return m_keyStatus; };
	void SetProcess(std::vector<QAssaySelectProcess*>& handers) { m_assaySelectHander = handers; };
	
	///
	/// @brief 取消项目选中标识
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年1月18日，新建函数
	///
	void ClearSelectd();

	///
	/// @brief 获取页面中项目的设置的最大位置编号
	///
	///
	/// @return 最大位置编号
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月21日，新建函数
	///
	int GetMaxPostion();

	///
	/// @brief 删除当前项目
	///
	///
	/// @return true:删除成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月21日，新建函数
	///
	bool DeleteCurrentAssay();

	///
	/// @brief 根据项目编号删除项目
	///
	/// @param[in]  assayCode  项目编号
	///
	/// @return true:删除成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月27日，新建函数
	///
	bool DeleteAssayByCode(int assayCode);

	///
	/// @brief 清除所有的项目位置信息
	///
	///
	/// @return true:清除成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月21日，新建函数
	///
	bool ClearAllAssay();

	///
	/// @brief 刷新页面到默认状态
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月18日，新建函数
	///
	void RefreshCard();

	///
	/// @brief 返回项目类型
	///
	///
	/// @return AssayType
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月19日，新建函数
	///
	AssayDeviceType GetAssayType() { return m_assayType; };

	///
	/// @brief 获取项目位置信息
	///
	///
	/// @return 项目位置信息
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月21日，新建函数
	///
	std::map<int, int> GetAssayPostion() { return m_changeMap; };

	///
	/// @brief 更新项目状态函数（项目遮蔽，校准屏蔽，试剂异常）
	///
	/// @param[in]  assayCodes  项目编号列表
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月26日，新建函数
	///
	void UpdateAssayStatus(int assayCodes);

	///
	/// @brief 设置项目信息
	/// @param[in]  postion		项目位置
	///
	/// @param[in]  assayCode  项目编号
	///
	/// @return true:设置成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月19日，新建函数
	///
	virtual bool SetAssayCardInfo(int assayCode, int postion);

	///
	/// @brief 设置组合项目信息
	///
	/// @param[in]  profileId  组合项目编号
	/// @param[in]  postion		项目位置
	///
	/// @return true:设置成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月19日，新建函数
	///
	virtual bool SetProfileCardInfo(int profileId, int postion);

	///
	/// @brief 获取样本的项目信息
	///
	/// @param[in]  testItems  单项目数据
	/// @param[in]  profiles   组合项目数据
	///
	/// @return :请求测试项目的数目-手工稀释的倍数
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月12日，新建函数
	///
	virtual std::pair<int, int> GetSampleTestItemInfo(std::vector<::tf::TestItem>& testItems) { return std::make_pair(0, 0); };

	///
	/// @brief 获取所有的组合项目的信息
	///
	/// @param[in]  profiles  组合项目信息
	///
	/// @return 项目信息
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月14日，新建函数
	///
	virtual void GetProfiles(std::vector<int64_t>& profiles) {};

	///
	/// @brief 更新项目状态
	///
	/// @param[in]  testItems  项目信息
	///
	/// @return true:若正确更新，则返回true
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月15日，新建函数
	///
	virtual bool UpdateTestItemStatus(std::shared_ptr<tf::TestItem>& testItems);

	///
	/// @brief 更新组合项目信息
	///
	/// @param[in]  profiles  组合姓名code列表
	///
	/// @return true:更新成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月19日，新建函数
	///
	virtual bool UpdateAssayProfileStatus(int64_t profiles) { return false; };

	///
	/// @brief 更新项目测试次数
	///
	/// @param[in]  assayCode  项目编号
	/// @param[in]  testTimes  测试次数	
	///
	/// @return true:更新成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月27日，新建函数
	///
	bool UpdateTestItemTimes(int assayCode, int testTimes);

	///
	/// @brief 处理项目
	///
	/// @param[in]  assayCode  组合项目code
	///
	/// @return true:已处理
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月14日，新建函数
	///
	virtual bool ProcessAssay(int assayCode, int postion = 0) { return false; };

	///
	/// @brief 组合项目的子项目处理方式
	///
	/// @param[in]  assayCode  子项目编号
	///
	/// @return true:处理成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月18日，新建函数
	///
	virtual bool ProcessFromComBin(int assayCode, bool isRequest) { return false; };

	///
	/// @brief 按照页面Index生成生成对应的显示button
	/// 
	/// @param[in]  assayDatas		  显示button队列
	/// @param[in]  pageElementCount  页面元素个数
	/// @param[in]  index			  显示页面索引
	///
	/// @return true:生成成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月15日，新建函数
	///
	virtual bool GenAssayButtonDataByIndex(const int pageElementCount, std::map<int, AssayButtonData>& assayDatas, int index);

protected:
	///
	/// @brief 获取样本的项目信息
	///
	/// @param[in]  testItems  单项目数据
	/// @param[in]  profiles   组合项目数据
	///
	/// @return :请求测试项目的数目-手工稀释的倍数
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月12日，新建函数
	///
	std::pair<int, int> GetSampleTestItemInfo(std::vector<::tf::TestItem>& testItems, const std::map<int, AssayButtonData>& AssayButtonDataMap);

	///
	/// @brief 将ISE和SIND转成naKCL等
	///
	/// @param[in]  convertCodes  转换后的集合
	/// @param[in]  assayCode	  源码
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年4月28日，新建函数
	///
	void ConvertCodes(std::vector<int>& convertCodes, const int assayCode);

	///
	/// @brief 获取组合项目信息
	///
	/// @param[in]  profiles			组合项目Id合集
	/// @param[in]  AssayButtonDataMap  项目信息
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月14日，新建函数
	///
	void GetSampleProfileInfo(std::vector<int64_t>& profiles, const std::map<int, AssayButtonData>& AssayButtonDataMap);

	///
	/// @brief 检测免疫项目稀释倍数设置是否超过要求的范围
	///
	/// @param[in]  data  项目选择按钮属性
	///
	/// @return true：通过， false：不通过
	///
	/// @par History:
	/// @li 1556/Chenjianlin，2023年9月14日，新建函数
	///
	bool IsImDiluTionPolicy(const AssayButtonData& data);

	///
	/// @brief 稀释策略是否符合标准
	///
	/// @param[in]  data  项目属性
	///
	/// @return true:符合
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月12日，新建函数
	///
	bool IsDiluTionPolicy(const AssayButtonData& data);

	///
	/// @brief 项目是否允许被选中（当前样本类型下）
	///
	/// @param[in]  assayCode			项目编号
	/// @param[in]  sampleSourceType	样本类型
	///
	/// @return true:被禁止
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月12日，新建函数
	///
	virtual bool IsEnableAssayCode(int assayCode, int sampleSourceType) { return false; };

	///
	/// @brief 根据项目基础信息生成按钮信息
	///
	/// @param[in]  rawAssayInfo  项目基础信息
	/// @param[in]  keyMap         位置映射关系
	/// @param[in]  buttonData    项目按钮信息
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月11日，新建函数
	///
	void GenButtonInfo(const std::vector<tf::GeneralAssayPageItem>& rawAssayInfo, boost::bimap<int, int>& keyMap, std::map<int, AssayButtonData>& buttonDataMap);

	///
	/// @brief 根据项目的状态，刷新项目card状态
	///
	/// @param[in]  status		项目展示状态
	/// @param[in]  testItem	项目
	///
	/// @return true:刷新成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月12日，新建函数
	///
	bool MergeAssayStatus(AssayButtonData& attribute, const tf::TestItem& testItem);

	///
	/// @brief 稀释倍数的转换
	///
	/// @param[in]  status		项目展示状态
	/// @param[in]  testItem	项目
	///
	/// @return true:刷新成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月12日，新建函数
	///
	bool DulitionConvert(AssayButtonData& attribute, const tf::TestItem& testItem);

	///
	/// @brief 删除bimap中的信息
	///
	/// @param[in]  first  位置信息
	/// @param[in]  second 项目编号
	///
	/// @return true:删除
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年1月2日，新建函数
	///
	bool DeleteBimap(boost::bimap<int, int>& biMap, boost::optional<int> first, boost::optional<int> second);

protected:
	std::vector<tf::GeneralAssayPageItem>	m_singleAssays;				///< 项目基础信息
	std::map<int, AssayButtonData>			m_basicAssayButtons;		///< 项目基础信息（转化后）
	std::map<int, AssayButtonData>			m_currentButtons;			///< 项目信息合并后
	boost::bimap<int, int>					m_singleMaps;				///< 生化项目位置-code
	boost::optional<int>					m_postion = boost::none;	///< 当前页选中位置
	KeyDataIn								m_keyStatus;				///< 关键状态信息
	std::vector<QAssaySelectProcess*>		m_assaySelectHander;		///< 项目处理的句柄(存储句柄)
	AssayDeviceType							m_assayType;				///< 项目类型（生化免疫组合）
	std::map<int, int>						m_changeMap;				///< 修改位置的地图
	QAssayCardPage*							m_cardPage = nullptr;		///< 生化免疫所有页面句柄
};

class QButtonModel : public QObject
{
	Q_OBJECT
public:
	QButtonModel(int elementCount = 0);

	///
	/// @brief 根据位置信息，设置数据
	///
	/// @param[in]  index  位置索引（row*cloumn + currentIndex）
	/// @param[in]  data   设置信息
	///
	/// @return true设置成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月11日，新建函数
	///
	bool setData(int index, AssayButtonData& data);

	///
	/// @brief 获取model当前的项目个数
	///
	///
	/// @return 项目个数
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月11日，新建函数
	///
	int GetTotal() { return m_data.size(); };

	///
	/// @brief 清除界面所有的值
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月11日，新建函数
	///
	void clearData();

	///
	/// @brief 返回所有的数据
	///
	///
	/// @return 所有的数据
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月11日，新建函数
	///
	const std::vector<AssayButtonData>& GetAllData() {return m_data; };

	///
	/// @brief 根据编号获取按钮的信息
	///
	/// @param[in]  index  编号
	///
	/// @return 按钮信息
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月11日，新建函数
	///
	boost::optional<AssayButtonData> GetDataByIndex(const int index) const;

Q_SIGNALS:
	///
	/// @brief 通知view，当前位置的按钮的状态有所变化
	///
	/// @param[in]  index  位置信息
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月11日，新建函数
	///
	void dataChanged(int index);

private:
	std::vector<AssayButtonData>	m_data;						///< 模型中保存的数据，用于刷新view
};

class QButtonBasicView : public  QWidget
{
	Q_OBJECT
public:
	QButtonBasicView(QWidget* parent = Q_NULLPTR, const int row = 5, const int column = 7);

	///
	/// @brief 设置数据模型
	///
	/// @param[in]  model  数据模型
	///
	/// @return true:设置成功，false:设置失败
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月11日，新建函数
	///
	bool setModel(QButtonModel* model);

	///
	/// @brief 返回当前模型
	///
	///
	/// @return 当前模型句柄
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月11日，新建函数
	///
	QButtonModel* GetModel() { return m_model; };

	///
	/// @brief 获取view的行和列
	///
	///
	/// @return 行+列
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月11日，新建函数
	///
	std::pair<int, int> GetSizeOfView() { return std::make_pair(m_row, m_column); };

	///
	/// @brief 调整view的结构（行数和列数）
	///
	/// @param[in]  row		行数
	/// @param[in]  column  列数
	///
	/// @return true:调整成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月11日，新建函数
	///
	bool AdjustView(const int row, const int column);

Q_SIGNALS:
	///
	/// @brief 项目被选中
	///
	/// @param[in]  assayCode  项目编号
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月12日，新建函数
	///
	void dataSelected(int postion, int assayCode);

protected slots:
	///
	/// @brief 模型更新界面的状态
	///
	/// @param[in]  index  未知
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月11日，新建函数
	///
	void OnUpdateButtonStatus(int index);

private:
	///
	/// @brief 根据参数生成按钮的view
	///
	/// @param[in]  row		行
	/// @param[in]  column  列
	///
	/// @return true:生成成功，反之：失败
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月11日，新建函数
	///
	bool GenTemplateOfView(const int row = 5, const int column = 7);

private:
	std::vector<QAssaySelectButton*> m_buttonElements;			/// view对应的元素集合
	int								m_row;						///< 行数
	int								m_column;					///< 列数
	QGridLayout*					m_layout;
	QButtonModel*					m_model;					///< 当前数据模
};

class QButtonViewPage : public  QWidget
{
	Q_OBJECT
public:
	QButtonViewPage(QWidget* parent);

	///
	/// @brief 设置页面的布局模板
	///
	/// @param[in]  totalRow		总共的行数
	/// @param[in]  singelRow		单项目行数
	/// @param[in]  columnCount		列数
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月11日，新建函数
	///
	void setViewMapTmp(int totalRow, int singelRow, int columnCount);

	///
	/// @brief 更新单项目和组合项目的数据状态
	/// 
	/// @param[in]  singleMap   单项目状态map
	/// @param[in]  combinMap   组合项目状态map
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月11日，新建函数
	///
	void updateViewMap(std::map<int, AssayButtonData>& singleMap, std::map<int, AssayButtonData>& combinMap);

	///
	/// @brief 更新对应的模型的数据状态
	///
	/// @param[in]  singleMap  数据
	/// @param[in]  model      模型
	///
	/// @return true:更新成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月11日，新建函数
	///
	bool upDateMap(std::map<int, AssayButtonData>& singleMap, QButtonModel* model);

Q_SIGNALS:
	///
	/// @brief 项目被选中
	///
	/// @param[in]  assayCode  项目编号
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月12日，新建函数
	///
	void dataSelected(bool isCombin, int postion, int assayCode);

private:
	Ui::QButtonViewPage*			ui;
};

class QAssayCardPage : public  QWidget
{
	Q_OBJECT
public:
	QAssayCardPage(QWidget* parent = Q_NULLPTR);

	///
	/// @brief 显示当前页面(让外部页面手动调用刷新)
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月26日，新建函数
	///
	void Show();

	///
	/// @brief 重做上次的选择
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年1月9日，新建函数
	///
	void Redo();

	///
	/// @brief 是否是生化模块
	///
	///
	/// @return true:生化模块
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月26日，新建函数
	///
	bool IsChemistry() { return m_chemistry; };

	///
	/// @brief 获取所有样本信息，包括生化和免疫
	///
	/// @param[in]  testItems  项目信息
	///
	/// @return 请求测试项目的数目-手工稀释的倍数
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月12日，新建函数
	///
	std::pair<int, int> GetSampleTestItemInfo(std::vector<::tf::TestItem>& testItems);

	///
	/// @brief 获取所有的组合项目的信息
	///
	/// @param[in]  profiles  组合项目信息
	///
	/// @return 项目信息
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月14日，新建函数
	///
	void GetProfiles(std::vector<int64_t>& profiles);

	///
	/// @brief 获取项目配置以后的项目位置信息
	///
	///
	/// @return 项目位置信息
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月21日，新建函数
	///
	std::map<int, int> GetAssayPostion();

	///
	/// @brief 获取当前的单项目行数
	///
	///
	/// @return 单项目行数
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年4月2日，新建函数
	///
	int GetSingleCount() { return m_singleCount; };

	///
	/// @brief 刷新界面
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月27日，新建函数
	///
	void RefreshCard();

	///
	/// @brief 根据项目编号删除项目
	///
	/// @param[in]  assayCode  项目编号
	///
	/// @return true:删除成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月27日，新建函数
	///
	bool DeleteAssayByCode(int assayCode);

	///
	/// @brief 更新组合项目信息
	///
	/// @param[in]  profiles  组合姓名code列表
	///
	/// @return true:更新成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月19日，新建函数
	///
	bool UpdateAssayProfileStatus(const std::vector<int64_t>& profiles);

	///
	/// @brief 更新单项目信息
	///
	/// @param[in]  testItems  单项目列表
	///
	/// @return true:更新成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月11日，新建函数
	///
	bool UpdateTestItemStatus(const std::vector<std::shared_ptr<tf::TestItem>>& testItems);

	///
	/// @brief 更新项目测试次数
	///
	/// @param[in]  assayCode  项目编号
	/// @param[in]  testTimes  测试次数	
	///
	/// @return true:更新成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月27日，新建函数
	///
	bool UpdateTestItemTimes(int assayCode, int testTimes);

	///
	/// @brief 显示第几页
	///
	/// @param[in]  index  页数（1,2,3）
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月11日，新建函数
	///
	void OnShowPage(int index);

	///
	/// @brief 获取项目申请的关键属性
	///
	///
	/// @return 属性信息
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月12日，新建函数
	///
	KeyDataIn GetKeyStatus();

	///
	/// @brief 设置关键属性
	///
	/// @param[in]  keyStatus  属性
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月12日，新建函数
	///
	void SetKeyStatus(KeyDataIn& keyStatus);
	void SetCurrentSampleType(int currentType);
	void SetAudit(bool audit);
	void SetDilution(SampleSize absortSize, int dulitions);

	///
	/// @brief 设置项目信息
	///
	/// @param[in]  assayCode  项目编号
	///
	/// @return true:设置成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月19日，新建函数
	///
	bool SetAssayCardInfo(int assayCode);

	///
	/// @brief 设置组合项目信息
	///
	/// @param[in]  profileId  组合项目编号
	///
	/// @return true:设置成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月19日，新建函数
	///
	bool SetProfileCardInfo(int profileId);

signals:
	///
	/// @brief 设备类型改变的消息
	///
	/// @param[in]  deviceType  0:生化类型 1：免疫类型
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月27日，新建函数
	///
	void deviceTypeChanged(int deviceType);

	///
	/// @brief 项目被选中的信息
	///
	/// @param[in]  postion  
	/// @param[in]  assayCode  
	///
	/// @return 
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年1月9日，新建函数
	///
	void assaySelected(int postion, int assayCode);

    ///
    /// @bref
    ///		按钮选中状态变化
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年5月7日，新建函数
    ///
    void selectChanged();

public slots:
	///
	/// @brief 更新view的样式或者是更新assayCard的数据
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月27日，新建函数
	///
	void OnUpdateView();

protected:
	void Init();

	///
	/// @brief 获取当前页面的项目数目
	///
	/// @param[in]  isSingle	 是否单项目
	/// @param[in]  singleCount  单项目行数
	///
	/// @return 项目个数
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年4月2日，新建函数
	///
	int PageViewCount(bool isSingle, int singleCount);

	///
	/// @brief 设置按钮属性
	///
	/// @param[in]  pushButton  按钮
	/// @param[in]  name		属性名称
	/// @param[in]  propertyMessage  属性
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年2月5日，新建函数
	///
	void SetProperty(std::vector<QPushButton*> groupButtons, QPushButton* pushButton, const char* name, const QString& propertyMessage);

	void showEvent(QShowEvent * event);

	///
	/// @brief 根据需求生成页面数和对应的按钮
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月21日，新建函数
	///
	void GenButtonPages();

	///
	/// @brief 根据项目基础信息生成按钮信息(组合项目)
	///
	/// @param[in]  rawAssayInfo   项目基础信息
	/// @param[in]  keyMap         位置映射关系
	/// @param[in]  buttonData     项目按钮信息
	///
	/// @return 
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月11日，新建函数
	///
	void GenButtonCombinInfo(const std::vector<tf::ProfileAssayPageItem>& rawAssayInfo, boost::bimap<int, int>& keyMap, std::map<int, AssayButtonData>& buttonData);

protected slots:
	///
	/// @brief 项目选中消息
	///
	/// @param[in]  assayCode  项目编号
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月12日，新建函数
	///
	void OnSelectAssay(bool isManual = false, bool isCombin = false, int postion = -1, int assayCode = -1);

	///
	/// @brief 改变设备类型的响应函数
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月19日，新建函数
	///
	void OnChangeDeviceType();

	///
	/// @brief 改变组合项目的行数的槽函数
	///
	/// @param[in]  pageNumber  组合项目行数
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月20日，新建函数
	///
	void OnChangeRowNumber(int pageNumber);

	///
	/// @brief 清空所有的项目
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月21日，新建函数
	///
	void OnClear();

	///
	/// @brief 删除选中项目
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月21日，新建函数
	///
	void OnDelete();

	///
	/// @brief 更新项目状态的槽函数
	///
	/// @param[in]  assayCodes  项目编号之间用“,”来分割
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月26日，新建函数
	///
	void OnUpdateAssayStatus(QString assayCodes);

private:
	Ui::QAssayCardPage*					ui;
	bool								m_chemistry;			///< 当前是生化模式
	int									m_currentPageIndex = 0;	///< 当前显示第几页
	int									m_singleCount;			///< 单项目的行数
	boost::optional<int>				m_postion = boost::none;///< 当前页选中位置
	std::vector<QAssaySelectProcess*>	m_assaySelectHander;	///< 项目处理的句柄
	KeyDataIn							m_keyStatus;			///< 关键状态信息
	std::vector<QPushButton*>			m_pageButton;			///< 页面选择按钮
	std::vector<std::shared_ptr<tf::TestItem>> m_testItems;		///< 当前页面显示的项目信息
	std::tuple<bool, int, int>			m_redokey;				///< bool isCombin, int postion, int assayCode（重做上一次的选中，自动）
};

class QAssayChSelectProcess : public QAssaySelectProcess
{
	Q_OBJECT
public:
	QAssayChSelectProcess();

	///
	/// @brief 获取样本的项目信息
	///
	/// @param[in]  testItems  单项目数据
	/// @param[in]  profiles   组合项目数据
	///
	/// @return :请求测试项目的数目-手工稀释的倍数
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月12日，新建函数
	///
	std::pair<int, int> GetSampleTestItemInfo(std::vector<::tf::TestItem>& testItems) override;

	///
	/// @brief 处理项目
	///
	/// @param[in]  assayCode  组合项目code
	///
	/// @return true:已处理
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月14日，新建函数
	///
	bool ProcessAssay(int assayCode, int postion = 0) override;

	///
	/// @brief 组合项目的子项目处理方式
	///
	/// @param[in]  assayCode  子项目编号
	///
	/// @return true:处理成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月18日，新建函数
	///
	bool ProcessFromComBin(int assayCode, bool isRequest) override;

	///
	/// @brief 初始化数据结构
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月15日，新建函数
	///
	void Init() override;

	///
	/// @brief 项目是否允许被选中（当前样本类型下）
	///
	/// @param[in]  assayCode			项目编号
	/// @param[in]  sampleSourceType	样本类型
	///
	/// @return true:被禁止
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月12日，新建函数
	///
	bool IsEnableAssayCode(int assayCode, int sampleSourceType) override;
};

class QAssayImSelectProcess : public QAssaySelectProcess
{
	Q_OBJECT
public:
	QAssayImSelectProcess();

	///
	/// @brief 获取样本的项目信息
	///
	/// @param[in]  testItems  单项目数据
	/// @param[in]  profiles   组合项目数据
	///
	/// @return :请求测试项目的数目-手工稀释的倍数
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月12日，新建函数
	///
	std::pair<int, int> GetSampleTestItemInfo(std::vector<::tf::TestItem>& testItems) override;

	///
	/// @brief 处理项目
	///
	/// @param[in]  assayCode  组合项目code
	///
	/// @return true:已处理
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月14日，新建函数
	///
	bool ProcessAssay(int assayCode, int postion = 0) override;

	///
	/// @brief 组合项目的子项目处理方式
	///
	/// @param[in]  assayCode  子项目编号
	///
	/// @return true:处理成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月18日，新建函数
	///
	bool ProcessFromComBin(int assayCode, bool isRequest) override;

	///
	/// @brief 初始化数据结构
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月15日，新建函数
	///
	void Init() override;

	///
	/// @brief 项目是否允许被选中（当前样本类型下）
	///
	/// @param[in]  assayCode			项目编号
	/// @param[in]  sampleSourceType	样本类型
	///
	/// @return true:被禁止
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月12日，新建函数
	///
	bool IsEnableAssayCode(int assayCode, int sampleSourceType) override { return false; };

private:
	///
	/// @brief 获取免疫项目的默认稀释倍数
	///
	/// @param[in]  assayCode  免疫项目的项目code
	///
	/// @return 稀释倍数
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年1月5日，新建函数
	///
	int GetDefaultDuliTon(int assayCode);
};

class QAssayComBinSelectProcess : public QAssaySelectProcess
{
	Q_OBJECT
public:
	QAssayComBinSelectProcess();

	///
	/// @brief 获取所有的组合项目的信息
	///
	/// @param[in]  profiles  组合项目信息
	///
	/// @return 项目信息
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月14日，新建函数
	///
	virtual void GetProfiles(std::vector<int64_t>& profiles) override;

	///
	/// @brief 更新项目状态
	///
	/// @param[in]  testItems  项目信息
	///
	/// @return true:若正确更新，则返回true
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月15日，新建函数
	///
	///bool UpdateTestItemStatus(std::shared_ptr<tf::TestItem>& testItems) override;

	///
	/// @brief 更新组合项目信息
	///
	/// @param[in]  profiles  组合姓名code列表
	///
	/// @return true:更新成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月19日，新建函数
	///
	virtual bool UpdateAssayProfileStatus(int64_t profiles) override;

	///
	/// @brief 处理项目
	///
	/// @param[in]  assayCode  组合项目code
	///
	/// @return true:已处理
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月14日，新建函数
	///
	bool ProcessAssay(int assayCode, int postion = 0) override;

	///
	/// @brief 按照页面Index生成生成对应的显示button
	/// 
	/// @param[in]  assayDatas  显示button队列
	/// @param[in]  index		显示页面索引
	///
	/// @return true:生成成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月15日，新建函数
	///
	bool GenAssayButtonDataByIndex(const int pageElementCount, std::map<int, AssayButtonData>& assayDatas, int index) override;

	///
	/// @brief 根据项目基础信息生成按钮信息(组合项目)
	///
	/// @param[in]  rawAssayInfo   项目基础信息
	/// @param[in]  keyMap         位置映射关系
	/// @param[in]  buttonData     项目按钮信息
	///
	/// @return 
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月11日，新建函数
	///
	void GenButtonCombinInfo(const std::vector<tf::ProfileAssayPageItem>& rawAssayInfo, boost::bimap<int, int>& keyMap, std::map<int, AssayButtonData>& buttonData);

	///
	/// @brief 初始化数据结构
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年12月15日，新建函数
	///
	void Init() override;
private:
	std::vector<::tf::ProfileAssayPageItem>	m_combinAssays;	///< 生化项目基础信息
	SOFTWARE_TYPE							m_softwareType;	///< 设备类型
};
