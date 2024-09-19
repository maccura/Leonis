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
/// @file     QAssayTabWidgets.h
/// @brief    项目选择控件
///
/// @author   5774/WuHongTao
/// @date     2022年5月7日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月7日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QTabWidget>
#include <vector>
#include <map>
#include <memory>
#include <unordered_map>

class QAssaySelectButton;
struct AssayButtonData;
struct CornerButtonInfo;
class QGridLayout;
class QLabel;

namespace Ui {
    class statusLabel;
    class QAssayTabWidgets;
};

// 项目选择器类型
enum AssayType
{
    ASSAYTYPE1_WORKPAGE = 0,      // 项目选择器类型1, 工作页面，有属性的按钮
    ASSAYTYPE2,                   // 项目选择器类型2, 应用--项目位置设置框、备用订单，增加项目的项目选择
    ASSAYTYPE3_POSITION_SET       // 项目选择器类型3, 应用--项目位置设置框中的按钮
};

///
/// @brief 单项目或组合项目按钮分类保存容器
///
class PageBtnContainer : public QWidget
{
    Q_OBJECT

        ///
        /// @brief 单项目或组合项目按钮分类保存容器
        ///
        struct BtnContainer
    {
        ///
        /// @bref 构造BtnContainer
        ///
        BtnContainer(int idxPage, QGridLayout* pLay, QLabel* lable);

        int m_pageIndex;
        QGridLayout* m_layout;
        QLabel* m_titleLable;
        std::vector<QAssaySelectButton*> m_buttons;
    };

public:
    ///
    /// @bref
    ///		构造PageBtnContainer
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年6月13日，新建函数
    ///
    PageBtnContainer(int totalPage, int totalRowCnt, int pageSingleRowCnt, int col);

    ///
    /// @bref
    ///		添加一个按钮到缓存容器
    ///
    /// @param[in] pageIdx 页面索引
    /// @param[in] isUnion 是否是组合项目
    /// @param[in] pLayout 按钮的Layout
    /// @param[in] pBtn 按钮
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年6月13日，新建函数
    ///
    void AddButton(int pageIdx, bool isUnion, QGridLayout* pLayout, QAssaySelectButton* pBtn, QLabel* titleLable);

    ///
    /// @bref
    ///		设置页数对齐，避免空联合项或单项找不到页的情况(在每页按钮加完后调用)
    ///
    /// @param[in] pageIndex 页码
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年6月15日，新建函数
    ///
    void ProEmptyPageSingle(int pageIndex, QGridLayout* pLayout, QLabel* tLable);
    void ProEmptyPageUnion(int pageIndex, QGridLayout* pLayout, QLabel* tLable);

    ///
    /// @bref
    ///		调整单项目与组合项目的行数占比
    ///
    /// @param[in] singleRowAmount 单项目在一页中的行数
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年6月13日，新建函数
    ///
    void ResetSingleRowAmount(int singleRowAmount);

    ///
    /// @bref
    ///		设置父Widget
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年6月16日，新建函数
    ///
    inline void SetParentWidget(QWidget* parent) { m_parent = parent; }

    ///
    /// @bref
    ///		设置容器类型
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年6月16日，新建函数
    ///
    inline void SetDlgType(AssayType dType) { m_dlgType = dType; }

    ///
    /// @bref
    ///		获取最新的全局按钮顺序列表
    ///
    /// @param[out] vctBtns 按钮顺序列表
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年6月16日，新建函数
    ///
    inline void GetAllButtonsVec(std::vector<QAssaySelectButton*>& vctBtns) { vctBtns = m_allBtns; }

    ///
    /// @bref
    ///		获取位置存在编号的项目与位置
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年6月16日，新建函数
    ///
    inline std::vector<std::pair<int, int>>& GetChangedAssays() { return m_changedAssayCodes; }

