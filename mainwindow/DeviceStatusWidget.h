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
#include <QTime>
#include <QWidget>
#include <QTimer>

class QLabel;
namespace Ui { class DeviceStatusWidget; };

class DeviceStatusWidget : public QWidget
{
    Q_OBJECT

public:

    enum DeviceWidgetType
    {
        OPTIONAL_DEVICE,        // 选配设备  
        ANALYSIS_ONE_DEVICE,    // 分析仪单设备
        ANALYSIS_TWO_DEVICE     // 分析仪两个设备
    };

    DeviceStatusWidget(DeviceWidgetType type, QString name, QVector<int> devTypeVec, QWidget *parent = Q_NULLPTR);
    ~DeviceStatusWidget();

    void AddDeviceSn(QString sn) { m_snList.push_back(sn); }

    QVector<int> GetDeviceType() { return m_deviceTypeVec; }
    QStringList GetDeviceSns(int devType);

    void SetGroupName(QString name);
    QString GetGroupName(){ return m_groupName; }

    ///
    /// @brief  设置名字
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2024年8月30日，新建函数
    ///
    void SetName(QString name, bool isFirst);

    ///
    /// @brief  更新设备状态
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2024年4月25日，新建函数
    ///
    void UpdateStatus(int status, bool isFirst);

    ///
    /// @brief  更新设备屏蔽状态
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2024年4月25日，新建函数
    ///
    void UpdateMasked(bool isMasked, bool isFirst);

    ///
    /// @brief  更新倒计时
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2024年4月25日，新建函数
    ///
    void UpdateTime(int time, bool isFirst);

    ///
    /// @brief  处理试剂耗材类报警
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2024年7月12日，新建函数
    ///
    void UpdateReagentAlarm(bool existReagentAlarm);

private:
    ///
    /// @brief  显示倒计时
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2024年5月8日，新建函数
    ///
    void ShowTimeToLabel(QTime& time, QLabel* lab, int status);

private Q_SLOTS:
    ///
    /// @brief  显示倒计时
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2024年8月29日，新建函数
    ///
    void OnTimeOut();

private:
    Ui::DeviceStatusWidget*							ui;
    DeviceWidgetType                                m_DeviceWidgetType;     // 当前的模式
    QStringList                                     m_snList;               // 设备序列号列表
    QVector<int>                                    m_deviceTypeVec;        // 设备类型
    QTimer                                          m_timer;                // 倒计时定时器
    QTime                                           m_timeA;                // A设备倒计时
    QTime                                           m_timeB;                // B设备倒计时
    int                                             m_curStatusA;           // 当前状态
    int                                             m_curStatusB;           // 当前状态
    bool                                            m_isMaskedA;            // 是否被屏蔽
    bool                                            m_isMaskedB;            // 是否被屏蔽
    bool                                            m_isReagentAlarm;       // 是否存在试剂报警
    QString                                         m_groupName;            // 生化2000的设备组名
};
