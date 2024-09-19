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
/// @file     QStatisBaseWgt.h
/// @brief    统计分析ui基类
///
/// @author   7656/zhang.changjiang
/// @date     2023年11月9日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月9日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QWidget>
#include <QListView>
#include <QStandardItemModel>
#include <QDateEdit>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <QRadioButton>
#include <QStyle>
#include <QStyleOptionComboBox>
#include <QStylePainter>
#include <QToolTip>
#include "src/thrift/gen-cpp/statis_types.h"

class QStatisBaseWgt : public QWidget
{
	Q_OBJECT

public:
	QStatisBaseWgt(QWidget *parent = Q_NULLPTR);
	~QStatisBaseWgt();
protected:

	///
	/// @brief  获取选中的设备序列号
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月7日，新建函数
	///
	std::vector<std::string> GetVecDeviceSN();

	///
	/// @brief  获取选中的项目名称
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月7日，新建函数
	///
	std::vector<int> GetVecAssayCode();

	///
	/// @brief  获取选中的试剂批号
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月7日，新建函数
	///
	std::vector<std::string> GetVecReagentLot();

	///
	///  @brief 连接信号槽
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年7月17日，新建函数
	///
	virtual void ConnectSlots();
protected slots:
	
	///
	/// @brief  更新模块下拉列表
	///
	/// @param[in]  bIsDefault  是否是默认模式（true ：默认全部查询并显示，fasle ：更具用户选择的日期查询并显示）
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月7日，新建函数
	///
	void UpdateDevList(const bool bIsDefault = false);
	
	///
	/// @brief  更新项目名称下拉列表
	///
	/// @param[in]  bIsDefault  是否是默认模式（true ：默认全部查询并显示，fasle ：更具用户选择的日期查询并显示）
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月9日，新建函数
	///
	void UpdateAssayItemList(const bool bIsDefault = false);

	///
	/// @brief  更新试剂批号下拉列表
	///
	/// @param[in]  bIsDefault  是否是默认模式（true ：默认全部查询并显示，fasle ：更具用户选择的日期查询并显示）
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月7日，新建函数
	///
	void UpdateReagentLotList(const bool bIsDefault = false);

	///
	/// @brief  筛选按钮被点击
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月7日，新建函数
	///
	virtual void OnQueryBtnClicked();

	///
	/// @brief  重置按钮被点击
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月7日，新建函数
	///
	virtual void OnResetBtnClicked();

	///
	/// @brief  模块复选框
	///
	/// @param[in]  state  复选框状态
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月7日，新建函数
	///
	void CheckDevStateChanged(int state);
	
	///
	/// @brief  项目名称复选框
	///
	/// @param[in]  state  复选框状态
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月7日，新建函数
	///
	void CheckAssayStateChanged(int state);
	
	///
	/// @brief  试剂批号复选框 
	///
	/// @param[in]  state  复选框状态
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月7日，新建函数
	///
	void CheckLotStateChanged(int state);
	
	///
	/// @brief  起止日期改变
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月27日，新建函数
	///
	virtual void DateEditChanged();
	
	///
	/// @brief  下拉列表选中响应槽
	///
	/// @param[in]  pComboBox  下拉列表
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2024年4月23日，新建函数
	///
	void ComboBoxActivated(QComboBox *pComboBox);
private:

	///
	/// @brief  生成复选框选选中的项目显示字符串
	///
	/// @param[in]  vecStr  选中的项目名称列表
	///
	/// @return 选中的项目显示字符串
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2023年11月24日，新建函数
	///
	QString CreateSelectedItemShowInfo(const QVector<QString>& vecStr);
protected:
	QDateEdit *                             m_pLowerDateEit;        // 开始时间
	QDateEdit *                             m_pUpperDateEit;        // 结束时间
	QStandardItemModel *					m_pDevModel;			// 模块下拉列表模型
	QListView *								m_pDevView;				// 模块下拉列表视图
	QComboBox *                             m_pDevComboBox;         // 模块下拉列表控件
	QStandardItemModel *					m_pAssayModel;			// 项目名称下拉列表模型
	QListView *								m_pAssayView;	        // 项目名称下拉列表视图
	QComboBox *                             m_pAssayComboBox;       // 项目名称下拉列表控件
	QRadioButton *                          m_pReagRadioBtn;        // 试剂单选按钮
	QStandardItemModel *					m_pReagModel;			// 试剂批号下拉列表模型
	QListView *								m_pReagView;			// 试剂批号下拉列表视图
	QComboBox *                             m_pReagComboBox;        // 试剂批号下拉列表控件
	QPushButton *                           m_pQueryBtn;            // 筛选按钮
	QPushButton *                           m_pResetBtn;            // 重置按钮
	::tf::StatisType::type					m_pStatisType;          // 统计类型
};

///
/// 自定义下拉列表
///
class CustomComboBox : public QComboBox {
public:
	CustomComboBox(QWidget *parent = nullptr) : QComboBox(parent) {}

protected:
	
	///
	/// @brief  绘制函数
	///
	/// @param[in]  event  绘制事件
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2024年4月23日，新建函数
	///
	void paintEvent(QPaintEvent *event) override;
	
	///
	/// @brief  鼠标进入事件
	///
	/// @param[in]  event  事件
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2024年4月23日，新建函数
	///
	void enterEvent(QEvent *event) override;

	///
	/// @brief  鼠标离开事件
	///
	/// @param[in]  event  事件
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2024年4月23日，新建函数
	///
	void leaveEvent(QEvent *event) override;
};

///
/// 自定义复选框
///
class EllipsisCheckBox : public QCheckBox {
public:
	explicit EllipsisCheckBox(QWidget *parent = Q_NULLPTR) : QCheckBox(parent) {}
	explicit EllipsisCheckBox(const QString &text, QWidget *parent = Q_NULLPTR) : QCheckBox(text, parent) {}
protected:

	///
	/// @brief  绘制函数
	///
	/// @param[in]  event  绘制事件
	///
	/// @return 
	///
	/// @par History:
	/// @li 7656/zhang.changjiang，2024年4月23日，新建函数
	///
	void paintEvent(QPaintEvent *event) override;
};
