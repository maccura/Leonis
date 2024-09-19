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
/// @file     QCaliBrateMethodApp.h
/// @brief    校准方法
///
/// @author   5774/WuHongTao
/// @date     2022年3月4日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月4日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QWidget>
#include "shared/basedlg.h"
#include "src/thrift/gen-cpp/defs_types.h"

namespace Ui {
    class QCaliBrateMethodApp;
};


class QCaliBrateMethodApp : public BaseDlg
{
    Q_OBJECT

public:
    QCaliBrateMethodApp(QWidget *parent = Q_NULLPTR);
    ~QCaliBrateMethodApp();


	///
	/// @brief
	///     设置当前方法和状态
	///
	/// @param[in]  method  方法
	/// @param[in]  isEableBlank  状态   true：空白可选    false：空白不可选
	///
	/// @return 
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年3月2日，新建函数
	/// @li 8580/GongZhiQiang，2022年12月28日，增加空白是否可选功能
	///
    void SetMethod(tf::CaliMode::type method, bool isEableBlank);

	///
	/// @brief 设置界面上全点以外按钮的可用性
	///
	/// @param[in]  enable  控件是否可用
	///
	/// @return 
	///
	/// @par History:
	/// @li 8090/YeHuaNing，2022年10月9日，新建函数
	///
	void SetContrlEnable(bool enable);

Q_SIGNALS:
    ///
    /// @brief
    ///     校准申请方法结果
    ///
    /// @param[in]   校准方法
	/// @param[in]   是否选择（true表示选择） 
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月2日，新建函数
    ///
    void CaliBrateMethod(int, bool);

private slots:
    ///
    /// @brief
    ///     保存数据
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月2日，新建函数
    ///
    void OnSaveButton();

private:
    Ui::QCaliBrateMethodApp* ui;
};
