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
/// @file     MaintainGroupCfgModel.cpp
/// @brief 	  维护项目组配置模型
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
#include "MaintainGroupCfgModel.h"
#include "MaintainGroupModel.h"
#include "shared/uicommon.h"
#include "../MaintainDefaultSetDlg.h"
#include "manager/UserInfoManager.h"
#include "src/common/Mlog/mlog.h"
#include <QMimeData>

MaintainGroupCfgModel::MaintainGroupCfgModel(QObject *parent)
    : QAbstractTableModel(parent)
	, m_headerNames{tr(" ")}
{
}

MaintainGroupCfgModel::~MaintainGroupCfgModel()
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
MaintainGroupCfgModel & MaintainGroupCfgModel::Instance()
{
	static MaintainGroupCfgModel model;
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
void MaintainGroupCfgModel::SetData(const std::vector<tf::MaintainItem>& data)
{
	beginResetModel();
	m_showData = MapDataToShowData(data);
	endResetModel();
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
std::vector<tf::MaintainItem> MaintainGroupCfgModel::GetData()
{
	return MapShowDataToData(m_showData);
}

///
/// @brief  获取选中的维护项目
///
/// @param[in]  iRow  行号
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2024年2月1日，新建函数
///
::tf::MaintainItem MaintainGroupCfgModel::GetSelectedItemByRow(const int iRow)
{
	if (iRow < 0 || iRow >= m_showData.size())
	{
		return ::tf::MaintainItem();
	}
	return m_showData[iRow].item;
}

///
///  @brief 将from位置的元素移动到to位置
///
///
///  @param[in]   from  移动的初始位置
///  @param[in]   to	移动的目标位置
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年2月14日，新建函数
///
void MaintainGroupCfgModel::MoveItem(int from, int to)
{
	beginResetModel();
	Move(m_showData, from, to);
	endResetModel();
}

///
///  @brief 复制一个维护项到指定inde
///
///
///  @param[in]   index  
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年3月7日，新建函数
///
void MaintainGroupCfgModel::CopyItemByIndex(const QModelIndex & index)
{
	if (MaintainGroupModel::Instance().IsSelectedGroupSingleType())
	{
		return;
    }
    if (index.row() < m_showData.size() &&
        !UserInfoManager::GetInstance()->IsPermissonMaint(m_showData[index.row()].item.itemType))
    {
        ULOG(LOG_WARN, "Not permit to copy.");
        return;
    }
	beginResetModel();
	m_showData.insert(m_showData.begin() + index.row(), m_showData[index.row()]);
	endResetModel();
}

///
///  @brief 删除指定index的维护项
///
///
///  @param[in]   index  
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年3月7日，新建函数
///
void MaintainGroupCfgModel::DelItemByIndex(const QModelIndex & index)
{
	if (MaintainGroupModel::Instance().IsSelectedGroupSingleType())
	{
		return;
    }
    if (index.row() < m_showData.size() &&
        !UserInfoManager::GetInstance()->IsPermissonMaint(m_showData[index.row()].item.itemType))
    {
        ULOG(LOG_WARN, "Not permit to delete.");
        return;
    }
	beginResetModel();
	m_showData.erase(m_showData.begin() + index.row());
	endResetModel();
}

///
///  @brief 获取指定index的维护项
///
///  @param[in]   index  需要获取该维护项的索引
///  @param[in]   itemData  用于返回的维护项信息
///
///  @return	true:获取成功
///
///  @par History: 
///  @li 8580/GongZhiQiang，2023年6月27日，新建函数
///
bool MaintainGroupCfgModel::GetItemByIndex(const QModelIndex &index, MaintainGroupCfgModel::ItemData& itemData)
{
	if (MaintainGroupModel::Instance().IsSelectedGroupSingleType())
	{
		return false;
	}

	if (m_showData.size() - 1 < index.row() || index.row() < 0)
	{
		return false;
	}

	itemData = m_showData[index.row()];

	return true;
}

///
///  @brief 修改指定index的维护项
///
///  @param[in]   index  需要获取该维护项的索引
///  @param[in]   itemData  修改的维护项信息
///
///  @return	true:修改成功
///
///  @par History: 
///  @li 8580/GongZhiQiang，2023年6月27日，新建函数
///
bool MaintainGroupCfgModel::ModifyItemByIndex(const QModelIndex &index, const MaintainGroupCfgModel::ItemData& itemData)
{
	if (MaintainGroupModel::Instance().IsSelectedGroupSingleType())
	{
		return false;
	}

	if (m_showData.size() - 1 < index.row() || index.row() < 0)
	{
		return false;
	}

	beginResetModel();
	m_showData[index.row()] = itemData;
	endResetModel();

	return true;
}

///
///  @brief 全部清除
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年3月7日，新建函数
///
void MaintainGroupCfgModel::ClearData()
{
	if (MaintainGroupModel::Instance().IsSelectedGroupSingleType())
	{
		return;
	}
	beginResetModel();
	m_showData.clear();
	endResetModel();
}

///
///  @brief	配置数据
///
///
///  @param[in]   iCheckState  选中状态
///  @param[in]   item		   配置的维护项
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年9月7日，新建函数
///
void MaintainGroupCfgModel::ConfigData(bool isChecked, const tf::MaintainItem &item)
{
	if (MaintainGroupModel::Instance().IsSelectedGroupSingleType())
	{
		return;
	}
	// 添加
	if (isChecked)
	{
		beginResetModel();
		ItemData itemData;
		itemData.item = item;
		m_showData.push_back(itemData);
		endResetModel();
	}
	// 删除
	else
	{
		// 删除对应的所有维护项类型
		beginResetModel();
		m_showData.erase(std::remove_if(m_showData.begin(), m_showData.end(), [&](const ItemData & iter_item)
		{
			return item.itemType == iter_item.item.itemType;
		}), m_showData.end());
		endResetModel();
	}
}

int MaintainGroupCfgModel::rowCount(const QModelIndex &parent) const
{
	return m_showData.size();
}

int MaintainGroupCfgModel::columnCount(const QModelIndex &parent) const
{
	return m_headerNames.size();
}

Qt::ItemFlags MaintainGroupCfgModel::flags(const QModelIndex & index) const
{
	if (index.isValid())
	{
        if (index.row() < m_showData.size() &&
            !UserInfoManager::GetInstance()->IsPermissonMaint(m_showData[index.row()].item.itemType))
        {
            return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | QAbstractTableModel::flags(index) & ~Qt::ItemIsEnabled;
        }
		return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | QAbstractTableModel::flags(index);
	}
	return QAbstractTableModel::flags(index);
}

bool MaintainGroupCfgModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
	QAbstractItemModel::setData(index, value, role);
	return true;
}

QVariant MaintainGroupCfgModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
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

Qt::DropActions MaintainGroupCfgModel::supportedDropActions() const
{
	return Qt::MoveAction | QAbstractTableModel::supportedDropActions();
}

QMimeData * MaintainGroupCfgModel::mimeData(const QModelIndexList & indexes) const
{
	QMimeData *data = QAbstractTableModel::mimeData(indexes);
	if (data) {
		// parent mimeData中已判断indexes有效性，无效的会返回nullptr
		// 也可以把信息放到model的mutable成员中
		data->setData("row", QByteArray::number(indexes.at(0).row()));
		data->setData("col", QByteArray::number(indexes.at(0).column()));
	}
	return data;
}

bool MaintainGroupCfgModel::dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent)
{
	if (!data || action != Qt::MoveAction)
	{
		return false;
	}
	const QModelIndex old_index = index(data->data("row").toInt(), data->data("col").toInt());
	const QModelIndex current_index = parent;
	Move(m_showData, old_index.row(), current_index.row());

	// 同步更新维护组表格的维护项顺序
	std::vector<tf::MaintainItem> items;
	for (auto & it : m_showData)
	{
		items.push_back(it.item);
	}
	MaintainGroupModel::Instance().SetSelectedGroup(std::move(items));
	return true;
}

