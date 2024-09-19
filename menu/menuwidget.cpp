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
/// @file     menuwidget.h
/// @brief    菜单模块
///
/// @author   5774/WuHongTao
/// @date     2021年6月2日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2021年6月2日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "menuwidget.h"

#include "QApplication"
#include <QPushButton>

#include "src/common/Mlog/mlog.h"

///
/// @brief
///     菜单模块的构造函数
///
/// @param[in]  parent  父模块
///
/// @par History:
/// @li 5774/WuHongTao，2021年6月2日，新建函数
///
MenuWidget::MenuWidget(QWidget *parent)
    : QWidget(parent)
{
    m_pExitBtn = new QPushButton(this);
    m_pExitBtn->setText(tr("Exit"));
    connect(m_pExitBtn, SIGNAL(clicked()), this, SLOT(OnExitBtnClicked()));
}

///
/// @brief
///     析构函数
///
/// @par History:
/// @li 5774/WuHongTao，2021年6月2日，新建函数
///
MenuWidget::~MenuWidget()
{
}

void MenuWidget::SetExitBtnEnable(bool bEnable)
{
	m_pExitBtn->setEnabled(bEnable);
}

///
/// @brief
///     退出按钮的响应函数
///
///
/// @return 
///
/// @par History:
/// @li 5774/WuHongTao，2021年6月2日，新建函数
///
void MenuWidget::OnExitBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    QApplication::quit();
}
