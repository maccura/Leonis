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
/// @file     ImRangeParamWidget.h
/// @brief    项目范围
///
/// @author   1226/ZhangJing
/// @date     2023年2月14日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 1226/ZhangJing，2023年2月14日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <QWidget>
#include <map>
#include "model/AssayListModel.h"
#include "src/thrift/im/i6000/gen-cpp/i6000_types.h"

// 参考范围表各列数据
enum IM_REF_TABLE_COLUMN
{
	IM_REF_TABLE_NUM_COLUMN,						// 序号
	IM_REF_TABLE_DEFAULT_COLUMN,				    // 是否缺省
	IM_REF_TABLE_SAMP_SOURCE,                       // 样本源
	IM_REF_TABLE_LOW_COLUMN,						// 低值
	IM_REF_TABLE_UP_COLUMN,						    // 高值
	IM_REF_TABLE_REF_COLUMN,					    // 参考值范围
    IM_REF_TABLE_F_AGE_COLUMN_LOW,					// 危急低值
    IM_REF_TABLE_F_AGE_COLUMN_UPPER,				// 危急高值
    IM_REF_TABLE_GENDER_COLUMN,					    // 性别
    IM_REF_TABLE_AGE_LOW_COLUMN,					// 年龄下限	
    IM_REF_TABLE_AGE_UP_COLUMN,					    // 年龄上限
	IM_REF_TABLE_AGEUNIT_COLUMN					    // 单位
};

class CRangParaSetWidget;
class QComboBox;

namespace Ui { class ImRangeParamWidget; };

