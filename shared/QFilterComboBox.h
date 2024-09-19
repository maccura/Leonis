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
/// @file     QFilterComboBox.h
/// @brief    筛选组合框（有提示功能、可编辑、自动补全）
///
/// @author   8090/YeHuaNing
/// @date     2023年5月8日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8090/YeHuaNing，2023年5月8日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QComboBox>
#include <QListView>

class QCompleter;

// 重写QListView，加上提示框功能。提示当前鼠标选择项
class QTipListView : public QListView
{
    Q_OBJECT

public:
    QTipListView(QWidget* parent = nullptr);

protected:
    virtual void mouseMoveEvent(QMouseEvent *event) override;
};

// 重写QComboBox，可以让ComboBox自带补全提示功能
class QFilterComboBox : public QComboBox
{
	Q_OBJECT

public:
	QFilterComboBox(QWidget *parent = nullptr);

    void addItem(const QString &text, const QVariant &userData = QVariant());
    void addItem(const QIcon &icon, const QString &text, const QVariant &userData = QVariant());
    void addItems(const QStringList &texts);
    void insertItem(int index, const QString &text, const QVariant &userData = QVariant());
    void insertItem(int index, const QIcon &icon, const QString &text, const QVariant &userData = QVariant());
    void insertItems(int index, const QStringList &list);
    void removeItem(int index);

    QTipListView* GetTipListView();

    ///
    /// @brief 显示下拉弹出框
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年12月6日，新建函数
    ///
    virtual void showPopup() override; 

	///
	/// @brief 设置输入框最长字符（只有可输入时能使用）
	///
	/// @param[in]  length  长度
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年12月18日，新建函数
	///
	void SetMaxEditLength(const int& length);

signals:
    void ResetText();

public slots:
    void clear();

    void UpdateDownIcon();

protected:
    ///
    /// @brief 新增单个项目
    ///
    /// @param[in]  text  显示文字
    /// @param[in]  addOrIsert  插入或者新增 (true:新增 false:插入）
    /// @param[in]  index  插入索引位置
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年5月8日，新建函数
    ///
    void AddItem(const QString& text, bool addOrInsert = true, int index = -1);

    ///
    /// @brief 新增多个项目
    ///
    /// @param[in]  text  显示文字集合
    /// @param[in]  addOrIsert  插入或者新增 (true:新增 false:插入）
    /// @param[in]  index  插入开始索引位置
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年5月8日，新建函数
    ///
    void AddItems(const QStringList& texts, bool addOrInsert = true, int index = -1);

    ///
    /// @brief 删除项目
    ///
    /// @param[in]  index  删除项目的索引位置
    /// @param[in]  removeAll  是否全部删除（true:全部删除 false:删除索引位置）
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年5月8日，新建函数
    ///
    void RemoveItems(int index, bool removeAll = false);

    ///
    /// @brief 更新一次自动补全
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年5月8日，新建函数
    ///
    void UpdateCompleter();

    ///
    /// @brief 重写事件
    ///
    /// @param[in]  event  
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年12月6日，新建函数
    ///
    virtual void enterEvent(QEvent *event) override;
    virtual void leaveEvent(QEvent *event) override;


private:
    bool                            m_bMouseEntered;            /// 鼠标是否进入了控件
    QCompleter*                     m_completer;                /// 自动补全提示框
    QStringList                     m_itemLabels;               /// 选项集合
    QTipListView*                   m_tipListView;              /// 带有提示功能的列表
    QString                         m_StyleSheet;               /// 控件的styleSheet
};

// 助记符
class QNoteComboBox : public QComboBox
{
    Q_OBJECT

public:
    QNoteComboBox(QWidget *parent = nullptr);
    ~QNoteComboBox();

    virtual bool event(QEvent* event);
    void SetNoteMap(const QMap<QString, QString>& noteMap)
    {
        m_NoteMap = noteMap;
    }

private:
    QMap<QString, QString>          m_NoteMap;                  ///< 助记符地图索引
};
