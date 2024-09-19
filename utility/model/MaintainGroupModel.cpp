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
/// @file     MaintainGroupModel.cpp
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
//#include <unordered_set>
#include "shared/uicommon.h"
#include "MaintainGroupModel.h"
#include "thrift/DcsControlProxy.h"
#include "manager/UserInfoManager.h"
#include "utility/maintaindatamng.h"
#include "src/common/Mlog/mlog.h"

MaintainGroupModel::MaintainGroupModel(QObject *parent)
	: QAbstractTableModel(parent)
	, m_headerNames{ 
	  tr("序号")
	, tr("组合名称") }
	, m_iSelectedRow(0)
{
	// 获取DCS客户端
	std::shared_ptr<DcsControlProxy> spCilent = DcsControlProxy::GetInstance();
	if (spCilent == nullptr)
	{
		return;
	}
	// 查询维护组
	::tf::MaintainGroupQueryResp retGroup;
	::tf::MaintainGroupQueryCond miqcGroup;
	if (!spCilent->QueryMaintainGroup(retGroup, miqcGroup))
	{
        ULOG(LOG_WARN, "Failed to query all maintain group.");
		return;
	}

	// 过滤不支持的维护组
	MaintainDataMng::GetInstance()->FilterGroups(retGroup.lstMaintainGroups);

	m_data = retGroup.lstMaintainGroups;
}

MaintainGroupModel::~MaintainGroupModel()
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
MaintainGroupModel & MaintainGroupModel::Instance()
{
	static MaintainGroupModel model;
	return model;
}

///
///  @brief	设置维护项目组数据
///
///
///  @param[in]   data  
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年9月1日，新建函数
///
void MaintainGroupModel::SetData(std::vector<tf::MaintainGroup>& data)
{
	beginResetModel();
	m_data.clear();
	m_data = data;
	endResetModel();
}

void MaintainGroupModel::UpdateAutoMantains(const std::string& groupName, const std::vector<tf::AutoMaintainCfg>& autoCfg)
{
    for (auto& gItem : m_data)
    {
        if (GetGroupName(gItem).toStdString() == groupName)
        {
            // 周维护的设置时间如果存在，则不再变化，用以保持首次开机时间
            if (gItem.groupType == ::tf::MaintainGroupType::type::MAINTAIN_GROUP_WEEK)
            {
                UpdateGroupCfgWithNoSetTimeChanged(gItem, autoCfg);
            }
            else
            {
                // TODO:Compare and set.
                gItem.__set_autoMaintainCfgs(autoCfg);
            }

            break;
        }
    }
}

void MaintainGroupModel::UpdateGroupCfgWithNoSetTimeChanged(tf::MaintainGroup& mtGroup, const std::vector<tf::AutoMaintainCfg>& autoCfg)
{
    for (const auto& cfgItem : autoCfg)
    {
        bool existFlg = false;

        // 如果存在对应设备的维护配置，则更新除设置时间外的配置内容
        for (auto& orgItem : mtGroup.autoMaintainCfgs)
        {
            if (orgItem.deviceSN == cfgItem.deviceSN)
            {
                orgItem.__set_lstWeekdays(cfgItem.lstWeekdays);
                orgItem.__set_maintainTime(cfgItem.maintainTime);
                orgItem.__set_timeInterval(cfgItem.timeInterval);
                
                if (orgItem.intervalSetTime.empty())
                {
                    orgItem.__set_intervalSetTime(cfgItem.intervalSetTime);
                }
                existFlg = true;
                break;
            }
        }

        // 如果不存在，则新增并保留存在的配置的设置时间作为当前配置时间
        if (!existFlg)
        {
            if (mtGroup.autoMaintainCfgs.size() > 0 && !mtGroup.autoMaintainCfgs[0].intervalSetTime.empty())
            {
                tf::AutoMaintainCfg cfgtmp = cfgItem;
                cfgtmp.__set_intervalSetTime(mtGroup.autoMaintainCfgs[0].intervalSetTime);
                mtGroup.autoMaintainCfgs.push_back(cfgtmp);
            }
            else
            {
                mtGroup.autoMaintainCfgs.push_back(cfgItem);
            }
        }
    }
}

bool MaintainGroupModel::TakeMaintainGroup(const QString& groupName, tf::MaintainGroup& mGroup)
{
    for (const tf::MaintainGroup& gItem : m_data)
    {
        if (GetGroupName(gItem) == groupName)
        {
            mGroup = gItem;
            return true;
        }
    }

    return false;
}

