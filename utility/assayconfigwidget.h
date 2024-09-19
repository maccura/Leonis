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
/// @file     assayparamwidget.h
/// @brief    应用界面->项目参数
///
/// @author   4170/TangChuXian
/// @date     2020年5月25日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2020年5月25日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QSet>
#include <QWidget>
#include <QTableView>
#include <QStyledItemDelegate>
#include "model/AssayListModel.h"
#include "src/thrift/ise/gen-cpp/ise_types.h"
#include "src/thrift/ch/gen-cpp/ch_types.h"
#include "src/thrift/im/gen-cpp/im_types.h"

namespace Ui { class AssayConfigWidget; };

// 生化特殊项目信息key:assaycode+deviceType val:specialAssayInfos
using ChSAIMaps = std::map<std::pair<int, int>, std::vector<std::shared_ptr<ch::tf::SpecialAssayInfo>>>;

// ISE特殊项目信息key:assaycode+deviceType val:specialAssayInfos
using IseSAIMaps = std::map<std::pair<int, int>, std::vector<std::shared_ptr<ise::tf::SpecialAssayInfo>>>;

// 免疫项目信息key:assaycode+deviceType val:GeneralAssayInfo
using ImGenMaps = std::map<std::pair<int, int>, std::vector<std::shared_ptr<im::tf::GeneralAssayInfo>>>;

// 项目追加
class AddAssayDlg;
class BackUnitDlg;
class AssayConfigWidget;
class QUtilitySortFilterProxyModel;

class SelectRowChangedDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    bool editorEvent(QEvent *event, QAbstractItemModel *model,
        const QStyleOptionViewItem &option, const QModelIndex &index) Q_DECL_OVERRIDE;

    void setView(QTableView* view) { m_pView = view; }
    void setWidget(AssayConfigWidget* wid) { m_pAssayConfigWidget = wid; }
private:
    QTableView*                         m_pView;
    AssayConfigWidget*                  m_pAssayConfigWidget;
};



class AssayConfigWidget : public QWidget
{
    Q_OBJECT

public:
    AssayConfigWidget(QWidget *parent = Q_NULLPTR);
    ~AssayConfigWidget();

    ///
    /// @brief
    ///     当前页面是否有未保存的数据
    ///
    /// @param[out]  isExistUnSaveSample  是否存在未保存的样本类型
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年4月25日，新建函数
    ///
    bool HasUnSaveChangedData(bool& isExistUnSaveSample);

    ///
    /// @brief
    ///     设置界面需要刷新
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年7月31日，新建函数
    ///
    void setNeedUpdateWidget() { m_bNeedUpdateWidget = true; }

protected:
    ///
    /// @brief 界面显示后初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年5月25日，新建函数
    ///
    void InitAfterShow();

    ///
    /// @brief
    ///     初始化测试项目表
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年5月26日，新建函数
    ///
    void InitAssayTable();

    ///
    /// @brief
    ///     初始化信号槽连接
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年5月26日，新建函数
    ///
    void InitConnect();

    ///
    /// @brief 窗口显示事件
    ///     
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年5月25日，新建函数
    ///
    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);

    ///
    /// @brief
    ///     查询生化需要显示的项目
    ///
    /// @return key:assaycode+deviceType val:specialAssayInfos
    ///
    /// @par History:
    /// @li 6950/ChenFei，2022年05月13日，新建函数
    ///
    ChSAIMaps QueryChShowAssays();

    ///
    /// @brief
    ///     查询ISE需要显示的项目
    ///
    /// @return key:assaycode+deviceType val:specialAssayInfos
    ///
    /// @par History:
    /// @li 6950/ChenFei，2022年05月13日，新建函数
    ///
    IseSAIMaps QueryIseShowAssays();

    ///
    /// @brief
    ///     查询免疫需要显示的项目
    ///
    /// @return key:assaycode+deviceType val:specialAssayInfos
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年2月13日，新建函数
    ///
    void QueryImShowAssays(ImGenMaps& mapImGeneralAssays);

    ///
    /// @brief
    ///     添加新的显示项目
    ///
    ///@param[out]   items		显示数据
    ///@param[in]    assayCode	项目代号
    ///@param[in]    deviceType 设备型号
    ///@param[in]    sampleSourceType 样本源类型
    ///@param[in]    version    参数版本
    ///
    /// @par History:
    /// @li 6950/ChenFei，2022年05月13日，新建函数
    ///
    void AddShowAssay(std::vector<AssayListModel::StAssayListRowItem>& items,
        int32_t assayCode, int32_t deviceType, int32_t sampleSourceType, const QString& version);

    ///
    /// @brief
    ///     获取选中行数据
    ///
    /// @return     选中行数据，如果assayCode == -1，数据无效
    ///
    /// @par History:
    /// @li 6950/ChenFei，2022年05月16日，新建函数
    ///
    AssayListModel::StAssayListRowItem GetSelRowItem();

    ///
    /// @brief
    ///     初始化Tab页
    ///
    /// @par History:
    /// @li 7951/LuoXin，2022年6月23日，新建函数
    ///
    void InitTabWidget();

    ///
    /// @brief
    ///     处理选中行数据改变
    ///
    ///@param[in]    versionOrSampleType 参数版本或者样本源类型
    ///@param[in]    isVerSionChanged    是否是参数版本改变
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年12月19日，新建函数
    ///
    void DealSelectRowDataChanged(int versionOrSampleType, bool isVerSionChanged);

    ///
    /// @brief
    ///     删除项目
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年5月24日，新建函数
    ///
    void deleteAssay();

    bool eventFilter(QObject *object, QEvent *event) Q_DECL_OVERRIDE;

