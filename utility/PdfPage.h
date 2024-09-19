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
/// @file     PdfPage.h
/// @brief    Pdf页面
///
/// @author   6889/ChenWei
/// @date     2024年1月22日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <QObject>
#include <QMetaType>
#include <QGraphicsPixmapItem>
#include <QPrinter>
#include <QTimer>
#include "PdfPretreatment.h"

class PdfPage : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
public:
    PdfPage(int index, QSizeF size, QGraphicsItem *parent = Q_NULLPTR);
    ~PdfPage();

    // 加载页面
    void LoadPage(double dScale);

    // 缩放
    void Scaled(double dScale);

    // 页面高度
    double Height();

    // 页面宽度
    double Width();

    // 设置可见标志
    void SetVisibleFlag(bool flag);

private:

    // 渲染页面
    void Render();

private slots:

    // 大小调整
    void resizeDone();
    
private:
    std::shared_ptr<QImage> m_pImage;
    QSizeF m_PageSize;
    int m_iIndex;
    double m_dScale;            // 缩放比例
    double m_bBaseZoomFactor;   // 基础缩放因子
    bool m_bIsIsVisible;
    bool m_bIsRended;
    QTimer  m_resizeTimer;
};
