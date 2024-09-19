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
/// @file     C1005SingleDeviceWgt.cpp
/// @brief    C1005 单机版设备界面源文件
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
#include "C1005SingleDeviceWgt.h"
#include "ui_C1005SingleDeviceWgt.h"
#include "src/common/Mlog/mlog.h"
#include "shared/CommonInformationManager.h"
#include <QMovie>

#define		WIDGT_W		1820		//<	界面宽度
#define		WIDGT_H		500			//< 界面高度

C1005SingleDeviceWgt::C1005SingleDeviceWgt(const EnumDeviceType &type, QWidget *parent)
	: QBaseDeviceWgt(type, parent)
	, m_iomStatus(0)
{
    ui = new Ui::C1005SingleDeviceWgt();
	ui->setupUi(this);

	// 透明背景
	this->setStyleSheet("background: transparent;");
	this->setFixedSize(WIDGT_W, WIDGT_H);
	ui->label_devPicture->setPixmap(QPixmap(C1005_SINGLE_PICTURE));
	QMovie* mv = new QMovie(LONG_LIGHT_GRAY);
	ui->label_light->setMovie(mv);
	mv->start();

	ui->widget_reagent->installEventFilter(this);
	ui->widget_supply->installEventFilter(this);
	ui->widget_liquid_waste->installEventFilter(this);
	ui->label_status_ch->installEventFilter(this);

	connect(ui->btn_iom, &QPushButton::clicked, this, [=] { emit signalSampleRackMonitor(); });
}

C1005SingleDeviceWgt::~C1005SingleDeviceWgt()
{
}

void C1005SingleDeviceWgt::changeStatus()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	auto mv = ui->label_light->movie();
	if (!mv)
	{
		return;
	}

	// 指示灯
	mv->stop();
	mv->setFileName(GetLightColorStr());
	mv->start();

	// 状态+时间文字
	SetStatusTimeText();

	// iom状态文字
	ui->label_status_iom->setText(GetIomStatusText(m_iomStatus));
}

void C1005SingleDeviceWgt::upadteBottle(const QMap<EnumBottlePos, EnumWarnLevel> &data)
{
	// 先全部显示正常
	ui->label_reagent->setPixmap(QPixmap(CH_SINGLE_REAGENT_GREEN));
	ui->label_supply->setPixmap(QPixmap(CH_SINGLE_SUPPLY_GREEN));
	ui->label_liquid_waste->setPixmap(QPixmap(CH_SINGLE_LIQUID_WASTE_GREEN));

	if (data.empty())
	{
		return;
	}

	// 遍历有报警的瓶子
	for (auto it = data.begin(); it != data.end(); ++it)
	{
		// 试剂
		if (it.key()== EBP_CH_REAGENT)
		{
			it.value() == EWL_NOTICE ? ui->label_reagent->setPixmap(QPixmap(CH_SINGLE_REAGENT_YELLOW))
				: ui->label_reagent->setPixmap(QPixmap(CH_SINGLE_REAGENT_RED));
		}
		// 耗材
		else if (it.key()== EBP_CH_SUPPLY)
		{
			it.value() == EWL_NOTICE ? ui->label_supply->setPixmap(QPixmap(CH_SINGLE_SUPPLY_YELLOW))
				: ui->label_supply->setPixmap(QPixmap(CH_SINGLE_SUPPLY_RED));
		}
		// 废液
		else if (it.key()== EBP_CH_LIQUID_WASTE)
		{
			it.value() == EWL_NOTICE ? ui->label_liquid_waste->setPixmap(QPixmap(CH_SINGLE_LIQUID_WASTE_YELLOW))
				: ui->label_liquid_waste->setPixmap(QPixmap(CH_SINGLE_LIQUID_WASTE_RED));
		}
	}
}

void C1005SingleDeviceWgt::updateTime(const string& devSn, const int time)
{
	if (devSn == m_devCh.devSN)
	{
		m_devCh.time = time;

		// 如果时间<=0，停止定时器
		if (m_devCh.time <= 0)
		{
			m_devCh.timer.stop();
		}
		else if (!m_devCh.timer.isActive())
		{
			m_devCh.timer.start();
		}
	}
	else if(devSn == m_devIse.devSN)
	{
		m_devIse.time = time;

		// 如果时间<=0，停止定时器
		if (m_devIse.time <= 0)
		{
			m_devIse.timer.stop();
		}
		else if (!m_devIse.timer.isActive())
		{
			m_devIse.timer.start();
		}
	}

	SetStatusTimeText();
}

void C1005SingleDeviceWgt::setDeviceReagentAlarmReadFlag()
{
	bool rgAlarmReadCh = m_devCh.rgtAlarmReaded;
	m_rgtAlarmReaded = rgAlarmReadCh;

	// 必须两个设备试剂报警都已读才表示已读
	if (!m_devIse.devSN.empty())
	{
		m_rgtAlarmReaded = rgAlarmReadCh && m_devIse.rgtAlarmReaded;
	}
}

void C1005SingleDeviceWgt::updateChDeviceInfo(const string &sn, const string &name, const int status, const int iomStatus,
	const bool wasteLiquidEnable, const bool mask, const bool readed)
{
	m_devSN = sn;
	m_devName = name;
	m_haveWasteLiquid = wasteLiquidEnable;
	m_iomStatus = iomStatus;
	
	ui->label_name_ch->setText(QString::fromStdString(name));
	ui->label_name_iom->setText("IOM");
	ui->widget_liquid_waste->setVisible(wasteLiquidEnable);

	m_devCh.devSN = sn;
	m_devCh.mask = mask;
	m_devCh.rgtAlarmReaded = readed;
	m_devCh.status = status;
	m_devCh.timer.setInterval(1000);
	connect(&m_devCh.timer, &QTimer::timeout, this, &C1005SingleDeviceWgt::OnChTimerOut);

	setDeviceReagentAlarmReadFlag();
}

