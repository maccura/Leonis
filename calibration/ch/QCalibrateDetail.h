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
/// @file     QCalibrateDetail.h
/// @brief    校准品信息显示
///
/// @author   5774/WuHongTao
/// @date     2022年2月23日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月23日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QWidget>
#include "src/alg/ch/ch_alg/ch_alg.h"

namespace Ui {
    class QCalibrateDetail;
};
class QStandardItemModel;

class QCalibrateDetail : public QWidget
{
    Q_OBJECT

public:
    QCalibrateDetail(QWidget *parent = Q_NULLPTR);
    ~QCalibrateDetail();

    ///
    /// @brief
    ///     更新曲线参数
    ///
    /// @param[in]  curveParameter  参数曲线
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月23日，新建函数
    ///
    void UpdateCurveParameter(std::map<std::string, double>& curveParameter);

    ///
    /// @brief
    ///     更新曲线的详细数据
    ///
    /// @param[in]  curve  曲线信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月23日，新建函数
    ///
    void UpdateCurveDetail(const std::vector<tuple<double, double, std::vector<double>>>& curveData);

    ///
    /// @brief
    ///     清除表格内容
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月23日，新建函数
    ///
    void ClearTableContent();

    ///
    /// @brief
    ///     重置曲线数据框
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月10日，新建函数
    ///
    void ResetCaliBrateCurveTable();

    ///
    /// @brief
    ///     重置曲线的参数列表
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月10日，新建函数
    ///
    void ResetCValiBrateParameterTable();

private:
    Ui::QCalibrateDetail*            ui;
    QStandardItemModel*             m_calibrateConcMode;        ///< 校准品浓度mode
    QStandardItemModel*             m_calibrateParameterMode;   ///< 校准曲线参数vmod
};
