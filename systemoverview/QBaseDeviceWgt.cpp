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
/// @file     QBaseDeviceWgt.cpp
/// @brief    各仪器基类源文件
///
/// @author   7997/XuXiaoLong
/// @date     2023年7月11日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7997/XuXiaoLong，2023年7月11日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "QBaseDeviceWgt.h"
#include <QDateTime>

QBaseDeviceWgt::QBaseDeviceWgt(const string &sn, const string &name, const EnumDeviceType &type, const int status,
	const bool haveWasteLiquid, const bool mask, const bool rgtAlarmReaded, QWidget *parent /*= nullptr*/)
	: m_devSN(sn)
	, m_devName(name)
	, m_devType(type)
	, m_devStatus(status)
	, m_haveWasteLiquid(haveWasteLiquid)
	, m_mask(mask)
	, m_rgtAlarmReaded(rgtAlarmReaded)
	, QWidget(parent)
{
	initTimer();
}

QBaseDeviceWgt::QBaseDeviceWgt(const EnumDeviceType &type, QWidget *parent /*= nullptr*/)
	: m_devType(type)
	, QWidget(parent)
{
	initTimer();
}

QBaseDeviceWgt::~QBaseDeviceWgt()
{

}

void QBaseDeviceWgt::initTimer()
{
	m_timer = new QTimer(this);
	m_timer->setInterval(1000);
	connect(m_timer, &QTimer::timeout, this, [=]() { OnTimerOut(); });
}

void QBaseDeviceWgt::changeStatus()
{

}

void QBaseDeviceWgt::updateTime(const unsigned int time)
{

}

void QBaseDeviceWgt::upadteBottle(const QMap<EnumBottlePos, EnumWarnLevel> &data)
{

}

void QBaseDeviceWgt::OnTimerOut()
{

}

tuple<QString, QString, bool> QBaseDeviceWgt::GetDeviceStatusInfo(const bool longLight)
{
	QString strStatus;
	QString strLight;
	bool showTime = false;

	// 是否屏蔽
	QString sMask = m_mask ? tr("(屏蔽)") : "";

	// 判断设备状态
	switch (m_devStatus)
	{
		// 未连接
	case ::tf::DeviceWorkState::DEVICE_STATUS_DISCONNECT:
		strStatus = "--";
		strLight = longLight ? LONG_LIGHT_GRAY : SHORT_LIGHT_GRAY;
		break;
		// 待机
	case ::tf::DeviceWorkState::DEVICE_STATUS_STANDBY:
		strStatus = tr("待机") + sMask;
		strLight = longLight ? LONG_LIGHT_GREEN : SHORT_LIGHT_GREEN;
		if (!m_rgtAlarmReaded)
		{
			strLight = longLight ? LONG_LIGHT_YELLOW : SHORT_LIGHT_YELLOW;
		}
		break;
		// 运行
	case ::tf::DeviceWorkState::DEVICE_STATUS_RUNNING:
		strStatus = tr("运行") + sMask;
		strLight = longLight ? LONG_LIGHT_GREEN_SCROLL : SHORT_LIGHT_GREEN_SCROLL;
		if (!m_rgtAlarmReaded)
		{
			strLight = longLight ? LONG_LIGHT_YELLOW_SCROLL : SHORT_LIGHT_YELLOW_SCROLL;
		}
		showTime = true;
		break;
		// 维护
	case ::tf::DeviceWorkState::DEVICE_STATUS_MAINTAIN:
		strStatus = tr("维护") + sMask;
		strLight = longLight ? LONG_LIGHT_GREEN_SCROLL : SHORT_LIGHT_GREEN_SCROLL;
		if (!m_rgtAlarmReaded)
		{
			strLight = longLight ? LONG_LIGHT_YELLOW_SCROLL : SHORT_LIGHT_YELLOW_SCROLL;
		}
		showTime = true;
		break;
		// 预热
	case ::tf::DeviceWorkState::DEVICE_STATUS_WARMUP:
		strStatus = tr("维护") + sMask;
		strLight = longLight ? LONG_LIGHT_GREEN_SCROLL : SHORT_LIGHT_GREEN_SCROLL;
		if (!m_rgtAlarmReaded)
		{
			strLight = longLight ? LONG_LIGHT_YELLOW_SCROLL : SHORT_LIGHT_YELLOW_SCROLL;
		}
		showTime = true;
		break;
		// 正常加样停、进架停（绿色）
	case ::tf::DeviceWorkState::DEVICE_STATUS_SAMPSTOP:
	case ::tf::DeviceWorkState::DEVICE_STATUS_STOP_PUSH_RACK:
		strStatus = tr("暂停") + sMask;
		strLight = longLight ? LONG_LIGHT_GREEN_SCROLL : SHORT_LIGHT_YELLOW_SCROLL;
		if (!m_rgtAlarmReaded)
		{
			strLight = longLight ? LONG_LIGHT_YELLOW_SCROLL : SHORT_LIGHT_YELLOW_SCROLL;
		}
		showTime = true;
		break;
		// 异常加样停（黄色）
	case ::tf::DeviceWorkState::DEVICE_STATUS_SAMPSTOP_FAULT:
		strStatus = tr("暂停") + sMask;
		strLight = longLight ? LONG_LIGHT_YELLOW_SCROLL : SHORT_LIGHT_YELLOW_SCROLL;
		showTime = true;
		break;
		// 停机
	case ::tf::DeviceWorkState::DEVICE_STATUS_HALT:
		strStatus = tr("停止") + sMask;
		strLight = longLight ? LONG_LIGHT_RED : SHORT_LIGHT_RED;
		break;
		// 申请中
	case ::tf::DeviceWorkState::DEVICE_STATUS_WAIT_OPEN_LID:
		strStatus = tr("申请中") + sMask;
		strLight = longLight ? LONG_LIGHT_GREEN_SCROLL : SHORT_LIGHT_GREEN_SCROLL;
		break;
		// 申请成功
	case ::tf::DeviceWorkState::DEVICE_STATUS_ALLOW_OPEN_LID:
		strStatus = tr("申请成功") + sMask;
		strLight = longLight ? LONG_LIGHT_GREEN_SCROLL : SHORT_LIGHT_GREEN_SCROLL;
		break;
		// 其他状态返回对应的状态枚举方便排查
	default:
		strStatus = QString::number(m_devStatus);
		strLight = longLight ? LONG_LIGHT_GRAY : SHORT_LIGHT_GRAY;
		break;
	}

	return{ strStatus, strLight, showTime };
}

