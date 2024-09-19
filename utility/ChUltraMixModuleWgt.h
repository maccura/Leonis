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
/// @file     ChUltraMixModuleWgt.h
/// @brief    应用-节点调试-生化超声混匀模块
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

class ChNodeDebugWgt;
class QTableWidgetItem;

namespace Ui {
    class ChUltraMixModuleWgt;
};

class ChUltraMixModuleWgt : public QWidget
{
    Q_OBJECT

public:
    ChUltraMixModuleWgt(QWidget *parent = Q_NULLPTR);
    ~ChUltraMixModuleWgt();

	///
	/// @brief  更新Ui数据
	///
	/// @param[in]  devSn  设备序列号
	/// @param[in]  ultraMixDatas  超声混匀数据
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年3月27日，新建函数
	///
	void UpdateUi(const string& devSn, shared_ptr<UltraMixModule> ultraMixDatas);

	///
	/// @brief  更新电流标定返回值
	///
	/// @param[in]  strData  
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年9月1日，新建函数
	///
	void UpdateEleCaliVals(const string & strData);

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
	/// @brief  查询校准参数
	///
	/// @param[in]  name  节点名称
	/// @param[in]  addr  参数地址
	///
	/// @return (是否成功，校准参数)
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年4月24日，新建函数
	///
	pair<bool, int> GetCaliParam(const QString &name, const int addr);

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
	/// @brief  点击保存按钮
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年4月24日，新建函数
	///
	void OnSaveClicked();

	///
	/// @brief  点击电流标定按钮
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年4月24日，新建函数
	///
	void OnEleCaliClicked();

	///
	/// @brief  点击使用电流标定返回值按钮
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年4月24日，新建函数
	///
	void OnUseReturnClicked();

private:
    Ui::ChUltraMixModuleWgt*  ui;

	shared_ptr<UltraMixModule>		m_spDatas = nullptr;		// 节点数据指针
	string							m_devSN;					// 当前选中的设备序列号
	QVector<int>					m_eleCaliVals;				// 电流标定得到的返回值
	ChNodeDebugWgt*					m_parent = nullptr;			// 父指针
};