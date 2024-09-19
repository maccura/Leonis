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
/// @file     QOperationLogWgt.h
/// @brief 	  操作日志窗口（应用->日志->操作日志）
///
/// @author   7656/zhang.changjiang
/// @date      2023年4月14日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年4月14日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <thread>
#include <QWidget>
#include <condition_variable>
#include "src/thrift/gen-cpp/defs_types.h"

namespace Ui { class QOperationLogWgt; };
class OperationLogModel;
class QUtilitySortFilterProxyModel;

class QOperationLogWgt : public QWidget
{
	Q_OBJECT

public:
	QOperationLogWgt(QWidget *parent = Q_NULLPTR);
	~QOperationLogWgt();
private:

	///
	///  @brief 初始化
	///
	void Init();
	
	///
	/// @brief 窗口显示事件
	///     
	/// @param[in]  event  事件对象
	///
	/// @par History:
	///  @li 7656/zhang.changjiang，2023年4月14日，新建函数
	///
	void showEvent(QShowEvent *event);

protected slots:

	///
	///  @brief 重置界面
	///
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年4月14日，新建函数
	///
	void OnReset();

	///
	/// @brief
	///     查询按钮槽函数
	///
	/// @par History:
	///  @li 7656/zhang.changjiang，2023年4月14日，新建函数
	///
	void OnQueryBtnClicked();

	///
	/// @brief	查询指定的操作日志
	/// @par History:
	///  @li 7656/zhang.changjiang，2023年4月14日，新建函数
	///
	void OperationLogQuery();
private:
	Ui::QOperationLogWgt *ui;
	bool										 m_bInit;					// 初始化标志
	std::map<::tf::OperationType::type, QString> m_operationTypeMap;		// 操作类型映射表
	QUtilitySortFilterProxyModel*		         m_pModel;				    // 排序代理模型
	std::map<int, ::tf::AssayClassify::type>     m_devClassfiyMap;			// 系统下拉列表索引于设备分类的映射
};
