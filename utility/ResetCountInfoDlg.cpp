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
/// @file     ResetCountInfoDlg.cpp
/// @brief    应用-计数-重置
///
/// @author   7951/LuoXin
/// @date     2024年1月26日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2024年1月26日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "ResetCountInfoDlg.h"
#include "ui_ResetCountInfoDlg.h"

ResetCountInfoDlg::ResetCountInfoDlg(QWidget *parent /*= Q_NULLPTR*/)
    : BaseDlg(parent)
    , ui(new Ui::ResetCountInfoDlg)
{
    ui->setupUi(this);

    SetTitleName(tr("重置"));
}

ResetCountInfoDlg::~ResetCountInfoDlg()
{

}

void ResetCountInfoDlg::showDlg(const QString& name, const QString& sn)
{
    ui->name_edit->setText(name);
    ui->sn_edit->setText(sn);
    show();
}

void ResetCountInfoDlg::GetData(QString& name, QString& sn)
{
    name = ui->name_edit->text();
    sn = ui->sn_edit->text();
    close();
}

QPushButton* ResetCountInfoDlg::GetSaveBtnPtr()
{
    return ui->save_btn;
}
