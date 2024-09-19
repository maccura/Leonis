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
/// @file     QIsePerfusionDlg.h
/// @brief    ISE液路灌注弹窗
///
/// @author   7951/LuoXin
/// @date     2023年4月7日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2023年4月7日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <memory>
#include <QMap>
#include "shared/basedlg.h"

namespace tf { class DeviceInfo; };
namespace Ui {
    class QIsePerfusionDlg;
};
class QCheckBox;

class QIsePerfusionDlg : public BaseDlg
{
	Q_OBJECT

public:
	QIsePerfusionDlg(QWidget *parent = Q_NULLPTR);
	~QIsePerfusionDlg();

	///
	/// @brief
	///     加载项目信息到表格
	///
	/// @param[in]  di  设备信息
	///
	/// @par History:
	/// @li 7951/LuoXin，2023年4月7日，新建函数
	///
	void LoadDataToDlg(std::vector<std::shared_ptr<const ::tf::DeviceInfo>> vecDevs);

protected Q_SLOTS:

	///
	/// @brief
	///     保存按钮被点击
	/// @par History:
	/// @li 7951/LuoXin，2023年4月7日，新建函数
	///
	void OnSaveBtnClicked();

    ///
    /// @brief
    ///     填充次数文本改变
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年10月12日，新建函数
    ///
    void OnFillCntTextChanged();

    ///
    /// @brief
    ///     事件过滤器
    ///
    /// @param[in]  obj  ui对象
    /// @param[in]  evt  事件对象
    ///
    /// @return true表示已处理
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年10月12日，新建函数
    ///
    bool eventFilter(QObject *obj, QEvent *evt) override;

private:
	Ui::QIsePerfusionDlg*								ui;
};
