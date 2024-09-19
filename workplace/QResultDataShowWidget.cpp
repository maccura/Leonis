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
#include "QResultDataShowWidget.h"
#include "ui_QResultDataShowWidget.h"
#include <QLabel>
#include <QRegExp>

QResultDataShowWidget::QResultDataShowWidget(QWidget *parent)
	: QWidget(parent)
{
    ui = new Ui::QResultDataShowWidget();
	ui->setupUi(this);
	SetDefaultStatus();
}

QResultDataShowWidget::~QResultDataShowWidget()
{
}

///
/// @brief 显示样本信息
///
/// @param[in]  module  信息模块
/// @param[in]  detailMsg  信息列表
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月27日，新建函数
///
void QResultDataShowWidget::SetModuleInfo(MODULE_TYPE module, const QStringList& detailMsg)
{
	// 根据类型查找对应的模块列表(找不到，或者信息模块为空，都退出)
	int type = static_cast<int>(module);
	if (m_widgetMaps.count(type) <= 0 || detailMsg.size() == 0)
	{
		return;
	}

	// 若需要显示的信息和控件数目不对，则返回
	if (m_widgetMaps[type].size() != detailMsg.size())
	{
		return;
	}

	// 将内容设置到每个label里面
	for_each(m_widgetMaps[type].begin(), m_widgetMaps[type].end(), [&](auto iter) {iter.second->setText(detailMsg[iter.first]); });
}

///
/// @brief 设置默认信息
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月26日，新建函数
///
void QResultDataShowWidget::SetDefaultStatus()
{
	// 清除表中内容
	m_widgetMaps.clear();
	QList<QWidget*> widgetMember = findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly);
	for (auto widget : widgetMember)
	{
		// 查找每一个模块对应的widgets
		int index = GetKeyValueFromWidgetName(widget);
		if (index == -1)
		{
			continue;
		}

		QList<QLabel*> memberLables = widget->findChildren<QLabel*>();
		std::map<int, QLabel*> labelMap;
		for (auto meber : memberLables)
		{
			int index = GetKeyValueFromWidgetName(meber);
			if (index == -1)
			{
				continue;
			}

			labelMap.insert(std::make_pair(index, meber));
			meber->setText("");
		}

		// 插入对应的句柄和模块对应表
		m_widgetMaps.insert(make_pair(index, labelMap));
	}
}

///
/// @brief 清除控件内容
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月27日，新建函数
///
void QResultDataShowWidget::ClearWidgetContent()
{
	if (m_widgetMaps.empty())
	{
		return;
	}

	// 依次清空内容
	for (auto widget : m_widgetMaps)
	{
		for_each(widget.second.begin(), widget.second.end(), [](auto iter) {iter.second->setText(""); });
	}
}
