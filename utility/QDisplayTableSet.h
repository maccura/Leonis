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
#include "src/public/ConfigSerialize.h"

class QCheckBox;
namespace Ui {
    class QDisplayTableSet;
};
class QStandardItemModel;
class QPainter;


class QSelectDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    QSelectDelegate(QObject *parent);
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
        const QModelIndex & index) const;

Q_SIGNALS:
    void closeDelegate(QString name);
};

class QDisplayTableSet : public QWidget
{
    Q_OBJECT

public:
    QDisplayTableSet(QWidget *parent = Q_NULLPTR);
    ~QDisplayTableSet();

    ///
    /// @brief 设置表类型
    ///
    /// @param[in]  data  设置数据
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年2月2日，新建函数
    ///
    void SetTableData(const DisplaySet& data);

    void showEvent(QShowEvent *event);
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
    void OnCheckBoxClicked(int state);

    ///
    /// @brief 保存数据
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年2月6日，新建函数
    ///
    void OnSaveData();

    ///
    /// @brief 数据字典 数据项更新
    ///
    /// @param[in]  dsSet  更新后的数据
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年8月3日，新建函数
    ///
    void OnDataDictUpdated(DisplaySet dsSet);

    ///
    /// @brief 当全选被点击
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年8月15日，新建函数
    ///
    void OnSelecAllClicked();

protected:
    // 界面隐藏事件
    virtual void hideEvent(QHideEvent *event) override;

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
    /// @brief 更新按钮状态
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年2月2日，新建函数
    ///
    void UpdateBtnState();

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

    ///
    /// @brief 获取指定行对应的checkbox控件
    ///
    /// @param[in]  row  指定行
    ///
    /// @return 复选框对象（可能为 nullptr）
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年8月4日，新建函数
    ///
    QCheckBox* GetRowCheckBox(int row);

    ///
    /// @brief 更新全选的选择状态
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年8月15日，新建函数
    ///
    void UpdateSelectAllState();

    ///
    /// @brief 获取默认设置的顺序（重置功能使用）
    ///
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年10月26日，新建函数
    ///
    DisplaySet GetDefaultData();

    ///
    /// @brief 获取选择表的当前行
    ///
    ///
    /// @return 选中行
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2024年6月13日，新建函数
    ///
    QModelIndex GetSelectViewCurrentIndex();

private:
    Ui::QDisplayTableSet*            ui;
    DisplaySet                      m_oriData;                  ///< 未被修改过的原始数据
    DisplaySet                      m_data;                     ///< 当前数据
    QStandardItemModel*				m_beSelectModel;			///< 备选项目列表模式
    QStandardItemModel*				m_selectedModel;			///< 已选项目列表模式
    int                             m_selectedNum;              ///< 选中的项目数目
    QCheckBox*                      m_checkBoxAll;              ///< 全选 选择框
    bool                            m_bIsUpdateSelectAll;       ///< 是否正在执行全选，或者反向更新全选状态
};
