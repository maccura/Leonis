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
/// @file     QCaliCurveCover.h
/// @brief    校准覆盖
///
/// @author   5774/WuHongTao
/// @date     2022年3月8日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月8日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include "shared/basedlg.h"
#include "src/alg/ch/ch_alg/ch_alg.h"
#include "src/thrift/ch/gen-cpp/ch_types.h"

namespace Ui {
    class QCaliCurveCover;
};


class QCaliCurveCover : public BaseDlg
{
    Q_OBJECT

public:
    QCaliCurveCover(QWidget *parent = Q_NULLPTR);
    ~QCaliCurveCover();

    ///
    /// @brief
    ///    设置曲线列表
    ///
    /// @param[in]  curves  曲线列表（第一条表示选中曲线，第二条表示历史曲线） 
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月8日，新建函数
    ///
    void SetCurveList(std::vector<ch::tf::CaliCurve>& curves, ch::tf::ReagentGroup& group);

private slots:
    ///
    /// @brief
    ///     覆盖当前工作曲线
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月8日，新建函数
    ///
    void OnCoverCurrentCurve();

private:
    Ui::QCaliCurveCover*            ui;
    std::vector<ch::tf::CaliCurve>  m_curveList;                ///< 曲线列表
    std::shared_ptr<ch::ICalibrateCurve> m_caliCurve;           ///< 当前显示曲线
    ch::tf::ReagentGroup            m_ReagentGroup;             ///< 当前试剂组
};
