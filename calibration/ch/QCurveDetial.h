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
/// @file     QCurveDetial.cpp
/// @brief    校准曲线显示
///
/// @author   5774/WuHongTao
/// @date     2022年3月2日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月2日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include "src/alg/ch/ch_alg/ch_alg.h"
#include "src/thrift/ch/gen-cpp/ch_types.h"
#include "shared/basedlg.h"

namespace Ui {
    class QCurveDetial;
};
class QStandardItemModel;


class QCurveDetial : public BaseDlg
{
    Q_OBJECT

public:
    QCurveDetial(QWidget *parent = Q_NULLPTR);
    ~QCurveDetial();

    ///
    /// @brief
    ///     显示校准曲线信息
    ///
    /// @param[in]  caliCurve  校准曲线
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月28日，新建函数
    ///
    void ShowReactionCurve(const ch::tf::CaliCurve& caliCurve);

private:
    ///
    /// @brief
    ///     更新校准曲线的显示
    ///
    /// @param[in]  curve  校准曲线
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月22日，新建函数
    ///
    shared_ptr<ch::ICalibrateCurve> UpdateCurve(const ch::tf::CaliCurve& curve);

    ///
    /// @brief
    ///     曲线初始化
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月2日，新建函数
    ///
    void Init();

    ///
    /// @brief
    ///     显示曲线的检测点
    ///
    /// @param[in]  result  曲线检测点
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月2日，新建函数
    ///
    void ShowCurveDetectPoint(ch::tf::AssayTestResult& result);

    ///
    /// @brief
    ///     重置曲线数据框
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月10日，新建函数
    ///
    void ResetCurveTable();

private:
    QStandardItemModel*             m_CurvePointMode;   
    Ui::QCurveDetial* ui;
};
