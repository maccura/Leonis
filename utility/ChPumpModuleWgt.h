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
/// @file     ChPumpModuleWgt.h
/// @brief    应用-节点调试-生化泵模块
///
/// @author   7997/XuXiaoLong
/// @date     2024年3月25日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7997/XuXiaoLong，2024年3月25日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <QWidget>
#include "ChDebugStruct.h"

namespace Ui {
    class ChPumpModuleWgt;
};

class ChPumpModuleWgt : public QWidget
{
    Q_OBJECT

public:
    ChPumpModuleWgt(QWidget *parent = Q_NULLPTR);
    ~ChPumpModuleWgt();

	///
	/// @brief  更新Ui数据
	///
	/// @param[in]  devSn  设备序列号
	/// @param[in]  pumpDatas  泵模块数据
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年3月27日，新建函数
	///
	void UpdateUi(const string& devSn, shared_ptr<QVector<PVSModule>> pumpDatas);

private:
    ///
    /// @brief	初始化UI
    ///     
    /// @par History:
	/// @li 7997/XuXiaoLong，2024年3月25日，新建函数
    ///
    void InitUi();

	///
	/// @brief	初始化数据
	///     
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年3月25日，新建函数
	///
	void InitData();

	///
	/// @brief	初始化连接
	///     
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年3月25日，新建函数
	///
	void InitConnect();

	///
	/// @brief  查询当前PWM值
	///
	/// @param[in]  cmd  指令信息
	/// @param[in]  nodeName  节点名称
	/// @param[in]  nodeId  节点id
	///
	/// @return (是否成功，当前PWM值)
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年4月24日，新建函数
	///
	pair<bool, int> GetCurrPwm(const Cmd& cmd, const std::string& nodeName, const int nodeId);

	///
	/// @brief  查询当前电压值
	///
	/// @param[in]  cmd  指令信息
	/// @param[in]  nodeName  节点名称
	/// @param[in]  nodeId  节点id
	///
	/// @return (是否成功，当前电压值)
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年4月24日，新建函数
	///
	pair<bool, int> GetCurrElec(const Cmd& cmd, const std::string& nodeName, const int nodeId);

protected Q_SLOTS:

	///
	/// @brief  点击checkBox
	///
	/// @param[in]  ck  true表示勾选
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年4月24日，新建函数
	///
	void OnClickCheckBox(bool ck);

	///
	/// @brief  点击PushButton
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年4月24日，新建函数
	///
	void OnClickPushButton();

private:
    Ui::ChPumpModuleWgt*  ui;

	bool							m_bUpdate = false;			// 处于刷新阶段时无需触发checkbox槽函数
	shared_ptr<QVector<PVSModule>> m_spDatas = nullptr;		// 节点数据指针
	string							m_devSN;					// 当前选中的设备序列号
};