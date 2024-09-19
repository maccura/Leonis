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
/// @file     RangeParaWidget.h
/// @brief    项目范围
///
/// @author   5774/WuHongTao
/// @date     2021年7月19日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2021年7月19日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <cfloat>
#include <QWidget>
#include "shared/uicommon.h"
#include "model/AssayListModel.h"
#include "src/common/Mlog/mlog.h"
#include "src/thrift/ch/gen-cpp/ch_constants.h"
#include "src/thrift/ch/c1005/gen-cpp/c1005_types.h"
#include "src/thrift/ise/gen-cpp/ise_constants.h"

class CRangParaSetWidget;
class RecheckConditionsDlg;
namespace Ui { class CRangeParaWidget; };

// 参考范围表各列数据
enum REF_TABLE_COLUMN	
{
	REF_TABLE_NUM_COLUMN,						// 序号
	REF_TABLE_DEFAULT_COLUMN,				    // 是否缺省
    REF_TABLE_SAMPLE_SOURCE,                    // 样本源
	REF_TABLE_LOW_COLUMN,						// 低值
	REF_TABLE_UP_COLUMN,						// 高值
	REF_TABLE_REF_COLUMN,					    // 参考值范围
    REF_TABLE_F_LOW_COLUMN,						// 危急低值
	REF_TABLE_F_UP_COLUMN,						// 危急高值
	REF_TABLE_GENDER_COLUMN	,					// 性别
	REF_TABLE_AGE_LOW_COLUMN,					// 年龄下限
    REF_TABLE_AGE_UP_COLUMN,					// 年龄上限
	REF_TABLE_AGEUNIT_COLUMN					// 单位
};

class CRangeParaWidget : public QWidget
{
    Q_OBJECT

public:
    ///
    /// @brief
    ///     构造函数
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年7月19日，新建函数
    ///
    CRangeParaWidget(QWidget *parent = Q_NULLPTR);

    ///
    /// @brief
    ///     析构函数
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年7月19日，新建函数
    ///
    ~CRangeParaWidget();

    ///
    /// @brief
    ///     显示之前初始化
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年7月19日，新建函数
    ///
    void InitBeforeShow();

    ///
    /// @brief
    ///     界面显示后初始化
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年7月19日，新建函数
    ///
    void InitAfterShow();

	///
	/// @brief	为输入框设置正则表达式
	///
	/// @return 
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年10月24日，新建函数
	///
	void SetCtrlsRegExp();

	///
	/// @brief	检查用户输入数据是否合法
	///
    /// @param[in]  code  项目编号
    ///
	/// @return 数据合法返回true
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年12月14日，新建函数
	///
	bool CheckUserInputData(int code);

    ///
    /// @brief
    ///     初始化参考区间表
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年7月19日，新建函数
    ///
    void InitRefRangeTable();

	///
	/// @brief	重置控件
	///     
	/// @par History:
	/// @li 7951/LuoXin，2023年1月4日，新建函数
	///
	void Reset(bool updateSampleType = true);

    ///
    /// @brief  设置界面控间的显隐
    /// 
    /// @param[in]  isIse  是否为ISE项目
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年6月19日，新建函数
    ///
    void SetCtrlsVisible(bool isIse);

    ///
    /// @brief
    ///     初始化范围界面数据
    ///
    /// @param[in]  item  项目信息
    /// @param[in]  updateSampleType  是否刷新样本下拉框
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年7月20日，新建函数
    ///
    void LoadRangeParameter(const AssayListModel::StAssayListRowItem& item, bool updateSampleType = true);

    ///
    /// @brief
    ///     窗口显示事件
    ///
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年7月20日，新建函数
    ///
    void showEvent(QShowEvent *event);

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
	/// @brief	获取参考区间
	///  
	/// @param[in]   row		 行号  
	///
	/// @return		referrence  参考区间
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年12月14日，新建函数
	///
	::tf::AssayReferenceItem GetReferrenceByRow(int row);

	///
	/// @brief	显示参考区间
	///     
	/// @param[in]  row   行号
	/// @param[in]  item  参考区间
    /// @param[in]  factor  单位倍率
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年12月29日，新建函数
	///
	void ShowReferrence(int row, ::tf::AssayReferenceItem item, double factor);

    ///
    /// @brief	获取是否启用线性拓展
    ///    
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年11月30日，新建函数
    ///
    bool GetLinearExtension();

    ///
    /// @brief
    ///     当前页面是否有未保存的数据
    ///
    ///@param[in]    item 项目信息
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年4月11日，新建函数
    ///
    bool HasUnSaveChangedData(const AssayListModel::StAssayListRowItem& item);

    ///
    /// @brief
    ///     获取血清信息检查值
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年4月22日，新建函数
    ///
    std::vector<int32_t> GetSerumIndexCheckValue();

