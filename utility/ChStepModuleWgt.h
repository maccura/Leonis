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
/// @file     ChStepModuleWgt.h
/// @brief    应用-节点调试-生化步进模块
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

class QTableWidgetItem;
class QVBoxLayout;
class ChNodeDebugWgt;
class DirectionWgt;
class InjectorMachWgt;

namespace Ui {
    class ChStepModuleWgt;
};

class ChStepModuleWgt : public QWidget
{
    Q_OBJECT

public:
    ChStepModuleWgt(QWidget *parent = Q_NULLPTR);
    ~ChStepModuleWgt();

	///
	/// @brief  更新Ui数据
	///
	/// @param[in]  devSn  设备序列号
	/// @param[in]  moduleName 模块名称
	/// @param[in]  stepDatas  步进模块数据
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年3月27日，新建函数
	///
	void UpdateUi(const string& devSn, const QString& moduleName, shared_ptr<StepModule> stepDatas);

	///
	/// @brief  获取选中的行
	///
	/// @return 选中的行
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年3月27日，新建函数
	///
	set<int> getSelectRows() const;

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
	/// @brief  查询目标位置
	///
	/// @param[in]  data  方向信息
	/// @param[in]  nodeName  节点名称
	///
	/// @return (是否成功，目标位置)
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年4月24日，新建函数
	///
	pair<bool, pair<int, double>> GetTargetPos(const Direction& data, const std::string& nodeName);

	///
	/// @brief  清空方向控件
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年7月23日，新建函数
	///
	void clearWidgets();

	///
	/// @brief  替换参数（将参数=CALI_PARAM 的替换为读取的校准参数）
	///
	/// @param[in]  rows  选中的行
	/// @param[in]  cmds  替换的指令列表
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年7月26日，新建函数
	///
	void RepalceParams(const pair<int, int>& rows, QVector<CanSendData>& cmds);

protected Q_SLOTS:
	
	///
	/// @brief  点击方向的保存按钮
	///
	/// @param[in]  row  当前行
	/// @param[in]  currPos  当前位置
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年3月25日，新建函数
	///
	void OnSaveDirClicked(const int row, const int currPos);

	///
	/// @brief  点击注射器的保存按钮
	///
	/// @param[in]  row  当前行
	/// @param[in]  val  输入的校准参数值
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年3月25日，新建函数
	///
	void OnSaveInjectorClicked(const int row, const double val);
	
	///
	/// @brief  点击执行按钮
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年3月25日，新建函数
	///
    void OnExecuteClicked();

	///
	/// @brief  点击复位按钮
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年3月25日，新建函数
	///
	void OnResetClicked();

	///
	/// @brief  选中当前表格行
	///
	/// @param[in]  item  选中itemn 
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年4月24日，新建函数
	///
	void OnSelectTableItem(QTableWidgetItem *item);

private:
    Ui::ChStepModuleWgt*  ui;

	shared_ptr<StepModule>			m_spData = nullptr;			// 节点数据指针
	string							m_devSN;					// 当前选中的设备序列号
	QVBoxLayout*					m_layout = nullptr;			// 存放方向界面的布局
	int								m_oldCaliIndex = -1;		// 老的校准索引
	ChNodeDebugWgt*					m_parent = nullptr;			// 父指针
	QList<DirectionWgt *>			m_dirWgtList;				// 方向控件列表
	QString							m_moduleName;				// 当前选中的模块名
	InjectorMachWgt*				m_injectWgt = nullptr;		// 当前选中的注射器电机
};