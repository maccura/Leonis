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
/// @file     assayparamtabwidget.h
/// @brief    应用界面->项目设置界面->项目参数tab页面
///
/// @author   4170/TangChuXian
/// @date     2020年8月24日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2020年8月24日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QTabWidget>
#include <memory>
#include "model/AssayListModel.h"
#include "src/thrift/gen-cpp/defs_types.h"
#include "src/thrift/im/gen-cpp/im_types.h"

class AnalysisParamSindWidget;
class AnalysisParamIsewidget;
class IseCaliSettingWidget;
class RangeParaWidget;
class AnalysisParamWidget;
class CalibrateSettingWidget;
class CRangeParaWidget;
class ImRangeParamWidget;
class QcParamSettingsWidget;
class ImCalibrateSettingWidget;
class ImAnalysisParamWidget;

namespace tf { class QcDoc; }

namespace ch 
{
    namespace tf 
    {
        class GeneralAssayInfo; 
        class SpecialAssayInfo;
    } 
}

class AssayParamTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    AssayParamTabWidget(QWidget *parent);
    ~AssayParamTabWidget();

    ///
    /// @brief
    ///     页签类型
    ///
    enum TabType{ TabAnalsys, TabCali, TabQc, TabRange}; 

    ///
    /// @brief
    ///     页签信息
    ///
    struct TabItem
    {
        TabItem(TabType tabType, QWidget* widget)
            : m_tbType(tabType), m_tabWidget(widget)
        {}
        TabType m_tbType;
        QWidget* m_tabWidget;

        QString GetName() const;
    };

	///
	/// @brief
	///     加载项目参数
	///
	/// @param[in]  item  用于定位数据
    /// @param[in]  updateSampleType  是否刷新样本类型下拉框
	///
	/// @par History:
	/// @li 4170/TangChuXian，2020年11月4日，新建函数
	///
    void LoadAssayParam(const AssayListModel::StAssayListRowItem& item, bool updateSampleType = true);
    void ReloadAssayParam();

    ///
    /// @brief
    ///     保存参数
    ///
    ///@param[in]    item 保存项目定位信息
	///@param[in]    strOptLog 操作日志
    ///
    /// @return    成功返回true 
    ///
    /// @par History:
    /// @li 6950/ChenFei，2022年05月19日，新建函数
    ///
    /// @par History:
    /// @li 1556/Chenjianlin，2024年3月7日，增加操作日志
    ///
    bool SaveAssayParam(const AssayListModel::StAssayListRowItem& item, QString& strOptLog);

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
    ///     保存质控文档
    ///
    ///@param[in]    changedQc 修改的质控文档信息
    ///
    /// @return    成功返回true 
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年12月6日，新建函数
    ///
    bool SaveQcDoc(std::vector<std::shared_ptr<tf::QcDoc>>& changedQc);

    ///
    /// @brief
    ///     检查是否修改了校准相关的参数
    ///
    ///@param[in]    newChGAI 修改后的生化项目参数
    ///@param[in]    newChGAI 修改后的生化特殊项目参数
    ///
    /// @return    成功返回true 
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年02月27日，新建函数
    ///
    bool CheckIsModifyCaliParams(const ch::tf::GeneralAssayInfo& newChGAI, const ch::tf::SpecialAssayInfo& newChSai);

    ///
    /// @brief
    ///     保存生化参数
    ///
    ///@param[in]    item 保存项目定位信息
    ///@param[in]    strOptLog 操作日志
    ///
    /// @return    成功返回true 
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年11月30日，新建函数
    ///
    bool SaveChAssayParam(const AssayListModel::StAssayListRowItem& item, QString& strOptLog);

    ///
    /// @brief
    ///     保存ISE参数
    ///
    ///@param[in]    item 保存项目定位信息
    ///
    /// @return    成功返回true 
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年11月30日，新建函数
    ///
    bool SaveIseAssayParam(const AssayListModel::StAssayListRowItem& item);

    ///
    /// @brief
    ///     保存IM参数
    ///
    ///@param[in]    item 保存项目定位信息
	///@param[in]    strOptLog 操作日志
	///
    /// @return    成功返回true 
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年11月30日，新建函数
    ///
	/// @par History:
	/// @li 1556/Chenjianlin，2024年3月7日，增加操作日志
	///
    bool SaveImAssayParam(const AssayListModel::StAssayListRowItem& item, QString& strOptLog);

	///
	/// @brief
	///     切换到生化模式
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年6月23日，更改函数
	///
	void OnchemistryPage();

	///
	/// @brief
	///     切换到免疫模式
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年6月23日，更改函数
	///
	void OnimmunePage();

