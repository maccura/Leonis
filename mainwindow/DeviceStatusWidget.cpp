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
/// @file     DeviceStatusWidget.h
/// @brief    设备状态展示控件
///
/// @author   7951/LuoXin
/// @date     2024年4月25日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2024年4月25日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include "shared/uidef.h"
#include "shared/uicommon.h"
#include "shared/CommonInformationManager.h"
#include "DeviceStatusWidget.h"
#include "ui_DeviceStatusWidget.h"
#include "src/common/Mlog/mlog.h"

#define  INVALID_TIME           QTime(100,0,0);     // 未定义时间
#define  WORLD_AREA_WIDTH       (97)                // 文字区域宽度

DeviceStatusWidget::DeviceStatusWidget(DeviceWidgetType type, QString name, QVector<int> devTypeVec, QWidget *parent)
    : QWidget(parent)
    , m_DeviceWidgetType(type)
    , m_deviceTypeVec(devTypeVec)
    , m_curStatusA(0)
    , m_curStatusB(0)
    , m_isMaskedA(false)
    , m_isMaskedB(false)
    , m_isReagentAlarm(false)
    , ui(new Ui::DeviceStatusWidget)
{
    ui->setupUi(this);

    setObjectName(name);
    ui->name_label_first->setText(name);

    if (type == OPTIONAL_DEVICE)
    {

        ui->verticalLayout_second->deleteLater();
        ui->horizontalLayout_3->deleteLater();
        ui->name_label_second->deleteLater();
        ui->time_label_second->deleteLater();
        ui->status_label_second->deleteLater();

        setMaximumWidth(width()- WORLD_AREA_WIDTH);
    }

    // 按秒倒计时
    m_timer.setInterval(1000);
    m_timeA = INVALID_TIME;
    m_timeB = INVALID_TIME;

    connect(&m_timer, &QTimer::timeout, this, &DeviceStatusWidget::OnTimeOut);
}

DeviceStatusWidget::~DeviceStatusWidget()
{

}

QStringList DeviceStatusWidget::GetDeviceSns(int devType)
{
    if (!m_deviceTypeVec.contains(devType))
    {
        return QStringList();
    }

    return m_snList;
}
void DeviceStatusWidget::SetGroupName(QString name)
{
    setObjectName(name);
    m_groupName = name;
}

void DeviceStatusWidget::SetName(QString name, bool isFirst)
{
    if (isFirst)
    {
        ui->name_label_first->setText(name);
    } 
    else
    {
        ui->name_label_second->setText(name);
    }
}

void DeviceStatusWidget::UpdateStatus(int status, bool isFirst)
{
    isFirst? m_curStatusA = status : m_curStatusB = status;
    auto CIM = CommonInformationManager::GetInstance();

    EnumLightColor color;

    // 获取颜色
    switch (m_DeviceWidgetType)
    {
        case OPTIONAL_DEVICE:
            color = CIM->GetLightColor(m_curStatusA, !m_isReagentAlarm);
            break;
        case ANALYSIS_ONE_DEVICE:
            color = CIM->GetLightColor(m_curStatusA, !m_isReagentAlarm);
            break;
        case ANALYSIS_TWO_DEVICE:
            if (m_deviceTypeVec.contains(::tf::DeviceType::DEVICE_TYPE_ISE1005))
            {
                color = CIM->GetChSingleLightColor(m_curStatusA, m_curStatusB, !m_isReagentAlarm);
            } 
            else
            {
                color = CIM->GetC2000LightColor(m_curStatusA, m_curStatusB, !m_isReagentAlarm);
            }

            break;
        default:
            break;
    }  

    // 显示颜色
    switch (color)
    {
        case ELC_GRAY:
            ui->status_color_label->setStyleSheet("QLabel{ background-image: url(:/Leonis/resource/image/device-state-gray.png);}");
            break;
        case ELC_RED:
            ui->status_color_label->setStyleSheet("QLabel{ background-image: url(:/Leonis/resource/image/device-state-red.png);}");
            break;
        case ELC_YELLOW:
            ui->status_color_label->setStyleSheet("QLabel{ background-image: url(:/Leonis/resource/image/device-state-yellow.png);}");
            break;
        case ELC_GREEN:
            ui->status_color_label->setStyleSheet("QLabel{ background-image: url(:/Leonis/resource/image/device-state-green.png);}");
            break;
        default:
            break;
    }

    // 运行、暂停、维护、开盖才显示倒计时
    if (status != ::tf::DeviceWorkState::DEVICE_STATUS_RUNNING
        && status != ::tf::DeviceWorkState::DEVICE_STATUS_SAMPSTOP
        && status != ::tf::DeviceWorkState::DEVICE_STATUS_SAMPSTOP_FAULT
        && status != ::tf::DeviceWorkState::DEVICE_STATUS_STOP_PUSH_RACK
        && status != ::tf::DeviceWorkState::DEVICE_STATUS_MAINTAIN
        && status != ::tf::DeviceWorkState::DEVICE_STATUS_WAIT_OPEN_LID)
    {
        if (isFirst)
        {
            if (m_DeviceWidgetType == DeviceStatusWidget::ANALYSIS_ONE_DEVICE)
            {
                m_timeB = INVALID_TIME;
                ui->time_label_first->clear();
                ui->time_label_second->setText("--");
            } 
            else
            {
                ui->time_label_first->setText("--");
            }

            m_timeA = INVALID_TIME;
        }
        else
        {
            m_timeB = INVALID_TIME;
            ui->time_label_second->setText("--");
        }
    }

    // 设置文字
    UpdateMasked(m_isMaskedA, true);

    setEnabled(color != ELC_GRAY);
}

