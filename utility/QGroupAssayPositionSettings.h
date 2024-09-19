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
/// @file     QGroupAssayPositionSettings.h
/// @brief    应用->项目位置->组合项目新增
///
/// @author   8276/huchunli
/// @date     2023年3月16日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8276/huchunli，2023年3月16日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QWidget>
#include <set>
#include "shared/basedlg.h"

namespace Ui { class QGroupAssayPositionSettings; };

class QScrollArea;
class QVBoxLayout;
class QGridLayout;

///
/// @bref
///		当前对话框类型（新增、修改、 项目选择）
///
enum EnumDlgType { DlgTypeGroupAssayAdd, DlgTypeGroupAssayModify, DlgTypeAssaySelect };

struct ViewCtrlInfo
{
    QScrollArea* m_vScro = Q_NULLPTR;
    QWidget* m_btnContainer = Q_NULLPTR;
    QVBoxLayout* m_qVLayout = Q_NULLPTR;
    QGridLayout* m_qAssayLayout = Q_NULLPTR;

    ViewCtrlInfo(QScrollArea* vs, QWidget* bc, QVBoxLayout* vl, QGridLayout* gl)
        :m_vScro(vs), m_btnContainer(bc), m_qVLayout(vl), m_qAssayLayout(gl)
    {};

    inline bool IsNull()
    {
        return m_vScro == Q_NULLPTR || m_btnContainer == Q_NULLPTR || m_qVLayout == Q_NULLPTR || m_qAssayLayout == Q_NULLPTR;
    }
};


class QGroupAssayPositionSettings : public BaseDlg
{
    Q_OBJECT

public:
    QGroupAssayPositionSettings(QWidget *parent, EnumDlgType dlgType);
    ~QGroupAssayPositionSettings();

    ///
    /// @bref
    ///		设置对话框类型为组合项目编辑或添加
    ///
    /// @param[in] dlgType 对话框类型
    /// @param[in] groupAssayName 组合项目名字
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年9月2日，新建函数
    ///
    void SetDlgTypeAndUpdate(EnumDlgType dlgType, const QString& groupAssayName = "");

    ///
    /// @bref
    ///		获取选择的项目名列表
    ///
    /// @param[out] checkedAssays 项目名列表
    /// @param[out] assayCodes 项目通道号列表
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年9月4日，新建函数
    ///
    void GetSelectedAssayNames(QStringList& checkedAssays, std::set<int>& assayCodes);

    ///
    /// @bref
    ///		清除选中的项目
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年9月4日，新建函数
    ///
    void ClearSelectedAssays();

    ///
    /// @bref
    ///		设置选中的项目
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年9月20日，新建函数
    ///
    void UpdateButtonCheckStat(const QStringList& assayNames);

signals:
        void SignalUpdateProfileList(QString strName);

        ///
        /// @bref
        ///		在选择框模式下的确认按钮信号
        ///
        /// @par History:
        /// @li 8276/huchunli, 2023年9月4日，新建函数
        ///
        void selected();

    protected Q_SLOTS:

    ///
    /// @bref
    ///		保存按钮的槽函数
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年3月16日，新建函数
    ///
    void OnSaveButtonPress();

    ///
    /// @bref
    ///		按钮筛选的槽函数
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年9月4日，新建函数
    ///
    void OnFilerLetter(bool stat);
    void OnAssayChecked(bool bChecked);

    ///
    /// @bref
    ///		取消按钮触发时，清空选择状态并退出
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年9月20日，新建函数
    ///
    void OnClearSelectedQuit();

    ///
    /// @bref
    ///		响应项目变化
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年1月10日，新建函数
    ///
    void UpdateItemShowDatas();

    ///
    /// @bref
    ///		页签切换后需要执行的处理
    ///
    /// @param[in] index 页签序号
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年5月22日，新建函数
    ///
    void OnCurrentTabChanged(int index);


private:
    ///
    /// @bref
    ///		初始化表格按钮，或重新初始化表格按钮
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年1月10日，新建函数
    ///
    void InitCalcAssayTable();

    void InitLetterWidget();

    void InitAssayTable();
    void InitAssayTableWidget(QWidget* pWidget, const QStringList& assayNames, \
        std::map<QString, QPushButton*>& btns, std::vector<QPushButton*>& vecBtn);

    ///
    /// @bref
    ///		组合项目添加与修改的保存
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年9月2日，新建函数
    ///
    bool SaveGroupAssayInfo();

    ///
    /// @bref
    ///		对页面按钮进行过滤后的显示和隐藏
    ///
    /// @param[in] strFilter 过滤字符
    /// @param[in] mapButtons 页面按钮数据
    /// @param[in] vecButtons 页面按钮数据
    /// @param[in] qNm 所有项目的名字，用于恢复全显示
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年9月4日，新建函数
    ///
    void FilterOnButtons(const QString& strFilter, std::map<QString, QPushButton*>* mapButtons, std::vector<QPushButton*>* vecButtons, QStringList* qNm);

    ///
    /// @bref
    ///		根据过滤条件执行筛选
    ///
    /// @param[in] strLetter 过滤条件
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年10月13日，新建函数
    ///
    void ProcFilterButtons(const QString& strLetter);

    ///
    /// @bref
    ///		通过字母筛选
    ///
    /// @param[in] litterBtn 被点击的字母按钮
    /// @param[in] bStat 是否勾选
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年5月22日，新建函数
    ///
    void FilterByLitter(QPushButton* litterBtn, bool bStat);

    ///
    /// @bref
    ///		重载隐藏事件
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年2月29日，新建函数
    ///
    void hideEvent(QHideEvent *event) override;

    ///
    /// @bref
    ///		获取页签的索引;如果获取失败则返回-1
    ///
    /// @param[in] qTbObjectName 页签页面名
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年6月5日，新建函数
    ///
    int GetTableIndex(const QString& qTbObjectName);

    ///
    /// @bref
    ///		通过名字获取对应的项目编号
    ///
    /// @param[in] assayName 项目名字
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年6月5日，新建函数
    ///
    int GetAssayCodeByName(const QString& assayName);
    std::string GetAssayNameByCode(int assayCode);

private:
    Ui::QGroupAssayPositionSettings *ui;
    EnumDlgType m_dlgType;
    int64_t m_modifyId; // 用于修改模式时存储被修改组合的id

    std::map<QString, QPushButton*> m_letterButtons;  // 字母筛选按钮
    std::map<QString, int> m_checkedButtons;          // 缓存已勾选的项目，用于字母筛选功能 <AssayName, AssayCode>

    std::map<QString, QPushButton*> m_assayButtonsCh; // 缓存界面生化项目按钮
    std::vector<QPushButton*> m_vecButtonCh;          // 有顺序的生化按钮表，（用于顺序隐藏）
    QStringList m_namesCh;

    std::map<QString, QPushButton*> m_assayButtonsIm; // 缓存界面免疫项目按钮
    std::vector<QPushButton*> m_vecButtonIm;          // 有顺序的免疫按钮表
    QStringList m_namesIm;

    std::map<QString, QPushButton*> m_assayButtonsCalc; // 缓存计算项目按钮
    std::vector<QPushButton*> m_vecButtonCalc;          // 有顺序的计算项目按钮
    QStringList m_namesCalc;

    std::map<QWidget*, ViewCtrlInfo> m_viewCtrls;       // 用于记录按钮框相关容器，再次使用；// TODO:把按钮列表包含
};
