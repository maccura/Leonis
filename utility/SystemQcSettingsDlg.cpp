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
/// @file     SystemQcSettingsDlg.h
/// @brief    应用->系统->质控设置框
///
/// @author   8276/huchunli
/// @date     2022年10月9日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8276/huchunli，2022年10月9日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "SystemQcSettingsDlg.h"
#include "ui_SystemQcSettingsDlg.h"
#include "qboxlayout.h"
#include "qlabel.h"
#include "qlineedit.h"


SystemQcSettingsDlg::SystemQcSettingsDlg(QWidget *parent /* = Q_NULLPTR */)
    :BaseDlg(parent)
    , ui(new Ui::SystemQcSettingsDlg)
{
    ui->setupUi(this);
    InitCtrls();
}

SystemQcSettingsDlg::~SystemQcSettingsDlg()
{}

void SystemQcSettingsDlg::InitCtrls()
{
    SetTitleName(tr("质控设置"));
    QVBoxLayout *mainLayout = new QVBoxLayout;

    // 批质控时间设置布局
    QHBoxLayout *batchQcTimeLayout = new QHBoxLayout;
    QLabel *labelTime1 = new QLabel(tr("联合质控\n批时间限"));
    QLabel *labelTime2 = new QLabel(tr("小时"));
    QLineEdit *editTime = new QLineEdit;

    batchQcTimeLayout->addWidget(labelTime1, 1);
    batchQcTimeLayout->addWidget(editTime, 1);
    batchQcTimeLayout->addWidget(labelTime2, 1);
    mainLayout->addLayout(batchQcTimeLayout);
    mainLayout->addWidget(new QLabel(tr("自动质控")));
    m_pContentFrame->setLayout(mainLayout);
}
