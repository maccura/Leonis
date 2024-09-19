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
/// @file     QSerialModel.cpp
/// @brief    序列号的model
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

#include "QSerialModel.h"

QSerialModel::QSerialModel(QObject *parent)
	:QStandardItemModel(parent)
	,m_bIsHoriTitleSetted(false)
	, m_bIsVertTitleSetted(false)
{
}

QSerialModel::~QSerialModel()
{
	
}

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
void QSerialModel::setHorizontalHeaderLabels(const QStringList & labels)
{
	m_bIsHoriTitleSetted = true;
	SetIndexMap(labels, m_colIndexMap);

	QStandardItemModel::setHorizontalHeaderLabels(labels);
}

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
void QSerialModel::setVerticalHeaderLabels(const QStringList & labels)
{
	m_bIsVertTitleSetted = true;
	SetIndexMap(labels, m_rowIndexMap);

	QStandardItemModel::setVerticalHeaderLabels(labels);
}

///
/// @brief 设置mapIndex
///
/// @param[in]  labels 列表 
/// @param[in]  indexMap  图索引
///
/// @par History:
/// @li 8090/YeHuaNing，2022年9月14日，新建函数
///
void QSerialModel::SetIndexMap(const QStringList & labels, IndexMap & indexMap)
{
	indexMap.clear();

	for (const auto& lable : labels)
	{
		indexMap[lable] = indexMap.size();
	}
}

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
bool QSerialModel::SetItem(const QString & rowTitle, const QString & colTitle, QStandardItem * item)
{
	if (GetRowIndex(rowTitle) == -1 || GetColIndex(colTitle) == -1)
	{
		return false;
	}

	QStandardItemModel::setItem(m_rowIndexMap[rowTitle], m_colIndexMap[colTitle], item);

	return true;
}

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
int QSerialModel::GetRowIndex(const QString & text)
{
	if (m_rowIndexMap.find(text) == m_rowIndexMap.end())
	{
		return -1;
	}

	return m_rowIndexMap[text];
}

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
int QSerialModel::GetColIndex(const QString & text)
{
	if (m_colIndexMap.find(text) == m_colIndexMap.end())
	{
		return -1;
	}

	return m_colIndexMap[text];
}

