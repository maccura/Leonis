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
/// @file     comboxtabledlg.h
/// @brief    下拉框的控件
///
/// @author   5774/WuHongTao
/// @date     2020年9月23日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2020年9月23日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "comboxtabledlg.h"
#include "ui_comboxtabledlg.h"
#include <QVector>
#include <QScreen>
#include "QGraphicsDropShadowEffect"
#include "shared/CReadOnlyDelegate.h"
#include "src/common/Mlog/mlog.h"
const int iRowHeight = 42;

///
/// @brief
///     构造函数
///
/// @param[in]  parent  父控件的句柄
///
/// @par History:
/// @li 5774/WuHongTao，2020年9月23日，新建函数
///
ComboxTableDlg::ComboxTableDlg(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::ComboxTableDlg),
    m_curCurveRowInx(-1)
{
    ui->setupUi(this);
    // 设置无边框，等属性
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    widgetShadow = new QGraphicsDropShadowEffect(ui->comboxTable);
    widgetShadow->setOffset(0);
    // 阴影颜色
    widgetShadow->setColor(QColor(38, 78, 119, 255));
    // 阴影圆角
    widgetShadow->setBlurRadius(8);
    ui->comboxTable->setGraphicsEffect(widgetShadow);

    // 单元格选中回调处理
    connect(ui->comboxTable, SIGNAL(cellClicked(int, int)), this, SLOT(tabSelectedChange(int, int)));
    // 单元格进入的回调处理
    //connect(ui->comboxTable, SIGNAL(cellEntered(int, int)), this, SLOT(myCellEntered(int, int)));
}

///
/// @brief
///     析构函数
///
/// @par History:
/// @li 5774/WuHongTao，2020年9月23日，新建函数
///
ComboxTableDlg::~ComboxTableDlg()
{
}

