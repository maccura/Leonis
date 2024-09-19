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
/// @file     MaintainLogModel.h
/// @brief 	  维护日志模型
///
/// @author   7656/zhang.changjiang
/// @date      2022年9月1日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2022年9月1日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <QAbstractTableModel>
#include <QIcon>
#include <QStyledItemDelegate>
#include <QPainter>
#include <src/thrift/gen-cpp/defs_types.h>
class Item
{
public:
	enum ResultIcon
	{
		NONE_ICON,				// 空图标
		FAILED_ICON,			// 失败图标
		ALRAM_ICON,				// 警告图标
		SUCCESS_ICON			// 成功图标
	};
	Item();
	~Item();
	
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
	void SetResult(int result);
	
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
	QIcon GetIcon() const;
	
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
	void Clear();
private:
	ResultIcon							m_iconType;
	QVector<int>						m_lstResult;
};
class MaintainLogModel : public QAbstractTableModel
{
	Q_OBJECT
private:
	MaintainLogModel(QObject *parent = Q_NULLPTR);
public:
	virtual ~MaintainLogModel();

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
	static MaintainLogModel& Instance();

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
	void SetData(std::vector<::tf::MaintainLog>& maintainLogData);

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
	void SetHorList(QVector<int> &hHeaderNames);
	
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
	void SetVorList(std::vector<tf::MaintainGroup> &vHeaderNames);
	
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
	void SetDeviceList(std::vector<::tf::DeviceInfo> & vDeviceInfo);
	
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
	std::string GetDeviceSN(int index);

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
	bool GetGroupByRow(::tf::MaintainGroup& group, const int row);

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
	std::string GetGroupNameById(const int64_t groupId);

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
	std::vector<::tf::MaintainLog>& GetCurShowMaintainLog();

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
	std::string GetResultShowDataByType(const ::tf::MaintainResult::type result);

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
	void ClearData();

protected:
	int rowCount(const QModelIndex &parent) const override;
	int columnCount(const QModelIndex &parent) const override;
	QVariant data(const QModelIndex &index, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
private:

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
	void CreateShowData(int64_t groupId, int day, ::tf::MaintainResult::type iResult);

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
	std::vector<tf::MaintainGroup> FilterGroup(std::vector<tf::MaintainGroup> &sourceGroups);
private:
	QVector<int>								m_hHeaderNames;             // 水平表头名称集合
	std::vector<tf::MaintainGroup>				m_vHeaderNames;             // 垂直表头名称集合
	QVector<QVector<Item>>						m_data;                     // 显示的数据
	std::vector<::tf::MaintainLog>				m_maintainLogData;			// 维护日志查询的原始数据
	QVector<::tf::DeviceInfo>					m_vDeviceInfo;				// 设备序列号
	std::map<int64_t, std::string>				m_groupNameMap;				// 维护组名称映射表
};
class CenteredDelegate : public QStyledItemDelegate
{
public:
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};
