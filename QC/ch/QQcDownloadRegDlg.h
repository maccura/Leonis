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
/// @file     QQcDownloadRegDlg.h
/// @brief    质控下载登记
///
/// @author   8580/GongZhiQiang
/// @date     2023年4月26日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年4月26日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <memory>
#include <QWidget>
#include "src/thrift/cloud/gen-cpp/cloud_adapter_types.h"

namespace Ui {
    class QQcDownloadRegDlg;
};

namespace tf {
    class QcDoc;
};


class QStandardItemModel;


class QQcDownloadRegDlg : public QWidget
{
	Q_OBJECT

public:
	QQcDownloadRegDlg(QWidget *parent = Q_NULLPTR);
	~QQcDownloadRegDlg();

    ///
    /// @brief 进入下载模式
    ///
    /// @return true 成功
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年12月1日，新建函数
    ///
    bool StartDownload(int rowIndex, const std::shared_ptr<tf::QcDoc>& curQcDoc);

protected Q_SLOTS:
    ///
    /// @brief  点击查询按钮
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年11月28日，新建函数
    ///
    void OnClickSearchBtn(void);

    ///
    /// @brief  点击下载按钮
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年11月29日，新建函数
    ///
    void OnClickDownload(void);

    ///
    /// @brief  质控下载表格被点击
    ///
    /// @param[in]  modIdx 被点击表格信息 
    ///
    /// @par History:
    /// @li 1226/zhangjing，2022年11月29日，新建函数
    ///
    void OnTblItemClicked(const QModelIndex& modIdx);

    ///
    /// @brief  点击重置按钮
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年12月1日，新建函数
    ///
    void OnClickReset(void);

	///
	/// @brief 查询处理
	///
	/// @param[in]  isSuccess  是否查询成功
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年4月8日，新建函数
	///
	void OnSearchHander(bool isSuccess);

private:
	///
	/// @brief 初始化对话框
	///
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年4月26日，新建函数
	///
	void InitDialg();

	///
	/// @brief 初始化信号槽
	///
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年4月26日，新建函数
	///
	void InitConnects();

    ///
    /// @bref 保存前ISE特殊检测
    ///		  ISE(包含Na, K, CL项目)样本类型只支持“血清血浆”或“尿液”
    ///
    /// @par History:
    /// @li 8580/GongZhiQiang, 2023年11月10日，新建函数
    ///
    bool IseSpecialCheck(const ::tf::QcDoc& saveQcDoc);

    ///
    /// @bref
    ///		检测是否在数据库中存在质控品编号批号水平重复的质控品
    ///
    /// @param[in] saveDoc 编辑后的质控品
    /// @return true 无重复可继续流程，false重复不可修改
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年8月17日，新建函数
    ///
    bool CheckRepeatQc(const ::tf::QcDoc& saveDoc);

    ///
    /// @brief  显示已筛选到的doc
    ///
    /// @param[in]  vtDocs  查询结果
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年11月29日，新建函数
    ///
    void OnDisplayCloudQcDocs(std::vector <::adapter::tf::CloudQcDoc> vtDocs);

	///
	/// @brief 根据项目编号获取机型
	///
	/// @param[in]  assayCode  项目编号
	///
	/// @return 机型，失败返回无效机型
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年5月17日，新建函数
	///
	tf::DeviceType::type GetDeciveTypeByAssaycode(int assayCode);

signals:
    ///
    /// @brief 更新数据的消息
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年12月12日，新建函数
    ///
    void Update();

    ///
    /// @brief
    ///     关闭信号
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年12月12日，新建函数
    ///
    void SigClosed();

	///
	/// @brief 查询结果信号
	///
	/// @param[in]  isSuccess  是否成功
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年4月8日，新建函数
	///
	void searchResult(bool isSuccess);

private:

	Ui::QQcDownloadRegDlg* ui;
	QStandardItemModel*             m_QcDownloadGroup;          // 质控品组

    std::shared_ptr<::adapter::tf::CloudQcDoc>    m_selCloudQcDoc;   // 选中云质控信息
    int                             m_rowIndex;                 // 质控品在界面的行索引
    int                             m_fixLengthSn;              // 质控编号长度
    std::shared_ptr<tf::QcDoc>      m_qcDoc;

	std::vector <::adapter::tf::CloudQcDoc> 		m_downloadDataCache;	///< 下载登记数据缓存
	bool											m_isRequesting;			///< 是否正在请求下载
};
