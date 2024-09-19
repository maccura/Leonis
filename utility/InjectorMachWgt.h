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
/// @file     InjectorMachWgt.h
/// @brief    注射器电机界面
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

class ChStepModuleWgt;

namespace Ui {
    class InjectorMachWgt;
};

class InjectorMachWgt : public QWidget
{
    Q_OBJECT

public:
	InjectorMachWgt(QWidget *parent = Q_NULLPTR);
    ~InjectorMachWgt();

	///
	/// @brief  设置数据
	///
	/// @param[in]  titleName  注射器校准参数名称
	/// @param[in]  val		   校准参数值
	/// @param[in]  devSN	   设备序列号
	/// @param[in]  row		   当前注射器校准参数对应的行
	/// @param[in]  nodeId		 当前注射器节点id
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年7月23日，新建函数
	///
	void SetData(const QString& titleName, const QString& val, const std::string& devSN, 
		const int row, const int nodeId);

	///
	/// @brief  更新当前位置
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年7月26日，新建函数
	///
	void UpdateCurrPos();

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

signals:

	///
	/// @brief  点击保存按钮发送信号
	///
	/// @param[in]  row  当前行
	/// @param[in]  val  界面输入的值
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年7月23日，新建函数
	///
	void saveClicked(const int row, const double val);

protected Q_SLOTS:

	///
	/// @brief  点击保存按钮
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年3月25日，新建函数
	///
	void OnSaveClicked();

	///
	/// @brief  点击正向按钮
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年3月25日，新建函数
	///
	void OnFrontMoveClicked();

	///
	/// @brief  点击反向按钮
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年3月25日，新建函数
	///
	void OnBackMoveClicked();

private:
    Ui::InjectorMachWgt*  ui;

	int								m_row;						// 当前行
	int								m_nodeId;					// 节点id
	std::string						m_devSN;					// 当前选中的设备序列号
	ChStepModuleWgt*				m_parent = nullptr;			// 父类指针
};