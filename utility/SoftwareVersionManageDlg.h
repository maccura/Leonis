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
/// @file     SoftwareVersionManageDlg.h
/// @brief    软件版本弹窗
///
/// @author   7951/LuoXin
/// @date     2024年03月04日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2024年03月04日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "shared/basedlg.h"

class QStandardItemModel;

namespace Ui {
    class SoftwareVersionManageDlg;
};

class SoftwareVersionManageDlg : public BaseDlg
{
    Q_OBJECT

public:
    SoftwareVersionManageDlg(QWidget *parent = Q_NULLPTR);
    ~SoftwareVersionManageDlg();

private:

    ///
    /// @brief
    ///     窗口显示事件
    ///
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年03月04日，新建函数
    ///
    void showEvent(QShowEvent *event);

    ///
    /// @brief	更新表格
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2024年03月04日，新建函数
    ///
    void UpdateTableView(std::string devSn);

	///
	/// @brief  资源转换
	///
	/// @param[in]  devSn 设备SN
	/// @param[in]   
	/// @param[in]  result 需转换的数据
	///
	/// @return 
	///
	/// @par History:
	/// @li 4058/WangZhiNang，2024年5月24日，新建函数
	///
	void ConvertKey(std::string devSn, std::map<std::string, std::string>& result);

private:
    Ui::SoftwareVersionManageDlg*						ui;
    QStandardItemModel*									m_tabViewMode;				///< 列表的模式
};
