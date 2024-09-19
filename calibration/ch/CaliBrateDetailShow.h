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
/// @file     CaliBrateDetailShow.h
/// @brief    校准曲线参数显示页面
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

#include "shared/basedlg.h"
#include "CaliBrateCurveShow.h"
#include "src/alg/ch/ch_alg/ch_alg.h"
#include "CaliBrationStatusPage.h"
#include "thrift/DcsControlProxy.h"
#include "src/common/Mlog/mlog.h"
#include "src/leonis/thrift/ch/c1005/C1005LogicControlProxy.h"
#include "src/thrift/ch/c1005/gen-cpp/C1005LogicControl.h"
#include "src/thrift/ch/gen-cpp/ch_types.h"
#include "ui_CaliBrateDetailShow.h"

// 前置声明
class CaliBrationStatusPage;

class CaliBrateDetailShow : public BaseDlg
{
    Q_OBJECT

        //  校准类型曲线显示类型
        enum CURVETYPE {CUURENTCALI, LOTCALI, PROJECTCALI };
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
    CaliBrateDetailShow(QWidget *parent = Q_NULLPTR);

    ///
    /// @brief
    ///     析构函数
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月17日，新建函数
    ///
    ~CaliBrateDetailShow();

    ///
    /// @brief
    ///     显示试剂组对应的校准曲线的详细信息
    ///
    /// @param[in]  db  试剂组的数据库主键
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月16日，新建函数
    ///
    void StartShowReagentCurve(int db);

    ///
    /// @brief
    ///     显示试剂状态信息（名称，位置等等）
    ///
    /// @param[in]  group  试剂组信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月16日，新建函数
    ///
    void ShowCaliBrateStatus(::ch::tf::ReagentGroup& group);
private:
    ///
    /// @brief
    ///     获取显示参数的控件列表
    ///
    /// @param[in]  type  参数类型
    ///
    /// @return 控件列表的组
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月16日，新建函数
    ///
    std::vector<QLineEdit*> GetParameterContainer(CURVETYPE type);

    ///
    /// @brief
    ///     显示各种类型的曲线
    ///
    /// @param[in]  type  曲线类型
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月16日，新建函数
    ///
    void ShowCaliBrateParameter(CURVETYPE type, ch::tf::ReagentGroup& group);

private slots:
    ///
    /// @brief
    ///     点击按钮curve的反应
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月16日，新建函数
    ///
    void OnCurveShow();

    ///
    /// @brief
    ///     关闭对话框
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月17日，新建函数
    ///
    void OnClose();

private:
    Ui::CaliBrateDetailShow         ui;                         ///< 校准详细信息显示
    CaliBrationStatusPage*          m_parent;                   ///< 保存父指针以备查询
    std::shared_ptr<::ch::ICalibrateCurve<>> m_curveObject;     ///< 曲线数据句柄
    std::shared_ptr<CaliBrateCurveShow> m_CurveShow;            ///< 当前显示曲线
};
