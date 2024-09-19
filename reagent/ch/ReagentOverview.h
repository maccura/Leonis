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
/// @file     ReagentOverview.h
/// @brief    试剂总览页面
///
/// @author   5774/WuHongTao
/// @date     2021年12月2日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2021年12月2日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QFrame>
#include "src/thrift/gen-cpp/defs_types.h"
#include "src/thrift/ch/gen-cpp/ch_types.h"
#define PAGENUMBER 25

class QComputNeedDialog;
class AssayCardsManage;
struct CardShowInfo;
namespace Ui {
    class ReagentOverview;
};


class ReagentOverview : public QFrame
{
    Q_OBJECT
public:
    ///
    /// @brief
    ///     构造函数
    ///
    /// @param[in]  parent  
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年12月2日，新建函数
    ///
    ReagentOverview(QWidget *parent = Q_NULLPTR);

    ///
    /// @brief
    ///     析构函数
    ///
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年12月2日，新建函数
    ///
    ~ReagentOverview();

    ///
    /// @brief
    ///     更新设备信息
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年1月10日，新建函数
    ///
    void UpdateReagentPages();

    ///
    /// @brief
    ///     更新设备耗材信息
    ///
    /// @param[in]  devList  设备信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月15日，新建函数
    ///
    void UpdateSupplyPage(std::vector<std::shared_ptr<const tf::DeviceInfo>>& devList);

    ///
    /// @brief
    ///     获取数据总览
    ///
    /// @param[out]  ReagentOverviews  数据总览
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月16日，新建函数
    ///
    void GetOverviewData(std::vector<::ch::tf::ReagentOverview>& ReagentOverviews);

private:

    ///
    /// @brief
    ///     显示生化试剂总览
    ///
    /// @param[in]  Reagents  生化试剂列表
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年1月10日，新建函数
    ///
    void ShowChemistryReagentOverview(std::vector<::ch::tf::ReagentOverview>& ReagentOverViews);

    ///
    /// @brief
    ///     判断和设置上下翻页键
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年1月10日，新建函数
    ///
    void JudgeStatus();

    ///
    /// @brief
    ///     生成试剂卡片的状态信息
    ///
    /// @param[in]  card  试剂卡片信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年1月11日，新建函数
    ///
    void ToImplementCardStatus(CardShowInfo& card);

    ///
    /// @brief
    ///     清除之前的page页面的数据，避免内存泄漏
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年1月11日，新建函数
    ///
    void ClearPages();

private slots:
    ///
    /// @brief
    ///     前一页按钮被选中
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年12月2日，新建函数
    ///
    void OnPreBtnClicked();

    ///
    /// @brief
    ///     后一页按钮被选中
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年12月2日，新建函数
    ///
    void OnNextBtnClicked();

    void OnComputDialogClicked();

private:
    Ui::ReagentOverview*            ui;
    std::vector<AssayCardsManage*>  m_managePages;              ///< 试剂卡片管理页表
    QComputNeedDialog*              m_ComputDialog;             ///< 需求计算设置对话框
    std::vector<::ch::tf::ReagentOverview> m_reagentOverViews;  ///< 试剂总览的保存
};