///
///  @brief 清空选中的维护组维护项列表
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年2月17日，新建函数
///
::tf::MaintainGroupType::type MaintainGroupModel::ClerCurRowItemData()
{
	::tf::MaintainGroupType::type groupType = m_data[m_iSelectedRow].groupType;
	if (groupType != ::tf::MaintainGroupType::type::MAINTAIN_GROUP_SINGLE)
	{
		beginResetModel();
		m_data[m_iSelectedRow].items.clear();
		endResetModel();
	}
	return groupType;
}

///
///  @brief 设置选中的维护组包含的维护项
///
///
///  @param[in]   itemTypes  维护项类型列表
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年10月13日，新建函数
///
void MaintainGroupModel::SetSelectedGroup(const std::vector<tf::MaintainItem>& items)
{
	m_data[m_iSelectedRow].__set_items(items);
}

///
///  @brief	添加维护组
///
///
///  @param[in]   data  
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年9月7日，新建函数
///
void MaintainGroupModel::AddGroupData(tf::MaintainGroup & data)
{
	beginResetModel();
	m_data.push_back(data);
	endResetModel();
}

bool MaintainGroupModel::IsExistEmptyNameGroup()
{
    for (const tf::MaintainGroup& gIter : m_data)
    {
        if (gIter.groupName.empty())
        {
            return true;
        }
    }

    return false;
}

///
///  @brief	删除维护组
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年9月7日，新建函数
///
void MaintainGroupModel::DeleteSelectGroupData()
{
	beginResetModel();
	for (auto it = m_data.begin(); it != m_data.end(); ++it)
	{
		if (it->id == m_data[m_iSelectedRow].id)
		{
			m_data.erase(it);
			break;
		}
	}
	if (m_iSelectedRow != 0)
	{
		--m_iSelectedRow;
	}
	endResetModel();
}

///
///  @brief 删除指定的维护组
///
///
///  @param[in]   lstMg  维护组列表
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年9月6日，新建函数
///
void MaintainGroupModel::DeleteGroups(const std::vector<::tf::MaintainGroup>& lstMg)
{
	beginResetModel();

	for (auto it = m_data.begin(); it != m_data.end();)
	{
		bool finded = false;

		// 不能删除系统维护组
		if (it->groupType != ::tf::MaintainGroupType::type::MAINTAIN_GROUP_CUSTOM)
		{
			++it;
			continue;
		}

		for (auto& group : lstMg)
		{
			if ((!it->__isset.id && !group.__isset.id) || (it->id == group.id))
			{
				finded = true;
				break;
			}
		}
		if (finded)
		{
			it = m_data.erase(it);
		}
		else
		{
			++it;
		}
	}

	m_iSelectedRow = m_data.size() - 1;
	endResetModel();
}

///
///  @brief 设置选中的维护组名称
///
///
///  @param[in]   text  维护组名称
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年8月8日，新建函数
///
void MaintainGroupModel::SetSelectedGroupName(const QString& text)
{
	beginResetModel();
	m_data[m_iSelectedRow].__set_groupName(text.toStdString());
	endResetModel();
}

bool MaintainGroupModel::GetSelectData(tf::MaintainGroup& selectedData)
{
    if (m_iSelectedRow < 0 || m_iSelectedRow >= m_data.size())
    {
        return false;
    }
    selectedData = m_data[m_iSelectedRow];

    return true;
}

void MaintainGroupModel::CleanSelectedGroupItems()
{
    if (m_iSelectedRow < 0 || m_iSelectedRow >= m_data.size())
    {
        return;
    }
    m_data[m_iSelectedRow].items.clear();
}

bool MaintainGroupModel::IsSelectedGroupSingleType()
{
    if (m_iSelectedRow < 0 || m_iSelectedRow >= m_data.size())
    {
        return false;
    }

    return m_data[m_iSelectedRow].groupType == ::tf::MaintainGroupType::type::MAINTAIN_GROUP_SINGLE;
}

///
///  @brief 获取指定索引值的维护组
///
///
///  @param[out]   outGroup  输出维护组
///  @param[in]    index     索引值
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年8月8日，新建函数
///
bool MaintainGroupModel::GetGroupByIndex(tf::MaintainGroup & outGroup, const QModelIndex& index)
{
	if (!index.isValid())
	{
		return false;
	}

	if (index.row() < 0 || index.row() >= m_data.size())
	{
		return false;
	}

	outGroup = m_data[index.row()];

	return true;
}

///
///  @brief 获取对应id的维护组
///
///
///  @param[out]   outGroup  输出维护组
///  @param[in]    groupId   维护组id
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年8月16日，新建函数
///
bool MaintainGroupModel::GetGroupById(tf::MaintainGroup & outGroup, const int64_t groupId)
{
	for (auto &it : m_data)
	{
		if (it.id == groupId)
		{
			outGroup = it;
			return true;
		}
	}
	return false;
}

