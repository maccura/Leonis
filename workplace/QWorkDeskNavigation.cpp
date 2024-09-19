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
/// @file     QWorkDeskNavigation.h
/// @brief    工作页面导航
///
/// @author   5774/WuHongTao
/// @date     2022年5月25日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月25日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "QWorkDeskNavigation.h"
#include "ui_QNavigationPage.h"
#include "QSampleAssaySelect.h"
#include "QSampleDataBrowse.h"
#include "QAbnormalSample.h"
#include "QSampleRackMonitors.h"
#include "QWorkShellPage.h"
#include "QHistory.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "src/common/Mlog/mlog.h"

#define TAB_INDEX_ASSAY_SELECT              0            // 项目选择tab索引
#define TAB_INDEX_DATA_BROWSE               1            // 数据浏览tab索引
#define TAB_INDEX_HIS_DATA                  2            // 历史数据tab索引
#define TAB_INDEX_RACK_MONITOR              3            // 样本架监控


QWorkDeskNavigation::QWorkDeskNavigation(QWidget *parent)
	: QNavigationPage(parent)
{
	Init();
}

QWorkDeskNavigation::~QWorkDeskNavigation()
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
void QWorkDeskNavigation::SetPageInfos(const std::vector<CornerButtonInfo>& pageInfos)
{
	Q_UNUSED(pageInfos);
	return;
}

void QWorkDeskNavigation::showPageDataBrowse()
{
	ui->tabWidget->setCurrentIndex(TAB_INDEX_DATA_BROWSE);
}

void QWorkDeskNavigation::showSampleRackMonitor()
{
	ui->tabWidget->setCurrentIndex(TAB_INDEX_RACK_MONITOR);
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
void QWorkDeskNavigation::RefreshCornerWidgets(int index)
{
	if (qobject_cast<QWorkShellPage*>(ui->tabWidget->currentWidget()) != nullptr)
	{
		qobject_cast<QWorkShellPage*>(ui->tabWidget->currentWidget())->RefreshCornerWidgets(index);
		QString tabText = ui->tabWidget->tabText(index);
		//POST_MESSAGE(MSG_ID_CURRENT_MODEL_NAME, tr("> 工作 > ") + tabText);
	}
}

///
/// @brief 初始化页面
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月25日，新建函数
///
void QWorkDeskNavigation::Init()
{
	m_handleMap.clear();

	ui->tabWidget->setObjectName(QStringLiteral("worktTabWidget"));

 	QWorkShellPage* page1 = new QSampleAssaySelect(ui->tabWidget);
 	m_handleMap.insert(std::make_pair(TAB_INDEX_ASSAY_SELECT, page1));
 
 	QWorkShellPage* page2 = new QSampleDataBrowse(ui->tabWidget);
 	// 传递筛选信息
 	connect(page2, SIGNAL(ShowTipMessage(QString)), this, SLOT(OnFilterCondNumber(QString)));
 	// 点击关闭
 	connect(ui->QryCondTipWgt, SIGNAL(SigResetQry()), page2, SIGNAL(CloseFilterLable()));
 	m_handleMap.insert(std::make_pair(TAB_INDEX_DATA_BROWSE, page2));
	connect(page2, SIGNAL(SignalHideFilterLable()), this, SLOT(OnHideFilterLable()));

	QWorkShellPage* page3 = new QHistory(ui->tabWidget);
	m_handleMap.insert(std::make_pair(TAB_INDEX_HIS_DATA, page3));
	// 传递筛选信息
	connect(page3, SIGNAL(ShowTipMessage(QString)), this, SLOT(OnFilterCondNumber(QString)));
	// 点击关闭
	connect(ui->QryCondTipWgt, SIGNAL(SigResetQry()), page3, SIGNAL(CloseFilterLable()));
	connect(page3, SIGNAL(SignalHideFilterLable()), this, SLOT(OnHideFilterLable()));

    QWorkShellPage* page4 = new QSampleRackMonitors(ui->tabWidget);
    m_handleMap.insert(std::make_pair(TAB_INDEX_RACK_MONITOR, page4));

    QStringList tableNameList = { tr("样本申请"), tr("数据浏览"), tr("历史数据"), tr("样本架监视") };
	for (const auto& iter : m_handleMap)
	{
		ui->tabWidget->insertTab(iter.first, iter.second, tableNameList[iter.first]);
	}
}
