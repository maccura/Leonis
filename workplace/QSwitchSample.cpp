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

#include "QSwitchSample.h"
#include "shared/QPageShellClass.h"
#include "shared/QAssayTabWidgets.h"

QSwitchSample::QSwitchSample(QWidget *parent)
	: QSwitchWidget(parent)
{
	Init();
}

QSwitchSample::~QSwitchSample()
{
}

///
/// @brief
///     注册切换后的模块主页面
///
/// @param[in]  index  模块序号
/// @param[in]  page   页面句柄
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月18日，新建函数
///
void QSwitchSample::RegisterPage(int index, const QString& name, QAssayTabWidgets* page)
{
	if (page == nullptr)
	{
		return;
	}

	// 添加生化页面句柄
	ui.stackedWidget->insertWidget(index, page);
	// 记录页面名称和编号

	m_pageInfos.push_back(CornerButtonInfo(index, name, name));
	m_pages.push_back(page);
}

///
/// @brief 初始化页面信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月9日，新建函数
///
void QSwitchSample::Init()
{
	// 生化页面的句柄
	RegisterPage(0, "chemistry", new QAssayTabWidgets(this));

	// 依次设置页面
	for (auto page : m_pages)
	{
		page->SetDeviceSelect(m_pageInfos);
	}

	// 设置默认的显示句柄
	ui.stackedWidget->setCurrentIndex(0);
}
