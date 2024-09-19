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

#pragma once

#include "shared/basedlg.h"

class QStandardItemModel;
namespace Ui {
    class QAssayShowPostion;
}

class QAssayShowPostion : public BaseDlg
{
    Q_OBJECT

public:
    QAssayShowPostion(QWidget *parent = Q_NULLPTR);
    ~QAssayShowPostion();

    // 显示事件
    void showEvent(QShowEvent *event);

    protected slots:

    // 刷新界面
    void OnReFresh();

    // 保存数据
    void OnSaveData();

    // 重置按钮槽函数，按升序排列项目
    void SortItemASC();

    ///
    /// @brief	更新控件的显示和编辑权限
    ///    
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年3月27日，新建函数
    ///
    void UpdateCtrlsEnabled();

private:

    // 初始化
    void Init();

    ///
    /// @brief
    ///     
    ///
    /// @param[in]  row         行
    /// @param[in]  direction   方向(true：向上) 
    ///
    /// @return     成功返回true
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年6月27日，新建函数
    ///
    bool MoveItem(int row, bool direction);

    // 更新按钮状态
    void UpdateBtnState();

private:
    Ui::QAssayShowPostion*           ui;
    QStandardItemModel*				m_model;			///< 自定义模式
};
