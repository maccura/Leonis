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
/// @file     mccustomqwtmarker.h
/// @brief    自定义坐标轴
///
/// @author   4170/TangChuXian
/// @date     2018年07月07日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2023年07月07日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <qwt_symbol.h>
#include <qwt_plot_marker.h>

// 悬浮指示符号
class QcIndicateSymbol : public QwtSymbol
{
public:
    ///
    /// @brief
    ///     质控指示符号
    ///
    /// @param[in]  strConc     浓度
    /// @param[in]  strTextList 文本列表
    /// @param[in]  textHorAlign 文本在气泡中显示位置
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月10日，新建函数
    ///
    QcIndicateSymbol(const QStringList& strTextList, int iAlign = Qt::AlignTop, int textHorAlign = Qt::AlignLeft);

private:
    int             m_ciFontSize;                   // 字体大小
    QStringList     m_cstrTextList;                 // 文本列表（每行一个text）
    QFont           m_font;                         // 文本字体
};

// 悬浮提示标记
class QcGraphicSelTipMaker : public QwtPlotMarker
{
public:
    QcGraphicSelTipMaker();

    ///
    /// @brief
    ///     设置质控时间
    ///
    /// @param[in]  strTextList 文本列表
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月7日，新建函数
    ///
    void SetQcTipContent(const QStringList& strTextList);

    ///
    /// @brief
    ///     设置标签对齐方式
    ///
    /// @param[in]  enAlignment  对齐方式
    /// @param[in]  textAlign  文字在气泡中的对齐方式
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月7日，新建函数
    ///
    void setLabelAlignment(Qt::Alignment enAlignment, Qt::Alignment textAlign = Qt::AlignLeft);

protected:
    ///
    /// @brief
    ///     更新标记曲线显示
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月10日，新建函数
    ///
    void UpdateMakerDisplay();

private:
    QStringList     m_strTextList;                  // 文本列表（每行一个text）
    Qt::Alignment   m_enAlign;                      // 对齐方式
    Qt::Alignment   m_textAlign;                    // 对齐方式
};

///////////////////////////////////////////////////////////////////////////
/// @file     mccustomqwtmarker.h
/// @brief 	  统计分析提示消息气泡
///
/// @author   7656/zhang.changjiang
/// @date      2023年7月24日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年7月24日，新建文件
///
///////////////////////////////////////////////////////////////////////////
class StatisticsIndicateSymbol : public QwtSymbol
{
public:

	///
	///  @brief 统计分析指示符号
	///
	///
	///  @param[in]   strTextList  
	///  @param[in]   iAlign  
	///  @param[in]   textHorAlign  
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月24日，新建函数
	///
	StatisticsIndicateSymbol(const QStringList& strTextList, int iAlign = Qt::AlignTop, int textHorAlign = Qt::AlignLeft);
private:
	int             m_ciFontSize;                   // 字体大小
	QStringList     m_cstrTextList;                 // 文本列表（每行一个text）
	QFont           m_font;                         // 文本字体
};

// 统计分析悬浮提示标记
class StatisticsGraphicSelTipMarker : public QwtPlotMarker
{
public:
	StatisticsGraphicSelTipMarker();

	///
	///  @brief 设置文本
	///
	///
	///  @param[in]   strTextList  文本列表
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月24日，新建函数
	///
	void SetTipContent(const QStringList& strTextList);

	///
	/// @brief 设置标签对齐方式
	///     
	///
	/// @param[in]  enAlignment  对齐方式
	/// @param[in]  textAlign  文字在气泡中的对齐方式
	///
	/// @par History:
	///  @li 7656/zhang.changjiang，2023年7月24日，新建函数
	///
	void SetLabelAlignment(Qt::Alignment enAlignment, Qt::Alignment textAlign = Qt::AlignLeft);

protected:
	///
	/// @brief
	///     更新标记曲线显示
	///
	/// @par History:
	/// @li 4170/TangChuXian，2023年7月10日，新建函数
	///
	void UpdateMakerDisplay();

private:
	QStringList     m_strTextList;                  // 文本列表（每行一个text）
	Qt::Alignment   m_enAlign;                      // 对齐方式
	Qt::Alignment   m_textAlign;                    // 对齐方式
};
