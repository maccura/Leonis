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

#include "AssayListModel.h"
#include <QTableView>
#include "shared/CReadOnlyDelegate.h"


AssayListModel::AssayListModel(QTableView* pView, QObject *parent)
	: QAbstractTableModel(parent)
	, m_headerNames{ tr("序号"), tr("项目名称") }
    , m_tbView(pView)
    , m_sortModel(nullptr)
{
}

AssayListModel::~AssayListModel()
{
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
void AssayListModel::SetData(const std::vector<StAssayListRowItem>& data)
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
const std::vector<AssayListModel::StAssayListRowItem>& AssayListModel::GetData()
{
	return m_data;
}

void AssayListModel::SetRowData(int row, const StAssayListRowItem& rowData)
{
    if (row < 0 || row >= m_data.size())
    {
        return;
    }

    m_data[row] = rowData;
}

int AssayListModel::GetRowAssayCode(int row)
{
    if (row < 0 || row >= m_data.size())
    {
        return INVALID_ASSAY_CODE;
    }

    return m_data[row].assayCode;
}

void AssayListModel::SetCurrentRowItemSampleSourceType(int row, int SampleSourceType)
{
    if (row < 0 || row >= m_data.size())
    {
        return;
    }

    m_data[row].sampleType = SampleSourceType;
}

int AssayListModel::rowCount(const QModelIndex &parent) const
{
	return m_data.size();	
}

int AssayListModel::columnCount(const QModelIndex &parent) const
{
	return m_headerNames.size();
}

QVariant AssayListModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
	if (orientation == Qt::Horizontal) {
		if (role == Qt::DisplayRole) {
			return m_headerNames.at(section);
		}
	}
	return QAbstractTableModel::headerData(section, orientation, role);
}

QVariant AssayListModel::data(const QModelIndex &index, int role) const
{
    // 表格内容居中
    if (role == Qt::TextAlignmentRole)
    {
        return Qt::AlignCenter;
    }

	const QVariant null;
	if (role == Qt::DisplayRole)
    {
        // 获取当前显示列
        const int rowIdx = index.row();
        if (rowIdx >= m_data.size())
        {
            return null;
        }
        const auto& rowData = m_data.at(rowIdx);
        switch (index.column())
        {
            // 行号
        case (int)ASSAY_LIST_COL::INDEX:
            if (m_sortModel != nullptr)
            {
                return m_sortModel->mapFromSource(index).row() + 1;
            }
            break;
            // 项目名
        case (int)ASSAY_LIST_COL::NAME:
            return rowData.name;
        default:
            break;
        }
	}

    // 显示不全的提示框
    if (role == Qt::ToolTipRole && index.column() == (int)ASSAY_LIST_COL::NAME)
    {
        QVariant qVri = index.data();
        if (qVri.isValid())
        {
            QFontMetrics fm(m_tbView->fontMetrics());
            int iCharc = fm.width(qVri.toString());
            int iColum = m_tbView->columnWidth(index.column());
            if (iCharc + 19 > iColum) // 19：字体外的边框冗余
            {
                return qVri.toString();
            }
        }
    }

	return null;
}
