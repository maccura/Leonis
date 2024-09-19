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
/// @file     QSpecialProgessBar.h
/// @brief    显示进度条
///
/// @author   5774/WuHongTao
/// @date     2022年3月14日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月14日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QSpecialProgessBar.h"
#include "ui_QSpecialProgessBar.h"

QSpecialProgessBar::QSpecialProgessBar(QWidget *parent)
    : QWidget(parent)
{
    ui = new Ui::QSpecialProgessBar();
    ui->setupUi(this);
    // 设置范围
    ui->progressBar->setRange(0, 100);
}

QSpecialProgessBar::~QSpecialProgessBar()
{
}

///
/// @brief
///     设置进度条的信息
///
/// @param[in]  status  进度条信息
///
/// @return 
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月14日，新建函数
///
void QSpecialProgessBar::SetProgressBar(const ProgressStatus& status)
{
    // 设置标题
    ui->Title_Label->setText(QString::fromStdString(status.title));
    // 具体显示的数据
    ui->BarData_Label->setText(QString::number(status.progressData));
    // 显示具体进度
    ui->progressBar->setValue(status.progress);

    // 根据状态设置显示
    switch (status.status)
    {
    case QSpecialProgessBar::INIT:
        break;
    case QSpecialProgessBar::NORMAL:
        break;
    case QSpecialProgessBar::WARING:
        break;
    case QSpecialProgessBar::ERRORSTATE:
        break;
    case QSpecialProgessBar::END:
        break;
    default:
        break;
    }
}
