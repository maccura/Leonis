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
/// @file     ChNodeDebugWgt.h
/// @brief    生化节点调试界面
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
#include <QEventLoop>
#include <QThread>
#include "ChDebugStruct.h"
#include "ChExeCmdObj.h"

using namespace std;

class ChStepModuleWgt;
class ChPumpModuleWgt;
class ChValveModuleWgt;
class ChSwitchModuleWgt;
class ChUltraMixModuleWgt;
class McProgressDlg;
class QTableWidgetItem;

namespace Ui {
    class ChNodeDebugWgt;
};

class ChNodeDebugWgt : public QWidget
{
    Q_OBJECT

public:
    ChNodeDebugWgt(QWidget *parent = Q_NULLPTR);
    ~ChNodeDebugWgt();

	///
	/// @brief  更新界面信息
	///
	/// @param[in]  devSN  设备序列号
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年3月26日，新建函数
	///
	void UpdateUi(const string& devSN);

	///
	/// @brief  设置数据并开启执行指令线程
	///
	/// @param[in]  type  指令类型
	/// @param[in]  cmds  指令列表
	/// @param[in]  tips  提示内容
	/// @param[in]  eleCmd  电流标定指令
	///
	/// @return 
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年7月26日，新建函数
	///
	void SetDataAndRunCmd(ChExeCmdObject::ExeType type, const QVector<CanSendData>& cmds, const QString& tips, const Cmd& eleCmd = Cmd());

	///
	/// @brief  导出文件
	///
	/// @param[in]  devSN  设备序列号
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年8月22日，新建函数
	///
	void Export(const string& devSN);

	///
	/// @brief  导入文件
	///
	/// @param[in]  devSN  设备序列号
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年8月22日，新建函数
	///
	void Import(const string& devSN);

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
	/// @brief  加载配置文件
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年3月26日，新建函数
	///
	void LoadConfigFile();

	///
	/// @brief  切出模块执行指令
	///
	/// @param[in]  moduleId  模块索引
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年7月23日，新建函数
	///
	void OutModuleExeCmds(const int moduleId);

	///
	/// @brief  点击模块执行指令
	///
	/// @param[in]  moduleId  模块索引
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年7月23日，新建函数
	///
	void EnterModuleExeCmds(const int moduleId);

protected Q_SLOTS:

	///
	/// @brief  选中当前表格行
	///
	/// @param[in]  item  选中itemn 
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年4月24日，新建函数
	///
	void OnSelectTableItem(QTableWidgetItem *item);

	///
	/// @brief  线程执行完成
	///
	/// @param[in]  type	执行的类型
	/// @param[in]  result  执行完成的返回结果
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年4月24日，新建函数
	///
	void OnThreadFinished(const int type, ::tf::StCanResult result);

private:
    Ui::ChNodeDebugWgt*  ui;

	ChStepModuleWgt*				m_stepModuleWgt = nullptr;	// 步进模块界面
	ChPumpModuleWgt*				m_pumpModuleWgt = nullptr;	// 泵模块界面
	ChValveModuleWgt*				m_valveModuleWgt = nullptr;	// 阀模块界面
	ChSwitchModuleWgt*				m_switchModuleWgt = nullptr;// 开关模块界面
	ChUltraMixModuleWgt*			m_ultraMixMoudleWgt = nullptr;// 超声混匀模块界面
	shared_ptr<QMap<string, QMap<int, ChNodeDebug>>> m_spMapChNodeDebugInfos;	// 生化节点调试数据(设备序列号，(模块id，节点数据))
	string							m_devSN;					// 当前选中的设备序列号
	pair<int, int>					m_oldModuleIdType{ -1,-1 };	// 老的选中模块id和类型
	QThread*						m_workThread = nullptr;		// 执行指令时的工作线程
	std::shared_ptr<McProgressDlg>	m_progressDlg = nullptr;    // 进度条对话框
	ChExeCmdObject*					m_chExeCmdObj = nullptr;	// 执行指令类
	QEventLoop						m_loop;						// 事件循环
};