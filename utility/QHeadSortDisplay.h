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
/// @file     QDisplayTableSet.cpp
/// @brief    显示设置页面
///
/// @author   5774/WuHongTao
/// @date     2023年2月1日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2023年2月1日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QWidget>
#include <QStyledItemDelegate>
#include "src/public/ConfigDefine.h"

namespace Ui {
    class QHeadSortDisplay;
};
class QPainter;
class QCheckBox;
class QStandardItemModel;
class QTableView;

///////////////////////////////////////////////////////////////////////////
/// @class     QShowTextDelegate
/// @brief 	   表格提示框代理
///////////////////////////////////////////////////////////////////////////
class QShowTextDelegate : public QStyledItemDelegate
{
public:
    QShowTextDelegate(QObject *parent = nullptr);
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
};


class QSelectNewDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    QSelectNewDelegate(QObject *parent);
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
        const QModelIndex & index) const;

Q_SIGNALS:
    void closeDelegate(QString name);
};

class QHeadSortDisplay : public QWidget
{
    Q_OBJECT

public:
    QHeadSortDisplay(QWidget *parent = Q_NULLPTR);
    ~QHeadSortDisplay();

    ///
    /// @brief 设置表类型
    ///
    /// @param[in]  data  设置数据
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年2月2日，新建函数
    ///
    void SetTableData(DisplaySet data);

    void showEvent(QShowEvent *event);

	///
	/// @brief 配置是否被修改过未保存
	///
	///
	/// @return true:被修改过
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年3月29日，新建函数
	///
	bool IsModified();

Q_SIGNALS:
    void closeDialog();

    protected slots:
    ///
    /// @brief 复选框的槽函数（点击复选框）
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年2月2日，新建函数
    ///
    void OnCheckBoxClicked();

    ///
    /// @brief 保存数据
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年2月6日，新建函数
    ///
    void OnSaveData();

    ///
    /// @bref
    ///		权限变化响应
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年2月24日，新建函数
    ///
    void OnPermisionChanged();

private:
    ///
    /// @brief 初始化函数
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年2月2日，新建函数
    ///
    void Init();

	///
	/// @brief 是否全部选中
	///
	/// @param[in]  selected  已经选中的条目数目
	///
	/// @return true:全部选中
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年11月16日，新建函数
	///
	bool IsSelectAll(int selected);

	///
	/// @brief 根据item类型和项目类型获取列表中的位置
	///
	/// @param[in]  type		项目类型
	/// @param[in]  dataType	item类型
	///
	/// @return 位置
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年11月16日，新建函数
	///
	int ConvertTypeToPostion(int type, DisplayType dataType);

	///
	/// @brief 通过类型获取项目名称
	///
	/// @param[in]  type		项目类型
	/// @param[in]  dataType    item类型
	///
	/// @return 项目名称
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年11月16日，新建函数
	///
	QString GetNameFormType(int type, DisplayType dataType);

	///
	/// @brief 根据数据生成对应的checkBox
	///
	/// @param[in]  tableView	目标tableView
	/// @param[in]  dataItem    数据
	/// @param[in]  dataType    item类型
	///
	/// @return 生成的checkBox
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年11月16日，新建函数
	///
	QCheckBox* GenCheckBoxFromData(QTableView* tableView, DisplaySetItem dataItem, DisplayType dataType);

	///
	/// @brief 获取行所对应的checkbox
	///
	/// @param[in]  row  行号
	///
	/// @return checkbox
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年10月8日，新建函数
	///
	QCheckBox* GetRowCheckBox(int row);

    ///
    /// @brief 更新按钮状态
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年2月2日，新建函数
    ///
    void UpdateBtnState();

    ///
    /// @brief 设置全选中与否
    ///
    /// @param[in]  enable  true:全选 false:全不选
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年6月15日，新建函数
    ///
    void SetCheckBoxStatus(bool enable);

    ///
    /// @brief 调整项目条目的位置信息
    ///
    /// @param[in]  name        项目名称
    /// @param[in]  postion     位置信息  
    ///
    /// @return true表示调整成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年2月6日，新建函数
    ///
    bool AdjustItemPostion(QString name, int postion);

	void ResetData();

private:
    Ui::QHeadSortDisplay*           ui;
    DisplaySet                      m_data;                     ///< 当前数据
    QStandardItemModel*				m_prePareItemModel;			///< 备选项目列表模式
    QStandardItemModel*				m_selectedModel;			///< 已选项目列表模式
    QStandardItemModel*				m_mainTabelModel;			///< 表头显示排序
    int                             m_selectedNum;              ///< 选中的项目数目
    std::map<DisplayType, DisplaySet> m_dataMap;                ///< 数据地图
    QCheckBox*                      m_selectAllButton;          ///< 全选Button
};
