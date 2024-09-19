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
/// @file     CupHistoryWidget.h
/// @brief    应用-状态-反应杯历史
///
/// @author   7951/LuoXin
/// @date     2023年6月28日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2023年6月28日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include<QWidget>
#include "src/thrift/ch/gen-cpp/ch_types.h"

struct CupHistoryInfo;
class QStandardItemModel;
namespace Ui {
    class CupHistoryWidget;
}


using namespace std;

///
/// @brief
///     信息在表格中的对应列
///
enum DATA_COLUNM
{
    COLUNM_CUP_NUM,					// 杯号
    COLUNM_STATUS,					// 状态
    COLUNM_TEST_NUM,			    // 总测试数
    COLUNM_TESTING_ASSAY,			// 正在测试的项目
    COLUNM_TEN_ASSAY,				// 前十次测试的项目
};


class CupHistoryWidget : public QWidget
{
    Q_OBJECT

public:
    CupHistoryWidget(QWidget *parent = Q_NULLPTR);
    ~CupHistoryWidget();

    ///
    /// @brief  设置反应盘中心显示的设备名字
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2023年8月10日，新建函数
    ///
    void SetDevName(QString text);

    ///
    /// @brief  获取打印/导出的反应杯历史信息
    /// 
    /// @return void
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年04月02日，新建函数
    ///
    void GetCupHistoryInfo(CupHistoryInfo& info);

private:

    ///
    /// @brief  初始化
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2023年6月28日，新建函数
    ///
    void Init();

    ///
    /// @brief
    ///     刷新反应盘界面
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年08月17日，新建函数
    ///
    void UpdateReactionCupWidget();

    ///
    /// @brief
    ///    通过项目编号列表获取项目名称字符串
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年04月02日，新建函数
    ///
    string GetTestedAssayNameByCodeList(const vector<int32_t>& codeList);

protected Q_SLOTS:
    
    ///
    /// @brief
    ///     刷新表格
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年08月17日，新建函数
    ///
    void OnUpdateTableView();

    ///
    /// @brief
    ///     表格选中行改变
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年08月17日，新建函数
    ///
    void OnCurrentRowChanged(const QModelIndex &current, const QModelIndex &previous);

    ///
    /// @brief
    ///     反应杯信息更新
    ///
    /// @param[in]  deviceSN  设备序列号
    /// @param[in]  rchis     反应杯更新信息
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年08月17日，新建函数
    ///
    void OnChReactionCupInfoUpdate(QString deviceSN,
         std::vector<ch::tf::ReactionCupHistoryInfo, std::allocator<ch::tf::ReactionCupHistoryInfo>> rchis);

private:
    Ui::CupHistoryWidget*								          ui;
    QStandardItemModel*                                           m_cupHistoryModel;
    map<string, vector<::ch::tf::ReactionCupHistoryInfo>>         m_mapDevAndCupInfo; // 设备的反应杯信息
};