    ///
    /// @bref
    ///		获取选中按钮的索引(如果没有找到目标按钮，则返回-1)
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年11月21日，新建函数
    ///
    int GetSelecteButtonIndex();

signals:
    ///
    /// @brief 中转项目按钮点击信号
    ///
    void selectAssay(int);

private:

    ///
    /// @bref
    ///		重新排列按钮的位置
    ///
    /// @param[in] it 当页按钮的容器迭代器
    /// @param[out] totalIdx 按钮总体的序号
    /// @param[out] tpIdx 按钮单项目或组合项目的序号
    /// @param[in] isUnion 是否是组合项目
    /// @param[in] curAllBtns 重新排列的按钮容器
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年6月16日，新建函数
    ///
    void RePositionButtons(std::unordered_map<int, std::shared_ptr<BtnContainer>>::iterator& it, \
        int& totalIdx, int& tpIdx, bool isUnion, std::vector<QAssaySelectButton*>& curAllBtns);

    int m_totalPage;            /// 总页数
    int m_pgSingleRowAmount;    /// 每页的单项目行数
    int m_pgTotalRowAmount;     /// 每页总行数
    int m_columAmount;          /// 列数

    QWidget* m_parent;
    AssayType m_dlgType;

    std::unordered_map<int, std::shared_ptr<BtnContainer>> m_pageBtnsSingle;  /// 单项目按钮容器
    std::vector<QAssaySelectButton*> m_btnsSingle;                            /// 单项目所有按钮

    std::unordered_map<int, std::shared_ptr<BtnContainer>> m_pageBtnsUnion;   /// 联合项目按钮容器
    std::vector<QAssaySelectButton*> m_btnsUnion;                             /// 联合项目所有按钮

    std::vector<QAssaySelectButton*> m_allBtns;                               /// 所有按钮
    std::vector<std::pair<int, int>> m_changedAssayCodes;                     ///< 位置信息变化了的项目编号<assayCode, globleIdx>
};


class QAssayTabWidgets : public QTabWidget
{
    Q_OBJECT

public:

    QAssayTabWidgets(QWidget *parent = Q_NULLPTR);
    ~QAssayTabWidgets();

    ///
    /// @brief 设置控件属性
    ///
    /// @param[in]  type	 控件类型
    /// @param[in]  firstRow 单项目的行数 
    /// @param[in]  row      总行数
    /// @param[in]  column   列数
    /// @param[in]  pages    页数
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月7日，新建函数
    ///
    void SetAssayTabAttribute(AssayType type, int firstRow, int row, int column, int pages);

    ///
    /// @brief 更新项目的控件列表
    ///
    /// @param[in]  assayDatas  控件列表数据
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月7日，新建函数
    ///
    void UpdateTabAssayStatus(std::vector<std::map<int, AssayButtonData>>& assayDatas);
    void UpdateTabAssayStatusWithNoClear(std::vector<std::map<int, AssayButtonData>>& assayDatas);

    ///
    /// @brief 设置是否显示状态栏
    ///
    /// @param[in]  flag  true显示
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月9日，新建函数
    ///
    void SetShowStatuFlag(bool flag);

    ///
    /// @brief 设置
    ///
    /// @param[in]  pageInfos  按钮信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月9日，新建函数
    ///
    void SetDeviceSelect(const std::vector<CornerButtonInfo>& pageInfos);

    ///
    /// @brief 设置项目选择属性
    ///
    /// @param[in]  indexPos   项目位置
    /// @param[in]  attribute  属性  
    ///
    /// @return true：表示设置成功，反之失败
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月11日，新建函数
    ///
    bool SetAssayButtonAttribute(int indexPos, AssayButtonData& attribute);

    ///
    /// @brief 通过位置获取对应的项目属性信息
    ///
    /// @param[in]  indexPos  位置信息
    /// @param[out] attribute 属性信息 
    ///
    /// @return true：表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月11日，新建函数
    ///
    bool GetAttributeByIndex(int indexPos, AssayButtonData& attribute);

