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
/// @file     QCaliBrateFilter.h
/// @brief    校准的筛选
///
/// @author   5774/WuHongTao
/// @date     2022年3月9日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月9日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QWidget>
#include "shared/basedlg.h"
#include "src/thrift/ch/gen-cpp/ch_types.h"

namespace Ui {
    class QCaliBrateFilter;
};


class QCaliBrateFilter : public BaseDlg
{
    Q_OBJECT

public:
    QCaliBrateFilter(QWidget *parent = Q_NULLPTR);
    ~QCaliBrateFilter();

    ///
    /// @brief
    ///     设置校准条件
    ///
    /// @param[in]  cond  校准条件
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月9日，新建函数
    ///
    void SetCaliBrateCond(::ch::tf::CaliCurveQueryCond& cond);

    ///
    /// @brief
    ///     获取查询条件
    ///
    ///
    /// @return 查询条件
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月9日，新建函数
    ///
    ch::tf::CaliCurveQueryCond& GetQueryFilterCond();

signals:

    ///
    /// @brief
    ///     条件筛选已经完成
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月9日，新建函数
    ///
    void HasFinshedSelect();

private:
    ///
    /// @brief
    ///     初始化选中框
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月9日，新建函数
    ///
    void Init();

private slots:
    ///
    /// @brief
    ///     保存查询条件
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月9日，新建函数
    ///
    void OnSaveQueryCond();

private:
    Ui::QCaliBrateFilter*            ui;
    ::ch::tf::CaliCurveQueryCond    m_QueryCaliCurveCond;       ///< 查询曲线校准条件
};
