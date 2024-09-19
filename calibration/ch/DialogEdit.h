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
/// @file     DialogEdit.h
/// @brief    位置信息编辑
///
/// @author   5774/WuHongTao
/// @date     2020年6月17日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2020年6月17日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include "shared/basedlg.h"
#include "share.h"
namespace Ui { class DialogEdit; };

class DialogEdit : public BaseDlg
{
    Q_OBJECT
public:
    ///
    /// @brief 构造函数
    ///     
    /// @param[in]  parent  父节点
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年5月9日，新建函数
    ///
    DialogEdit(QWidget *parent = Q_NULLPTR);

    ///
    /// @brief 析构函数
    ///     
    /// @par History:
    /// @li 5774/WuHongTao，2020年5月9日，新建函数
    ///
    ~DialogEdit();

signals:
    ///
    /// @brief 关闭对话框的信号
    ///     
    /// @param[in]  data  
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年5月9日，新建函数
    ///
    void closeWindowEdit(int, int);

private slots:
    ///
    /// @brief 执行具体的修改操作
    ///     
    /// @par History:
    /// @li 5774/WuHongTao，2020年5月9日，新建函数
    ///
    void modifyDeal();

    ///
    /// @brief
    ///     清除位置信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年6月17日，新建函数
    ///
    void clearInfo();

private:
    Ui::DialogEdit                  *ui;
};
