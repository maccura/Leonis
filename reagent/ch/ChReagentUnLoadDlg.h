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
/// @file     ChReagentUnLoadDlg.h
/// @brief    生化试剂卸载对话框
///
/// @author   7997/XuXiaoLong
/// @date     2024年8月14日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7997/XuXiaoLong，2024年8月14日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once
#include "shared/basedlg.h"
#include "src/thrift/gen-cpp/defs_types.h"

namespace Ui {
    class ChReagentUnLoadDlg;
};

class ChReagentUnLoadDlg : public BaseDlg
{
    Q_OBJECT

public:
	ChReagentUnLoadDlg(QWidget *parent);
    ~ChReagentUnLoadDlg();

	///
	/// @brief  设置数据
	///
	/// @param[in]  devInfo		设备信息
	/// @param[in]  selectPos	当前选中位置 
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年8月14日，新建函数
	///
	void SetData(std::shared_ptr<const tf::DeviceInfo> devInfo, const int selectPos);

private:

    ///
    /// @brief  初始化ui
    ///
    /// @par History:
    /// @li 7997/XuXiaoLong，2024年8月14日，新建函数
    ///
    void InitUi();

	///
	/// @brief  初始化连接
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年8月14日，新建函数
	///
	void InitConnect();

	///
	/// @brief  检查是否可卸载
	///
	/// @param[in]  setPos  输入的位置
	///
	/// @return 不为空表示不可卸载
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年8月14日，新建函数
	///
	QString CheckUnloadStatus(const std::set<int> &setPos);

private slots:

    ///
    /// @brief 确定按钮槽函数
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年6月7日，新建函数
    ///
    void OnSureClicked();

private:
    Ui::ChReagentUnLoadDlg* ui;

	std::shared_ptr<const tf::DeviceInfo>	m_spDevInfo;		///< 当前设备信息
};
