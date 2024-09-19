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
/// @file     QAssayIndexSetPage.h
/// @brief    项目位置设置
///
/// @author   5774/WuHongTao
/// @date     2022年6月1日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月1日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <map>
#include <QWidget>
#include "workplace/QAssayTabWidgetContainer.h"

class QStandardItemModel;
class QGroupAssayPositionSettings;
class QLabel;
class QGridLayout;
class QPushButton;
class QItemSelection;
class QVBoxLayout;

namespace Ui {
    class QAssayIndexSetPage;
};


class QAssayIndexSetPage : public QWidget
{
    Q_OBJECT

public:
    QAssayIndexSetPage(QWidget *parent = Q_NULLPTR);
    ~QAssayIndexSetPage();

    ///
    /// @brief 保存字母对应的控件元素（标题、按钮、占位控件）
    ///
    struct GridButtonsInfo
    {
        QLabel* m_letterLable;
        QGridLayout* m_gridLayout;
        std::vector<QPushButton*> m_buttons;
        std::vector<QWidget*> m_placeHolder;

        ///
        /// @bref
        ///		清除占位控件
        ///
        /// @par History:
        /// @li 8276/huchunli, 2023年8月9日，新建函数
        ///
        void ClearPlaceHolder();


        ///
        /// @bref
        ///		当前单元的全部控件隐藏
        ///
        /// @par History:
        /// @li 8276/huchunli, 2023年12月21日，新建函数
        ///
        void HideAll();
    };

protected:
    ///
    /// @brief 初始化模块代码
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年6月1日，新建函数
    ///
    void Init();

    ///
    /// @brief 保存组合项目的位置信息
    ///
    /// @param[in]  assayKeyid  组合项目的关键key= 10000+ 数据库主键
    /// @param[in]  assayInfo   项目信息
    ///
    /// @return true返回成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年6月28日，新建函数
    ///
    bool SaveProfileInfo(int assayKeyid, const int postion);

    ///
    /// @brief 保存单项目的项目位置信息
    ///
    /// @param[in]  assayKeyid  但项目的项目编号
    /// @param[in]  assayInfo   项目信息
    ///
    /// @return true返回成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年6月28日，新建函数
    ///
    bool SaveAssayInfo(int assayKeyid, const int postion);

    void showEvent(QShowEvent *event);

    protected slots:

    ///
    /// @brief 更新项目数据
    ///
    ///
    /// @return 成功 true
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年1月3日，新建函数
    ///
    bool UpdateAssayDataUnion(QString strName);

    ///
    /// @brief 点击单项目
    ///
    /// @param[in]  index  单项目的位置号
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年6月2日，新建函数
    ///
    void OnSingleAssaySelect();

    ///
    /// @brief 点击组合项目
    ///
    /// @param[in]  index   组合项目位置
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年6月2日，新建函数
    ///
    void OnCompostionAssaySelect(const QModelIndex& index);

    ///
    /// @brief 处理项目设置模块信息（生化或者免疫类型被选中）
    ///
    /// @param[in]  index  7：生化8：免疫
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年6月2日，新建函数
    ///
    void OnAssayTypeSelectChange(int index);

    ///
    /// @brief 保存项目位置信息
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年6月6日，新建函数
    ///
    void OnSaveAssaySetInfo();

    ///
    /// @bref
    ///		组合项目的增改删
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年3月15日，新建函数
    ///
    void OnUnionAssayAdd();
    void OnUnionAssayModify();
    void OnUnionAssayDelete();

    // 更新项目列表
    void UpdateSingleAssaysTable();

    // 搜索框槽函数
    void OnSearchEditChanged(const QString& searchText);

    // 单项目组合项目页签切换
    void OnTabChanged(int curSection);

    ///
    /// @bref
    ///		角色变更时的修改
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年8月11日，新建函数
    ///
    void OnPermisionChanged();

    ///
    /// @bref
    ///		检测组合项目列表选中变化
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年8月24日，新建函数
    ///
    void OnUnionTabelSelectChanged(const QItemSelection& currIndex, const QItemSelection& prevIndex);

    ///
    /// @bref
    ///		退出时处理
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年10月13日，新建函数
    ///
    void OnQuitBtn();

    ///
    /// @bref
    ///		往字符串中插入换行符，返回插入换行符后的字符串
    ///
    /// @param[in] strSource 目标字符串
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年2月29日，新建函数
    ///
    QString InsertSlashN(const QString& strSource);

    ///
    /// @bref
    ///		按钮选中状态变化
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年5月7日，新建函数
    ///
    void OnSelectedChagned();

private:

    ///
    /// @bref
    ///		初始化右侧项目名按钮展示框
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年3月14日，新建函数
    ///
    void InitSingleAssayWidget();
    void InitSingleAssayWidgetSpeci(const QStringList& assayList, QWidget* pWidget, std::map<QString, GridButtonsInfo>& ctrKe);

    // 刷新项目名
    void UpdateSingleAssaysTableSpeci(const QStringList& assayList, QWidget* pWidget, std::map<QString, GridButtonsInfo>& ctrKe);

    ///
    /// @bref
    ///		对项目名按首字母分类
    ///
    /// @param[in] srcData 总的项目列表
    /// @param[out] classfiedAss 分类后的项目名称表
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年3月16日，新建函数
    ///
    void ClassfyAssayByLitteral(const QStringList& srcData, std::map<QString, QStringList>& classfiedAss);

    ///
    /// @bref
    ///		更新组合项目列表
    ///
    /// @param[in] contentList 组合项目内容表格
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年3月16日，新建函数
    ///
    void UpdateUnionAssayTable(const std::vector<std::pair<QString, QString>>& contentList);

    // 添加一个字母段
    void AddLetterButtons(QWidget* btnContainer, QVBoxLayout* qvLayout, 
        const QString& letter, const QStringList& assayNames, std::map<QString, GridButtonsInfo>& ctrKe);

    void GetAllLetter(std::vector<QString>& allLetter);

    // 单项目框的搜索
    void FilterSingleAssay(const QString& strFilter);
    // 组合目框的搜索
    void FilterUnionAssay(const QString& strFilter);
    std::vector<std::pair<QString, QString>> GetCurrentUnionData();

    ///
    /// @bref
    ///		更新备用订单的设置（主要是因为删除了位置的项目，不能显示在备用订单中）
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年7月25日，新建函数
    ///
    void UpdateStandbyOrder();

    ///
    /// @bref
    ///		更新组合项目的编辑删除按钮状态
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年8月29日，新建函数
    ///
    void UpdateUnionDeleteButtonStuta();

    ///
    /// @bref
    ///		创建一个按钮
    ///
    /// @param[in] pParent 父类对象
    /// @param[in] strText 按钮字符
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年4月22日，新建函数
    ///
    QPushButton* CreatNewButton(QWidget* pParent, const QString& strText);

private:
    Ui::QAssayIndexSetPage*			ui;
    QStandardItemModel*				m_compstionModule;	///< 生化组合项目列表

    QGroupAssayPositionSettings*    m_groupAssaySetDlg; ///< 组合项目选择框

    std::map<QString, GridButtonsInfo> m_singleAssayGridCh;  ///< 单项目名分组表的控件缓存
    std::map<QString, GridButtonsInfo> m_singleAssayGridIm;  ///< 单项目名分组表的控件缓存
};
