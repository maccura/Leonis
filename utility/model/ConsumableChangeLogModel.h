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
#pragma once

#include <mutex>
#include <QObject>
#include <QAbstractTableModel>
#include "src/thrift/gen-cpp/defs_types.h"

class QUtilitySortFilterProxyModel;
class ConsumableChangeLogModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	///
	/// @brief
	///     操作日志表列定义
	///
	enum class CONSUMABLE_CHANGE_LOG_LIST_COL
	{
		INDEX = 0,                  // 序号
		SYSTEM,						// 系统
		MODULE,                     // 模块
		NAME,						// 名称
		ACTION,						// 操作类型
		STATUS,						// 状态
		USER,					    // 用户名
		BATCHNUM,					// 批号
		BOTTLENUM,					// 瓶号/序列号
		TIME,                       // 时间
		ID							// 数据库主键（该列隐藏，用于倒序显示的排序操作）
	};

private:
	ConsumableChangeLogModel(QObject *parent);
public:
	~ConsumableChangeLogModel();

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
	static ConsumableChangeLogModel& Instance();
	
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
	void SetData(std::vector<::tf::ConsumableChangeLog>& data);
	
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
	void SetCurQueryCond(const ::tf::ConsumableChangeLogQueryCond& curQryCond);
	
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
	const ::tf::ConsumableChangeLogQueryCond& GetCurQueryCond();

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
	QString GetChangeAction(const ::tf::ChangeAction::type changeAction) const;
	
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
	QString GetChangeStatus(const ::tf::ChangeStatus::type changeStatus) const;

	///
	/// @brief
	///     获取数据
	///
	/// @return 被显示的数据
	///
	/// @par History:
	///  @li 7656/zhang.changjiang，2023年4月14日，新建函数
	///
	const std::vector<::tf::ConsumableChangeLog>& GetData();

	///
	/// @brief
	///     追加显示数据到模型
	///
	/// @param[in]  data  
	/// @par History:
	///  @li 7656/zhang.changjiang，2023年4月14日，新建函数
	///
	void AppendData(::tf::ConsumableChangeLog& data);
	
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
	void ClearData();

	///
	/// @brief
	///     设置缓存数据的空间大小
	/// @param[in]  size  
	///
	/// @par History:
	///  @li 7656/zhang.changjiang，2023年4月14日，新建函数
	///
	void ReserveDataSize(int size) { m_data.reserve(size); }

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
	void SetSortModel(QUtilitySortFilterProxyModel* sortModel);

	///
	/// @brief  根据设备类型和耗材编号获取耗材名称
	///
	/// @param[in]  deviceClassify  设备体系
	/// @param[in]  suppliesName	耗材名称（生化和ise是存的耗材编号）  
	///
	/// @return 耗材名称
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年4月1日，新建函数
	///
	QString GetSuppliesName(const tf::AssayClassify::type &deviceClassify, const std::string &suppliesName) const;

protected:
	int rowCount(const QModelIndex &parent) const override;
	int columnCount(const QModelIndex &parent) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
	QStringList                             m_headerNames;              // 表头名称集合
	std::vector<::tf::ConsumableChangeLog>	m_data;						// 显示的数据
	std::map<std::string, std::string>      m_devNameMap;				// 设备序列号和设备名称的映射
	::tf::ConsumableChangeLogQueryCond		m_curQryCond;				// 当前显示数据的查询条件
	std::mutex								m_qryMutex;					// 查询条件锁
	QUtilitySortFilterProxyModel*           m_sortModel;                // 表格排序的model
};