///
/// @brief  更新维护组配置
///
/// @param[in]  group  新的维护组
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年12月11日，新建函数
///
void MaintainGroupModel::UpdateGroup(const::tf::MaintainGroup & group)
{
	beginResetModel();
	for (int i = 0; i < m_data.size(); ++i)
	{
		if (group.id == m_data[i].id)
		{
			m_data[i] = group;
			break;
		}
	}
	endResetModel();
}

///
///  @brief 设置选中的行
///
///
///  @param[in]   iSelectedRow  
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年9月7日，新建函数
///
void MaintainGroupModel::SetSelectedRow(int iSelectedRow)
{
	m_iSelectedRow = iSelectedRow;
}

///
///  @brief 获取选中的行
///
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年11月15日，新建函数
///
int MaintainGroupModel::GetSelectedRow()
{
	return m_iSelectedRow;
}

///
///  @brief 选中的维护组在指定位置插入维护项类型
///
///
///  @param[in]   iPos  
///  @param[in]   item  
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年3月1日，新建函数
///
void MaintainGroupModel::InsertItem(int iPos, ::tf::MaintainItem item)
{
	if (iPos <0 || iPos > m_data[m_iSelectedRow].items.size())
	{
		return;
	}
	m_data[m_iSelectedRow].items.insert(m_data[m_iSelectedRow].items.begin() + iPos, item);
}

///
///  @brief 将from位置的元素移动到to位置
///
///
///  @param[in]   from	 移动的初始位置
///  @param[in]   to	 移动的目标位置
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年10月13日，新建函数
///
void MaintainGroupModel::Move(int from, int to)
{
	std::vector<tf::MaintainItem>& array = m_data[m_iSelectedRow].items;
	if (array.empty() || from < 0 || from >= array.size() || to < 0 || to >= array.size() || to == from)
	{
		return;
	}
	tf::MaintainItem target = array[from];
	int increment = to < from ? -1 : 1;
	for (int i = from; i != to; i += increment)
	{
		array[i] = array[i + increment];
	}
	array[to] = target;
}

///
///  @brief 选中的维护组指定位置复制维护项
///
///
///  @param[in]   index  
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年3月7日，新建函数
///
void MaintainGroupModel::CopySelGroupItemByIndex(const QModelIndex & index)
{
	// 检查索引是否合法
	if (!index.isValid())
	{
		return;
	}

	// 检查当前选中的行是否合法
	if (m_iSelectedRow < 0 || m_iSelectedRow >= m_data.size())
	{
		return;
	}

	// 检查当前选中的维护组是否为空
	auto & vecSelectedGrp = m_data[m_iSelectedRow].items;
	if (vecSelectedGrp.empty())
	{
		return;
	}

	// 检查维护组索引是否合法
	if (index.row() < 0 || index.row() >= vecSelectedGrp.size())
	{
		return;
	}

	// 获取需要插入的item
	::tf::MaintainItem addItem = vecSelectedGrp[index.row()];

	m_data[m_iSelectedRow].items.insert(m_data[m_iSelectedRow].items.begin() + index.row(), addItem);
}

///
///  @brief 删除选中的维护组指定位置维护项
///
///
///  @param[in]   index  
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年3月7日，新建函数
///
void MaintainGroupModel::DelSelGroupItemByIndex(const QModelIndex & index)
{
	// 检查索引是否合法
	if (!index.isValid())
	{
		return;
	}

	// 检查当前选中的行是否合法
	if (m_iSelectedRow < 0 || m_iSelectedRow >= m_data.size())
	{
		return;
	}

	// 检查当前选中的维护组是否为空
	auto & vecSelectedGrp = m_data[m_iSelectedRow].items;
	if (vecSelectedGrp.empty())
	{
		return;
	}

	// 检查维护组索引是否合法
	if (index.row() < 0 || index.row() >= vecSelectedGrp.size())
	{
		return;
	}

	// 更具index获取需要删除的维护项
	::tf::MaintainItem delItem = vecSelectedGrp[index.row()];
    if (!UserInfoManager::GetInstance()->IsPermissonMaint(delItem.itemType))
    {
        ULOG(LOG_WARN, "Not permit to delete.");
        return;
    }

	// 删除维护组中对应索引的该维护项
	vecSelectedGrp.erase(vecSelectedGrp.begin() + index.row());

	// 没有该项，则取消勾选
	auto fCheckCond = [&delItem](::tf::MaintainItem &item) {return item.itemType == delItem.itemType; };
	auto findFlag = std::find_if(vecSelectedGrp.begin(), vecSelectedGrp.end(), fCheckCond);
	if (findFlag == vecSelectedGrp.end())
	{
		// 发出信号，通知ui刷新维护项表中的复选框状态
		CleanItemTableCheckBox(delItem.itemType);
	}
}

