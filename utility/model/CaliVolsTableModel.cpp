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

#include <QBrush>
#include "shared/uidef.h"
#include "shared/tipdlg.h"
#include "CaliVolsTableModel.h"

CaliVolsTableModel::CaliVolsTableModel(QObject *parent)
	: QAbstractTableModel(parent)
	, m_headerNames{ tr("校准品")
		, tr("样本量")
		, tr("稀释样本量")
		, tr("稀释液量")
        , tr("混匀等级")}
{
}

CaliVolsTableModel::~CaliVolsTableModel()
{
}

CaliVolsTableModel& CaliVolsTableModel::Instance()
{
	static CaliVolsTableModel model(nullptr);
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
void CaliVolsTableModel::SetData(const std::vector<ch::tf::SampleAspirateVol>& data)
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
const std::vector<ch::tf::SampleAspirateVol>& CaliVolsTableModel::GetData()
{
	return m_data;
}

int CaliVolsTableModel::rowCount(const QModelIndex &parent) const
{
	return m_data.size();
}

int CaliVolsTableModel::columnCount(const QModelIndex &parent) const
{
	return m_headerNames.size();
}

QVariant CaliVolsTableModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
	if (orientation == Qt::Horizontal) 
    {
		if (role == Qt::DisplayRole) {
			return m_headerNames.at(section);
		}
	}

    if ((section > 0) && (role == Qt::ForegroundRole) && (orientation == Qt::Horizontal))
    {
        return QColor(UI_HEADER_FONT_COLOR);
    }

	return QAbstractTableModel::headerData(section, orientation, role);
}

QVariant CaliVolsTableModel::data(const QModelIndex &index, int role) const
{
	const QVariant null;

    if (role == Qt::TextAlignmentRole)
    {
        return Qt::AlignCenter;
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
	// 校准品
	case (int)COL::INDEX:
		return tr("校准品") + QString::number(rowIdx+1);
	// 样本量
	case (int)COL::SAMPLE: {
			return rowData.originalSample > 0 ? rowData.originalSample / 10.0 : QVariant();
		break;
	}
	// 稀释样本量
	case (int)COL::SAMPLE4TEST: {
			return rowData.sample4Test > 0 ? rowData.sample4Test / 10.0 : QVariant();
		break;
	}
	// 稀释液量
	case (int)COL::DILUENT: {
			return rowData.diluent > 0 ? rowData.diluent / 10.0 : QVariant();
		break;
	}
    // 稀释液量
    case (int)COL::LEVEL: {
            return QString::number(rowData.mixingLevel);
            break;
        }
	default:
		break;
	}

	return null;
}

///
/// @brief
///     设置单元格数据 供视图调用
///
/// @param[in]    index      当前单元格索引
/// @param[in]    value      单元格的值
/// @param[in]    role       被修改的角色类型
///
/// @return       bool       数据处理成功返回true
///
/// @par History:
/// @li 6950/ChenFei，2022年5月24日，新建函数
///
bool CaliVolsTableModel::setData(const QModelIndex &index, const QVariant &value, int role /*= Qt::EditRole*/)
{
	if (role != Qt::EditRole)
	{
		return false;
	}

	// 获取编辑的行和列
	const int col = index.column();
	const int row = index.row();

	if (row >= m_data.size()
		|| col > m_headerNames.size())
	{
		return false;
	}

	// 改动数据
    QRegExp re("^([0-9]{1,3}(\\.[0-9]{0,1})?)?$");
    switch ((COL)col)
    {
        case COL::SAMPLE:
            {
                int32_t val = value.toDouble() * 10;
                if (!re.exactMatch(value.toString()) || val < SAMPLE_PROBE_MIN_VOL || val > SAMPLE_PROBE_MAX_VOL)
                {
                    TipDlg(tr("提示"), tr("值错误：1≤样本量≤25，最多可输入1位小数")).exec();
                    return false;
                }

                m_data.at(row).__set_originalSample(val);
                break;
            }

        case COL::SAMPLE4TEST: 
            {
                int32_t val = value.toDouble() * 10;
                if (!re.exactMatch(value.toString()) || (val != 0 && (val < SAMPLE_PROBE_MIN_VOL || val > SAMPLE_PROBE_MAX_VOL)))
                {
                    TipDlg(tr("提示"), tr("值错误：1≤稀释样本量≤25，最多可输入1位小数")).exec();
                    return false;
                }

                m_data.at(row).__set_sample4Test(val);
                break;
            }

        case COL::DILUENT: 
            {              
                int32_t val = value.toDouble() * 10;

                if (!value.toString().isEmpty()
                    && ((val % 5) != 0 || (val < REAGENT_PROBE_MIN_VOL || val > REAGENT_REACTION_MAX_VOL)))
                {
                    TipDlg(tr("提示"), tr("值错误：10≤稀释液量≤100，步进值0.5")).exec();
                    return false;
                }

                m_data.at(row).__set_diluent(val);
                break;
            }

        case COL::LEVEL:
            {
                m_data.at(row).__set_mixingLevel(value.toInt());
                break;
            }

        default:
            return false;
            break;
    }

	return true;
}

///
/// @brief
///     获取单元格属性(可编辑、可选择)
///
/// @param[in]    index      当前单元格索引
///
/// @return       QVariant   包装的数据
///
/// @par History:
/// @li 6950/ChenFei，2021年8月10日，新建函数
///
Qt::ItemFlags CaliVolsTableModel::flags(const QModelIndex &index) const
{
	const int col = index.column();
	const int row = index.row();

	const Qt::ItemFlags default_flag = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

	if (col >= static_cast<int>(COL::SAMPLE)
		&& (col <= static_cast<int>(COL::LEVEL)))
	{
		return default_flag | Qt::ItemIsEditable;
	}

	return default_flag;
}
