﻿/***************************************************************************
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
/// @file     QReagentNavigation.h
/// @brief    试剂导航
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
#include "QReagentNavigation.h"
#include "ui_QNavigationPage.h"
#include "src/common/Mlog/mlog.h"
#include "shared/QPageShellClass.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "QReagentShellClass.h"

QReagentNavigation::QReagentNavigation(QWidget *parent)
    : QNavigationPage(parent)
{

}

QReagentNavigation::~QReagentNavigation()
{
}

///
/// @brief
///     设置页面需要显示的按钮等，名称和对应的切换Index
///  
/// @param[in]  pageInfos  按钮信息队列
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月12日，新建函数
///
void QReagentNavigation::SetPageInfos(const std::vector<CornerButtonInfo>& pageInfos)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 1:试剂总览
    //RegisterPageShell(REAGENTOVERVIEW, tr("试剂总览"), pageInfos);

    // 2:试剂盘
    RegisterPageShell(REAGENTPLATE, tr("试剂耗材"), pageInfos);
    // 3:试剂列表
    RegisterPageShell(REAGENTLIST, tr("试剂信息"), pageInfos);
    // 4:耗材列表
    RegisterPageShell(SUPPLIESLIST, tr("耗材信息"), pageInfos);

	connect(ui->tabWidget, &QTabWidget::currentChanged, this, [=](int idx) {
		QString tabText = ui->tabWidget->tabText(idx);
		POST_MESSAGE(MSG_ID_CURRENT_MODEL_NAME, tr("> 试剂 > ") + tabText);
	});

	ui->tabWidget->setCurrentIndex(REAGENTPLATE);
}

void QReagentNavigation::showReagentPlatePage(const std::string &devSn)
{
	ULOG(LOG_INFO, "%s(): devSn: %s", __FUNCTION__, devSn);

	auto reagentWgt = qobject_cast<QReagentShellClass*>(ui->tabWidget->widget(REAGENTPLATE));
	if (reagentWgt)
	{
		reagentWgt->SetSelectByDevSn(devSn);
	}

	ui->tabWidget->setCurrentIndex(REAGENTPLATE);
}

QString QReagentNavigation::getTabText()
{
	return ui->tabWidget->tabText(ui->tabWidget->currentIndex());
}

///
/// @brief
///     刷新右上角按钮
///
/// @param[in]  index  刷新页面index
///
/// @par History:
/// @li 5774/WuHongTao，2022年1月12日，新建函数
///
void QReagentNavigation::RefreshCornerWidgets(int index)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (qobject_cast<QPageShellClass*>(ui->tabWidget->currentWidget()) != nullptr)
	{
		qobject_cast<QPageShellClass*>(ui->tabWidget->currentWidget())->RefreshCornerWidget(index);
	}
}

///
/// @brief
///     注册功能页面
///
/// @param[in]  type  页面类型
/// @param[in]  name  页面名称
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月21日，新建函数
///
void QReagentNavigation::RegisterPageShell(PAGENAVIGATION type, const QString& name, const std::vector<CornerButtonInfo>& pageInfos)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    QReagentShellClass *pageShell = new QReagentShellClass(ui->tabWidget, type);
    // 传递连接消息
    connect(pageShell, SIGNAL(ShowTipMessage(QString)), this, SLOT(OnFilterCondNumber(QString)));
    connect(pageShell, SIGNAL(HideFilterLable()), this, SLOT(OnHideFilterLable()));
    // 点击关闭
    connect(ui->QryCondTipWgt, SIGNAL(SigResetQry()), pageShell, SIGNAL(CloseFilterLable()));
    ui->tabWidget->insertTab(type, pageShell, QString(name));
    pageShell->SetCornerButtonWidget(pageInfos);
    // 信号传递
    connect(pageShell, SIGNAL(ChangePage(int)), this, SIGNAL(ChangePage(int)));
}