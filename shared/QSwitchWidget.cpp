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
#include "QSwitchWidget.h"
#include "shared/QPageShellClass.h"
#include "QNavigationPage.h"
#include "src/common/Mlog/mlog.h"

QSwitchWidget::QSwitchWidget(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}

QSwitchWidget::~QSwitchWidget()
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
void QSwitchWidget::RegisterPage(int index, const QString& name, const QString& cornerbtnObjectName, QNavigationPage* page)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (page == nullptr)
    {
        return;
    }

    // 添加生化页面句柄
    ui.stackedWidget->insertWidget(index, page);

    // 记录页面名称和编号
    m_pageInfos.push_back(CornerButtonInfo(index, cornerbtnObjectName, name));

    connect(page, SIGNAL(ChangePage(int)), this, SLOT(ChangePage(int)));
    m_subPages.push_back(page);
}

///
/// @brief
///     页面切换槽函数
///
/// @param[in]  index  切换页面的Index
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月12日，新建函数
///
void QSwitchWidget::ChangePage(int index)
{
    // 越界判断
    if (index < TAB_INDEX_CHEMISTRY || index > TAB_INDEX_IMMUNITY)
    {
		ULOG(LOG_ERROR, "Invalid page index.");
        return;
    }

    ui.stackedWidget->setCurrentIndex(index);
}
