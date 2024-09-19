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
/// @file     qcapplywidget.h
/// @brief    质控申请界面
///
/// @author   4170/TangChuXian
/// @date     2022年12月14日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月27日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include "src/thrift/gen-cpp/defs_types.h"
#include "src/thrift/im/gen-cpp/im_types.h"
#include "src/thrift/ch/gen-cpp/ch_types.h"
#include "uidcsadapter/adaptertypedef.h"
#include <QWidget>

namespace Ui { class QcApplyWidget; };
class QcApplyModel;

// 前置声明
class DefaultQcDlg;             // 默认质控对话框
class BackupRgntQcDlg;          // 备用瓶质控对话框
class QTableWidgetItem;         // 表格单元项
class SortHeaderView;           // 三序排序表头
class QStandardItem;            // 表格单元项
class QSortFilterProxyModel;    // 排序模型

class QcApplyWidget : public QWidget
{
    Q_OBJECT

public:
    QcApplyWidget(QWidget *parent = Q_NULLPTR);
    ~QcApplyWidget();

protected:
    ///
    /// @brief
    ///     界面显示前初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年11月5日，新建函数
    ///
    void InitBeforeShow();

    ///
    /// @brief
    ///     显示之后初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月16日，新建函数
    ///
    void InitAfterShow();

    ///
    /// @brief
    ///     初始化子控件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月26日，新建函数
    ///
    void InitChildCtrl();

    ///
    /// @brief
    ///     初始化信号槽连接
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月26日，新建函数
    ///
    void InitConnect();

    ///
    /// @brief
    ///     初始化信号槽连接
    ///
    /// @param[in]  pDlg  目标窗口
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月16日，新建函数
    ///
    void InitConnect(DefaultQcDlg* pDlg);

    ///
    /// @brief
    ///     初始化信号槽连接
    ///
    /// @param[in]  pDlg  目标窗口
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月16日，新建函数
    ///
    void InitConnect(BackupRgntQcDlg* pDlg);

    ///
    /// @brief
    ///     加载对话框
    ///
    /// @param[in]  pDlg  目标窗口
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月16日，新建函数
    ///
    void LoadDlg(DefaultQcDlg* pDlg);

    ///
    /// @brief
    ///     加载对话框
    ///
    /// @param[in]  pDlg  目标窗口
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月16日，新建函数
    ///
    void LoadDlg(BackupRgntQcDlg* pDlg);

    ///
    /// @brief
    ///     更新对话框
    ///
    /// @param[in]  pDlg  目标窗口
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月16日，新建函数
    ///
    void UpdateDlg(DefaultQcDlg* pDlg);

    ///
    /// @brief
    ///     更新对话框
    ///
    /// @param[in]  pDlg  目标窗口
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月16日，新建函数
    ///
    void UpdateDlg(BackupRgntQcDlg* pDlg);

    ///
    /// @brief
    ///     更新对话框质控品下拉框
    ///
    /// @param[in]  pDlg  目标窗口
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月18日，新建函数
    ///
    void UpdateDlgQcDocCombo(DefaultQcDlg* pDlg);

    ///
    /// @brief
    ///     更新对话框质控品下拉框
    ///
    /// @param[in]  pDlg  目标窗口
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月18日，新建函数
    ///
    void UpdateDlgQcDocCombo(BackupRgntQcDlg* pDlg);

    ///
    /// @brief
    ///     设置为默认排序
    ///
    /// @param[in]  pTableModel  表格控件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年5月30日，新建函数
    ///
    //void SetForDefaultSort(QStandardItemModel* pTableModel);

    ///
    /// @brief
    ///     解析质控申请更新
    ///
    /// @param[in]  chQcApply  质控申请
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年7月27日，新建函数
    ///
    void TranslateQcApplyUpdate(const ch::tf::QcApply& chQcApply);

	///
	/// @brief 获取日期Item,如果过期，则用红色填充单元格，并且字体也为红色
	///
	/// @param[in]  StrDate  yyyy-MM-dd固定格式的日期字符串
	///
	/// @return 设置好的item
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年11月14日，新建函数
	///
	QStandardItem* GetDateItem(const QString & StrDate);

    ///
    /// @brief
    ///     重写显示事件
    ///
    /// @param[in]  event  事件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月26日，新建函数
    ///
    virtual void showEvent(QShowEvent* event) override;

protected Q_SLOTS:
    ///
    /// @brief
    ///     更新质控申请表
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月26日，新建函数
    ///
    void ReloadQcReqTable();

