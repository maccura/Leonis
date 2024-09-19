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
/// @file     backuprgntqcdlg.h
/// @brief    备用瓶质控对话框
///
/// @author   4170/TangChuXian
/// @date     2021年6月22日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2021年6月22日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once

#include "shared/basedlg.h"
#include "SortHeaderView.h"
namespace Ui { class BackupRgntQcDlg; };

// 前置声明
class SortHeaderView;                       // 三序排序表头
class QcApplyModel;                         // 表格单元项
class QRadioButton;                         // 单选按钮
class QSortFilterProxyModel;                // 排序模型

class BackupRgntQcDlg : public BaseDlg
{
    Q_OBJECT

public:
    BackupRgntQcDlg(const QStringList strDevNameList, QWidget *parent = Q_NULLPTR);
    ~BackupRgntQcDlg();

    ///
    /// @brief
    ///     排序内容
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月21日，新建函数
    ///
    void SortContent();

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

Q_SIGNALS:
    ///
    /// @brief
    ///     默认质控表单元项被点击
    ///
    /// @param[in]  index  被点击单元项索引
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年11月3日，新建函数
    ///
    void SigItemClicked(const QModelIndex& index);

    ///
    /// @brief
    ///     当前质控名改变
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年6月23日，新建函数
    ///
    void SigCurQcNameChanged();

    ///
    /// @brief
    ///     当前设备名改变
    ///
    /// @param[in]  strDevName  设备名
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月16日，新建函数
    ///
    void SigCurDevNameChanged(const QString& strDevName);

    ///
    /// @brief
    ///     数据更新
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年1月4日，新建函数
    ///
    void SigDataUpdate();

    ///
    /// @brief
    ///     选择按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年6月6日，新建函数
    ///
    void SigSelBtnClicked();

    ///
    /// @brief
    ///     选中项改变
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年6月6日，新建函数
    ///
    void SigSelectionChanged();

protected Q_SLOTS:
    ///
    /// @brief
    ///     设备单选框被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月16日，新建函数
    ///
    void OnDevRBtnClicked();

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
    ///     查询条件改变
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年1月10日，新建函数
    ///
    void OnQryCondChanged();

private:
    // 备用瓶列表，表头枚举
    enum BackupRgntQcHeader {
        Bqh_Pos = 0,            // 位置
        Bqh_AssayName,          // 项目名称
        Bqh_RgntLot,            // 试剂批号
        Bqh_RgntNo,             // 试剂瓶号
        Bqh_UseStatus,          // 使用状态
        Bqh_QcNo,               // 质控品编号
        Bqh_QcName,             // 质控品名称
        Bqh_QcBriefName,        // 质控品简称
        Bqh_QcType,             // 质控品类型
        Bqh_QcLevel,            // 质控品水平
        Bqh_QcLot,              // 质控品批号
        Bqh_ExpDate,            // 失效日期
        Bqh_Selected,           // 选择
        Bqh_DbIndex             // 数据ID
    };

private:
    Ui::BackupRgntQcDlg        *ui;                                 // ui指针
    bool                        m_bInit;                            // 是否已经初始化

    QStringList                 m_strDevNameList;                   // 设备名列表
    QString                     m_strCurDevName;                    // 当前设备名
    QRadioButton*               m_pAllDevRBtn;                      // 全部设备单选框

    int                         m_iSortCol;                         // 排序列
    SortHeaderView::SortOrder   m_enSortOrder;                      // 排序顺序
    SortHeaderView*             m_pSortHeader;                      // 表格的排序头
    QcApplyModel*               m_pStdModel;                        // 标准模型
    QSortFilterProxyModel*      m_pSortModel;                       // 排序模型

    // 常量定义
    const int                   m_ciBackupRgntQcTblDefaultRowCnt;   // 备用瓶质控表默认行数
    const int                   m_ciBackupRgntQcTblDefaultColCnt;   // 备用瓶质控表默认列数

    friend class                QcApplyWidget;                      // 声明友元类
};
