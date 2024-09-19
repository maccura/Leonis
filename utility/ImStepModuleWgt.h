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
/// @file     ImStepModuleWgt.h
/// @brief    应用-节点调试-生化步进模块
///
/// @author   4170/TangChuXian
/// @date     2024年3月25日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月25日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include "thrift/im/ImLogicControlProxy.h"
#include <QWidget>

namespace Ui {
    class ImStepModuleWgt;
};

class ImStepModuleWgt : public QWidget
{
    Q_OBJECT

public:
    ImStepModuleWgt(QWidget *parent = Q_NULLPTR);
    ~ImStepModuleWgt();

	///
	/// @brief  更新Ui数据
	///
	/// @param[in]  stepDatas  步进模块数据
	///
	/// @par History:
	/// @li 4170/TangChuXian，2024年3月27日，新建函数
	///
	void UpdateUi(const im::tf::DebugModule& stuTfDbgMd, bool reset = true);

    ///
    /// @brief  更新界面信息
    ///
    /// @param[in]  devSN  设备序列号
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年4月8日，新建函数
    ///
    inline void UpdateDevSn(const std::string& devSN) { m_strDevSn = devSN; }

protected:
    ///
    /// @brief
    ///     显示事件
    ///
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年8月7日，新建函数
    ///
    void showEvent(QShowEvent *event) override;

    ///
    /// @brief
    ///     显示事件
    ///
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年8月7日，新建函数
    ///
    void hideEvent(QHideEvent *event) override;

private:
    ///
    /// @brief	初始化UI
    ///     
    /// @par History:
	/// @li 4170/TangChuXian，2024年3月25日，新建函数
    ///
    void InitUi();

	///
	/// @brief	初始化数据
	///     
	/// @par History:
	/// @li 4170/TangChuXian，2024年3月25日，新建函数
	///
	void InitData();

	///
	/// @brief	初始化连接
	///     
	/// @par History:
	/// @li 4170/TangChuXian，2024年3月25日，新建函数
	///
	void InitConnect();

protected Q_SLOTS:

	///
	/// @brief  点击正向按钮
	///
	/// @par History:
	/// @li 4170/TangChuXian，2024年3月25日，新建函数
	///
    void OnAddClicked();

	///
	/// @brief  点击反向按钮
	///
	/// @par History:
	/// @li 4170/TangChuXian，2024年3月25日，新建函数
	///
	void OnDelClicked();

	///
	/// @brief  点击执行按钮
	///
	/// @par History:
	/// @li 4170/TangChuXian，2024年3月25日，新建函数
	///
    void OnExecuteClicked();

	///
	/// @brief  点击保存按钮
	///
	/// @par History:
	/// @li 4170/TangChuXian，2024年3月25日，新建函数
	///
	void OnSaveClicked();

    ///
    /// @brief  点击正向按钮
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年3月25日，新建函数
    ///
    void OnAdd2Clicked();

    ///
    /// @brief  点击反向按钮
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年3月25日，新建函数
    ///
    void OnDel2Clicked();

    ///
    /// @brief  点击保存按钮
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年3月25日，新建函数
    ///
    void OnSave2Clicked();

    ///
    /// @brief
    ///     选中项改变
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年4月7日，新建函数
    ///
    void OnSelectListItem();

private:
    Ui::ImStepModuleWgt*            ui;
    std::string                     m_strDevSn;                 // 设备序列号
    im::tf::DebugModule             m_stiTfDbgModule;           // 调试模块
};
