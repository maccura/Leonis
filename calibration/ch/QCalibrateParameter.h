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
/// @file     QCalibrateParameter.h
/// @brief    显示校准参数的详细信息
///
/// @author   5774/WuHongTao
/// @date     2022年2月28日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月28日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QPushButton>
#include <vector>
#include "src/alg/ch/ch_alg/ch_alg.h"
#include "src/thrift/ch/gen-cpp/ch_types.h"

namespace Ui {
    class QCalibrateParameter;
};
class QLabel;


// 显示的表格类型
enum TABLETYPE
{
    CURRENT,
    BOTTLE,
    LOT,
    PROJECT
};

class QCalibrateParameter : public QPushButton
{
    Q_OBJECT

     enum PARAMETERTYPE
    {
        USENOW,
        OTHER
    };

public:
    QCalibrateParameter(QWidget *parent = Q_NULLPTR);
    ~QCalibrateParameter();

    ///
    /// @brief
    ///     显示校准曲线的参数信息
    ///
    /// @param[out] caliCurve  校准曲线信息
    /// @param[in]  group      试剂信息
    /// @param[in]  curveType  曲线类型
    ///
    /// @return true代表成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月28日，新建函数
    ///
    bool ShowCurveParameter(ch::tf::CaliCurve& caliCurve, const ch::tf::ReagentGroup group, const TABLETYPE curveType);

    ///
    /// @brief
    ///     显示校准参数
    ///
    /// @param[in]  curve  曲线信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月9日，新建函数
    ///
    void ShowCaliParameter(std::shared_ptr<ch::ICalibrateCurve>& curve);

    ///
    /// @brief
    ///     显示校准详细信息
    ///
    /// @param[in]  caliCurve  曲线数据
    /// @param[in]  group      试剂组数据
    /// @param[in]  type       类型
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月9日，新建函数
    ///
    void ShowCaliCaliGroupInfo(ch::tf::CaliCurve& caliCurve, const ch::tf::ReagentGroup group, PARAMETERTYPE type = OTHER);

private:
    Ui::QCalibrateParameter*        ui;
    std::vector<QLabel*>            m_labelParameter;
};
