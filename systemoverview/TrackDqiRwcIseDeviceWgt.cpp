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
/// @file     TrackDqiRwcIseDeviceWgt.h
/// @brief    轨道DQI RWC 和ISE 设备界面源文件
///
/// @author   7997/XuXiaoLong
/// @date     2023年7月17日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7997/XuXiaoLong，2023年7月17日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "TrackDqiRwcIseDeviceWgt.h"
#include "ui_TrackDqiRwcIseDeviceWgt.h"
#include <QMovie>

#define		WIDGT_W		90			//<	界面宽度
#define		WIDGT_H		420			//< 界面高度

TrackDqiRwcIseDeviceWgt::TrackDqiRwcIseDeviceWgt(const string &sn, const string &name, const EnumDeviceType &type, const int status, 
	const bool wasteLiquidEnable, const bool mask, const bool rgtAlarmReaded, QWidget *parent)
	: QBaseDeviceWgt(sn, name, type, status, wasteLiquidEnable, mask, rgtAlarmReaded, parent)
{
    ui = new Ui::TrackDqiRwcIseDeviceWgt();
	ui->setupUi(this);

	// 透明背景
	this->setStyleSheet("background: transparent;");
	this->setFixedSize(WIDGT_W, WIDGT_H);
	QMovie* mv = new QMovie(SHORT_LIGHT_GRAY);
	ui->label_light->setMovie(mv);
	mv->start();

	ui->label_supply->hide();
	ui->label_name->setText(QString::fromStdString(name));
	switch (type)
	{
	case EnumDeviceType::EDT_TRACK_DQI:
	{
		ui->label_devPicture->setPixmap(QPixmap(TRACK_DQI_PICTURE));
		break;
	}
	case EnumDeviceType::EDT_TRACK_RWC:
	{
		ui->label_devPicture->setPixmap(QPixmap(TRACK_RWC_PICTURE));
		break;
	}
	case EnumDeviceType::EDT_ISE1005:
	{
		ui->label_supply->show();
		ui->label_devPicture->setPixmap(QPixmap(ISE1005_PICTURE));
		ui->label_status->installEventFilter(this);
		//ui->label_status->setWordWrap(true);
		ui->label_status->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		break;
	}
	default:
		break;
	}

	ui->label_supply->installEventFilter(this);
}

TrackDqiRwcIseDeviceWgt::~TrackDqiRwcIseDeviceWgt()
{

}

void TrackDqiRwcIseDeviceWgt::changeStatus()
{
	auto mv = ui->label_light->movie();
	if (!mv)
	{
		return;
	}
	
	// 获取状态信息
	auto statusInfo = GetDeviceStatusInfo(false);

	// 指示灯
	mv->stop();
	mv->setFileName(std::get<1>(statusInfo));
	mv->start();

	// 非ise设备不显示时间
	if (m_devType != EnumDeviceType::EDT_ISE1005)
	{
		ui->label_status->setText(std::get<0>(statusInfo));
		return;
	}

	// 状态+时间文字
	SetStatusTimeText();
}

void TrackDqiRwcIseDeviceWgt::upadteBottle(const QMap<EnumBottlePos, EnumWarnLevel> &data)
{
	// 先全部隐藏
	ui->label_supply->setVisible(false);

	if (data.empty())
	{
		return;
	}

	// 遍历有报警的瓶子
	for (auto it = data.begin(); it != data.end(); ++it)
	{
		if (it.key() != EBP_ISE_SUPPLY)
		{
			continue;
		}

		ui->label_supply->setVisible(true);

		it.value() == EWL_NOTICE ? ui->label_supply->setPixmap(QPixmap(ISE_BOTTLE_SUPPLY_YELLOW))
			: ui->label_supply->setPixmap(QPixmap(ISE_BOTTLE_SUPPLY_RED));
	}
}

void TrackDqiRwcIseDeviceWgt::updateTime(const unsigned int time)
{
	// 非ise设备不显示时间
	if (m_devType != EnumDeviceType::EDT_ISE1005)
	{
		return;
	}

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

	SetStatusTimeText();
}

void TrackDqiRwcIseDeviceWgt::OnTimerOut()
{
	// 非ise设备不显示时间
	if (m_devType != EnumDeviceType::EDT_ISE1005)
	{
		return;
	}

	m_currTime--;

	// 如果时间已经减到0了，停止定时器
	if (m_currTime <= 0)
	{
		m_currTime = 0;
		m_timer->stop();
	}

	SetStatusTimeText();
}

bool TrackDqiRwcIseDeviceWgt::eventFilter(QObject *watched, QEvent *event)
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

	if (watched == ui->label_supply)
	{
		emit signalShowReagentPage(QString::fromStdString(m_devSN));
	}
	// 维护态点击状态跳转维护界面
	else if (watched == ui->label_status 
		&& m_devStatus == tf::DeviceWorkState::DEVICE_STATUS_MAINTAIN
		&& m_devType == EnumDeviceType::EDT_ISE1005)
	{
		emit signalMaintain();
	}

	return QObject::eventFilter(watched, event);
}

void TrackDqiRwcIseDeviceWgt::SetStatusTimeText()
{
	// 获取状态信息
	auto statusInfo = GetDeviceStatusInfo(false);

	// 状态文字
	QString statusText = std::get<0>(statusInfo);

	// 显示时间
	if (std::get<2>(statusInfo))
	{
		statusText += "    " + GetTimeText(m_currTime);
	}

	// 冒泡文字
	ui->label_status->setToolTip(statusText);

	// 文字太长后面加...
	QFontMetrics fm(ui->label_status->font());
	QString showName = fm.elidedText(statusText, Qt::ElideRight, ui->label_status->width());

	// 状态文字
	ui->label_status->setText(showName);
}
