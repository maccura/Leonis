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
/// @file     MaintainLogModel.cpp
/// @brief 	  维护日志模型
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
#include "shared/uicommon.h"
#include <QDateTime>
#include "MaintainLogModel.h"
#include "MaintainGroupModel.h"

MaintainLogModel::MaintainLogModel(QObject *parent)
	: QAbstractTableModel(parent)
{

}

MaintainLogModel::~MaintainLogModel()
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
MaintainLogModel & MaintainLogModel::Instance()
{
	static MaintainLogModel model;
	return model;
}

///
///  @brief  设置维护日志查询的原始数据
///
///
///  @param[in]   maintainLogData  维护日志查询的原始数据
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年4月20日，新建函数
///
void MaintainLogModel::SetData(std::vector<::tf::MaintainLog>& maintainLogData)
{
	m_maintainLogData = std::move(maintainLogData);
	// 遍历查询结果
	for (auto & log : m_maintainLogData)
	{
		// 创建显示数据到表格
		int	day = QDateTime::fromString(QString::fromLocal8Bit(log.exeTime.data()), "yyyy-MM-dd HH:mm:ss").date().day();
		CreateShowData(log.groupId, day, log.exeResult);
	}
}

///
///  @brief 设置水平表头
///
///
///  @param[in]   hHeaderNames  
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年9月3日，新建函数
///
void MaintainLogModel::SetHorList(QVector<int> &hHeaderNames)
{
	beginResetModel();
	m_hHeaderNames = hHeaderNames;
	for (int i = 0; i < m_data.size(); ++i)
	{
		m_data[i].clear();
		for (int j = 0; j < hHeaderNames.size(); ++j)
		{
			m_data[i].append(Item());
		}
	}
	endResetModel();
}

///
///  @brief	设置垂直表头
///
///
///  @param[in]   vHeaderNames  
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年9月3日，新建函数
///
void MaintainLogModel::SetVorList(std::vector<tf::MaintainGroup> & vHeaderNames)
{
	beginResetModel();
	auto vecGroups = FilterGroup(vHeaderNames);
	m_data.clear();
	for (int i = 0; i < vecGroups.size(); ++i)
	{
		m_data.append(QVector<Item>());
	}
	m_data.resize(vecGroups.size());
	m_vHeaderNames = vecGroups;
	m_groupNameMap.clear();

	for (auto& vh : m_vHeaderNames)
	{
		m_groupNameMap[vh.id] = ConvertTfEnumToQString(vh.groupType).toStdString();
	}

	for (int i = 0; i < m_data.size(); ++i)
	{
		m_data[i].clear();
		for (int j = 0; j < m_hHeaderNames.size(); ++j)
		{
			m_data[i].append(Item());
		}
	}
	endResetModel();
}

///
///  @brief	设置设备信息
///
///
///  @param[in]   vDeviceInfo  设备信息
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年9月5日，新建函数
///
void MaintainLogModel::SetDeviceList(std::vector<::tf::DeviceInfo>& vDeviceInfo)
{
	beginResetModel();
	m_vDeviceInfo.clear();
	for (auto dev : vDeviceInfo)
	{
		m_vDeviceInfo.append(dev);
	}
	endResetModel();
}

///
///  @brief	获取设备序列号
///
///
///  @param[in]   index  索引
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年9月5日，新建函数
///
std::string MaintainLogModel::GetDeviceSN(int index)
{
	if (m_vDeviceInfo.empty())
	{
		return std::string();
	}

    if (index >= 0 || index < m_vDeviceInfo.size())
	{
		return m_vDeviceInfo[index].deviceSN;
	}
	return std::string();
}

///
///  @brief 获取对应行的维护组
///
///
///  @param[out]   group  输出维护组
///  @param[in]    row    维护组所在行
///
///  @return	true 获取成功 false 获取失败
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年8月16日，新建函数
///
bool MaintainLogModel::GetGroupByRow(::tf::MaintainGroup & group, const int row)
{
	if (row < 0 || row >= m_vHeaderNames.size())
	{
		return false;
	}
	group = m_vHeaderNames[row];

	return true;
}

///
///  @brief 获取维护组名称
///
///
///  @param[in]   groupId  维护组id
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年4月23日，新建函数
///
std::string MaintainLogModel::GetGroupNameById(const int64_t groupId)
{
	// 拿到对应的维护组
	tf::MaintainGroup group;
	if (!MaintainGroupModel::Instance().GetGroupById(group, groupId))
	{
		return "";
	}

	return MaintainGroupModel::GetGroupName(group).toStdString();
}

