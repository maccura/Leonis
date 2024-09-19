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
/// @file     statusitemdelegate.h
/// @brief    状态代理
///
/// @author   4170/TangChuXian
/// @date     2021年8月25日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2021年8月25日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <QStyledItemDelegate>

#define BK_STATE_ROLE              (Qt::UserRole + 2)               // 背景状态
#define BK_HEADER_ROLE             (Qt::UserRole + 3)               // 表头背景
#define BK_FLAG_ROLE               (Qt::UserRole + 4)               // 文字右上角标识

// 失控处理完成标识
#define OUT_CONTROL_HAND_PNG		(":/Leonis/resource/image/icon-outcontrol-handled.png")

class StatusItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    enum BackGroundState
    {
        STATE_NORMAL = 0,
        STATE_WARNING,
        STATE_ERROR,
        STATE_WORK,
        STATE_INVALID
    };

public:
    StatusItemDelegate(QObject *parent = Q_NULLPTR);
    ~StatusItemDelegate();

protected:
    ///
    /// @brief
    ///     创建编辑器
    ///
    /// @param[in]  parent  父对象
    /// @param[in]  option  选项
    /// @param[in]  index   索引
    ///
    /// @return 编辑器对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年8月25日，新建函数
    ///
    //QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    ///
    /// @brief
    ///     绘制displaymode的时候的函数
    ///
    /// @param[in]  painter  图形绘制句柄
    /// @param[in]  option  选项
    /// @param[in]  index  索引
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年1月9日，新建函数
    ///
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex & index) const override;

    ///
    /// @brief
    ///     设置编辑器数据
    ///
    /// @param[in]  editor  编辑器
    /// @param[in]  index   索引
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年8月25日，新建函数
    ///
    //void setEditorData(QWidget *editor, const QModelIndex &index) const override;

    ///
    /// @brief
    ///     设置模型数据
    ///
    /// @param[in]  editor  编辑器
    /// @param[in]  model   模型
    /// @param[in]  index   索引
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年8月25日，新建函数
    ///
    //void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
};
