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

#include "FaultLogTableModel.h"
#include "shared/uidef.h"
#include "shared/uicommon.h"
#include "shared/CReadOnlyDelegate.h"
#include <QBrush>

#define REAGENT_ALARM_CODE                  "99-21-"        // 试剂耗材类报警码前缀
#define DELEGATE_COLOR_OFFSET_POS           5               // 只读单元格代理中，颜色数据的存储位置相对于Qt::UserRole的偏移

FaultLogTableModel::FaultLogTableModel(QObject *parent)
	: QAbstractTableModel(parent)
	, m_headerNames{ tr("序号")
	, tr("系统")
	, tr("模块")
	, tr("报警代码")
	, tr("报警级别")
	, tr("报警名称") 
	, tr("报警时间")}
    , m_sortModel(nullptr)
{
}

FaultLogTableModel::~FaultLogTableModel()
{
}

FaultLogTableModel& FaultLogTableModel::Instance()
{
	static FaultLogTableModel model(nullptr);
	return model;
}

///
/// @brief
///     设置数据到模型
///
///@param[in]    data 显示数据
///
/// @par History:
/// @li 6950/ChenFei，2022年05月16日，新建函数
///
void FaultLogTableModel::SetData(const std::vector<StFaultLogItem>& data)
{
	beginResetModel();
	m_data = data;
	endResetModel();
}

///
/// @brief
///     获取数据
///
/// @return 被显示的数据
///
/// @par History:
/// @li 6950/ChenFei，2022年05月16日，新建函数
///
const std::vector<FaultLogTableModel::StFaultLogItem>& FaultLogTableModel::GetData()
{
	return m_data;
}

void FaultLogTableModel::AppendData(StFaultLogItem& data)
{
	// 更新数据
    int start = m_data.size();
    beginInsertRows(index(start, 0).parent(), start, start);
	m_data.push_back(std::move(data));
    endInsertRows();
}

bool FaultLogTableModel::GetRowData(int row, StFaultLogItem& rowData)
{
    if (m_data.size() > row)
    {
        rowData = m_data[row];
        return true;
    }

    return false;
}

void FaultLogTableModel::RemoveAllRows()
{
    // 更新数据
    beginResetModel();
    m_data.clear();
    endResetModel();
}

int FaultLogTableModel::rowCount(const QModelIndex &parent) const
{
	return m_data.size();
}

int FaultLogTableModel::columnCount(const QModelIndex &parent) const
{
	return m_headerNames.size();
}

QVariant FaultLogTableModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
	if (orientation == Qt::Horizontal) {
		if (role == Qt::DisplayRole) {
			return m_headerNames.at(section);
		}
	}
	return QAbstractTableModel::headerData(section, orientation, role);
}

QVariant FaultLogTableModel::data(const QModelIndex &index, int role) const
{
	const QVariant null;

    if (role == Qt::TextAlignmentRole)
    {
        return Qt::AlignCenter;
    }

    // 获取当前显示列
    const int colIdx = index.column();
    const int rowIdx = index.row();

    if (rowIdx >= m_data.size())
    {
        return null;
    }
    const auto& rowData = m_data.at(rowIdx);

    QString codeFlag = QString::number(REAGENT_ALARM_MAINCODE) + "-" + QString::number(REAGENT_ALARM_MIDCODE);

    // 字体颜色
    if (role == Qt::TextColorRole && colIdx == LEVEL)
    {
        if (rowData.level == ConvertTfEnumToQString(::tf::AlarmLevel::ALARM_LEVEL_TYPE_STOP))
        {
            // 停机显示红色字体
            return QBrush(UI_REAGENT_WARNFONT);
        }
        else if (rowData.level == ConvertTfEnumToQString(::tf::AlarmLevel::ALARM_LEVEL_TYPE_STOPSAMPLING)
                    || rowData.code.startsWith(codeFlag))
        {
            // 加样停显示黄色字体
            return QBrush(UI_ALARM_YELLOW_WARNFONT);
        }
    }

    // 背景色
    if (role == Qt::UserRole + DELEGATE_COLOR_OFFSET_POS && colIdx == LEVEL)
    {
        if (rowData.level == ConvertTfEnumToQString(::tf::AlarmLevel::ALARM_LEVEL_TYPE_STOP))
        {
            // 停机显示红色背景
            return UI_REAGENT_WARNCOLOR;
        }
        else if (rowData.level == ConvertTfEnumToQString(::tf::AlarmLevel::ALARM_LEVEL_TYPE_STOPSAMPLING)
                    || rowData.code.startsWith(codeFlag))
        {
            // 加样停显示黄色背景
            return UI_ALARM_YELLOW_WARNCOLOR;
        }
    }

	// 只处理显示角色
	if (role != Qt::DisplayRole)
	{
		return null;
	}

	switch (colIdx)
	{
		// 行号
	case (int)FAULT_LOG_LIST_COL::INDEX:    
		return m_sortModel->mapFromSource(index).row() + 1;

		// 系统
	case (int)FAULT_LOG_LIST_COL::SYSTEM:
		return rowData.system;

		// 模块名
	case (int)FAULT_LOG_LIST_COL::MODULE:
		return rowData.model;

		// 报警码
	case (int)FAULT_LOG_LIST_COL::CODE:
		return rowData.code;

		// 报警名称
	case (int)FAULT_LOG_LIST_COL::NAME:
		return rowData.name;

		// 报警等级
	case (int)FAULT_LOG_LIST_COL::LEVEL:
		return rowData.level;

		// 报警时间
	case (int)FAULT_LOG_LIST_COL::TIME:
		return rowData.time;

	default:
		break;
	}

	return null;
}
