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
/// @file     QcParamSettingsWidget.h
/// @brief    应用->项目设置->质控设置界面
///
/// @author   8276/huchunli
/// @date     2023年2月18日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8276/huchunli，2023年2月18日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QWidget>
#include <memory>
#include "model/AssayListModel.h"
#include "src/thrift/gen-cpp/defs_types.h" // hava to intruduce due to std::allocator<tf::QcDocUpdate>

class QStandardItemModel;

namespace Ui {
    class QcParamSettingsWidget;
};


class QcParamSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    QcParamSettingsWidget(QWidget *parent = Q_NULLPTR);
    ~QcParamSettingsWidget();

    ///
    /// @bref
    ///		设置参数，项目名称参数变化
    ///
    /// @param[in] item 项目信息
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年2月16日，新建函数
    ///
    void LoadQcParameter(const AssayListModel::StAssayListRowItem& item);

    ///
    /// @bref
    ///		保存质控设置的参数
    ///
    /// @param[in] pGen      通用项目信息
    /// @param[in] changedQc 质控文档信息
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年2月18日，新建函数
    ///
    bool GetQcParameter(tf::GeneralAssayInfo& pGen, std::vector<std::shared_ptr<tf::QcDoc>>& changedQc);

	///
	/// @brief 获取更改操作细节信息
	///
	/// @param[in]  strOptLog  返回的日志记录信息
	/// @param[in]  spAssayInfoUi  界面修改的项目信息
	/// @param[in]  vecQcDocUi  界面修改的项目质控信息
	/// @param[in]  vecQcDocs  修改前的项目质控信息
	///
	/// @return 
	///
	/// @par History:
	/// @li 1556/Chenjianlin，2024年3月7日，新建函数
	///
	void GetImAssayOptLog(QString& strOptLog, const std::shared_ptr<tf::GeneralAssayInfo> spAssayInfoUi, const std::vector<std::shared_ptr<tf::QcDoc>>& vecQcDocsUi, const std::vector<tf::QcDoc>& vecQcDocs);

    ///
    /// @brief
    ///     当前页面是否有未保存的数据
    ///
    ///@param[in]    item 项目信息
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年4月11日，新建函数
    ///
    bool HasUnSaveChangedData(const AssayListModel::StAssayListRowItem& item);

protected slots :

    ///
    /// @bref
    ///		质控品表格的行选中发生变化
    ///
    /// @param[in] currIdx xxx
    /// @param[in] prevIdx xxx
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年3月20日，新建函数
    ///
    void OnTabelSelectChanged(const QModelIndex& currIdx, const QModelIndex& prevIdx);

    ///
    /// @bref
    ///		权限变化响应
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年3月21日，新建函数
    ///
    void OnPermisionChanged();

    ///
    /// @bref
    ///		质控品信息发生变化
    ///
    /// @param[in] infos 变化信息
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年5月31日，新建函数
    ///
    void OnQcDocUpdate(std::vector<tf::QcDocUpdate, std::allocator<tf::QcDocUpdate>> infos);
    void OnAssayInfoUpdate();

private:
    // 更新界面上的质控设置信息
    void UpdateQcUi(const std::string& qcSettings);

    ///
    /// @bref
    ///		查找相同项目通道号的compisition的索引
    ///
    /// @param[in] qcDoc 质控品
    /// @param[in] assayCode 目标项目通道号
    ///
    /// @return Invalid:-1, else index.
    /// @par History:
    /// @li 8276/huchunli, 2023年2月18日，新建函数
    ///
    int  FindAutoCtrlPosition(const std::shared_ptr<tf::QcDoc>& qcDoc, int assayCode);

    void ClearQcSettingUi();

    ///
    /// @bref
    ///		把界面上的自动质控设置保存到行缓存中
    ///
    /// @param[in] rowIdx 对应行号
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年11月13日，新建函数
    ///
    void SaveUiAutoCtrlToCache(int rowIdx);

    ///
    /// @bref
    ///		把当前行的质控品信息进行入库保存
    ///
    /// @param[in] rowIdx 当前行号
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年11月13日，新建函数
    ///
    void GetChangedQcAutoCtrl(int rowIdx, std::vector<std::shared_ptr<::tf::QcDoc>>& chengedQc);

    ///
    /// @bref
    ///		把质控时间填写到界面
    ///
    /// @param[in] qcSettings 质控设置
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年11月21日，新建函数
    ///
    void AssignQcTimeToUi(const tf::QcAssaySettings& qcSettings);

protected:
	///
	/// @brief 窗口显示事件
	///     
	/// @param[in]  event  事件对象
	///
	/// @par History:
	/// @li 1226/zhangjing，2023年4月27日，新建函数
	///
	void showEvent(QShowEvent *event);

private:
    Ui::QcParamSettingsWidget		*ui;
    QStandardItemModel*				m_qcModel;						// 质控composition表Model
    std::map<int64_t, std::string> m_origAutoCtrl;                  // 缓存最初的自动质控配置，用于判断界面是否修改

    bool                            m_disableAutoQcChk;             // 禁用自动质控勾选框
    bool							m_bSampleTypeFlag;				// 样本源下拉框是否可用
    int								m_currentAssayCode;             // 记录当前左侧选中的项目通道号
	AssayListModel::StAssayListRowItem	m_rowItem;				    // 当前选中行信息
};
