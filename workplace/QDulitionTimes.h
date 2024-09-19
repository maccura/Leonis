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
/// @file     QDulitionTimes.h
/// @brief    稀释倍数/次数
///
/// @author   5774/WuHongTao
/// @date     2022年5月12日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月12日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include "shared/basedlg.h"
#include <boost/bimap.hpp>
#include "src/thrift/gen-cpp/defs_types.h"

class QStandardItemModel;

namespace Ui {
    class QDulitionTimes;
};


class QDulitionTimes : public BaseDlg
{
	Q_OBJECT

public:
	QDulitionTimes(QWidget *parent = Q_NULLPTR);
	~QDulitionTimes();

	///
	/// @brief 设置项目列表
	///
	/// @param[in]  testItems  项目列表信息
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月12日，新建函数
	///
	void SetAssayParamter(std::vector<::tf::TestItem>& testItems);

	///
	/// @brief 获取参数信息
	///
	/// @param[out]  testItems  参数信息
	///
	/// @return true：成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月13日，新建函数
	///
	bool GetAssayparameterInfo(std::vector<::tf::TestItem>& testItems, std::map<int,int>& assayRepeatTimes);

signals:
	void dulitionTimesSelected();

protected slots:
	///
	/// @brief 确定按钮槽函数
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月12日，新建函数
	///
	void OnOkBtnClicked();

protected:
	///
	/// @brief 初始化稀释页面
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月13日，新建函数
	///
	void Init();

private:
	Ui::QDulitionTimes*				ui;
	QStandardItemModel*				m_itemConfigModel;			///< 样本列表的model
	int								m_dataRow;					///< 数据长度
	boost::bimap<int, QString>		m_dulition;					///< 稀释比例对照表
};