protected:
	///
	/// @brief 定义界面显示的样式
	/// 
	enum class ShowStyle
	{
		CHEMISTRY,					/// 生化普通项目
		SIND,						/// SIND项目
		NA_K_CL,					/// ISE项目
		IMMUNE,                     /// 免疫
		EMPTY						/// 无效显示
	};

    ///
    /// @brief 界面显示后初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年8月24日，新建函数
    ///
    void InitAfterShow();

    ///
    /// @brief 窗口显示事件
    ///     
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年8月24日，新建函数
    ///
    void showEvent(QShowEvent *event);

    ///
    /// @brief
    ///     切换界面的显示
    ///
    ///@param[in]    style	新的样式 
    ///
    /// @return     
    ///
    /// @par History:
    /// @li 6950/ChenFei，2022年05月24日，新建函数
    ///
	void SwitchShowStyle(ShowStyle style);
	
	///
	/// @brief 获取更改操作细节信息
	///
	/// @param[in]  strOptLog  返回的日志记录信息
	/// @param[in]  spAssayInfoUi  界面修改的项目信息
	/// @param[in]  vecImAssayInfoUi  界面修改的免疫项目信息
	/// @param[in]  spChangeQc  界面修改的项目质控信息
	/// @param[in]  vecQcDocs  修改前的项目质控信息
	///
	/// @return 
	///
	/// @par History:
	/// @li 1556/Chenjianlin，2024年3月7日，新建函数
	///
	void GetImAssayOptLog(QString& strOptLog, const std::shared_ptr<tf::GeneralAssayInfo> spAssayInfoUi, const std::vector<std::shared_ptr<im::tf::GeneralAssayInfo>>& vecImAssayInfoUi, const std::vector<std::shared_ptr<tf::QcDoc>>& spChangeQc, const std::vector<tf::QcDoc>& vecQcDocs);

protected Q_SLOTS:
	///
	/// @brief	页面切换的槽函数
	///     
	/// @par History:
	/// @li 7951/LuoXin，2023年3月16日，新建函数
	///
	void OnCurrentChanged(int index);

    ///
    /// @bref
    ///		当前用户权限变更处理
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年4月3日，新建函数
    ///
    void OnPermisionChanged();

    void ChangeCurrParamVersion(const QString& strVer);


private:
    bool							m_bInit;					/// 是否已经初始化
	int								m_itemType;					/// 项目类型
	AnalysisParamWidget				*m_analyParamWidget;		/// 生化参数配置界面
	AnalysisParamSindWidget			*m_analyParamSindWidget;	/// S.IND配置界面
	CalibrateSettingWidget			*m_calibrateSettingWidget;	/// 校准参数配置界面
    CRangeParaWidget				*m_rangeParamWidget;		/// 范围参数配置界面
	IseCaliSettingWidget			*m_IseCaliSettingWidget;	/// ISE校准配置界面
	AnalysisParamIsewidget			*m_AnalysisParamIsewidget;	/// ISE分析配置界面
	ImRangeParamWidget              *m_ImRangeParamWidget;      /// 免疫参考范围界面

    QcParamSettingsWidget           *m_qcParamSettingsWidget;   /// 质控参数设置
	ImCalibrateSettingWidget        *m_ImCaliSettingWidget;     /// 免疫校准界面
	ImAnalysisParamWidget           *m_ImAnalysisParamWidget;   /// 免疫分析界面

	ShowStyle						m_lastShowStyle;			/// 上次的显示样式
	AssayListModel::StAssayListRowItem m_currentItem;		    /// 当前加载的信息

    std::vector<TabItem>            m_tabInfo;                  /// 暂存Tab页签信息
};
