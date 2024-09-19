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
/// @file     QFaultLogWidget.h
/// @brief    故障日志功能展示
///
/// @author   7951/LuoXin
/// @date     2022年7月5日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2022年7月7日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <thread>
#include <QWidget>
#include <atomic>
#include <condition_variable>
#include "model/FaultLogTableModel.h"
#include "src/thrift/gen-cpp/defs_types.h"

class QTimer;
class QUtilitySortFilterProxyModel;
namespace Ui {
    class QFaultLogWidget;
};

class QFaultLogWidget : public QWidget
{
	Q_OBJECT

public:
	QFaultLogWidget(QWidget *parent = Q_NULLPTR);
	~QFaultLogWidget();

    ///
    /// @brief 重置界面
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年1月11日，新建函数
    ///
    void ResetWidget();

    ///
    /// @brief 停止查询线程
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年10月23日，新建函数
    ///
    void StopQueryAlarmLog() { m_ambNeedStop = true; }

private:
	///
	/// @brief 初始化日志展示模块
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年7月5日，新建函数
	///
	void Init();

	///
	/// @brief	查询指定的告警日志
	/// @par History:
	/// @li 7951/LuoXin，2022年7月28日，新建函数
	///
	void FaultLogQuery();

	///
	/// @brief
	///     报警描述信息解码
	///
	/// @param[in]  alarmDesc  报警描述信息
	///
	/// @return item
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年7月18日，新建函数
	///
	FaultLogTableModel::StFaultLogItem AlarmDescDecode(::tf::AlarmDesc alarmDesc);

protected Q_SLOTS:

	///
	/// @brief
	///     告警日志查询页面确定按钮槽函数
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年7月27日，新建函数
	///
	void OnFaultLogQueryBtn();


	///
	/// @brief	查询完成响应的槽函数
	/// @par History:
	/// @li 7951/LuoXin，2022年7月28日，新建函数
	///
	void OnQueryFinished();

	///
	/// @brief
	///     获取异步查询缓存
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年12月23日，新建函数
	///
	void GetBuffer();

    ///
    /// @brief
    ///     当前选中行改变
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年6月8日，新建函数
    ///
    void OnCurrentRowChanged(const QModelIndex &current, const QModelIndex &previous);

    ///
    /// @bref
    ///		权限变化响应
    ///
    /// @par History:
    /// @li 7951/LuoXin, 2024年9月12日，新建函数
    ///
    void OnPermisionChanged();

Q_SIGNALS:
	///
	/// @brief
	///     查询完成信号
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年7月28日，新建函数
	///
	void QueryFinished();
private:
	Ui::QFaultLogWidget							*ui;

	// 线程同步对象
	std::shared_ptr<std::thread>				 m_pQryTrd;					// 告警日志查询线程
	std::mutex									 s_mtxAlarmData;			// 告警日志查询结果队列互斥锁
	std::atomic_bool					         m_ambNeedStop;			    // 线程停止原子变量
	std::condition_variable					     m_condPutBuffer;			// 写缓存条件变量
	std::vector<::tf::AlarmDesc>				 m_vecAlarmDescs;			// 告警日志查询结果队列
	QTimer*										 m_pGetBufferTimer;			// 告警日志查询结果队列
    QUtilitySortFilterProxyModel*                m_sortModel;               // 表格排序的model
    bool                                         m_isManager;               // 当前登录的是否是工程师以上的权限
};