void DeviceStatusWidget::UpdateMasked(bool isMasked, bool isFirst)
{
    isFirst ? m_isMaskedA = isMasked : m_isMaskedB = isMasked;

    QString tempStrA = m_isMaskedA ? tr("(屏蔽)") : "";
    ui->status_label_first->setText(ConvertTfEnumToQString((::tf::DeviceWorkState::type)m_curStatusA) + tempStrA);

    if (m_DeviceWidgetType == ANALYSIS_TWO_DEVICE)
    {
        QString tempStrB = m_isMaskedB ? tr("(屏蔽)") : "";
        ui->status_label_second->setText(ConvertTfEnumToQString((::tf::DeviceWorkState::type)m_curStatusB) + tempStrB);
    }
}

void DeviceStatusWidget::UpdateTime(int time, bool isFirst)
{   
    m_timer.stop();

    auto timeTemp = QTime(0, 0, 0).addSecs(time);
    isFirst ? m_timeA = timeTemp : m_timeB = timeTemp;

    m_timer.start();
}

void DeviceStatusWidget::UpdateReagentAlarm(bool existReagentAlarm)
{
    m_isReagentAlarm = existReagentAlarm;
    UpdateStatus(m_curStatusA, true);
}

void DeviceStatusWidget::ShowTimeToLabel(QTime& time, QLabel* lab, int status)
{
    int timeSecond = time.hour() * 3600 + time.minute() * 60 + time.second();

    // 小于0s不倒计时
    if (timeSecond > 0)
    {
        time = time.addSecs(-1);
    }
    else
    {
        time = INVALID_TIME;
    }

    // 断开连接、待机、停机、允许开盖不显示倒计时
    if (!time.isValid()
        || status == ::tf::DeviceWorkState::DEVICE_STATUS_DISCONNECT
        || status == ::tf::DeviceWorkState::DEVICE_STATUS_STANDBY
        || status == ::tf::DeviceWorkState::DEVICE_STATUS_HALT
        || status == ::tf::DeviceWorkState::DEVICE_STATUS_ALLOW_OPEN_LID)
    {
        time = INVALID_TIME;
        lab->setText("--");
        return;
    }

    // 倒计时大于1h特殊显示
    if (timeSecond > 60 * 60)
    {
        lab->setText(QString(">%1h").arg(timeSecond / (60 * 60)));
    }
    // 倒计时小于1min特殊显示
    else if(timeSecond < 60)
    {

        lab->setText("<1min");
    }
    // [1min,1h]按00:00倒计时
    else
    {
        lab->setText(time.toString("mm:ss"));
    }
}

void DeviceStatusWidget::OnTimeOut()
{
    if (!m_timeA.isValid() && !m_timeB.isValid())
    {
        m_timer.stop();
        return;
    }

    if (m_DeviceWidgetType == OPTIONAL_DEVICE)
    {
        ShowTimeToLabel(m_timeA, ui->time_label_first, m_curStatusA);
    }
    else if (m_DeviceWidgetType == ANALYSIS_ONE_DEVICE)
    {
        ShowTimeToLabel(m_timeA, ui->time_label_second, m_curStatusA);
    }
    else
    {
        ShowTimeToLabel(m_timeA, ui->time_label_first, m_curStatusA);
        ShowTimeToLabel(m_timeB, ui->time_label_second, m_curStatusB);
    }
}
