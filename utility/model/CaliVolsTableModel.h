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
/// @file     CaliVolsTableModel.h
/// @brief    稀释浓度设置（应用->分析参数->校准）
///
/// @author   6950/ChenFei
/// @date     2022年5月16日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 6950/ChenFei，2022年5月24日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once

#include "src/thrift/ch/gen-cpp/ch_types.h"
#include <QAbstractTableModel>

class CaliVolsTableModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	///
	/// @brief
	///     列定义
	///
	enum class COL
	{
		INDEX = 0,                  /// 序号
		SAMPLE,                     /// 样本量
		SAMPLE4TEST,                /// 稀释样本量
		DILUENT,                    /// 稀释液量
        LEVEL,                      /// 混匀等级
		COUNT						/// 列数
	};

private:
	CaliVolsTableModel(QObject *parent);

public:
	virtual ~CaliVolsTableModel();

public:
    ///
    /// @brief
    ///     获取单例
    ///
    /// @return 单例对象
    ///
    /// @par History:
    /// @li 6950/ChenFei，2022年05月24日，新建函数
    ///
	static CaliVolsTableModel& Instance();

    ///
    /// @brief
    ///     设置数据到模型
    ///
    ///@param[in]    data 显示数据
    ///
    /// @par History:
    /// @li 6950/ChenFei，2022年05月24日，新建函数
    ///
	void SetData(const std::vector<ch::tf::SampleAspirateVol>& data);

    ///
    /// @brief
    ///     获取数据
    ///
    /// @return 被显示的数据
    ///
    /// @par History:
    /// @li 6950/ChenFei，2022年05月24日，新建函数
    ///
	const std::vector<ch::tf::SampleAspirateVol>& GetData();

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
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

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
	Qt::ItemFlags flags(const QModelIndex &index) const override;

protected:
	int rowCount(const QModelIndex &parent) const override;
	int columnCount(const QModelIndex &parent) const override;
	QVariant data(const QModelIndex &index, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
	QStringList                     m_headerNames;              /// 表头名称集合
	std::vector<ch::tf::SampleAspirateVol> m_data;				/// 显示的数据
};
