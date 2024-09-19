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
/// @file     ImDevFacadeModuleWgt.h
/// @brief    应用-节点调试-生化泵模块
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
#include <QMap>
#include <QCheckBox>

namespace Ui {
    class ImDevFacadeModuleWgt;
};

class ImDevFacadeModuleWgt : public QWidget
{
    Q_OBJECT

public:
    ImDevFacadeModuleWgt(QWidget *parent = Q_NULLPTR);
    ~ImDevFacadeModuleWgt();

	///
	/// @brief  更新Ui数据
	///
	/// @param[in]  pumpDatas  泵模块数据
	///
	/// @par History:
	/// @li 4170/TangChuXian，2024年3月27日，新建函数
	///
	void UpdateUi(const im::tf::DebugModule& stuTfDbgMd);

    ///
    /// @brief  更新界面信息
    ///
    /// @param[in]  devSN  设备序列号
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年4月8日，新建函数
    ///
    inline void UpdateDevSn(const std::string& devSN) { m_strDevSn = devSN; }

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
    /// @brief
    ///     复选框被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年4月9日，新建函数
    ///
    void OnCheckBoxClicked();

private:
    Ui::ImDevFacadeModuleWgt*  ui;

	bool							        m_bUpdate = false;			// 处于刷新阶段时无需触发checkbox槽函数
    std::string                             m_strDevSn;                 // 设备序列号
    QMap<QCheckBox*, im::tf::DebugPart>     m_mapCBdata;                // 映射【复选框-调试部件数据】
    im::tf::DebugModule                     m_stiTfDbgModule;           // 调试模块
};
