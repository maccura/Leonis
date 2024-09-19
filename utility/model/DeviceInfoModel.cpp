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
/// @file     DeviceInfoModel.cpp
/// @brief 	  维护项目组模型
///
/// @author   7656/zhang.changjiang
/// @date      2022年9月2日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2022年9月2日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "AssayListModel.h"
#include "DeviceInfoModel.h"

DeviceInfoModel::DeviceInfoModel(QObject *parent)
	: QAbstractTableModel(parent)
	, m_headerNames{ 
	  tr("序号")
	, tr("设备") }
{
}

DeviceInfoModel::~DeviceInfoModel()
{
}

///
///  @brief 获取单例
///
///
///
///  @return	单例对象
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年9月16日，新建函数
///
DeviceInfoModel & DeviceInfoModel::Instance()
{
	static DeviceInfoModel model;
	return model;
}

///
///  @brief	设置维护项目数据
///
///
///  @param[in]   data  
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年9月1日，新建函数
///
void DeviceInfoModel::SetData(std::vector<tf::DeviceInfo>& data)
{
	beginResetModel();
	m_data.clear();
	for (auto it : data)
	{
		DeviceInfoData devData;
		devData.dev = it;
		m_data.push_back(std::move(devData));
	}
	endResetModel();
}

///
///  @brief 设置维护组选中状态(多选)
///
///
///  @param[in]   row          维护组所在行
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年10月12日，新建函数
///
void DeviceInfoModel::SetCheckedState(int row)
{
    beginResetModel();
    if (row < 0 || row >= m_data.size() || m_data.empty())
    {
        return;
    }

    if (m_data[row].iCheckState == Qt::Checked)
    {
        m_data[row].iCheckState = Qt::Unchecked;
    }
    else
    {
        m_data[row].iCheckState = Qt::Checked;
    }
    
    endResetModel();
}

///
///  @brief 获取选中的设备
///
///
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年9月9日，新建函数
///
std::vector<std::string> DeviceInfoModel::GetSelectDevSN()
{
	std::vector<std::string> devs;
    if (m_data.empty())
    {
        return devs;
    }

	for (auto it : m_data)
	{
		if (it.iCheckState == Qt::Checked)
		{
			devs.push_back(it.dev.deviceSN);
		}
	}
	return devs;
}

///
///  @brief 获取选中的设备
///
///
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年9月9日，新建函数
///
std::vector<tf::DeviceInfo> DeviceInfoModel::GetSelectDevs()
{
	std::vector<tf::DeviceInfo> devs;
    if (m_data.empty())
    {
        return devs;
    }
	for (auto it : m_data)
	{
		if (it.iCheckState == Qt::Checked)
		{
			devs.push_back(it.dev);
		}
	}
	return devs;
}

int DeviceInfoModel::rowCount(const QModelIndex &parent) const
{
	return m_data.size();
}

int DeviceInfoModel::columnCount(const QModelIndex &parent) const
{
	return m_headerNames.size();
}

Qt::ItemFlags DeviceInfoModel::flags(const QModelIndex & index) const
{
    if (!index.isValid())
    {
        return QAbstractTableModel::flags(index);
    }
	if (index.column() == 0)
	{
		return QAbstractTableModel::flags(index) | Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
	}
	if (index.column() != 0)
	{
		return QAbstractTableModel::flags(index) | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
	}
	return QAbstractTableModel::flags(index);
}

bool DeviceInfoModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
	if (!index.isValid())
	{
		return false;
	}
	if (role == Qt::CheckStateRole && index.column() == 0)
	{
		if (value == Qt::Checked)
		{
			m_data[index.row()].iCheckState = Qt::Checked;
		}
		else
		{
			m_data[index.row()].iCheckState = Qt::Unchecked;
		}
		emit CheckStateChanged(m_data[index.row()].iCheckState, m_data[index.row()].dev);
	}
	return true;
}

QVariant DeviceInfoModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		return m_headerNames.at(section);
	}
	else
	{
		return QAbstractTableModel::headerData(section, orientation, role);
	}
}

QVariant DeviceInfoModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
	{
		return QVariant();
	}

	if (index.row() > m_data.size() || index.column() > m_headerNames.size())
	{
		return QVariant();
	}

	if (role == Qt::TextAlignmentRole)
	{
		return QVariant(Qt::AlignCenter);
	}

	if (role == Qt::CheckStateRole)
	{
		if (index.column() != 0)
		{
			return QVariant();
		}
		return m_data.at(index.row()).iCheckState;
	}

	if (role != Qt::DisplayRole)
	{
		return QVariant();
	}

	switch (index.column())
	{
	case MAINTAIN_GROUP_COL::INDEX:
		return index.row() + 1;
		break;
	case MAINTAIN_GROUP_COL::NAME:
		return QString(m_data.at(index.row()).dev.name.c_str());
		break;
	default:
		return QVariant();
		break;
	}
}