    ///
    /// @brief
    ///     更新质控申请原因
    ///
    /// @param[in]  stuQcApplyInfo  质控申请信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月22日，新建函数
    ///
    void UpdateQcApplyReason(QC_APPLY_INFO stuQcApplyInfo);

    ///
    /// @brief
    ///     更新质控品下拉框
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月27日，新建函数
    ///
    void UpdateQcDocCombo();

    ///
    /// @brief
    ///     更新设备列表
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月19日，新建函数
    ///
    void UpdateDevNameList();

    ///
    /// @brief
    ///     更新单条质控申请信息
    ///
    /// @param[in]  reagentInfo  试剂信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月11日，新建函数
    ///
    //void UpdateSingleQcReqInfo(const im::tf::ReagentInfoTable& stuRgntInfo);

    ///
    /// @brief
    ///     质控申请表格被点击
    ///
    /// @param[in]  modIdx 被点击表格信息 
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月26日，代码重构
    ///
    void OnQcReqTableItemClicked(const QModelIndex& modIdx);

    ///
    /// @brief
    ///     质控品下拉框当前选项改变
    ///
    /// @param[in]  iIndex  质控品下拉框当前选中索引
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月27日，新建函数
    ///
    void OnQcDocComboIndexChanged(int iIndex);

    ///
    /// @brief
    ///     质控品重置
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年3月28日，新建函数
    ///
    void OnQcDocComboReset();

    ///
    /// @brief
    ///     查询按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月28日，新建函数
    ///
    void OnQueryBtnClicked();

    ///
    /// @brief
    ///     重置按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月28日，新建函数
    ///
    void OnResetBtnClicked();

    ///
    /// @brief
    ///     质控申请信息更新
    ///
    /// @param[in]  qcAplInfo  质控申请信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年11月9日，新建函数
    ///
    //void UpdateQcApplyInfo(im::tf::QcApply qcAplInfo);

    ///
    /// @brief
    ///     默认质控按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月16日，新建函数
    ///
    void OnDefaultQcBtnClicked();

    ///
    /// @brief
    ///     备用瓶质控按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月16日，新建函数
    ///
    void OnBackupRgntQcBtnClicked();

    ///
    /// @brief
    ///     Tab角落按钮索引改变
    ///
    /// @param[in]  iBtnIdx  按钮索引
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月15日，新建函数
    ///
    void OnTabCornerIndexChanged(int iBtnIdx);

    ///
    /// @brief
    ///     角落查询条件被重置
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月15日，新建函数
    ///
    void OnTabCornerQryCondReset();

    ///
    /// @brief
    ///     默认质控对话框表格单元格被点击
    ///
    /// @param[in]  index  表格单元项索引
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月16日，新建函数
    ///
    void OnDefaultQcDlgTblItemClicked(const QModelIndex& index);

    ///
    /// @brief
    ///     默认质控对话框质控名选项改变
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月16日，新建函数
    ///
    void OnDefaultQcDlgQcNameChanged();

    ///
    /// @brief
    ///     默认质控对话框设备名选项改变
    ///
    /// @param[in]  strDevName  当前设备名
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月16日，新建函数
    ///
    void OnDefaultQcDlgDevNameChanged(const QString& strDevName);

    ///
    /// @brief
    ///     备用瓶质控对话框表格单元格被点击
    ///
    /// @param[in]  index  表格单元项索引
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月16日，新建函数
    ///
    void OnBackupRgntQcDlgTblItemClicked(const QModelIndex& index);

    ///
    /// @brief
    ///     备用瓶质控对话框质控名选项改变
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月16日，新建函数
    ///
    void OnBackupRgntQcDlgQcNameChanged();

    ///
    /// @brief
    ///     备用瓶质控对话框设备名选项改变
    ///
    /// @param[in]  strDevName  当前设备名
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月16日，新建函数
    ///
    void OnBackupRgntQcDlgDevNameChanged(const QString& strDevName);

    ///
    /// @brief
    ///     对话框数据更新
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年1月4日，新建函数
    ///
    void OnDlgDataUpdate();

    ///
    /// @brief
    ///     试剂信息更新(临时)
    ///
    /// @param[in]  reag  设备编号
    /// @param[in]  type  位置信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年01月04日，新建函数
    ///
    void OnImReagentInfoUpdate(const im::tf::ReagentInfoTable& reag, const im::tf::ChangeType::type type);

    ///
    /// @brief
    ///     试剂信息更新(临时)
    ///
    /// @param[in]  supplyUpdates  耗材更新信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年01月04日，新建函数
    ///
    void OnChReagentInfoUpdate(const std::vector<ch::tf::SupplyUpdate, std::allocator<ch::tf::SupplyUpdate>>& supplyUpdates);

