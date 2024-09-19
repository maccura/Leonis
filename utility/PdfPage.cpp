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
/// @file     PdfPage.cpp
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
#include "PdfPage.h"
#include "PdfPretreatment.h"

PdfPage::PdfPage(int index, QSizeF size, QGraphicsItem *parent)
    : QGraphicsPixmapItem(parent)
    , m_iIndex(index)
    , m_PageSize(size)
    , m_pImage(nullptr)
    , m_dScale(1.0)
    , m_bBaseZoomFactor(1.3)
    , m_bIsIsVisible(false)
    , m_bIsRended(false)
{
    setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
    m_pImage = std::make_shared<QImage>(m_PageSize.width() * m_bBaseZoomFactor * m_dScale, m_PageSize.height() * m_bBaseZoomFactor * m_dScale, QImage::Format_RGBA8888);
    QPixmap Pixmap = QPixmap::fromImage(*m_pImage);
    setPixmap(Pixmap);
    m_resizeTimer.setSingleShot(true);
    connect(&m_resizeTimer, SIGNAL(timeout()), this, SLOT(resizeDone()));
}

PdfPage::~PdfPage()
{
}

///
/// @brief
///     渲染页面
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
void PdfPage::Render()
{
    m_pImage = PdfPretreatment::GetInstance()->Render(m_iIndex, m_PageSize, m_bBaseZoomFactor * m_dScale);
    if (m_pImage != nullptr)
    {
        QPixmap Pixmap = QPixmap::fromImage(*m_pImage);
        setPixmap(Pixmap);
    }
}

///
/// @brief
///     调整页面大小
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
void PdfPage::resizeDone()
{
    if (!m_bIsRended && m_bIsIsVisible)
    {
        Render();
        m_bIsRended = true;
    }
}

///
/// @brief
///     返回页面高度
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
double PdfPage::Height()
{
    QRectF rect = boundingRect();
    return rect.height();
}

///
/// @brief
///     界面显示前初始化
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
double PdfPage::Width()
{
    QRectF rect = boundingRect();
    return rect.width();
}

///
/// @brief
///     设可见标志并刷新
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
void PdfPage::SetVisibleFlag(bool flag)
{
    m_bIsIsVisible = flag;
    if (m_bIsIsVisible)
    {
        m_resizeTimer.start(100);
    }
    else
    {
        m_resizeTimer.stop();
    }
    
}

///
/// @brief
///     加载页面
/// @param[in]  dScale  缩放比例
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
void PdfPage::LoadPage(double dScale)
{
    prepareGeometryChange();
    //m_dScale = dScale;
    if (m_pImage != nullptr)
    {
        QPixmap Pixmap = QPixmap::fromImage(*m_pImage);
        setPixmap(Pixmap);
    }
    //Render();

}

///
/// @brief
///     缩放页面
/// @param[in]  dScale  缩放比例
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
void PdfPage::Scaled(double dScale)
{
    if (dScale != m_dScale)
    {
        prepareGeometryChange();
        m_dScale = dScale;
        QPixmap Pixmap = this->pixmap().scaled(m_PageSize.width() * m_bBaseZoomFactor * m_dScale, m_PageSize.height()* m_bBaseZoomFactor * m_dScale, Qt::KeepAspectRatio);
        setPixmap(Pixmap);
        m_bIsRended = false;
    }
}




