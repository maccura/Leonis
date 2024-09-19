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
/// @file     QSampleDetectDlg.h 
/// @brief    样本针酸碱余量探测弹窗
///
/// @author   7997/XuXiaoLong
/// @date     2023年10月27日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7997/XuXiaoLong，2023年10月27日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include "shared/basedlg.h"
#include <memory>

namespace Ui {
    class QSampleDetectDlg;
};

namespace tf { class DeviceInfo; };

class QSampleDetectDlg : public BaseDlg
{
	Q_OBJECT

public:
	QSampleDetectDlg(QWidget *parent = Q_NULLPTR);
	~QSampleDetectDlg();

	///
	/// @brief  加载设备信息
	///
	/// @param[in]  vecDevs  设备信息 
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年10月27日，新建函数
	///
	void LoadDataToDlg(std::vector<std::shared_ptr<const ::tf::DeviceInfo>> vecDevs);

protected Q_SLOTS:

	///
	/// @brief 保存按钮被点击
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年10月27日，新建函数
	///
	void OnSaveBtnClicked();

signals:

	///
	/// @brief  余量探测
	///
	/// @param[in]  data  设备序列号和设备类型 
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年10月27日，新建函数
	///
	void signalRemainDetect(std::vector<std::pair<std::string, int>> data);

private:
	Ui::QSampleDetectDlg*			ui;
};
