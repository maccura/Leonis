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
/// @file     sensorstatedlg.h
/// @brief    流程检查对话框
///
/// @author   4170/TangChuXian
/// @date     2024年2月22日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2024年2月22日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once

#include "shared/basedlg.h"
#include "src/thrift/gen-cpp/defs_types.h"
#include "src/thrift/im/gen-cpp/im_types.h"
#include <QMap>
namespace Ui { class SensorStateDlg; };

// 前置声明
class QTableWidgetItem;                     // 表格单元项

class SensorStateDlg : public BaseDlg
{
    Q_OBJECT

public:
    SensorStateDlg(QWidget *parent = Q_NULLPTR);
    ~SensorStateDlg();

protected:
    ///
    /// @brief
    ///     界面显示前初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年2月22日，新建函数
    ///
    void InitBeforeShow();

    ///
    /// @brief
    ///     界面显示后初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年2月22日，新建函数
    ///
    void InitAfterShow();

    ///
    /// @brief
    ///     初始化字符串资源
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年1月17日，新建函数
    ///
    void InitStrResource();

    ///
    /// @brief
    ///     初始化信号槽连接
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年2月22日，新建函数
    ///
    void InitConnect();

    ///
    /// @brief
    ///     初始化子控件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年2月22日，新建函数
    ///
    void InitChildCtrl();

    ///
    /// @brief
    ///     窗口显示事件
    ///
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年2月22日，新建函数
    ///
    void showEvent(QShowEvent *event);

protected Q_SLOTS:

	///
	/// @brief
	///     开始监测按钮被点击
	///
	/// @par History:
	/// @li 4170/TangChuXian，2024年3月29日，新建函数
	///
	void OnCloseBtnClicked();

	///
	/// @brief
	///     开始监测按钮被点击
	///
    /// @par History:
    /// @li 4170/TangChuXian，2024年3月29日，新建函数
    ///
    void OnStartWatchBtnClicked();

    ///
    /// @brief
    ///     停止监测按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年3月29日，新建函数
    ///
    void OnStopWatchBtnClicked();

    ///
    /// @brief
    ///     设备单选框被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年3月28日，新建函数
    ///
    void OnDevRBtnClicked();

    ///
    /// @brief
    ///     更新传感器状态表
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年3月28日，新建函数
    ///
    void UpdateSensorStateTbl();

    ///
    /// @brief
    ///     传感器状态更新
    ///
    /// @param[in]  strDevSn    设备序列号
    /// @param[in]  mapStatus   传感器状态
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年3月29日，新建函数
    ///
    void OnSensorStateUpdate(const tf::SubDeviceInfo deviceInfo, QMap<int, tf::EmSensorResultType::type> mapStatus);

    ///
    /// @brief
    ///     更新表格传感器状态
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年3月29日，新建函数
    ///
    void UpdateTblSensorStatus();

private:
    // 传感器状态，表头枚举
    enum SensorStateHeader {
        Ssh_Module = 0,                 // 模块
        Ssh_FuncModule,                 // 功能模块
        Ssh_SensorNameOrTrackMd,        // 传感器名称/轨道模块
        Ssh_SensorFunction,             // 传感器功能
        Ssh_SensorState,                // 传感器状态
    };

private:
    Ui::SensorStateDlg                 *ui;                     // ui指针
    bool                                m_bInit;                // 是否已经初始化
	bool								m_bIsMonitor;			// 是否正在监控
    // 表格
    int                                 m_ciDefaultRowCnt;      // 默认行数
    int                                 m_ciDefaultColCnt;      // 默认列数

    // 设备列表
	::tf::SubDeviceInfo					m_strCurDevInfo;        // 当前设备名称
	std::vector<::tf::SubDeviceInfo>	m_listSubDevice;		// 设备列表

    // 状态缓存
    QMap<QString, QMap<int, tf::EmSensorResultType::type>>  m_mapStatusBuffer;      // 状态缓存[设备SN-传感器状态]
};
