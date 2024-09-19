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
/// @file     RecheckConditionsDlg.h
/// @brief    自动复查条件弹窗
///
/// @author   7951/LuoXin
/// @date     2023年2月1日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2023年2月1日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include "shared/basedlg.h"

class QUtilitySortFilterProxyModel;

namespace tf { class DataAlarmItem; }

namespace Ui {
    class RecheckConditionsDlg;
};
class QStandardItemModel;


class RecheckConditionsDlg : public BaseDlg
{
	Q_OBJECT

public:
	RecheckConditionsDlg(QWidget *parent = Q_NULLPTR);
	~RecheckConditionsDlg();

	///
	/// @brief	更新勾选行
	///   
	/// @param[in]  shieldStatusCodes	禁用的状态码列表
	/// @param[in]  isCh    是否是生化
	/// @par History:
	/// @li 7951/LuoXin，2023年2月1日，新建函数
	///
	void UpdateSelectRows(const std::vector<std::string>& shieldStatusCodes, bool isCh);

signals:
	void sigSaveBtnClicked(std::vector<std::string> statusCodes);

private:
	///
	/// @brief	加载数据到界面
	/// 
	/// @par History:
	/// @li 7951/LuoXin，2023年2月1日，新建函数
	///
	void LoadDataToDlg();

	///
	/// @brief		添加信息到表格
	///     
	/// @param[in]  type		复查样本量类型
	/// @param[in]  dataAlarm	数据报警信息
	///
	/// @par History:
	/// @li 7951/LuoXin，2023年2月1日，新建函数
	///
	void AddDataToTableView(int type, const std::vector<tf::DataAlarmItem>& dataAlarm);

	protected Q_SLOTS:

	///
	/// @brief
	///     保存按钮被点击
	/// @par History:
	/// @li 7951/LuoXin，2023年2月1日，新建函数
	///
	void OnSaveBtnClicked();

private:
	Ui::RecheckConditionsDlg*				ui;
	QStandardItemModel*						m_recheckConditionsMode;			// 复查条件列表的model
    QUtilitySortFilterProxyModel*           m_sortModel;                        //< 表格排序的model
};
