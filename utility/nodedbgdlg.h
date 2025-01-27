﻿/***************************************************************************
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
/// @file     nodedbgdlg.h
/// @brief    节点调试对话框
///
/// @author   4170/TangChuXian
/// @date     2024年1月12日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月12日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once

#include "shared/basedlg.h"
#include "src/thrift/gen-cpp/defs_types.h"
#include <QMap>
#include "iBITDebugger.h"
namespace Ui { class NodeDbgDlg; };

// 前置声明
class QTableWidgetItem;                     // 表格单元项
class ChNodeDebugWgt;
class ImNodeDebugWgt;
class TrkIOMDebugWgt;
class TrkRouterDebugWgt;
class TrkRWCDebugWgt;
class TrkDQIDebugWgt;

class NodeDbgDlg : public BaseDlg
{
    Q_OBJECT

public:
    NodeDbgDlg(QWidget *parent = Q_NULLPTR);
    ~NodeDbgDlg();

protected:
    ///
    /// @brief
    ///     界面显示前初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年1月12日，新建函数
    ///
    void InitBeforeShow();

    ///
    /// @brief
    ///     界面显示后初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年1月12日，新建函数
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
    /// @li 4170/TangChuXian，2024年1月12日，新建函数
    ///
    void InitConnect();

    ///
    /// @brief
    ///     初始化子控件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年1月12日，新建函数
    ///
    void InitChildCtrl();

    ///
    /// @brief
    ///     窗口显示事件
    ///
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年1月12日，新建函数
    ///
    void showEvent(QShowEvent *event);

protected Q_SLOTS:

	///
	/// @brief  点击右上角的设备
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年3月27日，新建函数
	///
	void OnDeviceClicked();

    ///
    /// @brief
    ///     导出按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年4月10日，新建函数
    ///
    void OnExportBtnClicked();

    ///
    /// @brief
    ///     导入按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年4月10日，新建函数
    ///
    void OnImportBtnClicked();

    ///
    /// @brief
    ///     关闭按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年4月11日，新建函数
    ///
    void OnCloseBtnClicked();

private:
    Ui::NodeDbgDlg                  *ui;						// ui指针
    bool                             m_bInit;					// 是否已经初始化
	ChNodeDebugWgt					*m_chNodeDebugWgt = nullptr;// 生化节点调试界面
    ImNodeDebugWgt                  *m_imNodeDebugWgt = nullptr;// 免疫节点调试界面
	string							m_currSelDevSn;				// 当前选中设备序列号
};

void OutputData(MLINEDEBUG::OutputData* datas, int size);
void EventHandle(MLINEDEBUG::FlowDebugStatus event);
void DebugerLog(const char *log);