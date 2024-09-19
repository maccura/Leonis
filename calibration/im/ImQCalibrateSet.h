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
/// @file     ImQCalibrateSet.h
/// @brief    校准设置界面
///
/// @author   1226/ZhangJing
/// @date     2022年5月10日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 1226/ZhangJing，2022年5月10日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <map>
#include <QWidget>
#include <QSortFilterProxyModel>
#include "src/thrift/im/gen-cpp/im_types.h"
#include "PrintExportDefine.h"

class QPostionEdit;
class QStandardItemModel;
class QItemSelectionModel;
class QImCalibrateSetFilterDataModule;
class ImCaliBrateRecordDialog;

namespace Ui {
    class ImQCalibrateSet;
};


class ImQCalibrateSet : public QWidget
{
    Q_OBJECT

public:
    ImQCalibrateSet(QWidget *parent = Q_NULLPTR);
    ~ImQCalibrateSet();

    ///
    /// @brief
    ///     初始化校准品组
    ///
    /// @par History:
    /// @li 1226/zhangjing，2022年5月10日，新建函数
    ///
    void ResetCaliBrateGroup();

	///
	/// @brief  刷新界面
	///     
	///
	/// @par History:
	/// @li 1226/zhangjing，2022年5月10日，新建函数
	///
	void RefreshPage();

	///
	/// @brief 更新下拉菜单 
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 1226/zhangjing，2022年12月12日，新建函数
	///
	void UpdateCaliCombox();

    ///
    /// @brief  添加了新的校准品
    ///
    /// @param[in]  iRow  添加的行
    /// @param[in]  grp  添加的校准组
    ///
    /// @par History:
    /// @li 1226/zhangjing，2024年1月8日，新建函数
    ///
    void AddCaliGrp(const int iRow, const im::tf::CaliDocGroup& grp);

public Q_SLOTS:

    ///
    /// @brief
    ///     点击校准组名称
    ///
    /// @param[in]  index  索引
    ///
    /// @par History:
    /// @li 1226/zhangjing，2022年5月10日，新建函数
    ///
    void OnSelectGroup(QModelIndex index);

    ///
    /// @brief
    ///     校准扫描
    ///
    /// @par History:
    /// @li 1226/zhangjing，2022年5月10日，新建函数
    ///
    void OnScanCalibrateGroup(const im::tf::CaliDocGroup& grp);

    ///
    /// @brief
    ///     更新校准数据
    ///
    /// @par History:
    /// @li 1226/zhangjing，2022年5月10日，新建函数
    ///
    void OnUpdateCaliBrateData();

    ///
    /// @brief
    ///     删除校准品组
    ///
    /// @par History:
    /// @li 1226/zhangjing，2022年5月10日，新建函数
    ///
    void OnDeleteCaliBrateGroup();

    ///
    /// @brief
    ///     校准设置表中单元项被点击
    ///
    /// @param[in]  modIdx  被点击的单元项索引
    ///
    /// @par History:
    /// @li 1226/zhangjing，2022年5月13日，新建函数
    ///
    void OnCalSetTblItemClicked(const QModelIndex& modIdx);

	///
	/// @brief
	///     刷新校准修改位置
	////
	/// @par History:
	/// @li 1226/zhangjing，2022年12月13日，新建函数
	///
	void OnRefreshPostion();

	///
	/// @bref
	///		校准品下拉框事件
	///
	/// @param[in] sel 选择的sel
	///
	/// @par History:
	/// @li 1226/zhangjing，2022年12月12日，新建函数
	///
	void OnCaliComboxChange(int sel);

	///
	/// @brief  查询按钮按下
	///
	/// @par History:
	/// @li 1226/zhangjing，2022年12月12日，新建函数
	///
	void OnQueryBtnClicked();

	///
  	/// @brief  校准组发生更新   
	///
	/// @param[in]  cdg   校准组信息
	/// @param[in]  type  更新类型
	///
	/// @par History:
	/// @li 1226/zhangjing，2023年2月23日，新建函数
	///
	void UpdateSingleCaliGrpInfo(const im::tf::CaliDocGroup& cdg, const im::tf::ChangeType::type type);

	///
	/// @brief  设备状态变化
	///
	/// @param[in]  deviceInfo  设备信息
	///
	/// @par History:
	/// @li 1226/zhangjing，2023年4月13日，新建函数
	///
	void OnDevStateChange(tf::DeviceInfo deviceInfo);

    ///
    /// @brief 响应打印按钮
    ///
    /// @par History:
    /// @li 6889/ChenWei，2023年12月14日，新建函数
    ///
    void OnPrintBtnClicked();

    ///
    /// @brief 响应打印按钮
    ///
    /// @par History:
    /// @li 6889/ChenWei，2023年12月14日，新建函数
    ///
    void OnExportBtnClicked();

    void GetPrintExportInfo(CaliRecordInfo& info);

    ///
    /// @bref
    ///		权限变化响应
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年5月14日，新建函数
    ///
    void OnPermisionChanged();

protected:
    // 重写隐藏事件
    virtual void hideEvent(QHideEvent *event) override;

    ///
    /// @brief
    ///     重写显示事件
    ///
    /// @param[in]  event  事件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年9月4日，新建函数
    ///
    virtual void showEvent(QShowEvent* event) override;

private:
    Ui::ImQCalibrateSet*            ui;
    QStandardItemModel*             m_CaliBrateGroup;           // 校准品组信息
	QImCalibrateSetFilterDataModule*m_ImCaliBrateGroupProxyModel; // 校准品组信息代理模型
    std::map<int,im::tf::CaliDocGroup> m_CaliGroupSave;         // 保存校准组信息
    QItemSelectionModel*            m_SelectModel;              // 选择模式
    int                             m_iClickSqe;                // 设置位置点击行对应序号
	bool                            m_bQuery;                   // 是否进行筛选
	QString                         m_queryName;                // 筛选名称
	QPostionEdit*					m_CaliPostionEditDialog;    // 校准位置编辑对话框
    ImCaliBrateRecordDialog*        m_caliRecordDlg;            // 校准登记界面
};

///////////////////////////////////////////////////////////////////////////
/// @file     ImQCalibrateSet.h
/// @brief 	 排序代理
///
/// @author   7656/zhang.changjiang
/// @date      2023年4月11日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年4月11日，新建文件
///
///////////////////////////////////////////////////////////////////////////
class QImCalibrateSetFilterDataModule : public QSortFilterProxyModel
{
public:
	QImCalibrateSetFilterDataModule(QObject* parent = nullptr);
	~QImCalibrateSetFilterDataModule();
    bool IsNumber(const QVariant & data, double & value) const;
	virtual bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;
};
