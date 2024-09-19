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
/// @file     OperationLogModel.h
/// @brief 	  操作日志列表模型（应用->日志->操作日志）
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
#include "OperationLogModel.h"
#include "shared/datetimefmttool.h"
#include "shared/CReadOnlyDelegate.h"

///
/// @brief 构造函数
///
/// @param[in]  parent  
///
/// @return 
///
/// @par History: 修改操作者为->用户名
/// @li 1556/Chenjianlin，2023年7月29日，新建函数
///
OperationLogModel::OperationLogModel(QObject *parent)
	: QAbstractTableModel(parent)
	, m_headerNames{ 
	  tr("序号")
	, tr("用户名")
	, tr("操作类型")
	, tr("操作记录")
	, tr("操作时间")
	, tr("ID")}
	, m_sortModel(nullptr)
{
}

OperationLogModel::~OperationLogModel()
{
}

///
///  @brief  获取单例
///
///
///
///  @return	单例对象
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年4月14日，新建函数
///
OperationLogModel & OperationLogModel::Instance()
{
	static OperationLogModel model(nullptr);
	return model;
}

///
///  @brief 设置显示数据
///
///
///  @param[in]   data  显示数据
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年4月14日，新建函数
///
void OperationLogModel::SetData(std::vector<::tf::OperationLog>& data)
{
	beginResetModel();
	m_data = data;
	endResetModel();
}

///
///  @brief 设置当前显示数据的查询条件
///
///
///  @param[in]   curQryCond  当前显示数据的查询条件
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年4月23日，新建函数
///
void OperationLogModel::SetCurQueryCond(const::tf::OperationLogQueryCond & curQryCond)
{
	std::lock_guard<std::mutex> lock(m_qryMutex);
	m_curQryCond = curQryCond;
}

///
///  @brief 获取当前显示数据的查询条件
///
///
///
///  @return	当前显示数据的查询条件
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年4月23日，新建函数
///
const ::tf::OperationLogQueryCond & OperationLogModel::GetCurQueryCond()
{
	std::lock_guard<std::mutex> lock(m_qryMutex);
	return m_curQryCond;
}

///
///  @brief 获取操作类型显示字符串
///
///
///  @param[in]   operationType  操作类型
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年4月23日，新建函数
///
QString OperationLogModel::GetOperationType(::tf::OperationType::type operationType) const
{
	switch (operationType)
	{
	case ::tf::OperationType::type::ADD:
		return QString(tr("新增"));
	case ::tf::OperationType::type::DEL:
		return QString(tr("删除"));
	case ::tf::OperationType::type::MOD:
		return QString(tr("修改"));
	default:
		return QString(tr("未知"));
	}
}

///
/// @brief
///     获取数据
///
/// @return 被显示的数据
///
/// @par History:
///  @li 7656/zhang.changjiang，2023年4月14日，新建函数
///
const std::vector<::tf::OperationLog>& OperationLogModel::GetData()
{
	return m_data;
}

///
/// @brief
///     追加显示数据到模型
///
/// @param[in]  data  
/// @par History:
///  @li 7656/zhang.changjiang，2023年4月14日，新建函数
///
void OperationLogModel::AppendData(::tf::OperationLog & data)
{
	// 更新数据
	beginResetModel();
	m_data.push_back(std::move(data));
	endResetModel();
}

///
///  @brief 清空数据
///
///
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年4月17日，新建函数
///
void OperationLogModel::ClearData()
{
	// 更新数据
	beginResetModel();
	m_data.clear();
	endResetModel();
}

int OperationLogModel::rowCount(const QModelIndex & parent) const
{
	return m_data.size();
}

int OperationLogModel::columnCount(const QModelIndex & parent) const
{
	return m_headerNames.size();
}

///
/// @brief  设置表格排序的model
///
/// @param[in]  sortModel  表格排序的model
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2024年3月13日，新建函数
///
void OperationLogModel::SetSortModel(QUtilitySortFilterProxyModel * sortModel)
{
	m_sortModel = sortModel;
}

QVariant OperationLogModel::data(const QModelIndex & index, int role) const
{
	const QVariant null;

	if (role == Qt::TextAlignmentRole)
	{
		return QVariant(Qt::AlignCenter);
	}

	// 只处理显示角色
	if (role != Qt::DisplayRole)
	{
		return null;
	}

	// 获取当前显示列
	const int colIdx = index.column();
	const int rowIdx = index.row();

	if (rowIdx >= m_data.size())
	{
		return null;
	}
	const auto& rowData = m_data.at(rowIdx);

	switch (colIdx)
	{
		// 行号
	case (int)OPERATION_LOG_LIST_COL::INDEX:
		return m_sortModel->mapFromSource(index).row() + 1;

		// 用户名
	case (int)OPERATION_LOG_LIST_COL::USER:
		return QString(rowData.user.c_str());

		// 操作类型
	case (int)OPERATION_LOG_LIST_COL::TYPE:
		return GetOperationType(rowData.operationType);

		// 操作记录
	case (int)OPERATION_LOG_LIST_COL::RECORD:
		return QString(rowData.operationRecord.c_str());

		// 操作时间
	case (int)OPERATION_LOG_LIST_COL::TIME:
		return ToCfgFmtDateTime(QString::fromStdString(rowData.operationTime));

		// 数据库主键（该列隐藏，用于倒序显示的排序操作）
	case (int)OPERATION_LOG_LIST_COL::ID:
		return static_cast<qlonglong>(rowData.id);

	default:
		break;
	}

	return null;
}

QVariant OperationLogModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal) 
	{
		if (role == Qt::DisplayRole) 
		{
			return m_headerNames.at(section);
		}
	}
	return QAbstractTableModel::headerData(section, orientation, role);
}
