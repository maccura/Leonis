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
/// @file     QComputNeedDialog.cpp
/// @brief    需求计算对话框
///
/// @author   5774/WuHongTao
/// @date     2022年3月17日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月17日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include "shared/basedlg.h"
#include "src/thrift/ch/gen-cpp/ch_types.h"

class QStandardItemModel;
class QTableView;
namespace Ui {
    class QComputNeedDialog;
};


class QComputNeedDialog : public BaseDlg
{
    Q_OBJECT

public:
    QComputNeedDialog(QWidget *parent = Q_NULLPTR);
    ~QComputNeedDialog();

private:
    ///
    /// @brief
    ///     初始化需求表
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月16日，新建函数
    ///
    void ResetNeedtable();

    ///
    /// @brief
    ///     初始化页面
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月16日，新建函数
    ///
    void InitTable();

    ///
    /// @brief
    ///     初始化
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月16日，新建函数
    ///
    void Init();

    ///
    /// @brief
    ///     通过项目编号获取，对应试剂的总览信息
    ///
    /// @param[in]  assaycode  项目编号
    ///
    /// @return 试剂信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月16日，新建函数
    ///
    bool GetReagentDataOverviewByCode(ch::tf::ReagentOverview& data, int assaycode);

    ///
    /// @brief
    ///     设置表格哪一列能修改
    ///
    /// @param[in]  tab  表格
    /// @param[in]  index  列
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月16日，新建函数
    ///
    void SetTabviewColumnEdit(QTableView* tab, int index);

private slots:
    ///
    /// @brief
    ///     显示当前日期的试剂需求量
    ///
    /// @param[in]  index  日期序号
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月16日，新建函数
    ///
    void OnShowCurrentDayReagent(int index);

    ///
    /// @brief
    ///     保存修改数据
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月17日，新建函数
    ///
    void OnSave();
private:
    Ui::QComputNeedDialog*          ui;
    QTableView*                     m_ComputView;
    QStandardItemModel*             m_ComputNeedMode;
    std::vector<::ch::tf::ReagentOverview> m_OverViewDatas;
    std::vector<QTableView*>        m_ComputViewList;
    std::vector<QModelIndex>        m_IndexList;
    int                             m_row;
};
