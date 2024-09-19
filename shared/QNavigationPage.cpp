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
/// @file     QNavigationPage.h
/// @brief    导航界面
///
/// @author   5774/WuHongTao
/// @date     2022年2月18日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月18日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QNavigationPage.h"
#include "shared/QPageShellClass.h"
#include "src/common/Mlog/mlog.h"

QNavigationPage::QNavigationPage(QWidget *parent)
    : QWidget(parent)
{
    ui = new Ui::QNavigationPage();
    ui->setupUi(this);
    m_status = std::make_pair(-1, false);
    // 页面切换时候，切换右上角小控件/确定过滤器是否显示
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, [&](int index)
    {
        // 刷新右上角控件
        RefreshCornerWidgets(index);
        // 刷新过滤器
        if (m_status.first == index)
        {
            SetShowFilterCond(m_status.second);
        }
        else
        {
            SetShowFilterCond(false);
        }
    });

    // 点击关闭
    connect(ui->QryCondTipWgt, &QryCondDisplayWgt::SigResetQry, this, [=]()
    {
//         ui->label->hide(); 
//         ui->icon_small_close->hide();

        ui->QryCondTipWgt->hide();
        // 清除
        m_status = std::make_pair(-1, false);
    });

    // 设置筛选条件默认不显示
    SetShowFilterCond(false);
}

QNavigationPage::~QNavigationPage()
{
}

///
/// @brief
///     设置修改过滤条件
///
/// @param[in]  cond  条件是否显示
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月11日，新建函数
///
void QNavigationPage::SetShowFilterCond(bool cond)
{
    int count = ui->tabWidget->tabBar()->count();
    int width = ui->tabWidget->tabBar()->tabRect(count - 1).x() + ui->tabWidget->tabBar()->tabRect(count - 1).width();
    if (cond)
    {
//         ui->label->move(width + 5, 20);
//         ui->icon_small_close->move(width + ui->label->width(), 20);
//         ui->icon_small_close->show();
//         ui->label->show();

        ui->QryCondTipWgt->move(width + 20, 30);
        ui->QryCondTipWgt->show();
    }
    else
    {
        // 平时关闭
//         ui->icon_small_close->hide();
//         ui->label->hide();

        ui->QryCondTipWgt->hide();
    }
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
void QNavigationPage::RefreshCornerWidgets(int index)
{
	Q_UNUSED(index);
	return;
}

///
/// @brief
///     显示提示消息
///
/// @param[in]  message  提示消息
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月11日，新建函数
///
void QNavigationPage::OnFilterCondNumber(QString message)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__, message);
    // 临时处理（后续可能需要tooltip单独的接口）
	auto messageVec =  message.split("|");
	if (messageVec.empty())
	{
		return;
	}

    ui->QryCondTipWgt->SetText(messageVec[0]);
	if (messageVec.size() >= 2)
	{
        // 显示提示消息
        Qt::TextElideMode elideMode = Qt::ElideRight;
        ui->QryCondTipWgt->SetToolTip(messageVec[1]);
	}

    // 显示控件
    SetShowFilterCond(true);
    auto index = ui->tabWidget->currentIndex();
    m_status = std::make_pair(index, true);
}

///
/// @brief 隐藏快捷筛选标签
///
///
/// @return 
///
/// @par History:
/// @li 1556/Chenjianlin，2023年8月16日，添加注释
///
void QNavigationPage::OnHideFilterLable()
{
    auto index = ui->tabWidget->currentIndex();
    m_status = std::make_pair(index, false);
    SetShowFilterCond(false);
}
