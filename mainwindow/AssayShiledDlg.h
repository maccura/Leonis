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
/// @file     AssayShiledDlg.h
/// @brief    项目遮蔽弹窗
///
/// @author   7951/LuoXin
/// @date     2023年2月3日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2023年2月3日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <memory>
#include "shared/basedlg.h"

class QTableView;
class QColorHeaderModel;
class QUtilitySortFilterProxyModel;

namespace tf { class GeneralAssayInfo; }
namespace Ui {
    class AssayShiledDlg; 
};

class AssayShiledDlg : public BaseDlg
{
	Q_OBJECT

public:
	AssayShiledDlg(QWidget *parent = Q_NULLPTR);
	~AssayShiledDlg();

	///
	/// @brief
	///     加载项目信息到表格
	///
	/// @par History:
	/// @li 7951/LuoXin，2023年2月3日，新建函数
	///
	void LoadDataToDlg();

protected:
    ///
    /// @brief 窗口显示事件
    ///     
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年5月9日，新建函数
    ///
    void showEvent(QShowEvent *event);

private:

	///
	/// @brief		为表格添加勾选功能 
	///
	/// @param[in]  view  表格
	/// @param[in]  model 表格的model 
	///
	/// @par History:
	/// @li 7951/LuoXin，2023年3月15日，新建函数
	///
	void AddSelectToView(QTableView* view, QColorHeaderModel* model);

	///
	/// @brief		初始化表格 
	///
	/// @param[in]  view  表格
	/// @param[in]  model 表格的model 
	///
	/// @par History:
	/// @li 7951/LuoXin，2023年3月15日，新建函数
	///
	void InitTableView(QTableView* view, QColorHeaderModel* model);

	///
	/// @brief		为表格添加项目 
	///
	/// @param[in]  model 表格的model 
	/// @param[in]  gai	  项目信息
	/// @param[in]  deviceClassify 设备的类型
	///
	/// @par History:
	/// @li 7951/LuoXin，2023年3月15日，新建函数
	///
	void AddAssayToView(QColorHeaderModel* model, std::shared_ptr<tf::GeneralAssayInfo> gai, int deviceClassify);

	///
	/// @brief	保存被屏蔽的项目
	///     
	/// @param[in]  model	tableView的模型
	/// @param[in]  column  tableView的列
	///
	/// @return 成功返回true
	///
	/// @par History:
	/// @li 7951/LuoXin，2023年2月3日，新建函数
	///
	bool SaveShiledByColumn(QColorHeaderModel* model, int column);

protected Q_SLOTS:
	///
	/// @brief
	///     保存按钮被点击
	/// @par History:
	/// @li 7951/LuoXin，2023年2月3日，新建函数
	///
	void OnSaveBtnClicked();

signals:

	///
	/// @brief 项目屏蔽修改信号
	///
	/// @par History:
	/// @li 7951/LuoXin，2023年2月24日，新建函数
	///
	void sigAssayShiledChanged();

private:
	Ui::AssayShiledDlg*								ui;
    QColorHeaderModel*								m_chTableViewMode;			// 生化项目列表的model
    QColorHeaderModel*								m_iseTableViewMode;			// ise项目列表的model
    QColorHeaderModel*								m_imTableViewMode;			// 免疫项目列表的model
    QUtilitySortFilterProxyModel*                   m_chSortModel;              // 生化表格排序的model
    QUtilitySortFilterProxyModel*                   m_imSortModel;              // 免疫表格排序的model
};
