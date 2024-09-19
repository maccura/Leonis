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
/// @file     imparamwidget.h
/// @brief    应用界面->参数免疫
///
/// @author   4170/TangChuXian
/// @date     2024年2月4日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2024年2月4日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QWidget>
#include "utility/model/AssayListModel.h"
#include "src/thrift/im/gen-cpp/im_types.h"

namespace Ui { class ImParamWidget; };


class ImParamWidget : public QWidget
{
    Q_OBJECT

public:
    ImParamWidget(QWidget *parent = Q_NULLPTR);
    ~ImParamWidget();

protected:
    ///
    /// @brief 界面显示后初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年2月4日，新建函数
    ///
    void InitAfterShow();

    ///
    /// @brief 窗口显示事件
    ///     
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年2月4日，新建函数
    ///
    void showEvent(QShowEvent *event);

    protected Q_SLOTS:
    ///
    /// @brief
    ///     更新项目表数据
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年5月26日，新建函数
    ///
    void UpdateAssayTableData();

    ///
    /// @bref
    ///		项目选择
    ///
    /// @param[in] current 当前选择的项目的Index
    /// @param[in] previous 上一个Index
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年3月23日，新建函数
    ///
    void OnSelectedAssayChanged(const QModelIndex &current, const QModelIndex &previous);

    ///
    /// @bref
    ///		保存按钮
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年3月23日，新建函数
    ///
    void OnSaveBtnClicked();

private:
    Ui::ImParamWidget			*ui;						// Ui指针
    bool							m_bInit;					// 是否已经初始化

    AssayListModel*                 m_imDataModel;              // 免疫表格排序的model
    QUtilitySortFilterProxyModel*   m_imSortModel;              // 免疫表格排序的model
};
