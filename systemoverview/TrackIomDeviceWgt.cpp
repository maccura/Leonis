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
/// @file     TrackIomDeviceWgt.h
/// @brief    轨道IOM设备界面源文件
///
/// @author   7997/XuXiaoLong
/// @date     2023年7月12日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7997/XuXiaoLong，2023年7月12日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "TrackIomDeviceWgt.h"
#include "ui_TrackIomDeviceWgt.h"
#include <QMovie>

#define		WIDGT_W		90			//<	界面宽度
#define		WIDGT_H		420			//< 界面高度

TrackIomDeviceWgt::TrackIomDeviceWgt(const string &sn, const string &name, const EnumDeviceType &type, const int status, 
	const bool wasteLiquidEnable, const bool mask, QWidget *parent)
	: QBaseDeviceWgt(sn, name, type, status, wasteLiquidEnable, mask, true, parent)
{
    ui = new Ui::TrackIomDeviceWgt();
	ui->setupUi(this);

	// 透明背景
	this->setStyleSheet("background: transparent;");
	this->setFixedSize(WIDGT_W, WIDGT_H);
	QMovie* mv = new QMovie(SHORT_LIGHT_GRAY);
	ui->label_light->setMovie(mv);
	mv->start();

	ui->label_name->setText(QString::fromStdString(name));
	ui->label_devPicture->setPixmap(QPixmap(TRACK_IOM_PICTURE));
}

TrackIomDeviceWgt::~TrackIomDeviceWgt()
{

}

void TrackIomDeviceWgt::changeStatus()
{
	auto mv = ui->label_light->movie();
	if (!mv)
	{
		return;
	}
	mv->stop();

	// 获取状态信息
	auto statusInfo = GetDeviceStatusInfo(false);

	// 状态文字
	ui->label_status->setText(std::get<0>(statusInfo));

	// 状态灯
	mv->setFileName(std::get<1>(statusInfo));
	mv->start();
}
