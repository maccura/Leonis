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
/// @file     QQcNavigation.h
/// @brief    质控导航
///
/// @author   5774/WuHongTao
/// @date     2022年1月10日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年1月10日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include "shared/QNavigationPage.h"

struct CornerButtonInfo;


class QQcNavigation : public QNavigationPage
{
    Q_OBJECT

public:
	QQcNavigation(QWidget *parent = Q_NULLPTR);
    ~QQcNavigation();

	enum class QcPageIndex
	{
		QCAPPLICATION,		// 质控申请
		LEVEYCHART,			// levey-Jennings图
		TWINCHART,			// Twin-Plot图
		QCSETTING			// 质控设置
	};

    ///
    /// @brief
    ///     设置页面需要显示的按钮等，名称和对应的切换Index
    ///  
    /// @param[in]  pageInfos  按钮信息队列
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月12日，新建函数
    ///
    virtual void SetPageInfos(const std::vector<CornerButtonInfo>& pageInfos) override;

protected slots:
	///
	/// @brief
	///     刷新右上角按钮
	///
	/// @param[in]  index  刷新页面index
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年1月12日，新建函数
	///
	void RefreshCornerWidgets(int index) override;

private:

    ///
    /// @brief
    ///     注册功能页面
    ///
    /// @param[in]  type  页面类型
    /// @param[in]  name  页面名称
    /// @param[in]  pageInfos  页面信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月21日，新建函数
    ///
    void RegisterPageShell(QcPageIndex type, const QString& name, const std::vector<CornerButtonInfo>& pageInfos);
};
