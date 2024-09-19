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
/// @file     FaultLogTableModel.h
/// @brief    故障日志列表模型（应用->日志->故障日志）
///
/// @author   7951/LuoXin
/// @date     2022年7月6日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2022年7月6日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <array>
#include <QAbstractTableModel>

class QUtilitySortFilterProxyModel;

class FaultLogTableModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	///
	/// @brief
	///     故障日志表列定义
	///
	enum  FAULT_LOG_LIST_COL
	{
		INDEX = 0,                  /// 序号
		SYSTEM,						/// 系统
		MODULE,                     /// 模块名
		CODE,						/// 报警代码
		LEVEL,						/// 报警级别
		NAME,						/// 报警名称
		TIME,						/// 报警时间
		COUNT                       /// 列数
	};

	///
	/// @brief
	///     显示数据结构
	///
	struct StFaultLogItem
	{
		QString system;						   /// 系统
		QString model;                         /// 模块
		QString code;						   /// 报警代码
		QString name;                          /// 报警名称（描述）
		QString	level;						   /// 报警级别
		QString time;						   /// 报警时间
		QString detail;                        /// 报警详情
		QString solution;                      /// 解决方案
        QString managerSolution;               /// 工程师解决方案

		bool operator==(const StFaultLogItem& rhs)
		{
			return this->system == rhs.system
				&& this->model == rhs.model
				&& this->code == rhs.code
				&& this->name == rhs.name
				&& this->level == rhs.level
				&& this->time == rhs.time
				&& this->detail == rhs.detail
				&& this->solution == rhs.solution
                && this->managerSolution == rhs.managerSolution;
		}
	};

private:
	FaultLogTableModel(QObject *parent);

public:
	virtual ~FaultLogTableModel();

public:
	///
	/// @brief
	///     获取单例
	///
	/// @return 单例对象
	///
	/// @par History:
	/// @li 6950/ChenFei，2022年05月16日，新建函数
	///
	static FaultLogTableModel& Instance();

	///
	/// @brief
	///     设置数据到模型
	///
	///@param[in]    data 显示数据
	///
	/// @par History:
	/// @li 6950/ChenFei，2022年05月16日，新建函数
	///
	void SetData(const std::vector<StFaultLogItem>& data);

	///
	/// @brief
	///     获取数据
	///
	/// @return 被显示的数据
	///
	/// @par History:
	/// @li 6950/ChenFei，2022年05月16日，新建函数
	///
	const std::vector<StFaultLogItem>& GetData();

	///
	/// @brief
	///     追加显示数据到模型
	///
	/// @param[in]  data  
	/// @par History:
	/// @li 7951/LuoXin，2022年8月25日，新建函数
	///
	void AppendData(StFaultLogItem& data);

	///
	/// @brief
	///     设置缓存数据的空间大小
	/// @param[in]  size  
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年8月25日，新建函数
	///
	void ReserveDataSize(int size) { m_data.reserve(size); }

    ///
    /// @brief  获取行信息
    ///     
    /// @param[in]  row 行  
    /// @param[out]  rowData 行信息
    ///
    /// @return 成功返回true
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年6月8日，新建函数
    ///
    bool GetRowData(int row, StFaultLogItem& rowData);

    ///
    /// @brief  移除所有行
    ///    
    void RemoveAllRows();

    ///
    /// @brief  设置排序模型
    ///  
    void SetSortModel(QUtilitySortFilterProxyModel* sortModel) { m_sortModel = sortModel; }

protected:
	int rowCount(const QModelIndex &parent) const override;
	int columnCount(const QModelIndex &parent) const override;
	QVariant data(const QModelIndex &index, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
	QStringList                     m_headerNames;              /// 表头名称集合
	std::vector<StFaultLogItem>		m_data;						/// 显示的数据
    QUtilitySortFilterProxyModel*   m_sortModel;                /// 表格排序的model
};