    ///
    /// @brief
    ///     获取线性范围或者复查界限
    ///
    /// @param[in]  isRepeat	是否获取复查界限值
    /// @param[in]  factor	    倍率
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年4月22日，新建函数
    ///
    ::tf::DoubleRange GetTechnicalOrRepeatLimit(bool isRepeat, double factor);

    ///
    /// @brief
    ///     更新原模块复查下拉框选项
    ///
    /// @param[in]  isChemistry  是否为生化项目
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年07月09日，新建函数
    ///
    void UpdateRecheckComboBox(bool isChemistry);

    ///
    /// @brief
    ///     设置原模块复查的当前选项
    ///
    /// @param[in]  box  下拉框
    /// @param[in]  RerunDispatchModule  复查模块指定模式的设置
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年07月09日，新建函数
    ///
    void SetRerunDispatchModule(QComboBox* box, const ::tf::RerunDispatchModule& rdm);

    ///
    /// @brief
    ///     获取原模块复查的当前选项
    ///
    /// @param[in]  box  下拉框
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年07月09日，新建函数
    ///
    ::tf::RerunDispatchModule GetRerunDispatchModule(QComboBox* box);

	///
	/// @brief
	///     加载数据到控件
	///
	/// @param[in]  qryResp				生化/ISE通用项目查询结果
	/// @param[in]  qrySpecialResp		生化/ISE特殊项目查询结果
	/// @par History:
	/// @li 7951/LuoXin，2022年6月27日，新建函数
	///
	template<typename GENERAL_ASSAY_INFO, typename SPECIAL_ASSAY_INFO >
	void LoadDataToCtrls(std::shared_ptr<GENERAL_ASSAY_INFO> spAssayInfo, std::shared_ptr<SPECIAL_ASSAY_INFO> spSpecialInfo);

	///
	/// @brief
	///     获取范围配置参数
	///
	/// @param[out]  assayCfgInfo		生化/ISE通用项目信息
	/// @param[out]  specialCfgInfo		生化/ISE通用项目信息
    /// @param[in]   assayCode          项目编号
    /// @param[in]   isCheck            是否检查用户输入参数
    ///
	/// @par History:
	/// @li 7951/LuoXin，2022年6月27日，新建函数
	///
	template<typename GENERAL_ASSAY_INFO, typename SPECIAL_ASSAY_INFO >
	bool GetAnalysisParam(GENERAL_ASSAY_INFO& assayCfgInfo, SPECIAL_ASSAY_INFO& specialCfgInfo, int assayCode, bool isCheck = false);

protected Q_SLOTS:

    ///
    /// @brief
    ///     参考区间添加按钮被点击
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年7月19日，新建函数
    ///
    void OnRefAddBtnClicked();

    ///
    /// @brief
    ///     参考区间修改按钮被点击
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年7月19日，新建函数
    ///
    void OnRefModBtnClicked();

    ///
    /// @brief
    ///     参考区间删除按钮被点击
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年7月19日，新建函数
    ///
    void OnRefDelBtnClicked();
    
	///
	/// @brief
	///     参考区间保存按钮被点击
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年8月12日，新建函数
	///
	void OnRefSaveBtnClicked();

	///
	/// @brief
	///     复查条件按钮被点击
	///
	/// @par History:
	/// @li 7951/LuoXin，2023年2月1日，新建函数
	///
	void OnRecheckConditionBtnClicked();

	///
	/// @brief
	///     保存复查条件
	///
	/// @par History:
	/// @li 7951/LuoXin，2023年2月1日，新建函数
	///
	void OnSaveRecheckConditions(std::vector<std::string> shieldStatusCodes);

	///
	/// @brief
	///     当前选中样本源改变
	///
	/// @par History:
	/// @li 7951/LuoXin，2023年3月1日，新建函数
	///
	void OnSampleSourceChanged(const QString &text);

    ///
    /// @brief
    ///     保存参考范围
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年11月30日，新建函数
    ///
    bool SaveAssayReferrence();

    ///
    /// @bref
    ///		新增项目
    ///
    /// @par History:
    /// @li 7951/LuoXin, 2024年2月22日，新建函数
    ///
    void OnAddAssay(QString name, QString unit, int code, QString version, QSet<int> sampleSources);

    ///
    /// @brief	更新控件的显示和编辑权限
    ///    
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年2月20日，新建函数
    ///
    void UpdateCtrlsEnabled();

private:
    Ui::CRangeParaWidget*				ui;                      // UI指针
	RecheckConditionsDlg*				m_RecheckConditionsDlg;	 // 复查条件弹窗
    int                                 m_currentRow;            // 当前选中行
    bool								m_bInit;                 // 是否已经初始化
	bool								m_bSampleTypeFlag;		 // 样本源下拉框是否可用
	AssayListModel::StAssayListRowItem	m_rowItem;				 // 当前选中行信息
    
    CRangParaSetWidget*				    m_RangeParaSetWidget;	 // 参考范围参数配置界面指针
    QStandardItemModel*                 m_RefModel;              // 参考范围表数据模型
};


