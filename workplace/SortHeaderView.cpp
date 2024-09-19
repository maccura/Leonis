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
/// @file     SortHeaderView.h
/// @brief 	 数据浏览表头（自定义表头，以支持取消排序状态）
///
/// @author   7656/zhang.changjiang
/// @date      2023年2月4日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年2月4日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "SortHeaderView.h"
#include <QMouseEvent>
#include <QCursor>

SortHeaderView::SortHeaderView(Qt::Orientation orientation, QWidget *parent)
    : QHeaderView(orientation,parent)
	, m_iPressLogicIndex(-1)			                         
	, m_iPressVisualIndex(-1)			                         
	, m_iPreviousLogicIndex(-1)	
	, m_sortOrder(NoOrder)
{
    //禁用默认的点击处理，这样就不会触发默认排序
    //自己处理点击来设置排序
    setSectionsClickable(false);
}

///
///  @brief 获取不排序的行列（为什么放header不放proxy？因为排序是点击header调用的）
///
///
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年2月4日，新建函数
///
QList<int> SortHeaderView::GetUnsortIndexs() const
{
    return m_unsortIndexs;
}

///
///  @brief 设置不排序的行列
///
///
///  @param[in]   rowOrColumns  
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年2月4日，新建函数
///
void SortHeaderView::SetUnsortIndexs(const QList<int> &rowOrColumns)
{
    m_unsortIndexs = rowOrColumns;
}

///
///  @brief 添加不排序的行列
///
///
///  @param[in]   rowOrColumn  不排序的行列
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年2月4日，新建函数
///
void SortHeaderView::AppendUnsortIndex(int rowOrColumn)
{
    m_unsortIndexs.push_back(rowOrColumn);
}

void SortHeaderView::ResetAllIndex()
{
    int pressLogicIndex = -1;       //鼠标按下时对应原model index
    int pressVisualIndex = -1;      //鼠标按下时对应显示的index，可能是交换了位置的
    int previousLogicIndex = -1;    //上一次点击对应的logic index
    m_sortOrder = NoOrder;          //当前排序状态
}

///
/// @brief  获取当前的排序方式
///
///
/// @return 排序方式
///
/// @par History:
/// @li 7656/zhang.changjiang，2024年1月9日，新建函数
///
SortHeaderView::SortOrder SortHeaderView::GetCurSortOrder()
{
	return m_sortOrder;
}

///
/// @brief  获取当前排序的列
///
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2024年1月9日，新建函数
///
int SortHeaderView::GetCurSortCol()
{
	return m_iPressVisualIndex;
}

void SortHeaderView::showEvent(QShowEvent *event)
{
    QHeaderView::showEvent(event);
    //因为初始化可能是用的sortByColumn来排序，但是这个接口不是虚函数没法自定义
    //所以就在显示的时候同步下状态
    m_iPreviousLogicIndex = sortIndicatorSection();
    if (m_iPreviousLogicIndex >= 0) {
        m_sortOrder = (sortIndicatorOrder() == Qt::AscendingOrder) ? AscOrder : DescOrder;
    }
    else {
        m_sortOrder = NoOrder;
    }
}

void SortHeaderView::mousePressEvent(QMouseEvent *event)
{
    QHeaderView::mousePressEvent(event); 

    if (event->button() != Qt::LeftButton)
	{
		return;
	}

    const int pos = (orientation() == Qt::Horizontal) ? event->x() : event->y();
    const int index = logicalIndexAt(pos);
    Qt::CursorShape shape = cursor().shape();

    //先判断index是否为有效的section，
    //判断 cursor-shape 是为了过滤点 resize-handle 情况
    m_iPressLogicIndex = -1;
    m_iPressVisualIndex = -1;
	if (index >= 0 && shape != Qt::SplitHCursor && shape != Qt::SplitVCursor)
    {
        //排除不排序的index
        if (!m_unsortIndexs.contains(index))
        {
            m_iPressLogicIndex = index;
            m_iPressVisualIndex = visualIndex(index);
        }
    }
}

