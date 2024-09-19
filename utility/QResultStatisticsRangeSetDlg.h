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
/// @file     QResultStatisticsRangeSetDlg.h
/// @brief 	  应用->统计分析->结果统计->结果分段设置
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
#include "shared/basedlg.h"
#include "src/thrift/gen-cpp/statis_types.h"
#include <QComboBox>
#include <QSpinBox>

namespace Ui { class QResultStatisticsRangeSetDlg; };
namespace tf { struct RangQueryCond; };

class QResultStatisticsRangeSetDlg : public BaseDlg
{
	Q_OBJECT

public:
	QResultStatisticsRangeSetDlg(QWidget *parent = Q_NULLPTR);
	~QResultStatisticsRangeSetDlg();
	virtual void showEvent(QShowEvent *event) override;
public:
    ///
    /// @brief  设置参数范围选中
    ///
    /// @param[in]    void
    ///
    /// @return void
    ///
    /// @par History:
    /// @li 7915/LeiDingXiang，2023年9月14日，新建函数
    ///
    void SetSpinBoxFocus();
	
	///
	/// @brief  获取分段条件列表
	///
	///
	/// @return 分段条件列表
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月15日，新建函数
	///
	std::vector<::tf::RangQueryCond> GetRangQueryConds();

private:

	///
	///  @brief 显示前初始化
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月17日，新建函数
	///
	void InitBeforeShow();
	
	///
	/// @brief  获取区间结构体
	///
	/// @param[in]  lc  左区间符号控件
	/// @param[in]  lv  右区间符号控件
	/// @param[in]  rc  区间左值
	/// @param[in]  rv  区间右值
	///
	/// @return 区间结构
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月14日，新建函数
	///
	::tf::RangQueryCond GetRangQueryCond(QComboBox *lc, QDoubleSpinBox *lv, QComboBox *rc, QDoubleSpinBox *rv);

	///
	/// @brief  创建区间列表
	///
	///
	/// @return 区间列表
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月14日，新建函数
	///
	std::vector<::tf::RangQueryCond> CreateRangQueryCondVec();
	
	///
	/// @brief  找出非法区间
	///
	/// @param[in]  vecRangQueryCond  区间列表
	///
	/// @return 非法区间
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月16日，新建函数
	///
	std::map<int, ::tf::RangQueryCond> FindInvalidRanges(std::vector<::tf::RangQueryCond>& vecRangQueryCond);

	///
	/// @brief  判断两个区间是否有交集
	///
	/// @param[in]  range1 区间1 
	/// @param[in]  range2 区间2
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月14日，新建函数
	///
	bool HasIntersection(const ::tf::RangQueryCond& range1, const ::tf::RangQueryCond& range2);

	///
	/// @brief  找到所有有交集的区间并返回
	///
	/// @param[in]  vecRangQueryCond  区间列表
	///
	/// @return 有交集的区间
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月16日，新建函数
	///
	std::map<int, ::tf::RangQueryCond> FindIntersectingRanges(const std::vector<::tf::RangQueryCond>& vecRangQueryCond);

	///
	/// @brief  显示上次设置的区间
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月15日，新建函数
	///
	void ShowLastSettingRange();
	
	///
	/// @brief  判断两个浮点数是否相等
	///
	/// @param[in]  f1  浮点数1
	/// @param[in]  f2  浮点数2
	///
	/// @return true 相等 false 不等
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月28日，新建函数
	///
	bool FloatIsEqual(double f1, double f2);
private slots:
	///
	/// @brief  确定按钮被点击
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月15日，新建函数
	///
	void OnOkBtnClicked();
private:
	Ui::QResultStatisticsRangeSetDlg *ui;

	std::vector<::tf::RangQueryCond>			m_vecRangQueryCond;    // 分段条件列表
};
