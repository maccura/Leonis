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
/// @file     QFilterComboBox.cpp
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
#include "QFilterComboBox.h"
#include "src/common/Mlog/mlog.h"
#include <QToolTip>
#include <QMouseEvent>
#include <QCompleter>
#include <QLineEdit>
#include <QStyledItemDelegate>
#include <QCursor>

QFilterComboBox::QFilterComboBox(QWidget *parent) :
    QComboBox(parent)
    ,m_bMouseEntered(false)
    ,m_completer(nullptr)
    ,m_tipListView(new QTipListView(this))
{
    m_tipListView->setObjectName("TipListView");
    setView(m_tipListView);

    m_StyleSheet = "::down-arrow\n{\n"
        "image:url(:/Leonis/resource/image/icon-down-arrow.png);\n"
        "background-repeat:no-repeat;\n"
        "background-position:center;\n}\n"
        "::down-arrow:hover\n{\n"
        "image:url(:/Leonis/resource/image/icon-down-arrow-hv.png);\n"
        "background-repeat:no-repeat;\n"
        "background-position:center;\n}\n"
        "::down-arrow:disabled\n{\n"
        "image:url(:/Leonis/resource/image/icon-down-arrow-dis.png);\n"
        "background-repeat:no-repeat;\n"
        "background-position:center;\n}\n";

    // 不允许用户通过回车等按键插入输入内容到下拉框
    setInsertPolicy(QComboBox::NoInsert);

    connect(this, &QComboBox::editTextChanged, this, [this] {
        UpdateDownIcon();
    });
    
}

void QFilterComboBox::addItem(const QString & text, const QVariant & userData)
{
    AddItem(text);
    QComboBox::addItem(text, userData);
}

void QFilterComboBox::addItem(const QIcon & icon, const QString & text, const QVariant & userData)
{
    AddItem(text);
    QComboBox::addItem(icon, text, userData);
}

void QFilterComboBox::addItems(const QStringList & texts)
{
    AddItems(texts);
    QComboBox::addItems(texts);
}

void QFilterComboBox::insertItem(int index, const QString & text, const QVariant & userData)
{
    AddItem(text, false, index);
    QComboBox::insertItem(index, text, userData);
}

void QFilterComboBox::insertItem(int index, const QIcon & icon, const QString & text, const QVariant & userData)
{
    AddItem(text, false, index);
    QComboBox::insertItem(index, icon, text, userData);
}

void QFilterComboBox::insertItems(int index, const QStringList & list)
{
    AddItems(list, false, index);
    QComboBox::insertItems(index, list);
}

void QFilterComboBox::removeItem(int index)
{
    RemoveItems(index);
    QComboBox::removeItem(index);
}

void QFilterComboBox::showPopup()
{
    //{ //1.如果有文字，直接清空返回，不弹出   2.否则弹出}
	if (!(QComboBox::currentText().isEmpty()) && !(m_completer->popup()->isVisible()))
    {
		QComboBox::setCurrentIndex(-1);
        UpdateDownIcon();
        emit ResetText();
        return;
    }

    QComboBox::showPopup();
}

void QFilterComboBox::AddItem(const QString & text, bool addOrInsert, int index)
{
    if (addOrInsert)
        m_itemLabels.append(text);
    else if(index >= 0)
        m_itemLabels.insert(index, text);

    UpdateCompleter();
}

void QFilterComboBox::AddItems(const QStringList & texts, bool addOrInsert, int index)
{
    if (addOrInsert)
        m_itemLabels.append(texts);
    else if (index >= 0)
    {
        for (int i = 0; i < texts.size(); ++i)
        {
            m_itemLabels.insert(index + i, texts[i]);
        }
    }

    UpdateCompleter();
}

void QFilterComboBox::RemoveItems(int index, bool removeAll)
{
    if (removeAll)
        m_itemLabels.clear();
    else if (index >= 0)
        m_itemLabels.removeAt(index);

    UpdateCompleter();
}

void QFilterComboBox::UpdateCompleter()
{
    if (m_completer != nullptr)
    {
        delete m_completer;
        m_completer = nullptr;
    }

    m_completer = new QCompleter(m_itemLabels, this);
    QTipListView* tipList = new QTipListView;
    m_completer->setCompletionMode(QCompleter::PopupCompletion);
    m_completer->setFilterMode(Qt::MatchContains);
    // 不区分大小写-bug27324
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer->setPopup(tipList);
    // 此处重置一次样式，因为自定义样式造成了补全对话框弹出list没有使用到自定义的qss
    // 需要在setPopup函数以后再调用
    tipList->setItemDelegate(new QStyledItemDelegate);  
    tipList->setTextElideMode(Qt::ElideRight);
    // 关闭水平滚动条，保持提示框和下拉框的样式一致
    tipList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setCompleter(m_completer);
}

void QFilterComboBox::enterEvent(QEvent *event)
{
    m_bMouseEntered = true;
    UpdateDownIcon();

    QComboBox::enterEvent(event);
}

void QFilterComboBox::leaveEvent(QEvent *event)
{
    m_bMouseEntered = false;
    UpdateDownIcon();

    QComboBox::leaveEvent(event);
}

void QFilterComboBox::clear()
{
    RemoveItems(0, true);
    QComboBox::clear();
}

void QFilterComboBox::UpdateDownIcon()
{
//     const auto& localPos = this->mapFromGlobal(QCursor::pos());
//     const auto& thisRect = this->rect();
//     if (thisRect.contains(localPos))
//         m_bMouseEntered = true;
//     else
//         m_bMouseEntered = false;

    // 有内容 && entered 显示X
	if (!(QComboBox::currentText().isEmpty()) && m_bMouseEntered)
    {
        setStyleSheet(m_StyleSheet.replace("icon-down-arrow", "icon-down-cancel"));
    }
    // 否则，显示正常
    else
    {
        setStyleSheet(m_StyleSheet.replace("icon-down-cancel", "icon-down-arrow"));
    }
}

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
void QFilterComboBox::SetMaxEditLength(const int& length)
{
	if (lineEdit() != Q_NULLPTR && length >= 0)
	{
		lineEdit()->setMaxLength(length);
	}
}

QTipListView::QTipListView(QWidget* parent /*= nullptr*/)
    : QListView(parent)
{
    setMouseTracking(true);
}

void QTipListView::mouseMoveEvent(QMouseEvent * event)
{
    QModelIndex&& index = indexAt(event->pos());

    if (index.isValid())
    {
        setToolTip(index.data().toString());
        setToolTipDuration(10000);
    }
    else
    {
        setToolTip("");
        setToolTipDuration(0);
    }
}


QNoteComboBox::QNoteComboBox(QWidget *parent)
    : QComboBox(parent)
{
}

QNoteComboBox::~QNoteComboBox()
{
}

bool QNoteComboBox::event(QEvent * event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>(event);

        // 如果按了回车键，需要匹配是否存在助记符
        if (keyEvent->key() == Qt::Key_Return ||
            keyEvent->key() == Qt::Key_Enter)
        {
            const QString& curText = currentText();
            if (m_NoteMap.contains(curText))
            {
                setCurrentText(m_NoteMap[curText]);
            }
        }
    }

    return QComboBox::event(event);
}