void C1005SingleDeviceWgt::updateIseDeviceInfo(const string &sn, const int status, const bool mask, const bool readed)
{
	m_devIse.devSN = sn;
	m_devIse.mask = mask;
	m_devIse.rgtAlarmReaded = readed;
	m_devIse.status = status;
	m_devIse.timer.setInterval(1000);
	connect(&m_devIse.timer, &QTimer::timeout, this, &C1005SingleDeviceWgt::OnIseTimerOut);

	setDeviceReagentAlarmReadFlag();
}

QString C1005SingleDeviceWgt::GetLightColorStr() const
{
	// 是否闪烁
	bool flicker = (m_devCh.status == ::tf::DeviceWorkState::DEVICE_STATUS_RUNNING
		|| m_devCh.status == ::tf::DeviceWorkState::DEVICE_STATUS_MAINTAIN
		|| m_devCh.status == ::tf::DeviceWorkState::DEVICE_STATUS_SAMPSTOP
		|| m_devCh.status == ::tf::DeviceWorkState::DEVICE_STATUS_SAMPSTOP_FAULT
		|| m_devCh.status == ::tf::DeviceWorkState::DEVICE_STATUS_STOP_PUSH_RACK
		|| m_devIse.status == ::tf::DeviceWorkState::DEVICE_STATUS_RUNNING
		|| m_devIse.status == ::tf::DeviceWorkState::DEVICE_STATUS_MAINTAIN
		|| m_devIse.status == ::tf::DeviceWorkState::DEVICE_STATUS_SAMPSTOP
		|| m_devIse.status == ::tf::DeviceWorkState::DEVICE_STATUS_SAMPSTOP_FAULT
		|| m_devIse.status == ::tf::DeviceWorkState::DEVICE_STATUS_STOP_PUSH_RACK);
 
	// 获取生化单机指示灯合成颜色
	auto composeColor = CIM_INSTANCE->GetChSingleLightColor(m_devCh.status, m_devIse.status, m_rgtAlarmReaded);

	// 根据合成的灯获得显示的字符串
	switch (composeColor)
	{
	case ELC_GRAY:
		return LONG_LIGHT_GRAY;
	case ELC_RED:
		return LONG_LIGHT_RED;
	case ELC_YELLOW:
		return flicker ? LONG_LIGHT_YELLOW_SCROLL : LONG_LIGHT_YELLOW;
	case ELC_GREEN:
		return flicker ? LONG_LIGHT_GREEN_SCROLL : LONG_LIGHT_GREEN;
	default:
		return LONG_LIGHT_GRAY;
	}
}

void C1005SingleDeviceWgt::SetStatusTimeText()
{
	// 获取原始文字
	QString strText;

	// 生化设备状态文字
	auto pairChStatus = GetStatusText(m_devCh.status, m_devCh.mask);
	strText = pairChStatus.second;
	if (pairChStatus.first)
	{
		strText += "    " + GetTimeText(m_devCh.time);
	}

	// 如果存在ISE设备
	if (!m_devIse.devSN.empty())
	{
		strText = pairChStatus.second;
		// ISE设备状态文字
		auto pairIseStatus = GetStatusText(m_devIse.status, m_devIse.mask);
		strText += "/" + pairIseStatus.second + "    ";

		// 显示生化设备时间
		if (pairChStatus.first)
		{
			strText += GetTimeText(m_devCh.time);
		}

		// 生化或者ISE其中一个需要显示时间才+“/”
		if (pairChStatus.first || pairIseStatus.first)
		{
			strText += "/";
		}

		// 显示ISE设备时间
		if (pairIseStatus.first)
		{
			strText += GetTimeText(m_devIse.time);
		}
	}

	// 冒泡文字
	ui->label_status_ch->setToolTip(strText);

	// 文字太长后面加...
	QFontMetrics fm(ui->label_status_ch->font());
	QString showName = fm.elidedText(strText, Qt::ElideRight, ui->label_status_ch->width());

	// 状态文字
	ui->label_status_ch->setText(showName);
}

bool C1005SingleDeviceWgt::eventFilter(QObject *watched, QEvent *event)
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
		|| watched == ui->widget_liquid_waste)
	{
		emit signalShowReagentPage(QString::fromStdString(m_devCh.devSN));
	}
	// 维护态点击状态跳转维护界面
	else if (watched == ui->label_status_ch && 
		(m_devCh.status == tf::DeviceWorkState::DEVICE_STATUS_MAINTAIN 
			|| m_devIse.status == tf::DeviceWorkState::DEVICE_STATUS_MAINTAIN))
	{
		emit signalMaintain();
	}

	return QObject::eventFilter(watched, event);
}

void C1005SingleDeviceWgt::OnChTimerOut()
{
	if (m_devCh.time > 0)
	{
		m_devCh.time--;
		SetStatusTimeText();
	}
	else
	{
		// 如果时间已经减到0了，停止定时器
		m_devCh.timer.stop();
	}
}

void C1005SingleDeviceWgt::OnIseTimerOut()
{
	if (m_devIse.time > 0)
	{
		m_devIse.time--;
		SetStatusTimeText();
	}
	else
	{
		// 如果时间已经减到0了，停止定时器
		m_devIse.timer.stop();
	}
}