///
///  @brief 获取当前显示的维护日志（一个月之内的日志）
///
///
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年4月23日，新建函数
///
std::vector<::tf::MaintainLog>& MaintainLogModel::GetCurShowMaintainLog()
{
	return m_maintainLogData;
}

///
///  @brief 获取结果显示字符串
///
///
///  @param[in]   result  结果类型
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年4月23日，新建函数
///
std::string MaintainLogModel::GetResultShowDataByType(const ::tf::MaintainResult::type result)
{
	switch (result)
	{
	case ::tf::MaintainResult::type::MAINTAIN_RESULT_SUCCESS:
		return tr("成功").toStdString();
	case ::tf::MaintainResult::type::MAINTAIN_RESULT_FAIL:
		return tr("失败").toStdString();
	case ::tf::MaintainResult::type::MAINTAIN_RESULT_EXECUTING:
		return tr("执行中").toStdString();
	default:
		return tr("未知").toStdString();
	}
}

///
///  @brief	清除数据
///
///
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年9月6日，新建函数
///
void MaintainLogModel::ClearData()
{
	beginResetModel();
	if (m_data.empty())
	{
		return;
	}

	for (int i = 0; i < m_data.size(); ++i)
	{
		for (int j = 0; j < m_data[i].size(); j++)
		{
			m_data[i][j].Clear();
		}
	}
	m_maintainLogData.clear();
	endResetModel();
}

int MaintainLogModel::rowCount(const QModelIndex &parent) const
{
	return m_data.size();
}

int MaintainLogModel::columnCount(const QModelIndex &parent) const
{
	return m_hHeaderNames.size();
}

QVariant MaintainLogModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
	if (role != Qt::DisplayRole)
	{
		return QVariant();
	}

	if (orientation == Qt::Horizontal)
	{
		if (section < m_hHeaderNames.size())
		{
			return m_hHeaderNames.at(section);
		}
		return QVariant();
	}

	if (orientation == Qt::Vertical)
	{
		if (section >= m_vHeaderNames.size())
		{
			return QVariant();
		}

		auto &group = m_vHeaderNames.at(section);

		if (group.groupType == ::tf::MaintainGroupType::type::MAINTAIN_GROUP_CUSTOM)
		{
			return QString(group.groupName.c_str());
		}

		if (group.groupType == ::tf::MaintainGroupType::type::MAINTAIN_GROUP_SINGLE)
		{
			auto& items = group.items;
			if (!group.__isset.items || items.empty())
			{
				return QVariant();
			}
			return ConvertTfEnumToQString(items.front().itemType);
		}

		return QString(ConvertTfEnumToQString(m_vHeaderNames.at(section).groupType));
	}

	return QVariant();
}

QVariant MaintainLogModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
	{
		return QVariant();
	}
	if (role == Qt::DecorationRole)
	{
		QSize size = QSize(24, 24);
		QPixmap pixmap = m_data[index.row()][index.column()].GetIcon().pixmap(size);
		QImage image = pixmap.toImage().scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		return image;
	}

	return QVariant();
}

Item::Item():
	m_iconType(NONE_ICON)
{
}

Item::~Item()
{
}

///
///  @brief 设置维护结果到item并统计，选择合适图标
///
///
///  @param[in]   result  维护结果
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年4月7日，新建函数
///
void Item::SetResult(int result)
{
	m_lstResult.append(result);
	if (m_lstResult.last() == ::tf::MaintainResult::type::MAINTAIN_RESULT_FAIL || 
		m_lstResult.last() == ::tf::MaintainResult::type::MAINTAIN_RESULT_UNKNOWN || 
		m_lstResult.last() == ::tf::MaintainResult::type::MAINTAIN_RESULT_EXECUTING)
	{
		m_iconType = FAILED_ICON;
		return;
	}
	if (m_lstResult.last() == ::tf::MaintainResult::type::MAINTAIN_RESULT_SUCCESS)
	{
		bool bHasFailed = false;
		for (int i = 0; i < m_lstResult.size(); ++i)
		{
			if (i != m_lstResult.size() - 1 && m_lstResult[i] == ::tf::MaintainResult::type::MAINTAIN_RESULT_FAIL)
			{
				bHasFailed = true;
			}
		}
		if (bHasFailed)
		{
			m_iconType = ALRAM_ICON;
		}
		else
		{
			m_iconType = SUCCESS_ICON;
		}
	}
}