    ///
    /// @brief 根据属性code获取项目信息
    ///
    /// @param[in]  assayCode  属性code
    /// @param[in]  attribute  属性
    ///
    /// @return true：表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月11日，新建函数
    ///
    bool GetAttributeByAssayCode(int assayCode, AssayButtonData& attribute, int& indexPostion);

    ///
    /// @bref
    ///		通过项目名查找项目按钮信息
    ///
    /// @param[in] assayName 项目名或组合项目名
    /// @param[out] attribute 返回的Attr信息
    /// @param[out] indexPostion 返回的Attr的索引
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年5月10日，新建函数
    ///
    bool GetAttributeByName(const QString& assayName, AssayButtonData& attribute, int& indexPostion);

    ///
    /// @brief 获取页面类型
    ///
    ///
    /// @return 页面类型
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年7月24日，新建函数
    ///
    AssayType GetAssayType() { return m_type; };

    ///
    /// @brief 获取项目按钮最大个数
    ///
    ///
    /// @return 项目最大数目
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月12日，新建函数
    ///
    int GetAssayButtonCount();

    ///
    /// @brief 完全清除状态（因为位置改变）
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年6月7日，新建函数
    ///
    void ClearComplete();

    ///
    /// @bref
    ///		清除所有的按钮信息
    ///
    /// @param[out] effectedAssayCodes 被清除了的项目编号
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年5月10日，新建函数
    ///
    void ClearAllButtons(std::vector<int>& effectedAssayCodes);

    ///
    /// @bref
    ///		清除选中状态
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年10月13日，新建函数
    ///
    void ClearAllSeletedStatu();

    ///
    /// @bref
    ///		更新按钮位置
    ///
    /// @param[in] singleRowAmount 当项目的行数
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年6月13日，新建函数
    ///
    void RefreshAllButtons(int singleRowAmount);

    ///
    /// @bref
    ///		获取位置存在变化的项目信息
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年6月13日，新建函数
    ///
    std::vector<std::pair<int, int>> GetChangedAssaysList();

    ///
    /// @bref
    ///		获取选中按钮的索引位置
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年11月21日，新建函数
    ///
    inline int GetCurrentSelectePosition() { return m_assayBtnContainer == nullptr ? -1 : m_assayBtnContainer->GetSelecteButtonIndex(); };

signals:
    ///
    /// @brief 信号
    ///
    /// @param[in]	 项目位置
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月11日，新建函数
    ///
    void selectAssay(int);

protected:
    ///
    /// @brief 清除当前项目状态
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月11日，新建函数
    ///
    void ClearStatus();

    ///
    /// @brief 初始化项目选择控件【普通控件】
    ///
    /// @param[in]  assayRow  单独项目的行数
    /// @param[in]  allRows   总项目行数（单独项目行数+组合项目行数）
    /// @param[in]  columns   控件列数
    /// @param[in]  pages     项目页数
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月7日，新建函数
    ///
    void LoadAssayButtons(int assayRow, int allRows, int columns, int pages);

    protected slots:
    void OnSelected(int indexPostion);

    ///
    /// @brief 更新项目状态的槽函数
    ///
    /// @param[in]  assayCodes  项目编号之间用“,”来分割
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年8月19日，新建函数
    ///
    void OnUpdateAssayStatus(QString assayCodes);

private:
    Ui::QAssayTabWidgets*			ui;
    Ui::statusLabel*				m_statusLabel;
    AssayType						m_type;                     ///< 项目对话框类型
    int								m_pages;

    std::vector<QAssaySelectButton*>m_vAssayBtn;                ///< 项目选择按钮
    std::map<int, int>				m_assayIndexMap;			///< 项目位置映射表（项目编号-项目位置）
    std::shared_ptr<PageBtnContainer> m_assayBtnContainer;      ///< 按钮容器

    std::vector<std::map<int, AssayButtonData>> m_pageData;		///< 项目数据
};
