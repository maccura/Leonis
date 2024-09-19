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
/// @file     QElectrodeCleanSetDlg.h
/// @brief    电极清洗设置
///
/// @author   8580/GongZhiQiang
/// @date     2024年3月14日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年3月14日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once

#include "shared/basedlg.h"
#include <QMap>
#include "src/public/ConfigDefine.h"

namespace Ui {
    class QElectrodeCleanSetDlg;
};
class QCheckBox;

class QElectrodeCleanSetDlg : public BaseDlg
{
	Q_OBJECT

public:
	QElectrodeCleanSetDlg(QWidget *parent = Q_NULLPTR);
	~QElectrodeCleanSetDlg();

protected:
	///
	/// @brief
	///     界面显示前初始化
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年3月14日，新建函数
	///
	void InitBeforeShow();

	///
	/// @brief
	///     界面显示后初始化
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年3月14日，新建函数
	///
	void InitAfterShow();

	///
	/// @brief
	///     初始化信号槽连接
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年3月14日，新建函数
	///
	void InitConnect();

	///
	/// @brief
	///     初始化子控件
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年3月14日，新建函数
	///
	void InitChildCtrl();

	///
	/// @brief
	///     窗口显示事件
	///
	/// @param[in]  event  事件对象
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年3月14日，新建函数
	///
	void showEvent(QShowEvent *event);

protected Q_SLOTS:
	///
	/// @brief
	///     确定按钮槽函数
	///
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年3月14日，新建函数
	///
	void OnOkBtnClicked();

private:

	///
	/// @brief 添加ISE设备
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年3月14日，新建函数
	///
	void AddIseDev();

private:
	Ui::QElectrodeCleanSetDlg*				ui;
	bool									m_bInit;                            // 是否已经初始化

	QMap<QPair<QString, int>, QCheckBox*>   m_devMap;							// 设备选择框
};