///
///  @brief 将from位置的元素移动到to位置
///
///
///  @param[in]   array  数组
///  @param[in]   from	 移动的初始位置
///  @param[in]   to	 移动的目标位置
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年10月13日，新建函数
///
void MaintainGroupCfgModel::Move(std::vector<ItemData>& array, int from, int to)
{
	if (array.empty() || from < 0 || from >= array.size() || to < 0 || to >= array.size() || to == from)
	{
		return;
	}
	ItemData target = array[from];
	int increment = to < from ? -1 : 1;
	for (int i = from; i != to; i += increment)
	{
		array[i] = array[i + increment];
	}
	array[to] = target;
}

QVariant MaintainGroupCfgModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
	{
		return QVariant();
	}

	if (index.row() >= m_showData.size() || index.column() >= m_headerNames.size())
	{
		return QVariant();
	}

	if (role == Qt::TextAlignmentRole)
	{
		return QVariant(Qt::AlignCenter);
	}

	if (role != Qt::DisplayRole)
	{
		return QVariant();
	}

	switch (index.column())
	{
	case 0:
		return GetShowStr(m_showData[index.row()]);
	default:
		return QVariant();
		break;
	}
}

///
///  @brief 将维护项列表映射为显示数据列表
///
///
///  @param[in]   data  维护项列表
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年8月15日，新建函数
///
std::vector<MaintainGroupCfgModel::ItemData> MaintainGroupCfgModel::MapDataToShowData(const std::vector<tf::MaintainItem>& data)
{
	std::vector<MaintainGroupCfgModel::ItemData> showDataVec;
	for (auto &it : data)
	{
		ItemData showData;
		showData.item = it;
		showDataVec.push_back(showData);
	}
	return showDataVec;
}

///
///  @brief 将显示数据列表映射为维护项列表
///
///
///  @param[in]   shwoData  显示数据列表
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年8月15日，新建函数
///
std::vector<tf::MaintainItem> MaintainGroupCfgModel::MapShowDataToData(const std::vector<MaintainGroupCfgModel::ItemData>& showData)
{
	std::vector<tf::MaintainItem> data;
	for (auto &it : showData)
	{
		data.push_back(it.item);
	}

	return data;
}

///
///  @brief 修改制定index的维护项重复次数
///
///
///  @param[in]   index  需要获取该维护项的索引
///  @param[in]   repetition  重复次数
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年8月15日，新建函数
///
bool MaintainGroupCfgModel::ModifyItemByIndex(const QModelIndex & index, int repetition)
{
	if (!index.isValid())
	{
		return false;
	}

	if (repetition <= 0)
	{
		return false;
	}

	if (index.row() < 0 || index.row() >= m_showData.size())
	{
		return false;
	}

	m_showData[index.row()].repetition = repetition;
	return true;
}

///
///  @brief 获取显示字符串
///
///
///  @param[in]   itemData  显示数据
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年8月15日，新建函数
///
QString MaintainGroupCfgModel::GetShowStr(const MaintainGroupCfgModel::ItemData & itemData) const
{
	QString itemName = ConvertTfEnumToQString(itemData.item.itemType);
	return itemName;
}