void SortHeaderView::mouseReleaseEvent(QMouseEvent *event)
{
    QHeaderView::mouseReleaseEvent(event);

	if (event->button() != Qt::LeftButton)
	{
		return;
	}
        
    const int pos = (orientation() == Qt::Horizontal) ? event->x() : event->y();
    const int index = logicalIndexAt(pos);
    const int visual_index = visualIndex(index);

    //弹起后再进行排序操作
    //此处还可以判断下鼠标的移动距离，暂略
    if (m_iPressLogicIndex >= 0 && visual_index == m_iPressVisualIndex)
    {
        //和上次点击的index比较，相同的就轮换排序状态，不同的就降序
        //放release不放pressed是为了避免双击时判断不准
        if (m_iPressLogicIndex == m_iPreviousLogicIndex)
        {
            m_sortOrder = GetNextOrder(m_sortOrder);
        }
        else
        {
            m_sortOrder = GetNextOrder(NoOrder);
        }
        emit SortOrderChanged(m_iPressLogicIndex, m_sortOrder);
        m_iPreviousLogicIndex = m_iPressLogicIndex;
    }
    //注释不复位是为了避免双击时判断不准
    //pressLogicIndex=-1;
}

///
///  @brief 鏍规嵁褰撳墠鎺掑簭鐘舵€佸緱鍒颁笅涓€涓姸鎬?
///
///
///  @param[in]   order  褰撳墠鎺掑簭鐘舵€?
///
///  @return	涓嬩竴涓姸鎬?
///
///  @par History: 
///  @li 7656/zhang.changjiang锛?023骞?鏈?鏃ワ紝鏂板缓鍑芥暟
///
SortHeaderView::SortOrder SortHeaderView::GetNextOrder(SortHeaderView::SortOrder order) const
{
    // 状态循环：原序-升序-降序
    switch (order)
    {
    case NoOrder: return AscOrder;
    case AscOrder : return DescOrder;
    case DescOrder: return NoOrder;
    }
    return NoOrder;
}

///////////////////////////////////////////////////////////////////////////
/// @file     SortHeaderView.cpp
/// @brief 	  排序筛选类
///
/// @author   7656/zhang.changjiang
/// @date      2023年4月17日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年4月17日，新建文件
///
///////////////////////////////////////////////////////////////////////////
SortFilterProxyModel::SortFilterProxyModel(QObject * parent)
	:QSortFilterProxyModel(parent)
{
}

SortFilterProxyModel::~SortFilterProxyModel()
{
}

bool SortFilterProxyModel::lessThan(const QModelIndex & source_left, const QModelIndex & source_right) const
{
	auto pSourceModel = sourceModel();
	if (pSourceModel == nullptr)
	{
		return false;
	}

	QVariant left = pSourceModel->data(source_left);
	QVariant right = pSourceModel->data(source_right);

	//如果左侧是空行，则将其排在右侧之后
	if (!left.isValid() || left.toString().isEmpty()) 
	{
		//如果当前是升序排列，空行排在最后
		//如果当前是降序排列，空行排在最前
		return (sortOrder() == Qt::AscendingOrder ? false : true);
	}

	//如果右侧是空行，则将其排在左侧之后
	if (!right.isValid() || right.toString().isEmpty()) 
	{
		//如果当前是升序排列，空行排在最后
		//如果当前是降序排列，空行排在最前
		return (sortOrder() == Qt::AscendingOrder ? true : false);
	}

	//否则按照字符串本地化排序规则进行比较
	return QString::localeAwareCompare(left.toString(), right.toString()) < 0;
}

///
///  @brief 获取指定行列的数据
///
///
///  @param[in]   row  行
///  @param[in]   col  列
///
///  @return	第row行第col列的数据
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年6月21日，新建函数
///
QVariant SortFilterProxyModel::GetData(int row, int col)
{
	if (row < 0 || col < 0)
	{
		return QVariant();
	}

	auto proxyIndex = this->index(row, col);
	if (!proxyIndex.isValid())
	{
		return QVariant();
	}

	auto sourceIndex = this->mapToSource(proxyIndex);
	if (!sourceIndex.isValid())
	{
		return QVariant();
	}
	return sourceModel()->data(sourceIndex);
}

