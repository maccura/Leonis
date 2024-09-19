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
/// @file     PdfPretreatment.cpp
/// @brief    Pdf文档预处理
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
#include <QFile>
#include <QSize>
#include <QImage>
#include <QByteArray>
#include <QTextCodec>
#include <string>
#include "PdfPretreatment.h"
#include "cpp/fpdf_scopers.h"
#include "src/common/Mlog/mlog.h"

std::shared_ptr<PdfPretreatment> PdfPretreatment::s_Instance = nullptr;
PdfPretreatment::PdfPretreatment()
    : m_bInitialized(false)
    , m_iPageCount(0)
    , m_iCurDocID(0)
{
    
}

PdfPretreatment::~PdfPretreatment()
{
    if (m_bInitialized) 
    {
        CloseDocument();
        FPDF_DestroyLibrary();
        m_bInitialized = false;
    }
}

bool PdfPretreatment::InitPdfLibrary()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    try
    {
        if (!m_bInitialized)
        {
            FPDF_InitLibrary();
            m_bInitialized = true;
        }

        return true;
    }
    catch (const std::exception& e)
    {
        ULOG(LOG_INFO, "%s", e.what());
        return false;
    }
    catch (...)
    {
        ULOG(LOG_WARN, "Failed to init PdfLibrary");
        return false;
    }
}

///
/// @brief
///     获取单例
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
std::shared_ptr<PdfPretreatment> PdfPretreatment::GetInstance()
{
    // 获取实例对象
    if (nullptr == s_Instance)
    {
        s_Instance.reset(new PdfPretreatment());
    }

    return s_Instance;
}

///
/// @brief
///     加载文件
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
PdfPretreatment::Error PdfPretreatment::loadFile(QString filename, QString password, int iDocID)
{
    if (!QFile::exists(filename)) 
    {
        m_status = FILE_NOT_FOUND_ERROR;
        return m_status;
    }

    m_iCurDocID = iDocID;
    auto Doc = m_mapDoc.find(m_iCurDocID);
    if (Doc == m_mapDoc.end())
    {
        m_mapDoc[m_iCurDocID] = FPDF_LoadDocument(filename.toUtf8().constData(), password.toUtf8().constData());
        if (m_mapDoc[m_iCurDocID] == nullptr)
        {
            m_iPageCount = 0;
            m_status = parseError(FPDF_GetLastError());
            return m_status;
        }
        else
        {
            m_status = SUCCESS;
        }
    }

    m_iPageCount = FPDF_GetPageCount(m_mapDoc[m_iCurDocID]);
    return m_status;
}

///
/// @brief
///     异常处理
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
PdfPretreatment::Error PdfPretreatment::parseError(int err)
{
    Error err_code = Error::SUCCESS;
    switch (err) 
    {
    case FPDF_ERR_SUCCESS:
        err_code = Error::SUCCESS;
        break;
    case FPDF_ERR_FILE:
        err_code = Error::FILE_ERROR;
        break;
    case FPDF_ERR_FORMAT:
        err_code = Error::FORMAT_ERROR;
        break;
    case FPDF_ERR_PASSWORD:
        err_code = Error::PASSWORD_ERROR;
        break;
    case FPDF_ERR_SECURITY:
        err_code = Error::HANDLER_ERROR;
        break;
    }

    return err_code;
}

///
/// @brief
///     获取页面数量
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
int PdfPretreatment::GetPageCount() const
{
    return m_iPageCount;
}

///
/// @brief
///     获取页码大小
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
QSizeF PdfPretreatment::GetPageSize(int page) const
{
    QSizeF result;
    auto Doc = m_mapDoc.find(m_iCurDocID);
    if (Doc == m_mapDoc.end() || Doc->second == nullptr)
    {
        return result;
    }

    FS_SIZEF size;
    FPDF_GetPageSizeByIndexF(Doc->second, page, &size);
    result.setWidth(size.width);
    result.setHeight(size.height);
    return result;
}

