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
/// @file     QRegisterSupplyInfo.cpp
/// @brief    试剂耗材信息录入
///
/// @author   5774/WuHongTao
/// @date     2022年3月18日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月18日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include "shared/basedlg.h"
#include "src/thrift/gen-cpp/defs_types.h"
#include "src/thrift/ch/gen-cpp/ch_types.h"

namespace Ui {
    class QRegisterSupplyInfo;
};

class QRegisterSupplyInfo : public BaseDlg
{
    Q_OBJECT

public:
    QRegisterSupplyInfo(QWidget *parent = Q_NULLPTR);
    ~QRegisterSupplyInfo();

	///
	/// @brief 设置试剂位置和状态
	///
	/// @param[in]  pos		   耗材信息
	/// @param[in]  devSn      设备序列号
	/// @param[in]  devName    设备名称+“-”+位置
	///
	/// @return 
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年6月10日，新建函数
	///
    void SetSupplyPostion(const ch::tf::SuppliesInfo& supply, const std::string& devSn, const std::string &devName);

private:
    ///
    /// @brief 根据情况初始化项目列表
    ///
    /// @param[in]  suppliesCode   当前耗材的编号（-1,选中空）
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年6月10日，新建函数
    ///
    void RefreshAssayList(int suppliesCode);

private slots:
    ///
    /// @brief
    ///     
    ///     保存数据
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月18日，新建函数
    ///
    void OnOkButtonClicked();
private:
    Ui::QRegisterSupplyInfo* ui;

    std::string						m_currentDevSn;				///< 当前设备序列号
	int								m_pos = 0;					///< 信息录入位置
};
