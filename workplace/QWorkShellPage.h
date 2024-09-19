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

#pragma once

#include <QWidget>

class QTabWidget;


class QWorkShellPage : public QWidget
{
	Q_OBJECT

public:
	QWorkShellPage(QWidget *parent);
	~QWorkShellPage();

	///
	/// @brief
	///     刷新右上角按钮
	///
	/// @param[in]  index  刷新页面index
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年1月12日，新建函数
	///
	virtual void RefreshCornerWidgets(int index);

signals:
	void ChangePage(int index);

	///
	/// @brief
	///     显示提示消息
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年1月13日，新建函数
	///
	void ShowTipMessage(QString msg);

	///
	/// @brief
	///     关闭查询筛选提示条
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年3月14日，新建函数
	///
	void CloseFilterLable();

	void SignalHideFilterLable();
protected:
	QTabWidget*                     m_parent;                   ///< 导航页面句柄
};