    ///
    /// @brief
    ///     质控申请信息更新(临时)
    ///
    /// @param[in]  qa  质控申请信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年01月04日，新建函数
    ///
    void OnImQcApplyInfoUpdate(im::tf::QcApply qa);

    ///
    /// @brief
    ///     质控申请信息更新(临时)
    ///
    /// @param[in]  sus  质控申请信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年01月04日，新建函数
    ///
    void OnChQcApplyInfoUpdate(std::vector<ch::tf::QcApply, std::allocator<ch::tf::QcApply>> sus);

    ///
    /// @brief
    ///     质控品信息更新
    ///
    /// @param[in]  vQcDoc  质控品信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年1月4日，新建函数
    ///
    void OnQcDocInfoInfoUpdate(std::vector<tf::QcDocUpdate, std::allocator<tf::QcDocUpdate>> vQcDoc);

    ///
    /// @brief
    ///     免疫试剂信息更新
    ///
    /// @param[in]  stuRgntInfo  试剂信息
    /// @param[in]  changeType   更新方式
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月31日，新建函数
    ///
    void UpdateImReagentChanged(const im::tf::ReagentInfoTable& stuRgntInfo, im::tf::ChangeType::type changeType);

    ///
    /// @brief
    ///     选择按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年6月5日，新建函数
    ///
    void OnSelBtnClicked();

    ///
    /// @brief
    ///     更新质控申请选择按钮
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年6月5日，新建函数
    ///
    void UpdateQcApplySelBtn();

    ///
    /// @brief
    ///     更新质控申请选择按钮且没有选中项改变
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年8月24日，新建函数
    ///
    void UpdateQcApplySelBtnWithoutSelChange();

    ///
    /// @brief
    ///     默认质控对话框选择按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年6月6日，新建函数
    ///
    void OnDefaultQcDlgSelBtnClicked();

    ///
    /// @brief
    ///     默认质控对话框选中项改变
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年6月6日，新建函数
    ///
    void OnDefaultQcDlgSelectionChanged();

    ///
    /// @brief
    ///     备用瓶质控对话框选择按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年6月6日，新建函数
    ///
    void OnBackupRgntQcDlgSelBtnClicked();

    ///
    /// @brief
    ///     备用瓶质控对话框选中项改变
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年6月6日，新建函数
    ///
    void OnBackupRgntQcDlgSelectionChanged();

Q_SIGNALS:
    ///
    /// @brief
    ///     对话框数据更新
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年01月04日，新建函数
    ///
    void SigDlgDataUpdate();

private:
    // 质控申请列表，表头枚举
    enum QcApplyHeader {
        Qah_Module = 0,         // 模块
        Qah_AssayName,          // 项目名称
        Qah_QcNo,               // 质控品编号
        Qah_QcName,             // 质控品名称
        Qah_QcBriefName,        // 质控品简称
        Qah_QcType,             // 质控品类型
        Qah_QcLevel,            // 质控品水平
        Qah_QcLot,              // 质控品批号
        Qah_QcReason,           // 质控品原因
        Qah_Selected,           // 选择
        Qah_DbIndex             // 数据ID
    };

private:
    Ui::QcApplyWidget*              ui;                                     // ui对象指针
    bool                            m_bInit;                                // 是否已经初始化
    QStringList                     m_strDevNameList;                       // 当前设备序列号列表
    QString                         m_strCurDevName;                        // 当前设备序列号
    QString                         m_strQryQcName;                         // 当前查询的质控名

    SortHeaderView*                 m_pSortHeader;                          // 表格的排序头
    QcApplyModel*                   m_pStdModel;                            // 标准模型
    QSortFilterProxyModel*          m_pSortModel;                           // 排序模型

    DefaultQcDlg*                   m_pDefaultQcDlg;                        // 默认质控对话框
    BackupRgntQcDlg*                m_pBackupRgntQcDlg;                     // 备用瓶质控对话框

    // 更新刷新效率
    bool                            m_bNeedUpdateQcApplyInfo;               // 需要更新

    // 常量定义
    const int                       m_ciQcApplyTblDefaultRowCnt;            // 质控申请表默认行数
    const int                       m_ciQcApplyTblDefaultColCnt;            // 质控申请表默认列数
    const char*                     m_cstrSelBtnStatusName;                 // 选择按钮状态名
    const bool                      m_cstrSelBtnStatusValSel;               // 选择按钮状态值——选择
    const bool                      m_cstrSelBtnStatusValCancelSel;         // 选择按钮状态值——取消选择
};
