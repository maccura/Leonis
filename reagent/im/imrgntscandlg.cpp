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
/// @file     imrgntscandlg.cpp
/// @brief    免疫试剂筛选对话框
///
/// @author   4170/TangChuXian
/// @date     2022年11月7日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2022年11月7日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "imrgntscandlg.h"
#include "ui_imrgntscandlg.h"
#include "src/common/Mlog/mlog.h"

ImRgntScanDlg::ImRgntScanDlg(QWidget *parent)
    : BaseDlg(parent),
      m_bInit(false)
{
    ui = new Ui::ImRgntScanDlg();
    ui->setupUi(this);

    // 显示前初始化
    InitBeforeShow();
}

ImRgntScanDlg::~ImRgntScanDlg()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
    delete ui;
}

///
/// @brief
///     显示之前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2022年11月7日，新建函数
///
void ImRgntScanDlg::InitBeforeShow()
{
    SetTitleName(tr("试剂扫描"));
    ui->SpecPosEdit->setEnabled(false);
}

///
/// @brief
///     显示之后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2022年11月7日，新建函数
///
void ImRgntScanDlg::InitAfterShow()
{
    // 连接信号槽
    connect(ui->OkBtn, SIGNAL(clicked()), this, SIGNAL(SigOkBtnClicked()));
    connect(ui->CancelBtn, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->ScanSpecPosRBtn, SIGNAL(toggled(bool)), this, SLOT(OnScanMethodChanged()));
    connect(ui->ScanAllRBtn, SIGNAL(toggled(bool)), this, SLOT(OnScanMethodChanged()));
}

///
/// @brief
///     重写显示事件
///
/// @param[in]  event  事件
///
/// @par History:
/// @li 4170/TangChuXian，2022年11月7日，新建函数
///
void ImRgntScanDlg::showEvent(QShowEvent* event)
{
    // 基类处理
    QWidget::showEvent(event);

    // 如果是第一次显示则初始化
    if (!m_bInit)
    {
        // 显示后初始化
        InitAfterShow();
        m_bInit = true;
    }
}

///
/// @brief
///     扫描方式改变
///
/// @par History:
/// @li 4170/TangChuXian，2022年11月7日，新建函数
///
void ImRgntScanDlg::OnScanMethodChanged()
{
    // 根据扫描方式更新编辑焦点
    if (ui->ScanAllRBtn->isChecked())
    {
        ui->SpecPosEdit->setEnabled(false);
    }
    else
    {
        ui->SpecPosEdit->setEnabled(true);
        ui->SpecPosEdit->setFocus();
    }
}
