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
/// @file     ConsumableChangeLogModel.h
/// @brief 	  试剂/耗材更换日志列表模型（应用->日志->试剂/耗材更换日志）
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
#include "ConsumableChangeLogModel.h"
#include "shared/CommonInformationManager.h"
#include "shared/ThriftEnumTransform.h"
#include "shared/datetimefmttool.h"
#include "shared/CReadOnlyDelegate.h"

ConsumableChangeLogModel::ConsumableChangeLogModel(QObject *parent)
	: QAbstractTableModel(parent),
	m_headerNames{
	tr("序号")
	, tr("系统")
	, tr("模块")
	, tr("名称")
	, tr("操作类型") 
	, tr("状态")
	, tr("用户名")
	, tr("批号")
	, tr("瓶号/序列号")
	, tr("更换时间") 
	, tr("ID") }
	, m_sortModel(nullptr)
{
	auto cmPtr = CommonInformationManager::GetInstance();
	if (cmPtr != nullptr)
	{
		const auto& devInfos = cmPtr->GetDeviceMaps();
		for (auto dev : devInfos)
		{
			m_devNameMap[dev.second->deviceSN] = dev.second->groupName + dev.second->name;
		}
	}
}

ConsumableChangeLogModel::~ConsumableChangeLogModel()
{
}

ConsumableChangeLogModel & ConsumableChangeLogModel::Instance()
{
	static ConsumableChangeLogModel model(nullptr);
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
void ConsumableChangeLogModel::SetData(std::vector<::tf::ConsumableChangeLog>& data)
{
	beginResetModel();
	m_data= data;
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
void ConsumableChangeLogModel::SetCurQueryCond(const::tf::ConsumableChangeLogQueryCond & curQryCond)
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
const ::tf::ConsumableChangeLogQueryCond & ConsumableChangeLogModel::GetCurQueryCond()
{
	std::lock_guard<std::mutex> lock(m_qryMutex);
	return m_curQryCond;
}

///
///  @brief 获取动作显示字符串
///
///
///  @param[in]   changeAction  动作类型
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年4月23日，新建函数
///
QString ConsumableChangeLogModel::GetChangeAction(const::tf::ChangeAction::type changeAction) const
{
	switch (changeAction)
	{
	case ::tf::ChangeAction::type::LOAD:
		return QString(tr("装载"));
	case ::tf::ChangeAction::type::UNLOAD:
		return QString(tr("卸载"));
	default:
		return QString(tr("未知"));
	}
}

///
///  @brief 获取状态显示字符串
///
///
///  @param[in]   changeStatus  状态类型
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年4月23日，新建函数
///
QString ConsumableChangeLogModel::GetChangeStatus(const::tf::ChangeStatus::type changeStatus) const
{
	switch (changeStatus)
	{
	case ::tf::ChangeStatus::type::SUCCESS:
		return QString(tr("成功"));
	case ::tf::ChangeStatus::type::FAILED:
		return QString(tr("失败"));
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
const std::vector<::tf::ConsumableChangeLog>& ConsumableChangeLogModel::GetData()
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
void ConsumableChangeLogModel::AppendData(::tf::ConsumableChangeLog & data)
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
void ConsumableChangeLogModel::ClearData()
{
	// 更新数据
	beginResetModel();
	m_data.clear();
	endResetModel();
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
void ConsumableChangeLogModel::SetSortModel(QUtilitySortFilterProxyModel * sortModel)
{
	m_sortModel = sortModel;
}

QString ConsumableChangeLogModel::GetSuppliesName(const tf::AssayClassify::type &deviceClassify, const std::string &suppliesName) const
{
	if (suppliesName.empty())
	{
		return "";
	}

	switch (deviceClassify)
	{
	case tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY:
	case tf::AssayClassify::ASSAY_CLASSIFY_ISE:
	{
		int supplyCode = 0;
		try
		{
			supplyCode = std::atoi(suppliesName.c_str());
		}
		catch (const std::exception&)
		{
			return QString(suppliesName.c_str());
		}

		// 生化
		if (deviceClassify == tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY)
		{
			if (supplyCode > 0 && supplyCode <= ::ch::tf::g_ch_constants.MAX_REAGENT_CODE)
			{
				// 试剂
				auto spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(supplyCode);
				if (spAssay == nullptr)
				{
					return QString(suppliesName.c_str());
				}
				return QString(spAssay->assayName.c_str());
			}
			else
			{
				// 仓内耗材
				return ThriftEnumTrans::GetSupplyName(ch::tf::SuppliesType::type(supplyCode), true);
			}
		}
		// ise
		else
		{
			return ThriftEnumTrans::GetSupplyName(ise::tf::SuppliesType::type(supplyCode));
		}
	}
	default:
		return QString(suppliesName.c_str());
	}
}

int ConsumableChangeLogModel::rowCount(const QModelIndex & parent) const
{
	return m_data.size();
}

int ConsumableChangeLogModel::columnCount(const QModelIndex & parent) const
{
	return m_headerNames.size();
}

QVariant ConsumableChangeLogModel::data(const QModelIndex & index, int role) const
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
	case (int)CONSUMABLE_CHANGE_LOG_LIST_COL::INDEX:
		return m_sortModel->mapFromSource(index).row() + 1;

		// 系统
	case (int)CONSUMABLE_CHANGE_LOG_LIST_COL::SYSTEM:
		return ThriftEnumTrans::GetAssayClassfiyName(rowData.deviceClassify, tr("其他"));

		// 模块
	case (int)CONSUMABLE_CHANGE_LOG_LIST_COL::MODULE:
		return m_devNameMap.count(rowData.deviceSN) > 0 ? QString(m_devNameMap.at(rowData.deviceSN).c_str()) : tr("");

		// 名称
	case (int)CONSUMABLE_CHANGE_LOG_LIST_COL::NAME:
		return QString::fromStdString(rowData.consumablName);

		// 操作类型
	case (int)CONSUMABLE_CHANGE_LOG_LIST_COL::ACTION:
		return GetChangeAction(rowData.changeAction);

		// 状态
	case (int)CONSUMABLE_CHANGE_LOG_LIST_COL::STATUS:
		return GetChangeStatus(rowData.changeStatus);
		
		// 用户名
	case (int)CONSUMABLE_CHANGE_LOG_LIST_COL::USER:
		return QString(rowData.user.c_str());

		// 批号
	case (int)CONSUMABLE_CHANGE_LOG_LIST_COL::BATCHNUM:
		return QString(tr("%1")).arg(rowData.batchNum.c_str());

		// 瓶号/序列号
	case (int)CONSUMABLE_CHANGE_LOG_LIST_COL::BOTTLENUM:
		return QString(tr("%1")).arg(rowData.bottleNum.c_str());

		// 更换时间
	case (int)CONSUMABLE_CHANGE_LOG_LIST_COL::TIME:
		return ToCfgFmtDateTime(QString::fromStdString(rowData.changeTime));

		// 更换时间
	case (int)CONSUMABLE_CHANGE_LOG_LIST_COL::ID:
		return static_cast<qlonglong>(rowData.id);

	default:
		break;
	}

	return null;
}

QVariant ConsumableChangeLogModel::headerData(int section, Qt::Orientation orientation, int role) const
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
