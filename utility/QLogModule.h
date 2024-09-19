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
/// @file     QLogModule.h
/// @brief    日志功能展示
///
/// @author   7951/LuoXin
/// @date     2022年7月5日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2022年7月5日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <QWidget>
#include "PrintExportDefine.h"

namespace Ui {
    class QLogModule;
};

class QFaultLogWidget;

class QLogModule : public QWidget
{
	Q_OBJECT

public:
	QLogModule(QWidget *parent = Q_NULLPTR);
	~QLogModule();

    void StopQueryAlarmLog();

private:
	///
	/// @brief 初始化日志展示模块
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年7月5日，新建函数
	///
	void Init();

    ///
    /// @brief 窗口显示事件
    ///     
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年10月17日，新建函数
    ///
    void showEvent(QShowEvent *event);

    ///
    /// @brief 获取报警信息
    ///
    /// @par History:
    /// @li 6889/ChenWei，2023年3月31日，新建函数
    ///
    void GetPrintAlarmInfo(AlarmInfoLog& Log);

    ///
    /// @brief 导出报警信息
    ///
    /// @par History:
    /// @li 6889/ChenWei，2023年3月31日，新建函数
    ///
    void ExportAlarmInfo(QString strFilepath);

    ///
    /// @brief 打印报警信息
    ///
    /// @par History:
    /// @li 6889/ChenWei，2023年3月31日，新建函数
    ///
    void PrintAlarmInfo();

    ///
    /// @brief 获取操作日志
    ///
    /// @par History:
    /// @li 6889/ChenWei，2023年3月31日，新建函数
    ///
    void GetPrintOperationLog(OperationLog& Log);

    ///
    /// @brief 导出操作日志
    ///
    /// @par History:
    /// @li 6889/ChenWei，2023年3月31日，新建函数
    ///
    void ExportOperationLog(QString strFilepath);

    ///
    /// @brief 打印操作日志
    ///
    /// @par History:
    /// @li 6889/ChenWei，2023年3月31日，新建函数
    ///
    void PrintOperationLog();

    ///
    /// @brief 打印试剂/耗材更换日志
    ///
    /// @par History:
    /// @li 6889/ChenWei，2023年3月31日，新建函数
    ///
    void GetPrintReagentSupplyChangeLog(ReagentChangeLog& Log);

    ///
    /// @brief 导出更换日志
    ///
    /// @par History:
    /// @li 6889/ChenWei，2023年3月31日，新建函数
    ///
    void ExportChangeLog(QString strFilepath);

    ///
    /// @brief 打印更换日志
    ///
    /// @par History:
    /// @li 6889/ChenWei，2023年3月31日，新建函数
    ///
    void PrintChangeLog();

protected Q_SLOTS:
    ///
    /// @brief 响应打印按钮
    ///
    /// @par History:
    /// @li 6889/ChenWei，2023年3月31日，新建函数
    ///
    void OnPrintBtnClicked();

    ///
    /// @brief 响应导出按钮
    ///
    /// @par History:
    /// @li 6889/ChenWei，2023年3月31日，新建函数
    ///
    void OnExportBtnClicked();

    ///
    /// @bref
    ///		权限变化响应
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年9月25日，新建函数
    ///
    void OnPermisionChanged();

    ///
    /// @bref
    ///		Tab页切换
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年10月23日，新建函数
    ///
    void OnTabWidgetChangePage();

    ///
    /// @bref
    ///		响应提示信息信号
    ///
    /// @par History:
    /// @li 6889/ChenWei，2024年1月29日，新建函数
    ///
    void OnHint(QString strInfo);
Q_SIGNALS:
    void hint(QString strInfo);         // 提示信息

private:
	Ui::QLogModule*							ui;
    QFaultLogWidget*                        m_pFaultLogWidget;
};