signals:

    // 开始删除
    void startDelete();

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
    /// @brief
    ///     测试项目选中项发生改变
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年5月26日，新建函数
    ///
    void OnSelectedAssayChanged(const QModelIndex &current, const QModelIndex &previous);

    ///
    /// @brief
    ///     当前项目的选中的样本源类型改变
    ///
    ///@param[in]    sampleType    样本源类型
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年12月19日，新建函数
    ///
    void OnSelectRowSampleTypeChanged(int sampleType);

    ///
    /// @brief
    ///     当前项目的选中的参数版本改变
    ///
    ///@param[in]    isVerSion    是否是参数版本改变
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年12月19日，新建函数
    ///
    void OnSelectRowVersionChanged(int version);

    ///
    /// @brief
    ///     新增按钮被点击
    /// @par History:
    /// @li 7951/LuoXin，2022年8月16日，新建函数
    ///
    void OnflatAddClicked();

	///
	/// @brief 通道号管理按钮被点击
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年5月20日，新建函数
	///
	void OnAssayCodeMagBtnClicked();

    ///
    /// @brief
    ///     备选单位按钮被点击
    /// @par History:
    /// @li 7951/LuoXin，2022年8月16日，新建函数
    ///
    void OnBackUnitClicked();

    ///
    /// @brief
    ///     保存按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年7月3日，新建函数
    ///
    void OnSaveBtnClicked();

    ///
    /// @brief
    ///     删除按钮被点击
    ///
    /// @par History:
    /// @li 7951/LuoXin，2022年9月2日，新建函数
    ///
    void OnDelBtnClicked();

    ///
    /// @brief
    ///     生化按钮选中
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年7月9日，新建函数
    ///
    void OnChemistryClicked();

    ///
    /// @brief
    ///     免疫模式
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年7月9日，新建函数
    ///
    void OnImmune();

    ///
    /// @brief
    ///     生化，免疫页面切换
    ///
    /// @param[in]  index  
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 7951/LuoXin，2022年6月23日，新建函数
    ///
    void OnChangePage(int index);

    ///
    /// @bref
    ///		参数下载按钮
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年8月1日，新建函数
    ///
    void OnParamDownload();

    ///
    /// @bref
    ///		参数导出按钮
    ///
    /// @par History:
    /// @li 8580/GongZhiQiang, 2023年8月10日，新建函数
    ///
    void OnParamExport();

    ///
    /// @bref	权限变化响应
    ///		
    ///
    /// @par History:
    /// @li 8580/GongZhiQiang, 2023年9月7日，新建函数
    ///
    void OnPermisionChanged();

    ///
    /// @bref	项目备选单位更新
    ///		
    ///
    /// @par History:
    /// @li 7951/LuoXin, 2024年7月24日，新建函数
    ///
    void OnAssayUnitUpdate();

    ///
    /// @bref	响应项目新增界面的保存按钮
    ///		
    ///
    /// @par History:
    /// @li 7951/LuoXin, 2024年2月21日，新建函数
    ///
    void OnAddAssayDlgSaveClicked();

    ///
    /// @bref
    ///		响应指定项目选中
    ///
    /// @param[in] strAssayName 希望被选中的项目名
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年3月19日，新建函数
    ///
    void OnSelectAssayByName(QString strAssayName);

private:

    void CheckAssayCanDelete();
    bool IsFitDeleteCondition(int assayCode, const QString& name);

	///
	/// @bref 控件显示影响因素改变
	///		
	///
	/// @par History:
	/// @li 8580/GongZhiQiang, 2023年11月22日，新建函数
	///
	void ControlDisplayFactorsChanged();

private:
    Ui::AssayConfigWidget			*ui;						// Ui指针
    QSet<int>                       m_curAssaySampleSources;    // 当前新增项目的样本类型列表(用户切换当前选中项目时提醒)
    bool                            m_bNeedUpdateWidget;        // 是否需要刷新界面
    bool							m_bInit;					// 是否已经初始化
    bool							m_chemistry;				// 是否生化模式（true代表生化模式）
    int                             m_addAssayCode;             // 新增项目的编号
    int                             m_deletedRowIdx;            // 删除项目后需要选中被删除项目的下一行，如果没有下一行，则选中上一行

    AddAssayDlg*					m_addAssayDlg;				// 项目追加界面
    BackUnitDlg*					m_backUnitDlg;				// 备选单位窗口
    AssayListModel*                 m_chDataModel;              // 生化表格排序的model
    AssayListModel*                 m_imDataModel;              // 免疫表格排序的model
    QUtilitySortFilterProxyModel*   m_chSortModel;              // 生化表格排序的model
    QUtilitySortFilterProxyModel*   m_imSortModel;              // 免疫表格排序的model
};
