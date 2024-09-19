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
/// @file     QCaliBrationNavigation.h
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
#include "QCaliBrationNavigation.h"
#include "shared/QNavigationPage.h"
#include "shared/CommonInformationManager.h"
#include "calibration/ch/QCaliBrationShellClass.h"
#include "src/common/Mlog/mlog.h"
#include "QPainter"

QCaliBrationNavigation::QCaliBrationNavigation(QWidget *parent)
    : QNavigationPage(parent)
{
}

QCaliBrationNavigation::~QCaliBrationNavigation()
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
void QCaliBrationNavigation::SetPageInfos(const std::vector<CornerButtonInfo>& pageInfos)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 1:校准概况
    RegisterPageShell(CALIBRATEOVERVIEW, tr("校准申请"), pageInfos);
    // 2：校准历史
    RegisterPageShell(CALIBRATEHIS, tr("校准历史"), pageInfos);
    // 3：校准设置
    RegisterPageShell(CALIBRATESET, tr("校准设置"), pageInfos);
    emit ui->tabWidget->currentChanged(CALIBRATEHIS);
}

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
void QCaliBrationNavigation::RegisterPageShell(CALIBRATEPAGENAVIGATION type, const QString& name, const std::vector<CornerButtonInfo>& pageInfos)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    QCaliBrationShellClass *pageShell = new QCaliBrationShellClass(ui->tabWidget, type);
    // 传递连接消息
    connect(pageShell, SIGNAL(ShowTipMessage(QString)), this, SLOT(OnFilterCondNumber(QString)));
    // 点击关闭
    connect(ui->QryCondTipWgt, SIGNAL(SigResetQry()), pageShell, SIGNAL(CloseFilterLable()));
    ui->tabWidget->insertTab(type, pageShell, name);
    pageShell->SetCornerButtonWidget(pageInfos);
    // 信号传递
    connect(pageShell, SIGNAL(ChangePage(int)), this, SIGNAL(ChangePage(int)));
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
void QCaliBrationNavigation::RefreshCornerWidgets(int index)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (static_cast<QCaliBrationShellClass*>(ui->tabWidget->currentWidget()) != nullptr)
	{
		static_cast<QCaliBrationShellClass*>(ui->tabWidget->currentWidget())->RefreshCornerWidget(index);
	}
}
