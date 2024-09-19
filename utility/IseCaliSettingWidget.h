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
/// @file     IseCaliSettingWidget.h
/// @brief    ISE项目分析参数配置
///
/// @author   7951/LuoXin
/// @date     2022年6月28日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2022年6月28日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <QWidget>
#include "model/AssayListModel.h"

namespace Ui { class IseCaliSettingWidget; };

namespace ise
{
    namespace tf
    {
        class GeneralAssayInfo;
        class SpecialAssayInfo;
    }
};


class IseCaliSettingWidget : public QWidget
{
	Q_OBJECT

public:
	///
	/// @brief
	///     构造函数
	///
	/// @param[in]  parent  
	/// @par History:
	/// @li 7951/LuoXin，2022年6月28日，新建函数
	///
	IseCaliSettingWidget(QWidget *parent = Q_NULLPTR);

	///
	/// @brief
	///     析构函数
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年6月28日，新建函数
	///
	~IseCaliSettingWidget();

	///
	/// @brief	重置控件 
	///
	/// @par History:
	/// @li 7951/LuoXin，2023年1月4日，新建函数
	///
	void Reset();

	///
	/// @brief
	///     加载范围界面数据
	///
	/// @param[in]  item  项目信息
	///
	/// @return   成功返回true  
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年6月28日，新建函数
	///
	bool LoadRangeParameter(const AssayListModel::StAssayListRowItem& item);
	
    ///
    /// @brief  获取编辑的项目分析参数
    ///     
    ///
    /// @param[in]  iseGAI       ISE通用项目参数
    /// @param[in]  iseSAI       ISE特殊项目参数
    /// @param[in]  assayCode   项目编号
    ///
    /// @return 成功返回true
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年11月30日，新建函数
    ///
    bool GetAnalysisParam(ise::tf::GeneralAssayInfo& iseGAI, ise::tf::SpecialAssayInfo& iseSAI, int assayCode);

    ///
    /// @brief
    ///     当前页面是否有未保存的数据
    ///
    ///@param[in]    item 项目信息
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年4月22日，新建函数
    ///
    bool HasUnSaveChangedData(const AssayListModel::StAssayListRowItem& item);

private:
	Ui::IseCaliSettingWidget		*ui;                         // UI指针
};
