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
/// @file     QCaliBratePostionEdit.cpp
/// @brief    位置编辑
///
/// @author   5774/WuHongTao
/// @date     2022年3月11日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月11日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QWidget>
#include "shared/basedlg.h"
#include "src/thrift/ch/gen-cpp/ch_types.h"

namespace Ui {
    class QCaliBratePostionEdit;
};


class QCaliBratePostionEdit : public BaseDlg
{
    Q_OBJECT

public:
    QCaliBratePostionEdit(QWidget *parent = Q_NULLPTR);
    ~QCaliBratePostionEdit();

    ///
    /// @brief
    ///     设置校准品的列表和选择的level
    ///
    /// @param[in]  docs  校准品列表
    /// @param[in]  level  水平
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月11日，新建函数
    ///
    void SetCaliDocListAndLevel(QList<ch::tf::CaliDoc>& docs, int level);

    ///
    /// @brief
    ///     获取校准文档列表
    ///
    /// @param[out]  docs  校准文档列表
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月11日，新建函数
    ///
    void GetModifyCaliDoc(QList<ch::tf::CaliDoc>& docs);

signals:
    ///
    /// @brief
    ///     成功修改位置类型信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月11日，新建函数
    ///
    void ModifyCaliPostion();

private:
    ///
    /// @brief
    ///     初始化位置选中对话框
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月11日，新建函数
    ///
    void Init();

private slots:
    ///
    /// @brief
    ///     确认按钮按下
    ///
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月11日，新建函数
    ///
    void OnSavePostion();

private:
    Ui::QCaliBratePostionEdit*      ui;
    int                             m_pos;                      ///< 位置
    std::string                     m_rack;                     ///< 架号
    tf::TubeType::type              m_tubeType;                 ///< 杯类型
    QList<ch::tf::CaliDoc>          m_CaliDocs;                 ///< 校准品组
    int                             m_Level;                    ///< 水平
};
