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
/// @file     CalibrationRegister.h
/// @brief    校准液的注册页面
///
/// @author   5774/WuHongTao
/// @date     2021年11月8日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2021年11月8日，新建文件
///
///////////////////////////////////////////////////////////////////////////#pragma once

#include <QWidget>
#include "ui_CalibrationRegister.h"
#include "AssayDialog.h"
#include "DialogEdit.h"
#include "src/common/Mlog/mlog.h"
#include "thrift/DcsControlProxy.h"

class CalibrationRegister : public QWidget
{
    Q_OBJECT

public:
    ///
    /// @brief
    ///     构造函数
    ///
    /// @param[in]  parent  
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月9日，新建函数
    ///
    CalibrationRegister(QWidget *parent = Q_NULLPTR);

    ///
    /// @brief
    ///     析构函数
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月9日，新建函数
    ///
    ~CalibrationRegister();

private:
    ///
    /// @brief
    ///     初始化函数
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月8日，新建函数
    ///
    void Init();

    ///
    /// @brief
    ///     查询校准组信息
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月8日，新建函数
    ///
    void QueryDocGroups();

    ///
    /// @brief
    ///     显示校准组信息列表
    ///
    /// @param[in]  groups   校准组信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月8日，新建函数
    ///
    void ShowCaliBrateGroupData(std::vector<::ch::tf::CaliDocGroup>& groups);

    ///
    /// @brief
    ///     显示校准品详细信息
    ///
    /// @param[in]  doc    校准品信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月8日，新建函数
    ///
    void ShowCaliDocData(int row, ::ch::tf::CaliDoc& doc);

    ///
    /// @brief
    ///     获取项目名称，根据项目码
    ///
    /// @param[in]  assayCode  项目码
    ///
    /// @return 项目名称
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月5日，新建函数
    ///
    QString GetAssayNameFromCode(const int assayCode);

signals:
    ///
    /// @brief
    ///     行信号
    ///
    /// @param[in]  index  行数目
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月9日，新建函数
    ///
    void currentRowSelect(int index);

private slots:
    ///
    /// @brief
    ///     显示校准组详细信息
    ///
    /// @param[in]  index  选中的校准组行数
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月8日，新建函数
    ///
    void OnShowGroupDetail(int index);

    ///
    /// @brief
    ///     单元格选中信号处理函数
    ///
    /// @param[in]  item  选中的单元格
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月9日，新建函数
    ///
    void OnItemClicked(QTableWidgetItem* item);

    ///
    /// @brief
    ///     编辑按钮杯选中
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月9日，新建函数
    ///
    void OnShowCaliGroupDetail();

    ///
    /// @brief
    ///     删除选中的校准组
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月11日，新建函数
    ///
    void OnDeleteCaliDroup();

    ///
    /// @brief
    ///     更新校准组数据
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月9日，新建函数
    ///
    void OnUpdateAllGroup();

    ///
    /// @brief
    ///     单元格被选中的槽函数
    ///
    /// @param[in]  row  选中单元格的行
    /// @param[in]  column  选中单元格的列
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月9日，新建函数
    ///
    void OnCellClicked(int row, int column);

    ///
    /// @brief
    ///     位置编辑完成
    ///
    /// @param[in]  rack  架号
    /// @param[in]  pos  位置号
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月11日，新建函数
    ///
    void OnEditFinishedPostion(int rack, int pos);

    ///
    /// @brief
    ///     表头选中的回调函数
    ///
    /// @param[in]  selction  表头选中标签
    /// @par History:
    /// @li 5774/WuHongTao，2021年11月11日，新建函数
    ///
    void OnHeaderClicked(int selction);

private:
    Ui::CalibrationRegister         ui;
    std::vector<::ch::tf::CaliDocGroup> m_groups;               ///< 校准组列表
    QList<::ch::tf::CaliDoc>        m_calicDocs;                ///< 校准文档列表
    int                             m_currentIndex;             ///< 当前选中的行
    AssayDialog*                    m_assayDialog;              ///< 项目组编辑对话框
    DialogEdit*                     m_posEditDialog;            ///< 位置编辑框
    int                             m_currentSize;              ///< 当前的行数
    int                             m_currentSection;           ///< 当前选中的列
};