///
/// @brief
///     渲染页面
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
std::shared_ptr<QImage> PdfPretreatment::Render(int iPage, QSizeF page_size, double scale)
{
    auto Doc = m_mapDoc.find(m_iCurDocID);
    if (Doc == m_mapDoc.end() || Doc->second == nullptr)
    {
        return nullptr;
    }
        
    if (page_size.width() <= 0 || page_size.height() <= 0)
    {
        return nullptr;
    }

    FPDF_PAGE page = FPDF_LoadPage(Doc->second, iPage);
    if (page == nullptr)
    {
        return nullptr;
    }

    int alpha = FPDFPage_HasTransparency(page) ? 1 : 0;
    auto image = std::make_shared<QImage>(page_size.width() * scale, page_size.height() * scale, QImage::Format_RGBA8888);
    FPDF_BITMAP bitmap = FPDFBitmap_CreateEx(
        image->width(), image->height(), FPDFBitmap_BGRA,
        image->scanLine(0), image->bytesPerLine());

    FPDF_DWORD fill_color = alpha ? 0x00000000 : 0xFFFFFFFF;
    FPDFBitmap_FillRect(bitmap, 0, 0,
        image->width(), image->height(), fill_color);

    int rotation = 0;
    int flags = FPDF_ANNOT;
    FPDF_RenderPageBitmap(bitmap, page, 0, 0,
        image->width(), image->height(), rotation, flags);

    FPDFBitmap_Destroy(bitmap);
    FPDF_ClosePage(page);

    for (int i = 0; i < image->height(); i++) 
    {
        uchar *pixels = image->scanLine(i);
        for (int j = 0; j < image->width(); j++) 
        {
            qSwap(pixels[0], pixels[2]);
            pixels += 4;
        }
    }

    return image;
}

///
/// @brief
///     获取标题
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
QString PdfPretreatment::GetTitle(FPDF_BOOKMARK bookmark)
{
    unsigned long length = FPDFBookmark_GetTitle(bookmark, NULL, 0);
    QString strTitle;
    if (length > 0)
    {
        wchar_t * buffer = new wchar_t[length];
        length = FPDFBookmark_GetTitle(bookmark, buffer, length);
        std::wstring str = buffer;
        strTitle = QString::fromStdWString(str);
        delete[] buffer;
    }

    return strTitle;
}

// 获取第一个标题
FPDF_BOOKMARK PdfPretreatment::GetFirstChild(FPDF_BOOKMARK bookmark)
{
    auto Doc = m_mapDoc.find(m_iCurDocID);
    if (Doc == m_mapDoc.end() || Doc->second == nullptr)
    {
        return nullptr;
    }

    FPDF_BOOKMARK child = FPDFBookmark_GetFirstChild(Doc->second, bookmark);
    return child;
}

// 获取下一个标题
FPDF_BOOKMARK PdfPretreatment::GetNextSibling(FPDF_BOOKMARK bookmark)
{
    auto Doc = m_mapDoc.find(m_iCurDocID);
    if (Doc == m_mapDoc.end() || Doc->second == nullptr)
    {
        return nullptr;
    }

    FPDF_BOOKMARK child = FPDFBookmark_GetNextSibling(Doc->second, bookmark);
    return child;
}


///
/// @brief
///     获取书签对应页面序号
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
int PdfPretreatment::GetDestPageIndex(FPDF_BOOKMARK bookmark)
{
    auto Doc = m_mapDoc.find(m_iCurDocID);
    if (Doc == m_mapDoc.end() || Doc->second == nullptr)
    {
        return -1;
    }

    FPDF_ACTION action = FPDFBookmark_GetAction(bookmark);
    FPDF_DEST dest = NULL;
    if (action)
    {
        if (FPDFAction_GetType(action) == PDFACTION_GOTO)
        {
            dest = FPDFBookmark_GetDest(Doc->second, bookmark);
        }
    }
    else 
    {
        dest = FPDFBookmark_GetDest(Doc->second, bookmark);
    }

    int pageIndex = -1;
    if (dest)
    {
        pageIndex = FPDFDest_GetDestPageIndex(Doc->second, dest);
    }

    return pageIndex;
}

///
/// @brief
///     关闭文档
///
/// @par History:
/// @li 6889/ChenWei，2024年1月22日，新建函数
///
void PdfPretreatment::CloseDocument()
{
    for (auto Doc : m_mapDoc)
    {
        FPDF_CloseDocument(Doc.second);
    }

    m_iPageCount = 0;
}

