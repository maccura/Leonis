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
/// @file     DevManageDlg.h
/// @brief    应用--系统--设备管理
///
/// @author   7951/LuoXin
/// @date     2022年9月30日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2022年9月30日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include "shared/basedlg.h"
#include "src/thrift/gen-cpp/defs_types.h"

class QCheckBox;
class QStandardItemModel;
namespace Ui {
    class DevManageDlg;
};

class DevManageDlg : public BaseDlg
{
	Q_OBJECT

public:
	DevManageDlg(QWidget *parent = Q_NULLPTR);
	~DevManageDlg();

	///
	/// @brief	加载数据到控件
	///     
	/// @par History:
	/// @li 7951/LuoXin，2022年9月30日，新建函数
	///
	void LoadDataToCtrls();

private:
	///
	/// @brief	初始化控件
	///     
	/// @par History:
	/// @li 7951/LuoXin，2022年9月30日，新建函数
	///
	void InitCtrls();

    ///
    /// @brief	解锁设备翻盖
    ///
    /// @param[in]  type  前/后翻盖,参见tf::EnumFlapType::type
    ///    
    /// @par History:
    /// @li 7951/LuoXin，2023年11月2日，新建函数
    ///
    void UnlockFlap(int type);

    ///
    /// @brief
    ///     试剂仓加卸载是否正在运行
    ///
    /// @param[in]  strDevSn  设备序列号
    /// @param[in]  bRunning  是否正在运行
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月30日，新建函数
    ///
    void OnRgntLoadStatusChanged(QString strDevSn, bool bRunning);

	protected Q_SLOTS:

	///
	/// @brief	下电按钮被点击
	///     
	/// @par History:
	/// @li 7951/LuoXin，2022年10月08日，新建函数
	///
	void OnPowerDownBtnClicked();

	///
	/// @brief	遮蔽按钮被点击
	///     
	/// @par History:
	/// @li 7951/LuoXin，2022年10月08日，新建函数
	///
	void OnMaskBtnClicked();

    ///
    /// @brief	勾选框状态改变
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2023年5月4日，新建函数
    ///
    void OnCheckBoxStatusChanged(bool checked);

    ///
    /// @brief
    ///     更新设备状态
    ///
    /// @param[in]  deviceInfo  设备状态信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年5月4日，新建函数
    ///
    void OnDevStateChange(tf::DeviceInfo deviceInfo);

    ///
    /// @brief
    ///     当前选中行改变
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年5月4日，新建函数
    ///
    void OnSelectRowChanged();

    ///
    /// @brief  双击表格响应
    ///     
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年1月24日，新建函数
    ///
    void OntableViewDoubleClicked(const QModelIndex &index);

    ///
    /// @brief 权限变更
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年3月29日，新建函数
    ///
    void OnPermisionChanged();

private:
	Ui::DevManageDlg*									ui;
	QStandardItemModel*									m_tabViewMode;				///< 列表的模式
	QCheckBox*											m_checkBox;					///< 表头的勾选框
    bool                                                m_ImReagLoadRuning;         ///< 试剂加卸载状态
    QString                                             m_powerOffStr;              ///< 界面展示文字（下电）
    QString                                             m_powerOnStr;               ///< 界面展示文字（上电）
};