///
///  @brief 是否是最后一行
///
///
///  @param[in]   iRow  行数
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年8月8日，新建函数
///
bool MaintainGroupModel::IsLastRow(int iRow)
{
	return iRow == m_data.size() - 1;
}

///
///  @brief 设置表头
///
///
///  @param[in]   headerNames  表头字符列表
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年8月11日，新建函数
///
void MaintainGroupModel::SetmHeaderNames(const QStringList headerNames)
{
	beginResetModel();
	m_headerNames = headerNames;
	endResetModel();
}

///
///  @brief 获取维护组名称
///
///
///  @param[in]   group  维护组
///
///  @return	维护组名称
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年8月17日，新建函数
///
QString MaintainGroupModel::GetGroupName(const tf::MaintainGroup& group)
{
	if (group.groupType == ::tf::MaintainGroupType::type::MAINTAIN_GROUP_CUSTOM)
	{
		return QString::fromStdString(group.groupName);
	}

	if (group.groupType == ::tf::MaintainGroupType::type::MAINTAIN_GROUP_SINGLE)
	{
		auto& items = group.items;
		if (!group.__isset.items || items.empty())
		{
			return QString();
		}
		return ConvertTfEnumToQString(items.front().itemType);
	}

	return ConvertTfEnumToQString(group.groupType);
}

void MaintainGroupModel::BackupCurrentData()
{
    m_backupData.bk_data = m_data;
    m_backupData.bk_headerNames = m_headerNames;
    m_backupData.bk_iSelectedRow = m_iSelectedRow;
}

void MaintainGroupModel::RecoverBackupData()
{
    beginResetModel();
    m_data = m_backupData.bk_data;
    endResetModel();

    m_iSelectedRow = m_backupData.bk_iSelectedRow;
}

///
///  @brief	配置数据
///
///  @param[in]   isChecked    选中状态
///  @param[in]   item		   配置的维护项
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年9月7日，新建函数
///
void MaintainGroupModel::ConfigData(bool isChecked, const tf::MaintainItem & item)
{
	// 如果数据为空则返回
	if (m_data.size() == 0)
	{
		return;
	}
	if (m_data[m_iSelectedRow].groupType == ::tf::MaintainGroupType::type::MAINTAIN_GROUP_SINGLE)
	{
		return;
	}

	// 勾选
	if (isChecked)
	{
		// 检查是否设置过项目列表（如果是新增维护组就需要设置）
		if (!m_data[m_iSelectedRow].__isset.items)
		{
			std::vector<tf::MaintainItem> items;
			m_data[m_iSelectedRow].__set_items(items);
		}

		// 附加维护项类型到维护组
		m_data[m_iSelectedRow].items.push_back(item);
	}
	// 取消
	else
	{
		// 删除对应的维护项类型
		m_data[m_iSelectedRow].items.erase(
            std::remove_if(m_data[m_iSelectedRow].items.begin(), m_data[m_iSelectedRow].items.end(), [&](const ::tf::MaintainItem & iter_item){
			return item.itemType == iter_item.itemType;
		}), m_data[m_iSelectedRow].items.end());
	}
}

bool MaintainGroupModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
	if (!index.isValid())
	{
		return false;
	}

	if (index.row() < 0 || index.row() >= m_data.size())
	{
		return false;
	}
	beginResetModel();
	m_data[index.row()].__set_groupName(value.toString().toStdString());
	endResetModel();
	return true;
}

int MaintainGroupModel::rowCount(const QModelIndex &parent) const
{
	return m_data.size();
}

int MaintainGroupModel::columnCount(const QModelIndex &parent) const
{
	return m_headerNames.size();
}

QVariant MaintainGroupModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
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

QVariant MaintainGroupModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
	{
		return QVariant();
	}

	if (role == Qt::TextAlignmentRole)
	{
		return QVariant(Qt::AlignCenter);
	}

	if (role == Qt::DisplayRole || Qt::EditRole == role)
	{
		if (index.row() >= m_data.size() || index.column() >= m_headerNames.size())
		{
			return QVariant();
		}
		switch (index.column())
		{
		case MAINTAIN_GROUP_COL::INDEX:
			return index.row() + 1;
		case MAINTAIN_GROUP_COL::NAME:
			return GetGroupName(m_data.at(index.row()));
		default:
			return QVariant();
		}
	}
	return QVariant();
}

