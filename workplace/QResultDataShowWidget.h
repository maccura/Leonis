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
/// @file     QResultDataShowWidget.cpp
/// @brief    结果详细数据显示
///
/// @author   5774/WuHongTao
/// @date     2022年5月27日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月27日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <map>
#include <QWidget>

namespace Ui {
    class QResultDataShowWidget;
};
class QLabel;


class QResultDataShowWidget : public QWidget
{
	Q_OBJECT

public:

	// 各模块分类标志
	enum MODULE_TYPE
	{
		CURRENT_RESULT,
		ITEM_SHOW,
		REAGENT_SHOW,
		CALIBRATE_SHOW,
		QUALIFY_SHOW,
		RESULT_STATUS_SHOW
	};

	QResultDataShowWidget(QWidget *parent = Q_NULLPTR);
	~QResultDataShowWidget();

	///
	/// @brief 显示样本信息
	///
	/// @param[in]  module  信息模块
	/// @param[in]  detailMsg  信息列表
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月27日，新建函数
	///
	void SetModuleInfo(MODULE_TYPE module, const QStringList& detailMsg);

	///
	/// @brief 清除控件内容
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月27日，新建函数
	///
	void ClearWidgetContent();

protected:
	///
	/// @brief 设置默认信息
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月26日，新建函数
	///
	void SetDefaultStatus();

	///
	/// @brief 根据名称获取编号
	///
	/// @param[in]  widget  控件句柄
	///
	/// @return 控件编号
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月27日，新建函数
	///
	template<typename WidgetType>
	int GetKeyValueFromWidgetName(WidgetType* widget);

private:
	Ui::QResultDataShowWidget*		ui;
	std::map<int, std::map<int, QLabel*>>	m_widgetMaps;		///< 结果详情的映射关系
};

///
/// @brief 根据名称获取编号
///
/// @param[in]  widget  控件句柄
///
/// @return 控件编号
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月27日，新建函数
///
template<typename WidgetType>
int QResultDataShowWidget::GetKeyValueFromWidgetName(WidgetType* widget)
{
	int index = -1;

	if (widget == nullptr)
	{
		return index;
	}

	// 为空则跳过
	if (widget->objectName().isEmpty())
	{
		return index;
	}

	// 空则跳过
	QStringList names = widget->objectName().split("_");
	if (names.size() < 2)
	{
		return index;
	}

	bool ok = false;
	index = names.back().toInt(&ok, 10);
	return index;
}
