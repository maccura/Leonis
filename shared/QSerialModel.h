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
/// @file     QSerialModel.h
/// @brief    序列化的model（本model暂时只适用于浏览功能的表格，如有有删除或者排序功能，可能不适用）
///
/// @author   8090/YeHuaNing
/// @date     2022年9月14日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8090/YeHuaNing，2022年9月14日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once
#include <QStandardItemModel>

using IndexMap = QMap<QString, int>;

class QSerialModel : public QStandardItemModel
{
	enum IndexType
	{
		RowIndex = 0,
		ColIndex
	};

public:
	QSerialModel(QObject *parent = Q_NULLPTR);
	~QSerialModel();

	///
	/// @brief 
	///
	/// @param[in]  labels  横向表头列表
	///
	/// @return 
	///
	/// @par History:
	/// @li 8090/YeHuaNing，2022年9月14日，新建函数
	///
	void setHorizontalHeaderLabels(const QStringList &labels);

	///
	/// @brief 
	///
	/// @param[in]  labels  纵向表头列表
	///
	/// @return 
	///
	/// @par History:
	/// @li 8090/YeHuaNing，2022年9月14日，新建函数
	///
	void setVerticalHeaderLabels(const QStringList &labels);

	///
	/// @brief 设置mapIndex
	///
	/// @param[in]  labels 列表 
	/// @param[in]  indexMap  图索引
	///
	/// @par History:
	/// @li 8090/YeHuaNing，2022年9月14日，新建函数
	///
	void SetIndexMap(const QStringList& labels, IndexMap& indexMap);

	///
	/// @brief 
	///
	/// @param[in]  rowTitle  行索引标题
	/// @param[in]  colTitle  列索引标题
	/// @param[in]  item  项目
	///
	/// @return 返回是否成功设置
	///
	/// @par History:
	/// @li 8090/YeHuaNing，2022年9月14日，新建函数
	///
	bool SetItem(const QString& rowTitle, const QString& colTitle, QStandardItem *item);

	///
	/// @brief 返回标题对应的索引号
	///
	/// @param[in]  text  标题
	///
	/// @return 返回对应的索引号，否则返回-1
	///
	/// @par History:
	/// @li 8090/YeHuaNing，2022年9月14日，新建函数
	///
	int GetRowIndex(const QString& text);

	///
	/// @brief 返回标题对应的索引号
	///
	/// @param[in]  text  标题
	///
	/// @return 返回对应的索引号，否则返回-1
	///
	/// @par History:
	/// @li 8090/YeHuaNing，2022年9月14日，新建函数
	///
	int GetColIndex(const QString& text);

private:
	bool							m_bIsHoriTitleSetted;		///< 行标题是否通过接口设置
	IndexMap						m_rowIndexMap;				///< 行的索引index
	bool							m_bIsVertTitleSetted;		///< 列标题是否通过接口设置
	IndexMap						m_colIndexMap;				///< 列的索引index
};
