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
/// @file     QResultActionCurve.h
/// @brief    反应曲线
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

#include <map>
#include "shared/basedlg.h"
#include "src/thrift/ch/c1005/gen-cpp/c1005_ui_control_constants.h"

namespace Ui {
    class QResultActionCurve;
};


class QResultActionCurve : public BaseDlg
{
	Q_OBJECT

public:
	QResultActionCurve(QWidget *parent = Q_NULLPTR);
	~QResultActionCurve();

	///
	/// @brief 显示项目反应曲线等信息
	///
	/// @param[in]  testItem  项目信息
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月30日，新建函数
	///
	void ShowAssayCurve(tf::TestItem& testItem);

private slots:
	///
	/// @brief 显示常规曲线信息
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月30日，新建函数
	///
	void OnShowRegularCurveInfo();

	///
	/// @brief 显示复查曲线信息
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月30日，新建函数
	///
	void OnShowRecheckCurveInfo();

private:
	Ui::QResultActionCurve*			ui;
	std::map<int, ::ch::tf::AssayTestResult> m_resultContainer; ///< 常规结果和复查结果的保存数据结构
};
