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
/// @file     comboxtabledlg.h
/// @brief    下拉框的控件
///
/// @author   5774/WuHongTao
/// @date     2020年9月23日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2020年9月23日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QWidget>

class QGraphicsDropShadowEffect;
namespace Ui {
    class ComboxTableDlg;
}

class ComboxTableDlg : public QWidget
{
    Q_OBJECT

public:
    ///
    /// @brief
    ///     构造函数
    ///
    /// @param[in]  parent  父控件的句柄
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年9月23日，新建函数
    ///
    ComboxTableDlg(QWidget *parent = Q_NULLPTR);

    ///
    /// @brief
    ///     析构函数
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年9月23日，新建函数
    ///
    ~ComboxTableDlg();

    ///
    /// @brief
    ///     设置链表头的属性
    ///
    /// @param[in]  listHead  链表名字的字符串列表
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年9月23日，新建函数
    ///
    void SetHeaderList(const QStringList& listHead);

    ///
    /// @brief
    ///     设置下拉表的内容
    ///
    /// @param[in]  listContent  具体内容列表
    /// @param[in]  index  控制背景色的控制变量
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年9月23日，新建函数
    ///
    void SetContentList(const QStringList& listContent, int index);

    ///
    /// @brief 设置表格里当前曲线所在行（用于效果展示）
    ///
    /// @param[in]  index  行索引
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年7月24日，新建函数
    ///
    void SetCurCurveNo(int index);

    ///
    /// @brief
    ///     鼠标按下的事件处理函数
    ///
    /// @param[in]  event  具体事件
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年9月23日，新建函数
    ///
    void mousePressEvent(QMouseEvent *event);

signals:
    ///
    /// @brief
    ///     选中信号
    ///
    /// @param[in]  index  信号的参数，具体选中行数
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年9月23日，新建函数
    ///
    void ComboxSelected(int index);

private slots:
    ///
    /// @brief
    ///     
    ///
    /// @param[in]  iRow  点击的行数
    /// @param[in]  iColmn  和列数
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年9月23日，新建函数
    ///
    void tabSelectedChange(int iRow, int iColmn);

    ///
    /// @brief
    ///     进入某单元格的按钮
    ///
    /// @param[in]  iRow  行数
    /// @param[in]  iColmn  列数
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年9月23日，新建函数
    ///
    void myCellEntered(int iRow, int iColmn);

private:
    Ui::ComboxTableDlg*             ui;                         //< UI界面
    QGraphicsDropShadowEffect       *widgetShadow;              //< 阴影
    int                             m_curCurveRowInx;           //< 当前行所在行号
    int                             m_selectedHisRow;           //< 当前选中历史曲线所在行
};
