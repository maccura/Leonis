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
/// @file     WarningSetWgt.h
/// @brief    报警设置界面头文件
///
/// @author   7997/XuXiaoLong
/// @date     2023年7月13日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7997/XuXiaoLong，2023年7月13日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QWidget>
#include "shared/basedlg.h"
#include "src/public/ConfigDefine.h"

class QCheckBox;
namespace Ui {
    class WarningSetWgt;
};

class WarningSetWgt : public BaseDlg
{
	Q_OBJECT

public:
	WarningSetWgt(QWidget *parent = Q_NULLPTR);
	~WarningSetWgt();

	///
	/// @brief  设置报警设置显隐
	///
	/// @param[in]  ise		是否包含ise设备
	/// @param[in]  ch		是否包含生化设备
	/// @param[in]  im		是否包含免疫设备
	/// @param[in]  chWaste 是否包含生化废液桶
	/// @param[in]  imWaste	是否包含免疫废液桶
	///
	/// @return 
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年8月2日，新建函数
	///
	void setWarnVisible(const bool ise, const bool ch, const bool im, const bool chWaste, const bool imWaste);

protected:
	///
	/// @brief 窗口显示事件
	///     
	/// @param[in]  event  事件对象
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年09月01日，新建函数
	///
	void showEvent(QShowEvent *event) override;

private:

	///
	/// @brief  初始化UI界面
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年7月13日，新建函数
	///
	void initUI();

	///
	/// @brief  初始化信号与槽连接
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年7月13日，新建函数
	///
	void initConnect();

	///
	/// @brief  初始化所有CheckBox
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年7月19日，新建函数
	///
	void initCheckBoxs();

	///
	/// @brief  更新界面
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年9月4日，新建函数
	///
	void updateUI();

protected Q_SLOTS:

	///
	/// @brief  点击确定按钮
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年7月13日，新建函数
	///
	void slotClickSure();

private:
	Ui::WarningSetWgt* ui;

	QVector<QCheckBox*>				m_iseBoxs;					//< ISE checkBox列表
	QVector<QCheckBox*>				m_chRBoxs;					//< 生化 试剂 checkBox列表
	QVector<QCheckBox*>				m_chSBoxs;					//< 生化 耗材 checkBox列表
	QVector<QCheckBox*>				m_chLBoxs;					//< 生化 废液 checkBox列表
	QVector<QCheckBox*>				m_imRBoxs;					//< 免疫 试剂 checkBox列表
	QVector<QCheckBox*>				m_imSBoxs;					//< 免疫 耗材 checkBox列表
	QVector<QCheckBox*>				m_imLBoxs;					//< 免疫 废液 checkBox列表
	QVector<QCheckBox*>				m_imTBoxs;					//< 免疫 废料 checkBox列表
	SystemOverviewWarnSet			m_cacheWarns;				//< 缓存之前的报警设置
};