template<typename GENERAL_ASSAY_INFO, typename SPECIAL_ASSAY_INFO >
void CRangeParaWidget::LoadDataToCtrls(std::shared_ptr<GENERAL_ASSAY_INFO> spAssayInfo, std::shared_ptr<SPECIAL_ASSAY_INFO> spSpecialInfo)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 获取当前单位与主单位的转化倍率
	double factor;
	if (!GetUnitFactor(spAssayInfo->assayCode,factor))
	{
		ULOG(LOG_ERROR, "GetUnitFactor By AssayCode[%d] Failed !", spAssayInfo->assayCode);
		return;
	}

	// 线性范围
    double temp;
	if (spSpecialInfo != nullptr && spSpecialInfo->__isset.technicalLimit)
	{
        temp = spSpecialInfo->technicalLimit.lower;
        ui->lower_range_edit->setText(temp == std::numeric_limits<double>::lowest() ? "" : UiCommon::DoubleToQString(temp * factor));

        temp = spSpecialInfo->technicalLimit.upper;
		ui->high_range_edit->setText(temp == DBL_MAX ? "" : UiCommon::DoubleToQString(temp * factor));
	}

	// 复查界限
	if (spSpecialInfo != nullptr && spSpecialInfo->__isset.repeatLimit)
	{
        temp = spSpecialInfo->repeatLimit.lower;
		ui->lower_recheck_edit->setText(temp == std::numeric_limits<double>::lowest() ? "" : UiCommon::DoubleToQString(temp * factor));

        temp = spSpecialInfo->repeatLimit.upper;
		ui->upper_recheck_edit->setText(temp == DBL_MAX ? "" : UiCommon::DoubleToQString(temp * factor));
	}

	// 是否自动复查
	if (spAssayInfo->__isset.autoRetest)
	{
		ui->auto_grbox->setChecked(spAssayInfo->autoRetest);
	}

	// 血清指数
	if (spAssayInfo->serumIndexCheckValues.size() == 3)
	{
        int temp = spAssayInfo->serumIndexCheckValues[0];
		ui->LValue->setText(temp > 0 ? QString::number(temp) : "");

        temp = spAssayInfo->serumIndexCheckValues[1];
		ui->HValue->setText(temp > 0 ? QString::number(temp) : "");

        temp = spAssayInfo->serumIndexCheckValues[2];
		ui->IValue->setText(temp > 0 ? QString::number(temp) : "");
	}

	// 更新参考区间表
    int row = 0;
    m_RefModel->removeRows(0, m_RefModel->rowCount());

    if (spSpecialInfo != nullptr)
    {
	    for (auto& stuReference : spSpecialInfo->referenceRanges.Items)
	    {
		    ShowReferrence(row, stuReference, factor);

		    // 行号自增
		    row++;																								
	    }
    }

    // 自动复查指定模块信息
    SetRerunDispatchModule(ui->auto_recheck_commbox, spAssayInfo->autoRerunDispModule);

    // 手动复查指定模块信息
    SetRerunDispatchModule(ui->recheck_combox, spAssayInfo->manualRerunDispModule);
}

template<typename GENERAL_ASSAY_INFO, typename SPECIAL_ASSAY_INFO >
bool CRangeParaWidget::GetAnalysisParam(GENERAL_ASSAY_INFO &assayCfgInfo ,SPECIAL_ASSAY_INFO &specialCfgInfo, int assayCode, bool isCheck)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 检查用户输入数据
    if (isCheck && !CheckUserInputData(assayCode))
    {
        ULOG(LOG_INFO, "The User Input Data illegal !");
        return false;
    }

	// 获取当前单位与主单位的转化倍率
	double factor;
	if (!GetUnitFactor(assayCode, factor))
	{
		ULOG(LOG_ERROR, "GetUnitFactor By AssayCode[%d] Failed !", assayCode);
		return false;
	}

    // 样本类型
    specialCfgInfo.sampleSourceType = ui->sampleSourceCobox->currentData().toInt();

	// 技术界限
	specialCfgInfo.__set_technicalLimit(std::move(GetTechnicalOrRepeatLimit(false, factor)));
	
	//是否自动复查
    assayCfgInfo.__set_autoRetest(ui->auto_grbox->isChecked());

	// 复查界限
    specialCfgInfo.__set_repeatLimit(std::move(GetTechnicalOrRepeatLimit(true, factor)));
	
	// 血清信息检查值，分别表示L、H、I的门限值
    assayCfgInfo.__set_serumIndexCheckValues(std::move(GetSerumIndexCheckValue()));

    // 自动复查指定模块信息
    assayCfgInfo.__set_autoRerunDispModule(GetRerunDispatchModule(ui->auto_recheck_commbox));

    // 手动复查指定模块信息
    assayCfgInfo.__set_manualRerunDispModule(GetRerunDispatchModule(ui->recheck_combox));

	return true;
}
