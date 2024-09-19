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
#include "QResultRangeCard.h"
#include "ui_QResultRangeCard.h"

QResultRangeCard::QResultRangeCard(QWidget *parent)
	: QWidget(parent)
	, m_iNormalStart(0)
	, m_iNormalEnd(0)
	, m_iRecheckStart(0)
	, m_iRecheckEnd(0)
	, m_iNormalPer(0)
	, m_iRecheckPer(0)
	, m_iIndex(-1)
{
	ui = new Ui::QResultRangeCard();
	ui->setupUi(this);
	InitBeforeShow();
}

QResultRangeCard::QResultRangeCard(const int index, QWidget * parent)
	: QWidget(parent)
	, m_iNormalStart(0)
	, m_iNormalEnd(0)
	, m_iRecheckStart(0)
	, m_iRecheckEnd(0)
	, m_iNormalPer(0)
	, m_iRecheckPer(0)
	, m_iIndex(index)
{
	ui = new Ui::QResultRangeCard();
	ui->setupUi(this);
	InitBeforeShow();
}

QResultRangeCard::~QResultRangeCard()
{
	delete ui;
}

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
void QResultRangeCard::SetNormalRange(const int start, const int end)
{
	ui->label_normal_range->setText(start >= end ? QString(">%1").arg(start) : QString("%1-%2").arg(start).arg(end));
	m_iNormalStart = start;
	m_iNormalEnd = end;
}

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
void QResultRangeCard::SetRecheckRange(const int start, const int end)
{
	ui->label_recheck_range->setText(start >= end ? QString(">%1").arg(start) : QString("%1-%2").arg(start).arg(end));
	m_iRecheckStart = start;
	m_iRecheckEnd = end;
}

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
void QResultRangeCard::SetNormalPer(const int percent)
{
	ui->label_normal_per->setText(QString("%1%").arg(percent));
	m_iNormalPer = percent;
}

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
void QResultRangeCard::SetRecheckPer(const int percent)
{
	ui->label_recheck_per->setText(QString("%1%").arg(percent));
	m_iRecheckPer = percent;
}

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
void QResultRangeCard::SetNormalVisible(const bool visable)
{
	ui->label_normal_range->setVisible(visable);
	ui->label_normal_per->setVisible(visable);
}

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
void QResultRangeCard::SetRecheckVisible(const bool visable)
{
	ui->label_recheck_range->setVisible(visable);
	ui->label_recheck_per->setVisible(visable);
}

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
void QResultRangeCard::SetCardIndex(const int index)
{
	m_iIndex = index;
}

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
int QResultRangeCard::GetCardIndex()
{
	return m_iIndex;
}

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
void QResultRangeCard::InitBeforeShow()
{
	ui->label_normal_range->setText(QString("%1-%2").arg(m_iNormalStart).arg(m_iNormalEnd));
	ui->label_normal_range->setText(QString("%1-%2").arg(m_iRecheckStart).arg(m_iRecheckEnd));
	ui->label_normal_per->setText(QString("%1%").arg(m_iNormalPer));
	ui->label_recheck_per->setText(QString("%1%").arg(m_iRecheckPer));
}
