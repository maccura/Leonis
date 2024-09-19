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
/// @file     QResultRangeCard.h
/// @brief 	  结果分段卡片
///
/// @author   7656/zhang.changjiang
/// @date      2023年7月19日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年7月19日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QWidget>
namespace Ui { class QResultRangeCard; };

class QResultRangeCard : public QWidget
{
	Q_OBJECT

public:
	QResultRangeCard(QWidget *parent = Q_NULLPTR);
	QResultRangeCard(const int index, QWidget *parent = Q_NULLPTR);
	~QResultRangeCard();
	
	///
	///  @brief 设置常规样本范围
	///
	///
	///  @param[in]   start  起始
	///  @param[in]   end    结束
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月19日，新建函数
	///
	void SetNormalRange(const int start, const int end);;
	
	///
	///  @brief 设置复查样本范围
	///
	///
	///  @param[in]   start  起始
	///  @param[in]   end    结束
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月19日，新建函数
	///
	void SetRecheckRange(const int start, const int end);
	
	///
	///  @brief 设置常规样本百分比
	///
	///
	///  @param[in]   percent  样本百分比
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月19日，新建函数
	///
	void SetNormalPer(const int percent);
	
	///
	///  @brief 设置复查样本百分比
	///
	///
	///  @param[in]   percent  复查样本百分比
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月19日，新建函数
	///
	void SetRecheckPer(const int percent);
	
	///
	///  @brief 设置常规样本数据是否显示
	///
	///
	///  @param[in]   visable  true 显示 false 隐藏
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月21日，新建函数
	///
	void SetNormalVisible(const bool visable);
	
	///
	///  @brief 设置复查样本数据是否显示
	///
	///
	///  @param[in]   visable    true 显示 false 隐藏
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月21日，新建函数
	///
	void SetRecheckVisible(const bool visable);

	///
	///  @brief 设置卡片索引
	///
	///
	///  @param[in]   index  索引
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月21日，新建函数
	///
	void SetCardIndex(const int index);
	
	///
	///  @brief 获取卡片索引
	///
	///
	///
	///  @return	卡片索引
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月21日，新建函数
	///
	int GetCardIndex();
private:
	
	///
	///  @brief 更新数据
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月21日，新建函数
	///
	void InitBeforeShow();
private:
	Ui::QResultRangeCard *ui;
	int						m_iNormalStart;		     // 常规样本范围启始
	int						m_iNormalEnd;			 // 常规样本范围终点
	int						m_iRecheckStart;		 // 复查样本范围启始
	int						m_iRecheckEnd;			 // 复查样本范围终点
	int						m_iNormalPer;			 // 常规样本所占百分比
	int						m_iRecheckPer;		     // 复查样本所占百分比
	int						m_iIndex;				 // 卡片索引
};
