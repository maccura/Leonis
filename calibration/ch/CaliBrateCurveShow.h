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
/// @file     CaliBrateCurveShow.h
/// @brief    校准曲线显示页面
///
/// @author   5774/WuHongTao
/// @date     2021年11月17日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月17日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QDialog>
#include "ui_CaliBrateCurveShow.h"
#include "src/alg/ch/ch_alg/ch_alg.h"
#define MAXPOINTER 200

class CaliBrateCurveShow : public QDialog
{
    Q_OBJECT

public:
    ///
    /// @brief
    ///     构造函数
    ///
    /// @param[in]  parent  父指针
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月17日，新建函数
    ///
    CaliBrateCurveShow(QDialog *parent = Q_NULLPTR);

    ///
    /// @brief
    ///     析构函数
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月17日，新建函数
    ///
    ~CaliBrateCurveShow();

    ///
    /// @brief
    ///     显示当前校准曲线
    ///
    /// @param[in]  curveInfo  校准曲线数据
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月17日，新建函数
    ///
    void StartShowCurrentCurve(std::shared_ptr<ch::ICalibrateCurve<>>& curveInfo);

private:
    ///
    /// @brief
    ///     显示曲线的参数信息
    ///
    /// @param[in]  curveInfo  曲线句柄
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月22日，新建函数
    ///
    void ShowCurveParameter(std::shared_ptr<ch::ICalibrateCurve<>>& curveInfo);

    ///
    /// @brief
    ///     显示曲线的数据
    ///
    /// @param[in]  curveInfo  曲线句柄
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月22日，新建函数
    ///
    void ShowCurveData(std::shared_ptr<ch::ICalibrateCurve<>>& curveInfo);

private slots:
    ///
    /// @brief
    ///     关闭窗口
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月17日，新建函数
    ///
    void OnCloseWindow();

private:
    Ui::CaliBrateCurveShow          ui;                         ///< 界面对象
};