QString QBaseDeviceWgt::GetIomStatusText(const int status)
{
	switch (status)
	{
		// 未连接
	case ::tf::DeviceWorkState::DEVICE_STATUS_DISCONNECT:
		return "--";
		// 待机
	case ::tf::DeviceWorkState::DEVICE_STATUS_STANDBY:
		return tr("待机");
		// 运行
	case ::tf::DeviceWorkState::DEVICE_STATUS_RUNNING:
		return tr("运行");
		// 预热（显示为维护）
	case ::tf::DeviceWorkState::DEVICE_STATUS_WARMUP:
		return tr("维护");
		// 维护
	case ::tf::DeviceWorkState::DEVICE_STATUS_MAINTAIN:
		return tr("维护");
		// 暂停
	case ::tf::DeviceWorkState::DEVICE_STATUS_SAMPSTOP:
	case ::tf::DeviceWorkState::DEVICE_STATUS_STOP_PUSH_RACK:
	case ::tf::DeviceWorkState::DEVICE_STATUS_SAMPSTOP_FAULT:
		return tr("暂停");
		// 停机
	case ::tf::DeviceWorkState::DEVICE_STATUS_HALT:
		return tr("停止");
		// 其他状态返回对应的状态枚举方便排查
	default:
		return QString::number(status);
	}
}

std::pair<bool, QString> QBaseDeviceWgt::GetStatusText(const int status, const bool mask)
{
	QString strStatus;
	bool showTime = false;

	// 是否屏蔽
	QString sMask = mask ? tr("(屏蔽)") : "";

	// 判断设备状态
	switch (status)
	{
		// 未连接
	case ::tf::DeviceWorkState::DEVICE_STATUS_DISCONNECT:
		strStatus = "--";
		break;
		// 待机
	case ::tf::DeviceWorkState::DEVICE_STATUS_STANDBY:
		strStatus = tr("待机") + sMask;
		break;
		// 运行
	case ::tf::DeviceWorkState::DEVICE_STATUS_RUNNING:
		strStatus = tr("运行") + sMask;
		showTime = true;
		break;
		// 维护、预热
	case ::tf::DeviceWorkState::DEVICE_STATUS_MAINTAIN:
	case ::tf::DeviceWorkState::DEVICE_STATUS_WARMUP:
		strStatus = tr("维护") + sMask;
		showTime = true;
		break;
		// 正常加样停、进架停、异常常加样停
	case ::tf::DeviceWorkState::DEVICE_STATUS_SAMPSTOP:
	case ::tf::DeviceWorkState::DEVICE_STATUS_STOP_PUSH_RACK:
	case ::tf::DeviceWorkState::DEVICE_STATUS_SAMPSTOP_FAULT:
		strStatus = tr("暂停") + sMask;
		showTime = true;
		break;
		// 停机
	case ::tf::DeviceWorkState::DEVICE_STATUS_HALT:
		strStatus = tr("停止") + sMask;
		break;
		// 申请中
	case ::tf::DeviceWorkState::DEVICE_STATUS_WAIT_OPEN_LID:
		strStatus = tr("申请中") + sMask;
		break;
		// 申请成功
	case ::tf::DeviceWorkState::DEVICE_STATUS_ALLOW_OPEN_LID:
		strStatus = tr("申请成功") + sMask;
		break;
		// 其他状态返回对应的状态枚举方便排查
	default:
		strStatus = QString::number(status);
		break;
	}

	return{ showTime, strStatus };
}

QString QBaseDeviceWgt::GetTimeText(const int timeS)
{
	if (timeS <= 0)
	{
		return "";
	}
	else if (timeS < 60)
	{
		return "<1 min";
	}
	else if (timeS >= 60 && timeS < 60 * 60)
	{
		return QDateTime::fromTime_t(timeS).toUTC().toString("mm:ss");
	}
	else
	{
		// 秒转为小时，向下取整
		int hours = timeS / (60 * 60);
		return QString("> %1h").arg(hours);
	}
}