///
/// @brief
///     设置链表头的属性
///
/// @param[in]  listHead  链表名字的字符串列表
///
/// @par History:
/// @li 5774/WuHongTao，2020年9月23日，新建函数
///
void ComboxTableDlg::SetHeaderList(const QStringList& listHead)
{
    // 根据大小设置列数
    m_selectedHisRow = -1;
    ui->comboxTable->setColumnCount(listHead.count());
    ui->comboxTable->setHorizontalHeaderLabels(listHead);
    // 设置点击的时候表头字体不加粗
    ui->comboxTable->horizontalHeader()->setHighlightSections(false);
    // 单行选择
    ui->comboxTable->setSelectionMode(QAbstractItemView::SingleSelection);
    // 行选中
    ui->comboxTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->comboxTable->setMouseTracking(true);
    ui->comboxTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->comboxTable->setShowGrid(false);
    ui->comboxTable->setFocusPolicy(Qt::NoFocus);
    ui->comboxTable->horizontalHeader()->setStyleSheet("QHeaderView::section::horizontal {background-color: #e5f2ff;}");
    ui->comboxTable->horizontalHeaderItem(0)->setForeground(QColor::fromRgb(0x02, 0x5b, 0xc7));
    ui->comboxTable->horizontalHeaderItem(1)->setForeground(QColor::fromRgb(0x02, 0x5b, 0xc7));
    ui->comboxTable->horizontalHeaderItem(2)->setForeground(QColor::fromRgb(0x02, 0x5b, 0xc7));

    // 设置每一列的大小
    QVector<int>vTableWidth{ 130,190/*,130*/ };
    int minHeaderSize = vTableWidth.size() < listHead.size() ? vTableWidth.size() : listHead.size();
    for (int i = 0; i < minHeaderSize/* - 1*/; i++)
    {
        ui->comboxTable->setColumnWidth(i, vTableWidth[i]);
        ui->comboxTable->setItemDelegateForColumn(i, new CReadOnlyDelegate(this));
    }
    // 设置代理
    for (int i = 0; i < listHead.size(); i++)
    {
        ui->comboxTable->setItemDelegateForColumn(i, new CReadOnlyDelegate(this));
    }
    //ui->comboxTable->horizontalHeader()->setSectionResizeMode(listHead.size() - 1, QHeaderView::Stretch);
    ui->comboxTable->horizontalHeader()->setStretchLastSection(true);

    ui->comboxTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->comboxTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

///
/// @brief
///     设置下拉表的内容
///
/// @param[in]  listContent  具体内容列表
/// @param[in]  index  控制背景色的控制变量
///
/// @par History:
/// @li 5774/WuHongTao，2020年9月23日，新建函数
///
void ComboxTableDlg::SetContentList(const QStringList& listContent, int index)
{
    // 获取列数目
    int iColumnCnt = ui->comboxTable->columnCount();
    // 总元素个数/每行多少个 = 多少行
    int iRowCount = listContent.count() / iColumnCnt;

    // 如果不足一行，则返回
    if (iRowCount == 0)
    {
        // 返回之前，先把数据清空
        ui->comboxTable->clearContents();
        return;
    }

    // 如果不等于0，则需要增加一行
    if (listContent.count() % iColumnCnt != 0)
    {
        iRowCount++;
    }

    // 设置行数
    ui->comboxTable->clearContents();
    ui->comboxTable->setRowCount(iRowCount);
    // 根据行数和行高计算出高度(需求最多显示10行数据）
    int iHeight = ((iRowCount > 10 ? 10 : iRowCount) + 1)*iRowHeight;

	// 判断计算的高度有没超过屏幕范围，否则会造成Table最下面的数据不能显示和选择
	if (qGuiApp->primaryScreen()->geometry().y() + qGuiApp->primaryScreen()->geometry().height() < this->geometry().y() + iHeight)
	{
		// 计算实际可用高度
		iHeight = qGuiApp->primaryScreen()->geometry().y() + qGuiApp->primaryScreen()->geometry().height() - this->geometry().y();

		// 按照iRowHeight 进行一下剪裁大小
		// 多减去一个iRowHeight 可以TableWidget控件边界清晰，不拥挤
		iHeight = iHeight - (iHeight % iRowHeight) - iRowHeight;
	}

    this->setFixedHeight(iHeight + 16);
    //ui->comboxBk->setFixedHeight(iHeight + 16);
    ui->comboxTable->setFixedHeight(iHeight);
    int lisesize = listContent.size();
    QFont font = ui->comboxTable->font();
    //根据内容来设置表格
    for (int i = 0; i < iRowCount; i++)
    {
        ui->comboxTable->setRowHeight(i, iRowHeight);
        for (int j = 0; j < iColumnCnt; j++)
        {
            // 越界检查
            int listPos = i*iColumnCnt + j;
            if (listPos >= lisesize)
            {
                ULOG(LOG_ERROR, "listPos is bigger than itemContent's size!");
                break;
                break;
            }

            QTableWidgetItem *itemContent = new QTableWidgetItem();
            if (m_curCurveRowInx == i)
                itemContent->setForeground(Qt::gray);
            
            itemContent->setText(listContent.at(listPos));
            itemContent->setFont(font);
            ui->comboxTable->setItem(i, j, itemContent);
            ui->comboxTable->item(i, j)->setTextAlignment(Qt::AlignCenter);
        }
    }

    // 背景色的设置
    if (index != -1)
    {
        for (int i = 0; i < iColumnCnt; i++)
        {
            ui->comboxTable->item(index, i)->setBackgroundColor(QColor("#e5f2ff"));
        }
    }

}

void ComboxTableDlg::SetCurCurveNo(int index)
{
    m_curCurveRowInx = index;
}

///
/// @brief
///     
///
/// @param[in]  iRow  点击的行数
/// @param[in]  iColmn  和列数
///
/// @return 
///
/// @par History:
/// @li 5774/WuHongTao，2020年9月23日，新建函数
///
void ComboxTableDlg::tabSelectedChange(int iRow, int iColmn)
{
    // 如果选中的是当前曲线，则不处理
    if (m_curCurveRowInx == iRow 
        || m_selectedHisRow == iRow)
    {
        ui->comboxTable->clearSelection();
        return;
    }

    QList<QTableWidgetItem*>items = ui->comboxTable->selectedItems();
    if (items.count() > 0)
    {
        // 清除原有颜色
        if (m_selectedHisRow >= 0)
        {
            for (int i = 0; i < ui->comboxTable->columnCount(); ++i)
            {
                auto item = ui->comboxTable->item(m_selectedHisRow, i);
                if (item)
                    item->setData(Qt::UserRole + 5, QVariant());
            }
        }

        // 更新历史数据选择项所在行
        m_selectedHisRow = iRow;
        for (int i = 0; i < ui->comboxTable->columnCount(); ++i)
        {
            auto item = ui->comboxTable->item(iRow, i);
            if (item)
                item->setData(Qt::UserRole + 5, QColor("#f0f0f0"));
        }
        // 清理选区，选区会造成表格背景高亮颜色与需求不一致
        ui->comboxTable->clearSelection();

        // 发出行选中的信号
        emit ComboxSelected(iRow);
        hide();
    }
}

///
/// @brief
///     进入某单元格的按钮
///
/// @param[in]  iRow  行数
/// @param[in]  iColmn  列数
///
/// @par History:
/// @li 5774/WuHongTao，2020年9月23日，新建函数
///
void ComboxTableDlg::myCellEntered(int iRow, int iColmn)
{
    ui->comboxTable->selectRow(iRow);
}

///
/// @brief
///     鼠标按下的事件处理函数
///
/// @param[in]  event  具体事件
///
/// @par History:
/// @li 5774/WuHongTao，2020年9月23日，新建函数
///
void ComboxTableDlg::mousePressEvent(QMouseEvent *event)
{
    setAttribute(Qt::WA_NoMouseReplay);
    QWidget::mousePressEvent(event);
}
