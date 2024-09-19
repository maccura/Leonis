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
/// @file     C1005V2000DeviceWgt.cpp
/// @brief    C1005 2000速设备界面源文件
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
#include "C1005V2000DeviceWgt.h"
#include "ui_C1005V2000DeviceWgt.h"
#include "src/common/Mlog/mlog.h"
#include "shared/CommonInformationManager.h"
#include <QMovie>

#define		WIDGT_W		380			//<	界面宽度
#define		WIDGT_H		420			//< 界面高度

C1005V2000DeviceWgt::C1005V2000DeviceWgt(const string &sn, const string &name, const EnumDeviceType &type,
	const bool wasteLiquidEnable, QWidget *parent)
	: QBaseDeviceWgt(sn, name, type, 0, wasteLiquidEnable, false, true, parent)
{
    ui = new Ui::C1005V2000DeviceWgt();
	ui->setupUi(this);

	// 透明背景
	this->setStyleSheet("background: transparent;");
	this->setFixedSize(WIDGT_W, WIDGT_H);
	ui->label_name->setText(QString::fromStdString(name));
	ui->label_devPicture->setPixmap(QPixmap(C1005_2000_PICTURE));
	QMovie* mv = new QMovie(LONG_LIGHT_GRAY);
	ui->label_light->setMovie(mv);
	mv->start();

	ui->label_reagent->installEventFilter(this);
	ui->label_supply->installEventFilter(this);
	ui->label_liquid_waste->installEventFilter(this);
	ui->label_statusA->installEventFilter(this);
	ui->label_statusB->installEventFilter(this);
}

C1005V2000DeviceWgt::~C1005V2000DeviceWgt()
{
}

void C1005V2000DeviceWgt::UpdateDevInfo(const string &devSn, const string &devName, const int status, const bool mask, const bool readed)
{
	if (devName == "A")
	{
		m_devA.devSN = devSn;
		m_devA.mask = mask;
		m_devA.rgtAlarmReaded = readed;
		m_devA.status = status;
		m_devA.timer.setInterval(1000);
		connect(&m_devA.timer, &QTimer::timeout, this, &C1005V2000DeviceWgt::OnChATimerOut);
	}
	else if (devName == "B")
	{
		m_devB.devSN = devSn;
		m_devB.mask = mask;
		m_devB.rgtAlarmReaded = readed;
		m_devB.status = status;
		m_devB.timer.setInterval(1000);
		connect(&m_devB.timer, &QTimer::timeout, this, &C1005V2000DeviceWgt::OnChBTimerOut);
	}
}

void C1005V2000DeviceWgt::changeStatus()
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

	auto pairStatusA = GetStatusText(m_devA.status, m_devA.mask);
	auto pairStatusB = GetStatusText(m_devB.status, m_devB.mask);

	// 状态文字
	ui->label_statusA->setText(pairStatusA.second);
	ui->label_statusB->setText(pairStatusB.second);

	// 时间显隐
	ui->label_timeA->setVisible(pairStatusA.first);
	ui->label_timeB->setVisible(pairStatusB.first);
}

void C1005V2000DeviceWgt::updateTime(const string& devSn, const int time)
{
	if (devSn == m_devA.devSN)
	{
		m_devA.time = time;

		// 如果时间<=0，停止定时器
		if (m_devA.time <= 0)
		{
			m_devA.timer.stop();
		}
		else if (!m_devA.timer.isActive())
		{
			m_devA.timer.start();
		}

		ui->label_timeA->setText(GetTimeText(m_devA.time));
	}
	else if (devSn == m_devB.devSN)
	{
		m_devB.time = time;

		// 如果时间<=0，停止定时器
		if (m_devB.time <= 0)
		{
			m_devB.timer.stop();
		}
		else if (!m_devB.timer.isActive())
		{
			m_devB.timer.start();
		}

		ui->label_timeB->setText(GetTimeText(m_devB.time));
	}
}

