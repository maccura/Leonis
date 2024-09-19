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
/// @brief 	  自定义表头，以支持取消排序状态
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
#pragma once
#include <QHeaderView>
#include <QSortFilterProxyModel>

class SortHeaderView : public QHeaderView
{
    Q_OBJECT
public:
    //排序状态
    enum SortOrder
    {
        NoOrder = 0 //未排序
        , DescOrder //降序
        , AscOrder  //升序
    };
public:
	explicit SortHeaderView(Qt::Orientation orientation = Qt::Horizontal, QWidget *parent = nullptr);
    
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
    QList<int> GetUnsortIndexs() const;
    
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
    void SetUnsortIndexs(const QList<int> &rowOrColumns);
    
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
    void AppendUnsortIndex(int rowOrColumn);

    ///
    /// @bref
    ///		重置所有索引记录
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年5月18日，新建函数
    ///
    void ResetAllIndex();
	
	///
	/// @brief  获取当前的排序方式
	///
	///
	/// @return 排序方式
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2024年1月9日，新建函数
	///
	SortOrder GetCurSortOrder();

	///
	/// @brief  获取当前排序的列
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2024年1月9日，新建函数
	///
	int GetCurSortCol();
protected:
    //显示的时候同步indicator三角状态，
    //因为初始化可能是用的sortByColumn来排序，但是这个接口不是虚函数没法自定义
    //所以就在显示的时候同步下状态
    void showEvent(QShowEvent *event) override;
    //点击表头
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    
    ///
    ///  @brief 根据当前排序状态得到下一个状态
    ///
    ///
    ///  @param[in]   order  当前排序状态
    ///
    ///  @return	下一个状态
    ///
    ///  @par History: 
    ///  @li 7656/zhang.changjiang，2023年2月4日，新建函数
    ///
    SortOrder GetNextOrder(SortOrder order) const;
signals:

    ///
    ///  @brief  排序状态改变信号（因为view里关联了自带的sortIndicatorChanged，所以自定义一个，避免冲突）
    ///
    ///
    ///  @param[in]   index  
    ///  @param[in]   order  
    ///
    ///  @return	
    ///
    ///  @par History: 
    ///  @li 7656/zhang.changjiang，2023年2月4日，新建函数
    ///
    void SortOrderChanged(int index, SortOrder order);

private: 
    QList<int>                  m_unsortIndexs;			             //不进行排序的行列
    int                         m_iPressLogicIndex;			         //鼠标按下时对应原model index
    int                         m_iPressVisualIndex;			     //鼠标按下时对应显示的index，可能是交换了位置的
    int                         m_iPreviousLogicIndex;		         //上一次点击对应的logic index
    SortOrder                   m_sortOrder;		                 //当前排序状态
};

///////////////////////////////////////////////////////////////////////////
/// @file     SortHeaderView.h
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
class SortFilterProxyModel : public QSortFilterProxyModel
{
public:
	SortFilterProxyModel(QObject* parent = nullptr);
	~SortFilterProxyModel();
	virtual bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;
	
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
	QVariant GetData(int row, int col);
};
