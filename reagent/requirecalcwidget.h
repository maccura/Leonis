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
/// @file     requirecalcdlg.h
/// @brief    需求计算信息页面
///
/// @author   4170/TangChuXian
/// @date     2023年6月26日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2023年6月26日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once

#include "uidcsadapter/adaptertypedef.h"
#include <QWidget>
#include <QStandardItemModel>
#include <QList>
#include <QSet>
#include "PrintExportDefine.h"

namespace Ui { class RequireCalcWidget; };

class RequireCalcWidget : public QWidget
{
    Q_OBJECT

public:
    RequireCalcWidget(Qt::DayOfWeek enDayOfWeek, QWidget *parent = Q_NULLPTR);
    ~RequireCalcWidget();

    ///
    /// @brief
    ///     加载数据
    ///
    /// @param[in]  iDevClassify  设备类别
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年9月28日，新建函数
    ///
    static void LoadData(int iDevClassify);

    ///
    /// @brief
    ///     保存数据
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年9月26日，新建函数
    /// @li 4170/TangChuXian，2023年9月28日，逻辑优化
    ///
    static void SaveData();

    ///
    /// @brief
    ///     更新所有表格列显示
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年10月7日，新建函数
    ///
    static void UpdateAllTblColVisible();

    ///
    /// @brief
    ///     更新所有表格行显示
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年10月7日，新建函数
    ///
    static void UpdateAllTblRowVisible();

    ///
    /// @brief
    ///     更新所有表格余量不足提示
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年10月7日，新建函数
    ///
    static void UpdateAllTblLackTipLab();

    ///
    /// @brief
    ///     更新当前表格余量文本颜色
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年10月7日，新建函数
    ///
    static void UpdateCurTblRemainValTextColor();

    ///
    /// @brief
    ///     获取打印信息
    ///
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 6889/ChenWei，2024年3月25日，新建函数
    ///
    void GetPrintInfo(RequireCalcInfo& Info);

protected:
    ///
    /// @brief
    ///     界面显示前初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年6月22日，新建函数
    ///
    void InitBeforeShow();

    ///
    /// @brief
    ///     界面显示后初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年6月22日，新建函数
    ///
    void InitAfterShow();

    ///
    /// @brief
    ///     初始化字符串资源
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年1月17日，新建函数
    ///
    void InitStrResource();

    ///
    /// @brief
    ///     初始化信号槽连接
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年6月22日，新建函数
    ///
    void InitConnect();

    ///
    /// @brief
    ///     初始化子控件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年6月22日，新建函数
    ///
    void InitChildCtrl();

    ///
    /// @brief
    ///     窗口显示事件
    ///
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年6月22日，新建函数
    ///
    void showEvent(QShowEvent *event);

    ///
    /// @brief
    ///     初始化数据模型
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年9月28日，新建函数
    ///
    static void InitModel();

protected Q_SLOTS:
    ///
    /// @brief
    ///     更新耗材需求表
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年9月26日，新建函数
    ///
    void UpdateSplReqTbl();

    ///
    /// @brief
    ///     设置是否只显示余量不足的数据
    ///
    /// @param[in]  bJustShowLack  是否只显示余量不足的数据
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年9月28日，新建函数
    ///
    void SetJustShowResidualLackingData(bool bJustShowLack);

    ///
    /// @brief
    ///     表格单元项改变
    ///
    /// @param[in]  pChangedItem  改变的表格单元项
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年4月28日，新建函数
    ///
    void OnItemChanged(QStandardItem* pChangedItem);

    ///
    /// @brief
    ///     更新列显示
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年10月7日，新建函数
    ///
    void UpdateColumnVisible();

    ///
    /// @brief
    ///     更新行显示
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年10月7日，新建函数
    ///
    void UpdateRowVisible();

    ///
    /// @brief
    ///     更新余量不足提示标签
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年10月7日，新建函数
    ///
    void UpdateLackTipLab();

    ///
    /// @brief
    ///     更新余量文本颜色
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年10月7日，新建函数
    ///
    void UpdateRemainValTextColor();

private:
    // 需求计算列表，表头枚举
    enum ReqCalcHeader {
        Rch_AssayName = 0,          // 项目名称
        Rch_ModuleRemain,           // 模块余量分布
        Rch_DynamicCalcAtMon,       // 动态计算(周一)
        Rch_DynamicCalcAtTue,       // 动态计算(周二)
        Rch_DynamicCalcAtWed,       // 动态计算(周三)
        Rch_DynamicCalcAtThur,      // 动态计算(周四)
        Rch_DynamicCalcAtFri,       // 动态计算(周五)
        Rch_DynamicCalcAtSat,       // 动态计算(周六)
        Rch_DynamicCalcAtSun,       // 动态计算(周天)
        Rch_ReqVolAtMon,            // 需求量(周一)
        Rch_ReqVolAtTue,            // 需求量(周二)
        Rch_ReqVolAtWed,            // 需求量(周三)
        Rch_ReqVolAtThur,           // 需求量(周四)
        Rch_ReqVolAtFri,            // 需求量(周五)
        Rch_ReqVolAtSat,            // 需求量(周六)
        Rch_ReqVolAtSun,            // 需求量(周天)
        Rch_RemainVol,              // 当前余量
        Rch_ReqLackAtMon,           // 需求缺口(周一)
        Rch_ReqLackAtTue,           // 需求缺口(周二)
        Rch_ReqLackAtWed,           // 需求缺口(周三)
        Rch_ReqLackAtThur,          // 需求缺口(周四)
        Rch_ReqLackAtFri,           // 需求缺口(周五)
        Rch_ReqLackAtSat,           // 需求缺口(周六)
        Rch_ReqLackAtSun,           // 需求缺口(周天)
        Rch_BottleSpeciffic,        // 瓶规格
        Rch_ReqBottleCountAtMon,    // 需载入瓶数(周一)
        Rch_ReqBottleCountAtTue,    // 需载入瓶数(周二)
        Rch_ReqBottleCountAtWed,    // 需载入瓶数(周三)
        Rch_ReqBottleCountAtThur,   // 需载入瓶数(周四)
        Rch_ReqBottleCountAtFri,    // 需载入瓶数(周五)
        Rch_ReqBottleCountAtSat,    // 需载入瓶数(周六)
        Rch_ReqBottleCountAtSun,    // 需载入瓶数(周天)
    };

public:
    static QList<SPL_REQ_VOL_INFO>      sm_splReqInfoLst;               // 耗材需求信息列表(全局共用)
    static QStandardItemModel*          sm_pStdModel;                   // 标准模型(全局共用)
    static const int                    sm_ciReqCalcTblDefaultRowCnt;   // 需求计算表默认行数
    static const int                    sm_ciReqCalcTblDefaultColCnt;   // 需求计算表默认列数
    static bool                         sm_bHideNormaData;              // 隐藏正常数据（余量充足）
    static QSet<RequireCalcWidget*>     sm_setAllInstance;              // 管理所有实例

private:
    Ui::RequireCalcWidget              *ui;                             // ui对象指针
    bool                                m_bInit;                        // 是否已经初始化

    // 常量定义
    const Qt::DayOfWeek                 m_cenDayOfWeek;                 // 周几
};