///
///  @brief 获取图标
///
///
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年4月7日，新建函数
///
QIcon Item::GetIcon() const
{
	switch (m_iconType)
	{
	case Item::NONE_ICON:
		return QIcon();
	case Item::FAILED_ICON:
		return QIcon(":/Leonis/resource/image/maintain_failed.png");
	case Item::ALRAM_ICON:
		return QIcon(":/Leonis/resource/image/maintain_alram.png");
	case Item::SUCCESS_ICON:
		return QIcon(":/Leonis/resource/image/maintain_sucess.png");
	default:
		return QIcon();
	}
	
}

///
///  @brief 清除该item数据
///
///
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年4月7日，新建函数
///
void Item::Clear()
{
	m_lstResult.clear();
	m_iconType = NONE_ICON;
}

///
///  @brief 创建显示数据
///
///
///  @param[in]   groupId	 维护组id
///  @param[in]   day		 维护时间（天）
///  @param[in]   iResult	 维护结果
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年9月5日，新建函数
///
void MaintainLogModel::CreateShowData(int64_t groupId, int day, ::tf::MaintainResult::type iResult)
{
	// 计算行
	int row = -1;
	// 拿到对应的维护组
	tf::MaintainGroup group;
	if (!MaintainGroupModel::Instance().GetGroupById(group, groupId))
	{
		return;
	}

	// 如果垂直表头为空
	if (m_vHeaderNames.empty())
	{
		return;
	}

	// 检查是否属于其他维护组
	if (group.groupType == ::tf::MaintainGroupType::type::MAINTAIN_GROUP_OTHER ||
		/*group.groupType == ::tf::MaintainGroupType::type::MAINTAIN_GROUP_CUSTOM ||*/
		group.groupType == ::tf::MaintainGroupType::type::MAINTAIN_GROUP_SINGLE)
	{
		row = m_vHeaderNames.size() - 1;
	}
	else
	{
		bool bRowFlag = false;
		for (auto it : m_vHeaderNames)
		{
			++row;
			if (it.id == groupId)
			{
				bRowFlag = true;
				break;
			}
		}

		// 检查是否找到行
		if (!bRowFlag)
		{
			return;
		}
	}

	// 计算列
	int col = -1;
	for (auto d : m_hHeaderNames)
	{
		if (d == day)
		{
			col = m_hHeaderNames.indexOf(d);
		}
	}

	// 检查是否找到列
	if (col == -1)
	{
		return;
	}

	// 检查是否找到行
	if (row == -1)
	{
		return;
	}

	// 更新数据
	beginResetModel();
	m_data[row][col].SetResult(iResult);
	endResetModel();
}

///
///  @brief 筛选出维护日志需要显示的维护组
///
///
///  @param[in]   sourceGroups  所有维护组
///
///  @return	筛选后的维护组
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年8月16日，新建函数
///
std::vector<tf::MaintainGroup> MaintainLogModel::FilterGroup(std::vector<tf::MaintainGroup>& sourceGroups)
{
	// 筛掉单项维护组和用户自定义维护组
	std::vector<tf::MaintainGroup> groups;
	for (auto& group : sourceGroups)
	{
		if (group.groupType == ::tf::MaintainGroupType::type::MAINTAIN_GROUP_SINGLE/* || group.groupType == ::tf::MaintainGroupType::type::MAINTAIN_GROUP_CUSTOM*/)
		{
			continue;
		}
		groups.push_back(group);
	}

	// 将其他维护组放到最后
	tf::MaintainGroup otherGroup;
	for (auto it = groups.begin(); it != groups.end();)
	{
		if (it->groupType == ::tf::MaintainGroupType::type::MAINTAIN_GROUP_OTHER)
		{
			otherGroup = *it;
			it = groups.erase(it);
			break;
		}
		else
		{
			++it;
		}
	}

	if (otherGroup.__isset.id)
	{
		groups.push_back(otherGroup);
	}

	return groups;
}

void CenteredDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
	if (option.state & QStyle::State_Selected)
	{
		QColor color = option.palette.highlight().color();
		color.setAlpha(50);
		painter->fillRect(option.rect, color);
		painter->setPen(option.palette.highlightedText().color());
	}
	else
	{
		painter->fillRect(option.rect, option.palette.base());
	}

	if (index.data(Qt::DecorationRole).canConvert<QPixmap>())
	{
		QSize size = QSize(24, 24);
		QPixmap pixmap = qvariant_cast<QPixmap>(index.data(Qt::DecorationRole));
		QRect rect = option.rect;
		QRect centeredRect = pixmap.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation).rect();
		centeredRect.moveCenter(rect.center());
		painter->drawPixmap(centeredRect, pixmap.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	}
	else
	{
		QStyledItemDelegate::paint(painter, option, index);
	}
}