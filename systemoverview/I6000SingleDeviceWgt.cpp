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
/// @file     I6000SingleDeviceWgt.cpp
/// @brief    I6000 单机版设备界面源文件
///
/// @author   7997/XuXiaoLong
/// @date     2023年8月15日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7997/XuXiaoLong，2023年8月15日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "I6000SingleDeviceWgt.h"
#include "ui_I6000SingleDeviceWgt.h"
#include <QMovie>

#define		WIDGT_W		1820		//<	界面宽度
#define		WIDGT_H		500			//< 界面高度

I6000SingleDeviceWgt::I6000SingleDeviceWgt(const string &sn, const string &name, const EnumDeviceType &type,
	const int status, const bool wasteLiquidEnable, const bool mask, const bool rgtAlarmReaded, QWidget *parent)
	: QBaseDeviceWgt(sn, name, type, status, wasteLiquidEnable, mask, rgtAlarmReaded, parent)
{
    ui = new Ui::I6000SingleDeviceWgt();
	ui->setupUi(this);

	// 透明背景
	this->setStyleSheet("background: transparent;");
	this->setFixedSize(WIDGT_W, WIDGT_H);
	ui->label_name_im->setText(QString::fromStdString(name));
	ui->label_name_iom->setText("IOM");
	ui->label_devPicture->setPixmap(QPixmap(I6000_SINGLE_PICTURE));
	QMovie* mv = new QMovie(LONG_LIGHT_GRAY);
	ui->label_light->setMovie(mv);
	mv->start();

	ui->widget_reagent->installEventFilter(this);
	ui->widget_supply->installEventFilter(this);
	ui->widget_garbage->installEventFilter(this);
	ui->widget_liquid_waste->installEventFilter(this);
	ui->label_status_im->installEventFilter(this);

	ui->widget_liquid_waste->setVisible(wasteLiquidEnable);

	connect(ui->btn_iom, &QPushButton::clicked, this, [=] { emit signalSampleRackMonitor(); });
}

I6000SingleDeviceWgt::~I6000SingleDeviceWgt()
{
}

void I6000SingleDeviceWgt::changeStatus()
{
	auto mv = ui->label_light->movie();
	if (!mv)
	{
		return;
	}
	mv->stop();

	// 获取状态信息
	auto statusInfo = GetDeviceStatusInfo(true);

	// 状态文字
	ui->label_status_im->setText(std::get<0>(statusInfo));

	// 状态灯
	mv->setFileName(std::get<1>(statusInfo));
	mv->start();

	// 倒计时
	ui->label_time->setVisible(std::get<2>(statusInfo));

	// iom状态文字
	ui->label_status_iom->setText(GetIomStatusText(m_iomStatus));
}

void I6000SingleDeviceWgt::upadteBottle(const QMap<EnumBottlePos, EnumWarnLevel> &data)
{
	// 先全部恢复正常
	ui->label_reagent->setPixmap(QPixmap(IM_SINGLE_REAGENT_GREEN));
	ui->label_supply->setPixmap(QPixmap(IM_SINGLE_SUPPLY_GREEN));
	ui->label_liquid_waste->setPixmap(QPixmap(IM_SINGLE_LIQUID_WASTE_GREEN));
	ui->label_garbage->setPixmap(QPixmap(IM_SINGLE_TRASH_CAN_GREEN));

	if (data.empty())
	{
		return;
	}

	// 遍历有报警的瓶子
	for (auto it = data.begin(); it != data.end(); ++it)
	{
		switch (it.key())
		{
		// 试剂瓶
		case::EBP_IM_REAGENT:
		{
			it.value() == EWL_NOTICE ? ui->label_reagent->setPixmap(QPixmap(IM_SINGLE_REAGENT_YELLOW))
				: ui->label_reagent->setPixmap(QPixmap(IM_SINGLE_REAGENT_RED));
			break;
		}
		// 耗材
		case::EBP_IM_SUPPLY:
		{
			it.value() == EWL_NOTICE ? ui->label_supply->setPixmap(QPixmap(IM_SINGLE_SUPPLY_YELLOW))
				: ui->label_supply->setPixmap(QPixmap(IM_SINGLE_SUPPLY_RED));
			break;
		}
		// 废液
		case::EBP_IM_LIQUID_WASTE:
		{
			it.value() == EWL_NOTICE ? ui->label_liquid_waste->setPixmap(QPixmap(IM_SINGLE_LIQUID_WASTE_YELLOW))
				: ui->label_liquid_waste->setPixmap(QPixmap(IM_SINGLE_LIQUID_WASTE_RED));
			break;
		}
		// 废料
		case::EBP_IM_TRASH_CAN:
		{
			it.value() == EWL_NOTICE ? ui->label_garbage->setPixmap(QPixmap(IM_SINGLE_TRASH_CAN_YELLOW))
				: ui->label_garbage->setPixmap(QPixmap(IM_SINGLE_TRASH_CAN_RED));
			break;
		}
		default:
			break;
		}
	}
}

void I6000SingleDeviceWgt::updateTime(const unsigned int time)
{
	m_currTime = time;

	// 如果时间<=0，停止定时器
	if (m_currTime <= 0)
	{
		m_currTime = 0;
		m_timer->stop();
	}
	else if (!m_timer->isActive())
	{
		m_timer->start();
	}

	ui->label_time->setText(GetTimeText(m_currTime));
}

void I6000SingleDeviceWgt::OnTimerOut()
{
	m_currTime--;

	// 如果时间已经减到0了，停止定时器
	if (m_currTime <= 0)
	{
		m_currTime = 0;
		m_timer->stop();
	}

	ui->label_time->setText(GetTimeText(m_currTime));
}

bool I6000SingleDeviceWgt::eventFilter(QObject *watched, QEvent *event)
{
	// 鼠标按下
	if (event->type() != QEvent::MouseButtonPress)
	{
		return false;
	}

	// 鼠标左键
	if (static_cast<QMouseEvent*>(event)->button() != Qt::LeftButton)
	{
		return false;
	}

	if (watched == ui->widget_reagent
		|| watched == ui->widget_supply
		|| watched == ui->widget_liquid_waste
		|| watched == ui->widget_garbage)
	{
		emit signalShowReagentPage(QString::fromStdString(m_devSN));
	}
	// 维护态点击状态跳转维护界面
	else if (watched == ui->label_status_im && m_devStatus == tf::DeviceWorkState::DEVICE_STATUS_MAINTAIN)
	{
		emit signalMaintain();
	}

	return QObject::eventFilter(watched, event);
}
