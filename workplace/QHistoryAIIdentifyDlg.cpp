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

#include "QHistoryAIIdentifyDlg.h"

QHistoryAIIdentifyDlg::QHistoryAIIdentifyDlg(QWidget *parent)
	: BaseDlg(parent)
{
	ui = new Ui::QHistoryAIIdentifyDlg();
	ui->setupUi(this);

	Init();
}

QHistoryAIIdentifyDlg::~QHistoryAIIdentifyDlg()
{
}

void QHistoryAIIdentifyDlg::Init()
{
	SetTitleName(tr("视觉识别"));

	connect(ui->button_ok, &QPushButton::clicked, this, &QHistoryAIIdentifyDlg::OnOkBtnClicked);
	connect(ui->button_cancel, &QPushButton::clicked, this, &QHistoryAIIdentifyDlg::OnCancelBtnClicked);
}

///
/// @brief 确定按钮点击
///
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年7月24日，新建函数
///
void QHistoryAIIdentifyDlg::OnOkBtnClicked()
{
	accept();
}

///
/// @brief 取消按钮点击
///
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年7月24日，新建函数
///
void QHistoryAIIdentifyDlg::OnCancelBtnClicked()
{
	reject();
}
