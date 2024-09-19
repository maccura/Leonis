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
/// @file     QSwitchWidget.h
/// @brief    页面切换选择
///
/// @author   5774/WuHongTao
/// @date     2022年2月12日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月12日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QSwitchReagentWidget.h"
#include "reagent/QReagentNavigation.h"
#include "src/common/Mlog/mlog.h"

QSwitchReagentWidget::QSwitchReagentWidget(QWidget *parent)
    : QSwitchWidget(parent)
{
    Init();
}

QSwitchReagentWidget::~QSwitchReagentWidget()
{
}

void QSwitchReagentWidget::showReagentPlatePage(const std::string &devSn)
{
	ULOG(LOG_INFO, "%s(): devSn: %s", __FUNCTION__, devSn);

	if (ui.stackedWidget->count() <= 0)
	{
		return;
	}

	auto reagentWgt = qobject_cast<QReagentNavigation*>(ui.stackedWidget->widget(TAB_INDEX_CHEMISTRY));
	if (reagentWgt)
	{
		reagentWgt->showReagentPlatePage(devSn);
	}
}

QString QSwitchReagentWidget::getTabText()
{
	if (ui.stackedWidget->count() <= 0)
	{
		return "";
	}

	auto reagentWgt = qobject_cast<QReagentNavigation*>(ui.stackedWidget->widget(TAB_INDEX_CHEMISTRY));
	if (reagentWgt)
	{
		return reagentWgt->getTabText();
	}

	return "";
}

///
/// @brief
///     页面初始化
///
///
/// @return 
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月12日，新建函数
///
void QSwitchReagentWidget::Init()
{
    RegisterPage(TAB_INDEX_CHEMISTRY, tr(""), QStringLiteral(""), new QReagentNavigation(this));

    // 依次设置页面
    for (auto page : m_subPages)
    {
        page->SetPageInfos(m_pageInfos);
    }

    // 设置默认的显示句柄
    ui.stackedWidget->setCurrentIndex(TAB_INDEX_CHEMISTRY);
}