class ImRangeParamWidget :
    public QWidget
{
    enum UiCtrlValidError {
        UV_NO_ERROR,
        // 自动复查
        UV_AUTORETEST_VALUE_LOWER,  // 值错误：自动复查范围1低值设置范围[0,999999].
        UV_AUTORETEST_VALUE_UPPER,  // 值错误：自动复查范围1高值设置范围[0,999999].
        UV_AUTORETEST_RANGE,        // 值错误：自动复查范围1低值大于等于高值.
        UV_AUTORETEST_DILU_RADIO,   // 值错误：自动复查1稀释倍数
        UV_AUTORETEST_LEAK_PARAM,   // 填写的值不完整
        UV_AUTORETEST_DILU_RADIO_LESS,  // 值错误：自动复查1稀释倍数小于默认稀释倍数
        UV_AUTORETEST_DILU_RADIO_MORE,  // 值错误：自动复查1稀释倍数大于最大稀释倍数
        UV_AUTORETEST_VALUE2_LOWER,  // 值错误：自动复查范围2低值设置范围[0,999999].
        UV_AUTORETEST_VALUE2_UPPER,  // 值错误：自动复查范围2高值设置范围[0,999999].
        UV_AUTORETEST_RANGE2,       // 值错误：自动复查范围2低值大于等于高值.
        UV_AUTORETEST_DILU_RADIO2,   // 值错误：自动复查2稀释倍数
        UV_AUTORETEST_DILU_RADIO2_LESS, // 值错误：自动复查2稀释倍数小于默认稀释倍数
        UV_AUTORESTES_DILU_RADIO2_MORE, // 值错误：自动复查2稀释倍数大于最大稀释倍数
        UV_AUTORESTES_RANGE_COVER       // 值错误：自动复查范围不能重叠包含
    };

    Q_OBJECT

public:
    ///
    /// @brief
    ///     构造函数
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年7月19日，新建函数
    ///
    ImRangeParamWidget(QWidget *parent = Q_NULLPTR);

    ///
    /// @brief
    ///     析构函数
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年7月19日，新建函数
    ///
    ~ImRangeParamWidget();

    ///
    /// @brief
    ///     界面显示后初始化
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年7月19日，新建函数
    ///
    void InitAfterShow();

    ///
    /// @brief
    ///     初始化参考区间表
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年7月19日，新建函数
    ///
    void InitRefRangeTable();

    ///
    /// @brief
    ///     初始化范围界面数据
    ///
    /// @param[in]  item  项目信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年7月20日，新建函数
    ///
    void LoadRangeParameter(const AssayListModel::StAssayListRowItem& item);

    ///
    /// @brief保存范围数据
    ///     
    ///
    /// @param[in]  db  生化数据库主键
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年7月20日，新建函数
    ///
    bool GetRangeParameter(const AssayListModel::StAssayListRowItem& item, std::vector<std::shared_ptr<::im::tf::GeneralAssayInfo>>& vecImAssayInfo);

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
	/// @brief	获取参考区间
	///  
	/// @param[in]   row		 行号  
	/// @param[out] referrence  参考区间
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年12月14日，新建函数
	///
	bool GetReferrenceByRow(int row, ::tf::AssayReferenceItem& refRang);

	///
	/// @brief	显示参考区间
	///     
	/// @param[in]  row   行号
	/// @param[in]  item  参考区间
	/// @param[double]  radio  单位倍率
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年12月29日，新建函数
	///
	void ShowReferrence(int row, const ::tf::AssayReferenceItem& item);

	///
	/// @brief 获取更改操作细节信息
	///
	/// @param[in]  strOptLog  返回的日志记录信息
	/// @param[in]  spAssayInfoUi  界面修改的项目信息
	/// @param[in]  vecImAssayInfoUi  界面修改的免疫项目信息
	///
	/// @return 
	///
	/// @par History:
	/// @li 1556/Chenjianlin，2024年3月7日，新建函数
	///
	void GetImAssayOptLog(QString& strOptLog, const std::shared_ptr<tf::GeneralAssayInfo> spAssayInfoUi, const std::vector<std::shared_ptr<im::tf::GeneralAssayInfo>>& vecImAssayInfoUi);


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
    /// @bref
    ///		数据报警复查设置按钮
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年8月30日，新建函数
    ///
    void OnAlarmRetestSettings();

    ///
    /// @bref
    ///		更新界面与按钮状态
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年4月20日，新建函数
    ///
    void UpdateCtrlStatu();

private:
    ///
    /// @brief
    ///     加载数据到控件
    ///
    /// @param[in]  assayCode  项目通道号
    /// @par History:
    /// @li 1226/zhangjing，2023年2月15日，新建函数
    ///
    void LoadRangeTbCtrlsData(const std::shared_ptr<im::tf::GeneralAssayInfo>& imAssay);

    ///
    /// @bref
    ///		加载自动复查的数据到界面
    ///
    /// @param[in] imAssay 免疫项目
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年8月31日，新建函数
    ///
    void LoadRetestSettings(std::shared_ptr<im::tf::GeneralAssayInfo>& imAssay);

    ///
    /// @brief
    ///     获取范围配置参数
    ///
    /// @param[out]  imAssay  免疫通用项目列表
    /// @param[in]  factor  单位倍率
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年2月15日，新建函数
    ///
    void TakeRangeTbCtrlsData(im::tf::GeneralAssayInfo& imAssay, double factor);

    ///
    /// @bref
    ///		获取自动复查的设置
    ///
    /// @param[in] imAssay 免疫项目
    /// @param[in] factor 单位倍率
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年8月31日，新建函数
    ///
    void TakeRetestSettinggs(im::tf::GeneralAssayInfo& imAssay, double factor);

    ///
    /// @bref
    ///		获取界面所有行设置(tbView表的第0列为缓存::tf::AssayReferenceItem的列，缓存id为Qt::UserRole)
    ///
    /// @param[out] rangeItems 界面所有设置项
    /// @param[out] rowIdxMap 界面行号与数据的映射
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年8月3日，新建函数
    ///
    void GetUiReferenceItem(::tf::AssayReference& rangeItems, std::map<int, int>& rowIdxMap);

    ///
    /// @bref
    ///		检测界面自动复查设置的合法性
    ///
    /// @param[in] imAssayInfo 免疫项目
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年8月31日，新建函数
    ///
    UiCtrlValidError CheckCtrlValueAutoRetest(const std::shared_ptr<im::tf::GeneralAssayInfo>& imAssayInfo);

    ///
    /// @bref
    ///		初始化控件输入值错误描述提示
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年8月21日，新建函数
    ///
    void InitCtrErrorDescription();

    ///
    /// @bref
    ///		保存设置信息，
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年12月4日，新建函数
    ///
    void SaveRangeInfo();

private:
    ///
    /// @bref
    ///		对模块选择下拉框进行赋值
    ///
    /// @param[in] pComb 目标下拉框
    /// @param[in] strDevName 设备名
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年4月29日，新建函数
    ///
    void AssignDeviceComb(QComboBox* pComb);

    ///
    /// @bref
    ///		通过复查模块类型和设备名获取设备模块Comb下拉列表显示文字
    ///
    /// @param[in] curType 复查模块指定类型
    /// @param[in] strDevName 设备名
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年4月29日，新建函数
    ///
    QString GetComDeviceShowText(::tf::RerunDispatchType::type curType, const std::string& strDevName);

private:
    Ui::ImRangeParamWidget*				ui;                      // UI指针
    bool								m_bInit;                 // 是否已经初始化
	int									m_optRow;				 // 当前操作行
	CRangParaSetWidget*				    m_RangeParaSetWidget;	 // 参考范围参数配置界面指针

    AssayListModel::StAssayListRowItem	m_rowItem;				 // 当前选中行信息
    std::map<UiCtrlValidError, QString> m_ctrlErrorDescription; // 控件合法性检查的错误描述
};


