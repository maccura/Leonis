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
/// @file     QSamplesDonutPie.h
/// @brief 	  饼状图
///
/// @author   7656/zhang.changjiang
/// @date      2023年7月18日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年7月18日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QVBoxLayout>

class QDonutPie;
namespace Ui { class QSamplesDonutPie; };
namespace tf { class SampleStatisPieData; };

class QSamplesDonutPie : public QWidget
{
	Q_OBJECT

public:
	enum SAMPLE_INDEX
	{
		INDEX_NORMAL_SAMPLE,			// 常规样本量
		INDEX_EMERGE_SAMPLE,			// 急诊样本量
		INDEX_QC_SAMPLE,				// 质控
		INDEX_CALI_SAMPLE,				// 校准
		INDEX_MAX						// 最大索引值
	};

	QSamplesDonutPie(QWidget *parent = Q_NULLPTR);
	~QSamplesDonutPie();
	
	///
	///  @brief 设置数据
	///
	///
	///  @param[in]   data  数据列表
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月19日，新建函数
	///
	void SetData(const QVector<double>& data);
	
	///
	/// @brief  设置数据
	///
	/// @param[in]  pieData  饼状图数据
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月7日，新建函数
	///
	void SetData(const ::tf::SampleStatisPieData &pieData);
	
	///
	///  @brief 根据索引值修改数据
	///
	///
	///  @param[in]   index  索引值
	///  @param[in]   num    数据
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月19日，新建函数
	///
	void SetDataByIndex(SAMPLE_INDEX index, const double num);

	///
	///  @brief 设置颜色
	///
	///
	///  @param[in]   colors  颜色列表
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月19日，新建函数
	///
	void SetColors(const QVector<QColor> &colors);
private:

	///
	///  @brief 显示前初始化
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月13日，新建函数
	///
	void InitBeforeShow();
	
private:
	Ui::QSamplesDonutPie *ui;
	QDonutPie *						m_pie;   // 饼状图
};

class QDonutPie : public QWidget {
    
    Q_OBJECT

public:
	explicit QDonutPie(QWidget *parent = nullptr);
	
	///
	///  @brief 设置边距
	///
	///
	///  @param[in]   iMargin  边距大小
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月19日，新建函数
	///
	void SetMargin(const int iMargin);
	
	///
	///  @brief 设置外圈半径
	///
	///
	///  @param[in]   iOuterRadius  外圈半径
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月19日，新建函数
	///
	void SetOuterRadius(const int iOuterRadius);
	
	///
	///  @brief 设置内圈半径
	///
	///
	///  @param[in]   iInnerRadius  内圈半径
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月19日，新建函数
	///
	void SetInnerRadius(const int iInnerRadius);
	
	///
	///  @brief 设置饼状图是否根据窗口大小自适应
	///
	///
	///  @param[in]   bAutoAdaption  （true 是 false 否）
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月19日，新建函数
	///
	void SetAutoAdaption(const bool bAutoAdaption);
	
	///
	///  @brief 设置内圈半径按照外圈半径比例显示
	///
	///
	///  @param[in]   bSetByOuter  
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月19日，新建函数
	///
	void SetByOuter(const bool bSetByOuter);
	
	///
	///  @brief 设置内外半径的比例
	///
	///
	///  @param[in]   rate  内外半径的比例
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月19日，新建函数
	///
	void SetInnerRadiusRate(const double rate);
	
	///
	///  @brief 设置圆心居中显示
	///
	///
	///  @param[in]   bCenterAutoAdaption  （true 居中, false 手动设置）
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月19日，新建函数
	///
	void SetCenterAutoAdaption(const bool bCenterAutoAdaption);
	
	///
	///  @brief 设置圆心
	///
	///
	///  @param[in]   center  圆心
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月19日，新建函数
	///
	void SetCenterPoint(const QPoint center);
	
	///
	///  @brief 设置数据
	///
	///
	///  @param[in]   data  数据列表
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月19日，新建函数
	///
	void SetData(const QVector<double>& data);
	
	///
	///  @brief 获取数据列表
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月19日，新建函数
	///
	QVector<double>& Getdata();

	///
	///  @brief 设置颜色
	///
	///
	///  @param[in]   colors  颜色列表
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月19日，新建函数
	///
	void SetColors(const QVector<QColor> &colors);
	
	///
	///  @brief 根据数据索引修改数据
	///
	///
	///  @param[in]   index  数据索引
	///  @param[in]   num    数据
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月19日，新建函数
	///
	void SetDataByIndex(const int index, const double num);

	///
	///  @brief 获取数据总量
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月19日，新建函数
	///
	double GetTotal();
	
	///
	/// @brief  检查是否只有一组数据
	///
	///
	/// @return true 只有一组 false 有多组数据
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年12月1日，新建函数
	///
	bool IsSingleType();

	///
	/// @brief  获取只有一组数据时的索引值
	///
	///
	/// @return 有数据的索引值
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年12月1日，新建函数
	///
	int GetSingleTypeIndex();
protected:
	void paintEvent(QPaintEvent *) override;

private:
	QLabel *			m_pOrderNumLabel;				// 总订单数数字
	QLabel *			m_pOrderNumLabelText;			// 总订单数文本
	QWidget *			m_pWidget;						// 饼状图中心空间
	QVector<double>		m_data;							// 饼状图数据
	int					m_iMargin;						// 饼状图的外边距
	int					m_iOuterRadius;					// 外圈半径
	int					m_iInnerRadius;					// 内圈半径
	double				m_dRate;						// 内外半径比例
	bool				m_bAutoAdaption;				// 是否根据widget的大小自动调整饼状图大小（true 是 false 否）
	bool				m_bSetByOuter;					// 是否根据外圈设置内圈比例（true 是 false 否）
	bool				m_bCenterAutoAdaption;			// 是否将圆心居中
	QPoint				m_center;						// 圆心;
	QVector<QColor>		m_colors;						// 曲线颜色
};
