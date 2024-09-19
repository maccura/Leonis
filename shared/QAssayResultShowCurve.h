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
/// @file     QAssayResultShowCurve.h
/// @brief    测试结果的曲线展示
///
/// @author   5774/WuHongTao
/// @date     2022年5月30日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月30日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QWidget>
#include "ui_QAssayResultShowCurve.h"
#include "src/thrift/ch/gen-cpp/ch_types.h"


class QAssayResultShowCurve : public QWidget
{
	Q_OBJECT

public:
	QAssayResultShowCurve(QWidget *parent = Q_NULLPTR);
	~QAssayResultShowCurve();

	///
	/// @brief 显示项目反应过程
	///
	/// @param[in]  result  项目测试结果
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月30日，新建函数
	///
	void ShowAssayResultReactionProcess(ch::tf::AssayTestResult& result);

	///
	/// @brief 清除历史残留数据
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月30日，新建函数
	///
	void ClearContent();

	///
	/// @brief 设置校准品反应次数信息
	///
	/// @param[in]  items  项目列表
	///
	/// @return 
	///
	/// @par History:
	/// @li 8090/YeHuaNing，2022年9月13日，新建函数
	///
	void SetCalibrateAbstract(const QStringList& items);

	///
	/// @brief 设置默认视图比例
	///
	/// @par History:
	/// @li 8090/YeHuaNing，2022年9月13日，新建函数
	///
	void setDefaultViewScale();

protected:
	///
	/// @brief 初始化控件
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月30日，新建函数
	///
	void InitWidget();

	///
	/// @brief
	///     显示曲线的检测点
	///
	/// @param[in]  result  曲线检测点
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年3月2日，新建函数
	///
	//void ShowDetectPointView(ch::tf::AssayTestResult& result);

	///
	/// @brief 清空测光点的表格内容
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月30日，新建函数
	///
	//void ClearViewContent();

private slots:
	///
	/// @brief 最大吸光度的修改
	///
	/// @param[in]  absorb 最大吸光度的值  
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月30日，新建函数
	///
	void OnModifyMaxAbsorb(QString absorb);

	///
	/// @brief 最小吸光度的修改
	///
	/// @param[in]  absorb  最小吸光度的值
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月30日，新建函数
	///
	void OnModifyMinAbsorb(QString absorb);

	///
	/// @brief 坐标系自动跟随
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月30日，新建函数
	///
	void OnAutoBtnClicked();

	///
	/// @brief
	///     根据设置的手动数据更新坐标系
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年3月4日，新建函数
	///
	void OnManualBtnClicked();

private:
	Ui::QAssayResultShowCurve		ui;
};
