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
/// @file     mcprogressdlg.cpp
/// @brief    进度条
///
/// @author   6889/ChenWei
/// @date     2024年1月22日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "mcprogressdlg.h"
#include "ui_mcprogressdlg.h"
#include "shared/uicommon.h"
#include "src/common/Mlog/mlog.h"

McProgressDlg::McProgressDlg(QWidget *parent)
    : BaseDlg(parent)
    , m_bShow(false)
{
    ui = new Ui::McProgressDlg();
    ui->setupUi(this);
    SetTitleName(tr("提示"));
    ui->progressBar->setValue(0);
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(100);

    // 安装事件过滤器
    ui->TitleLab->installEventFilter(this);
}

McProgressDlg::~McProgressDlg()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
    m_bShow = false;
    delete ui;
}

///
/// @brief
///     设置进度条模式
///
/// @param[in]    bIndicator: true:繁忙指示器模式，false:进度条模式
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
void McProgressDlg::SetModule(bool bIndicator)
{
    if (bIndicator)
    {
        ui->progressBar->setMinimum(0);
        ui->progressBar->setMaximum(0);
    }
    else
    {
        ui->progressBar->setMinimum(0);
        ui->progressBar->setMaximum(100);
    }
}

///
/// @brief
///     设置进度条提示信息
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
void McProgressDlg::SetProgressTitle(QString strTitle)
{
    ui->TitleLab->setText(strTitle);
    ui->progressBar->reset();
    ui->progressBar->setValue(0);
    m_bShow = true;
}

///
/// @brief
///     刷新进度
///
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
void McProgressDlg::UpdateProgressRate(int iPer)
{
    if (m_bShow)
    {
        ui->progressBar->setValue(iPer);
    }
}

void McProgressDlg::showEvent(QShowEvent *event)
{
    BaseDlg::showEvent(event);
    m_bShow = true;
}

void McProgressDlg::closeEvent(QCloseEvent *event)
{
    ui->progressBar->setValue(0);
    ui->TitleLab->setText("");
    m_bShow = false;
    BaseDlg::closeEvent(event);
}

///
/// @brief
///     事件过滤器
///
/// @param[in]  obj    目标对象
/// @param[in]  event  事件
///
/// @return true表示已处理
///
/// @par History:
/// @li 6889/ChenWei，2022年5月17日，新建函数
///
bool McProgressDlg::eventFilter(QObject *obj, QEvent *event)
{
    if ((obj == ui->TitleLab) && (event->type() == QEvent::Resize))
    {
        resize(470, 218 + ui->TitleLab->height() - 25);
    }

    return false;
}
