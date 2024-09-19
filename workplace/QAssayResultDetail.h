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
/// @file     QAssayResultDetail.cpp
/// @brief    结果详情
///
/// @author   5774/WuHongTao
/// @date     2022年5月26日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月26日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include "shared/basedlg.h"
#include "boost/optional.hpp"
#include "src/thrift/gen-cpp/defs_types.h"
#include "src/thrift/ch/gen-cpp/ch_types.h"

class QStandardItemModel;
class QResultActionCurve;

namespace Ui {
    class QAssayResultDetail;
};


class QAssayResultDetail : public BaseDlg
{
	Q_OBJECT

public:
	QAssayResultDetail(QWidget *parent = Q_NULLPTR);
	~QAssayResultDetail();

	///
	/// @brief 显示样本测试结果详情
	///
	/// @param[in]  sampleDb  样本的数据库主键
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月26日，新建函数
	///
	void ShowSampleResultDetail(int64_t sampleDb);

protected:
	///
	/// @brief 初始化结果页面
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月26日，新建函数
	///
	void Init();

	///
	/// @brief 设置页面的默认状态
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月26日，新建函数
	///
	void SetDefaultStatus();

	///
	/// @brief 显示样本基本信息
	///
	/// @param[in]  sampleInfo  样本信息
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月26日，新建函数
	///
	void ShowSampleBasicInfo(tf::SampleInfo& sampleInfo);

	///
	/// @brief 获取当前项目信息的列表
	///
	/// @param[in]  sampleInfo   样本信息
	/// @param[in]  testitemInfo  项目信息
	///
	/// @return 项目信息列表
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月27日，新建函数
	///
	QStringList GetCurrentTestInfo(int type, tf::SampleInfo sampleInfo, tf::TestItem testitemInfo);

	///
	/// @brief 根据数据库主键，获取项目测试结果
	///
	/// @param[in]  id  数据库主键
	///
	/// @return 成功则返回
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年7月11日，新建函数
	///
	boost::optional<ch::tf::AssayTestResult> GetAssayResultById(int64_t id);

	///
	/// @brief 根据测试项目结果信息获取试剂信息
	///
	/// @param[in]  testitemInfo  测试结果
	///
	/// @return 试剂信息
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月27日，新建函数
	///
	QStringList GetReagentInfo(int testResultId);

private slots:
	///
	/// @brief 项目被点击，槽函数
	///
	/// @param[in]  index  点击位置
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月26日，新建函数
	///
	void OnAssayClicked(const QModelIndex& index);

	///
	/// @brief 显示反应曲线
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月30日，新建函数
	///
	void OnShowReactCurve();

private:
	Ui::QAssayResultDetail*			ui;
	QStandardItemModel*				m_testItemModel;
	std::map<int64_t, tf::TestItem> m_testItems;
	QResultActionCurve*				m_actionDiglog;	
	tf::SampleInfo					m_sampleInfo;
};
