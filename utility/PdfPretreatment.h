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
/// @file     PdfPretreatment.h
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
#pragma once
#include <QtCore/qglobal.h>
#include <memory>
#include <QString>
#include "fpdfview.h"
#include "fpdf_dataavail.h"
#include "fpdf_doc.h"
#include "fpdf_formfill.h"
#include <boost/noncopyable.hpp>

class PdfPretreatment : public boost::noncopyable
{
public:
    enum Error {
        NOT_LOADED = -1,
        SUCCESS = 0,
        FILE_ERROR,
        FORMAT_ERROR,
        PASSWORD_ERROR,
        HANDLER_ERROR,
        FILE_NOT_FOUND_ERROR
    };

    PdfPretreatment();
    ~PdfPretreatment();

    // 获取单例
    static std::shared_ptr<PdfPretreatment> GetInstance();

    bool InitPdfLibrary();

    // 加载文件
    Error loadFile(QString filename, QString password, int iDocID);

    // 获取页数
    int GetPageCount() const;

    // 获取页面尺寸
    QSizeF GetPageSize(int page) const;

    // 渲染页面
    std::shared_ptr<QImage> Render(int page, QSizeF page_size, double scale = 1.0);

    // 获取标题
    QString GetTitle(FPDF_BOOKMARK bookmark);

    // 获取页面序号
    int GetDestPageIndex(FPDF_BOOKMARK bookmark);

    // 获取第一个标题
    FPDF_BOOKMARK GetFirstChild(FPDF_BOOKMARK bookmark);

    // 获取下一个标题
    FPDF_BOOKMARK GetNextSibling(FPDF_BOOKMARK bookmark);

    int GetCurDocID()
    {
        return m_iCurDocID;
    };

    // 关闭文档
    void CloseDocument();
private:

    // 报警处理
    Error parseError(int err);

private:
    static std::shared_ptr<PdfPretreatment> s_Instance;
    bool m_bInitialized;
    std::map<int, FPDF_DOCUMENT> m_mapDoc;
    int m_iCurDocID;
    Error m_status;
    int m_iPageCount;
};
