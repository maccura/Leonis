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
/// @file     QcRuleSettingsDlg.h
/// @brief    质控规则设置界面
///
/// @author   8276/huchunli
/// @date     2022年12月23日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8276/huchunli，2022年12月23日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QWidget>
#include "shared/basedlg.h"
#include "src/thrift/gen-cpp/defs_types.h"

class QItemSelection;
class QComboBox;
class QStandardItemModel;
namespace Ui {
    class QcRuleSettingsDlg;
};

// 规则状态结构
struct RuleStat
{
    RuleStat(bool selected, tf::QcStat::type qcType)
        : m_selected(selected), m_stat(qcType)
    {}
    bool m_selected;
    tf::QcStat::type m_stat;
};

// 指定项目的规则设定缓存
struct AssayRules {
    tf::GeneralAssayInfo m_assayInfo;

    std::map<int, RuleStat> m_assayRules;
    int64_t m_qcX;
    int64_t m_qcY;

    ///
    /// @bref
    ///		清空设定的规则
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年6月10日，新建函数
    ///
    void ClearRules()
    {
        m_qcX = -1;
        m_qcY = -1;
        m_assayRules.clear();
    }

};


class QcRuleSettingsDlg : public BaseDlg
{
    Q_OBJECT

public:
    QcRuleSettingsDlg(QWidget *parent = Q_NULLPTR);
    ~QcRuleSettingsDlg();

protected Q_SLOTS:

    ///
    /// @bref
    ///		左侧项目列表中的某个项目被点击
    ///
    /// @param[in] qModel 点击的位置
    ///
    /// @par History:
    /// @li 8276/huchunli, 2022年12月23日，新建函数
    ///
    void OnAssayTableCellClicked(const QItemSelection& selected, const QItemSelection& deselected);

    ///
    /// @bref
    ///		右侧规则列表中的某条规则被点击
    ///
    /// @param[in] qModel 点击的位置
    ///
    /// @par History:
    /// @li 8276/huchunli, 2022年12月23日，新建函数
    ///
    void OnRuleTableCellClicked(const QModelIndex& qModel);

    ///
    /// @bref
    ///		规则的标记状态下拉框被设置
    ///
    /// @param[in] strText 修改的标记状态值
    ///
    /// @par History:
    /// @li 8276/huchunli, 2022年12月23日，新建函数
    ///
    void OnStatComboxChanged(const QString& strText);

    ///
    /// @bref
    ///		单击保存按钮，执行设置的规则的保存入库
    ///
    /// @par History:
    /// @li 8276/huchunli, 2022年12月23日，新建函数
    ///
    void OnSaveSettings();

    ///
    /// @bref
    ///		取消选择
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年6月10日，新建函数
    ///
    void OnCancelSettings();

    ///
    /// @bref
    ///		双击质控规则列头的响应
    ///
    /// @param[in] logicalIndex 列序号
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年6月10日，新建函数
    ///
    void OnDoubleClickedHeader(int logicalIndex);

    void OnChangeTab(int sel);

    void OnUnionQcDocXChanged(int sel);
    void OnUnionQcDocYChanged(int sel);

private:

    ///
    /// @bref
    ///		初始化项目信息列表
    ///
    /// @par History:
    /// @li 8276/huchunli, 2022年12月23日，新建函数
    ///
    bool InitAssayList();

    ///
    /// @bref
    ///		初始化规则列表
    ///
    /// @par History:
    /// @li 8276/huchunli, 2022年12月23日，新建函数
    ///
    void InitQcRulesTable();

    ///
    /// @bref
    ///		更新当前项目对应的联合质控品设置界面
    ///
    /// @param[in] assayInfo 当前选择的项目
    ///
    /// @par History:
    /// @li 8276/huchunli, 2022年12月27日，新建函数
    ///
    void UpdateUnionQcDocUi(const tf::GeneralAssayInfo& assayInfo);

    ///
    /// @bref
    ///		更新指定项目的规则状态信息
    ///
    /// @param[in] assayInfo 指定的项目
    ///
    /// @par History:
    /// @li 8276/huchunli, 2022年12月23日，新建函数
    ///
    void UpdateRuleSelected(const tf::GeneralAssayInfo& assayInfo);

    ///
    /// @bref
    ///		翻译告警、失控状态信息为枚举状态信息
    ///
    /// @param[in] strQcStat 字符串状态信息
    ///
    /// @par History:
    /// @li 8276/huchunli, 2022年12月27日，新建函数
    ///
    tf::QcStat::type TransefromQcStat(const QString& strQcStat);

    ///
    /// @bref
    ///		更新规则选择界面的选择状态信息
    ///
    /// @param[in] ruleStat 选择信息与状态信息
    ///
    /// @par History:
    /// @li 8276/huchunli, 2022年12月27日，新建函数
    ///
    void UpdateRuleSelectedUi(const std::map<int, RuleStat>& ruleStat);
    void CheckStat(int rowIndex, tf::QcStat::type qcType);

    bool FindQcComposition(const tf::QcDoc &qcDoc, int assayCode, tf::QcComposition& composition);

    ///
    /// @bref
    ///		联合质控界面质控品下拉框选择质控品的处理
    ///
    /// @param[in] curComb 触发的下拉框
    /// @param[in] sel 下拉框中选择的项
    ///
    /// @par History:
    /// @li 8276/huchunli, 2022年12月27日，新建函数
    ///
    void ProcessUnionChanged(QComboBox* curComb, int sel);

    ///
    /// @bref
    ///		点击选择或取消选择一个规则
    ///
    /// @param[in] curRow 行号
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年6月10日，新建函数
    ///
    void ChoiceOneRow(int curRow);

    void showEvent(QShowEvent *event) override;

    ///
    /// @bref
    ///		用于过滤鼠标滚轮事件等
    ///
    virtual bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::QcRuleSettingsDlg* ui;
    bool                  m_bInit;                              // 是否已经初始化
    QStandardItemModel*   m_model;                              // 数据模型

    std::map<int, std::shared_ptr<AssayRules>> m_ruleSelected;  //< assayCode, AssayRules>
    std::shared_ptr<AssayRules> m_currentRuleList;              //< 当前界面操作的列表信息
};