void C1005V2000DeviceWgt::upadteBottle(const QMap<EnumBottlePos, EnumWarnLevel> &data)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 先全部隐藏
	ui->label_reagent->setVisible(false);
	ui->label_supply->setVisible(false);
	ui->label_liquid_waste->setVisible(false);

	// 如果没有报警直接返回
	if (data.empty())
	{
		return;
	}

	// 遍历有报警的瓶子
	for (auto it = data.begin(); it != data.end(); ++it)
	{
		// 试剂
		if (it.key() == EBP_CH_REAGENT)
		{
			ui->label_reagent->setVisible(true);

			it.value() == EWL_NOTICE ? ui->label_reagent->setPixmap(QPixmap(CH_BOTTLE_REAGENT_YELLOW))
				: ui->label_reagent->setPixmap(QPixmap(CH_BOTTLE_REAGENT_RED));
		}
		// 耗材
		else if (it.key() == EBP_CH_SUPPLY)
		{
			ui->label_supply->setVisible(true);

			it.value() == EWL_NOTICE ? ui->label_supply->setPixmap(QPixmap(CH_BOTTLE_SUPPLY_YELLOW))
				: ui->label_supply->setPixmap(QPixmap(CH_BOTTLE_SUPPLY_RED));
		}
		// 废液
		else if (it.key() == EBP_CH_LIQUID_WASTE)
		{
			// 包含废液桶才显示
			if (m_haveWasteLiquid)
			{
				ui->label_liquid_waste->setVisible(true);

				it.value() == EWL_NOTICE ? ui->label_liquid_waste->setPixmap(QPixmap(CH_BOTTLE_LIQUID_WASTE_YELLOW))
					: ui->label_liquid_waste->setPixmap(QPixmap(CH_BOTTLE_LIQUID_WASTE_RED));
			}
		}
	}
}

void C1005V2000DeviceWgt::setWasteLiquidVisible(const bool enable)
{
	// 有一个设备包含废液桶就需要显示废液桶
	m_haveWasteLiquid = m_haveWasteLiquid || enable;
	ui->label_liquid_waste->setVisible(m_haveWasteLiquid);
}

void C1005V2000DeviceWgt::setDeviceReagentAlarmReadFlag()
{
	// 必须两个设备试剂报警都已读才表示已读
	m_rgtAlarmReaded = m_devA.rgtAlarmReaded && m_devB.rgtAlarmReaded;
}

QString C1005V2000DeviceWgt::GetLightColorStr() const
{
	// 是否闪烁
	bool flicker = (m_devA.status == ::tf::DeviceWorkState::DEVICE_STATUS_RUNNING
		|| m_devA.status == ::tf::DeviceWorkState::DEVICE_STATUS_MAINTAIN
		|| m_devA.status == ::tf::DeviceWorkState::DEVICE_STATUS_SAMPSTOP
		|| m_devA.status == ::tf::DeviceWorkState::DEVICE_STATUS_SAMPSTOP_FAULT
		|| m_devA.status == ::tf::DeviceWorkState::DEVICE_STATUS_STOP_PUSH_RACK
		|| m_devB.status == ::tf::DeviceWorkState::DEVICE_STATUS_RUNNING
		|| m_devB.status == ::tf::DeviceWorkState::DEVICE_STATUS_MAINTAIN
		|| m_devB.status == ::tf::DeviceWorkState::DEVICE_STATUS_SAMPSTOP
		|| m_devB.status == ::tf::DeviceWorkState::DEVICE_STATUS_SAMPSTOP_FAULT
		|| m_devB.status == ::tf::DeviceWorkState::DEVICE_STATUS_STOP_PUSH_RACK);

	// 获取生化单机指示灯合成颜色
	auto composeColor = CIM_INSTANCE->GetC2000LightColor(m_devA.status, m_devB.status, m_rgtAlarmReaded);

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

bool C1005V2000DeviceWgt::eventFilter(QObject *watched, QEvent *event)
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

	if (watched == ui->label_reagent
		|| watched == ui->label_supply
		|| watched == ui->label_liquid_waste)
	{
		emit signalShowReagentPage(QString::fromStdString(m_devA.devSN));
	}
	// 维护态点击状态跳转维护界面
	else if ((watched == ui->label_statusA && m_devA.status == tf::DeviceWorkState::DEVICE_STATUS_MAINTAIN)
			|| (watched == ui->label_statusB && m_devB.status == tf::DeviceWorkState::DEVICE_STATUS_MAINTAIN))
	{
		emit signalMaintain();
	}

	return QObject::eventFilter(watched, event);
}

void C1005V2000DeviceWgt::OnChATimerOut()
{
	if (m_devA.time > 0)
	{
		m_devA.time--;
		ui->label_timeA->setText(GetTimeText(m_devA.time));
	}
	else
	{
		// 如果时间已经减到0了，停止定时器
		m_devA.timer.stop();
	}
}

void C1005V2000DeviceWgt::OnChBTimerOut()
{
	if (m_devB.time > 0)
	{
		m_devB.time--;
		ui->label_timeB->setText(GetTimeText(m_devB.time));
	}
	else
	{
		// 如果时间已经减到0了，停止定时器
		m_devB.timer.stop();
	}
}
