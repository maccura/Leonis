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
/// @file     ScatterPlotViewClass.h
/// @brief    视图类
///
/// @author   5774/WuHongTao
/// @date     2020年6月10日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2020年6月10日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once
#include "qgraphicsview.h"

class QWheelEvent;


class CScatterPlotViewClass :
    public QGraphicsView
{
    Q_OBJECT
public:

    ///
    /// @brief
    ///     视图类构造函数
    ///
    /// @param[in]  parent  
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年6月10日，新建函数
    ///
    CScatterPlotViewClass(QWidget *parent);

    ///
    /// @brief
    ///     视图类析构函数
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年6月10日，新建函数
    ///
    ~CScatterPlotViewClass();

protected:

    ///
    /// @brief
    ///     鼠标按下事件
    ///
    /// @param[in]  event 事件变量 
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年6月4日，新建函数
    ///
    void mousePressEvent(QMouseEvent  *event);

    ///
    /// @brief
    ///     鼠标释放事件
    ///
    /// @param[in]  event  
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年6月4日，新建函数
    ///
    void mouseReleaseEvent(QMouseEvent  *event);

    ///
    /// @brief
    ///     鼠标移动事件
    ///
    /// @param[in]  event  
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年6月4日，新建函数
    ///
    void mouseMoveEvent(QMouseEvent  *event);

    ///
    /// @brief
    ///     鼠标双击事件
    ///
    /// @param[in]  event  
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年6月4日，新建函数
    ///
    void mouseDoubleClickEvent(QMouseEvent  *event);

    ///
    /// @brief
    ///     滚轮事件
    ///
    /// @param[in]  event  事件参数
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年6月17日，新建函数
    ///
    void wheelEvent(QWheelEvent *event);

    ///
    /// @brief
    ///     按键处理函数
    ///
    /// @param[in]  event  按键事件
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年11月25日，新建函数
    ///
    void keyPressEvent(QKeyEvent *event);

    ///
    /// @brief
    ///     按键处理函数
    ///
    /// @param[in]  event  按键事件
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年11月25日，新建函数
    ///
    void keyReleaseEvent(QKeyEvent *event);

signals:
    ///
    /// @brief
    ///     放大缩小事件信号
    ///
    /// @param[in]    true:放大，false:缩小
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年6月17日，新建函数
    ///
    void mouseWheelZoom(bool);
private:
    bool                              keyCtrl;                  // ctrl键被按住与否
};

