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
/// @file     TPPPushButton.h
/// @brief    带下拉框的按钮
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
#include "TPPPushButton.h"
#include <QMouseEvent>
#include <QLabel>

///
/// @brief
///     构造函数
///
/// @param[in]  parent  父控件
///
/// @par History:
/// @li 5774/WuHongTao，2020年9月23日，新建函数
///
TPPPushButton::TPPPushButton(QWidget *parent) :
    QPushButton(parent)
{
    ui.setupUi(this);
    // 保存父控件指针以备使用
    pparent = parent;
    status = NORMAL;
    mouse_press = false;
    // 初始化表头
    newCombox.headList.clear();
    // 初始化内容
    newCombox.ContentList.clear();
    // 添加显示label到列表中
    m_LableList.push_back(ui.calilot);
    m_LableList.push_back(ui.time);
    m_LableList.push_back(ui.reagentLot);
    m_LableList.push_back(ui.method);
    // 初始化曲线的列表
    pCurveTable = new ComboxTableDlg(this);
    pCurveTable->hide();
    // 连接行选中事件
    connect(pCurveTable, SIGNAL(ComboxSelected(int)), this, SLOT(onSelectedRow(int)));
}

///
/// @brief
///     析构函数
///
/// @par History:
/// @li 5774/WuHongTao，2020年9月23日，新建函数
///
TPPPushButton::~TPPPushButton()
{
}

///
/// @brief
///     给按钮设置图片
///
/// @param[in]  pic_name  
///
/// @return 
///
/// @par History:
/// @li 5774/WuHongTao，2020年9月23日，新建函数
///
void TPPPushButton::setPicName(QString pic_name)
{
    this->pic_name = pic_name;
    setFixedSize(QPixmap(pic_name).size());
}

///
/// @brief
///     设置表的信息
///
/// @param[in]  dataCombox  
///
/// @return 
///
/// @par History:
/// @li 5774/WuHongTao，2020年9月23日，新建函数
///
void TPPPushButton::setComboxInfo(ComboxInfo& dataCombox)
{
    newCombox = dataCombox;
    // 设置表头信息
    pCurveTable->SetHeaderList(dataCombox.headList);

    // 获取第一组数据的信息，用于按钮信息的显示
    QString BtnText;
    QStringList convertTool;
    convertTool << " " << " " << " ";
    // 加入两个换行来显示按钮
    BtnText = convertTool.join("\n\n");

    // 设置下拉列表数据
    pCurveTable->SetContentList(dataCombox.ContentList, -1);
}

///
/// @brief 清空标签中的信息
///
///
/// @return 
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月1日，新建函数
///
void TPPPushButton::ClearLabelsInfo()
{
    for (const auto& lable : m_LableList)
    {
        lable->setText("");
    }
}

void TPPPushButton::SetCurrentCurveNo(int row)
{
    pCurveTable->SetCurCurveNo(row);
}

///
/// @brief
///     进入事件
///
/// @param[in]  *  具体事件句柄
///
/// @par History:
/// @li 5774/WuHongTao，2020年9月23日，新建函数
///
void TPPPushButton::enterEvent(QEvent *)
{
    status = ENTER;
    update();
}

///
/// @brief
///     离开事件
///
/// @param[in]  *  具体事件句柄
///
/// @par History:
/// @li 5774/WuHongTao，2020年9月23日，新建函数
///
void TPPPushButton::leaveEvent(QEvent *)
{
    status = NORMAL;
    update();
}

///
/// @brief
///     鼠标按下的事件
///
/// @param[in]  event  具体事件句柄
///
/// @par History:
/// @li 5774/WuHongTao，2020年9月23日，新建函数
///
void TPPPushButton::mousePressEvent(QMouseEvent *event)
{
    // 若点击鼠标左键
    if (event->button() == Qt::LeftButton)
    {
        mouse_press = true;
        status = PRESS;
        update();
    }
}

///
/// @brief
///     事件释放
///
/// @param[in]  event  具体事件句柄
///
/// @par History:
/// @li 5774/WuHongTao，2020年9月23日，新建函数
///
void TPPPushButton::mouseReleaseEvent(QMouseEvent *event)
{
    // 若点击鼠标左键
    if (mouse_press && this->rect().contains(event->pos()))
    {
        mouse_press = false;
        status = ENTER;
        update();
        emit clicked();

        // 获取坐标信息
        QPoint pCurveGlobal = pparent->mapToGlobal(pos());
        // 移动到具体的位置
        pCurveTable->move(pCurveGlobal.x() - 6, pCurveGlobal.y() + height() + 6);
        pCurveTable->setGeometry(pCurveGlobal.x() - 6, pCurveGlobal.y() + height() + 6, width() + 5, pCurveTable->height());

        // 如果表头是空，直接返回
        if (newCombox.headList.isEmpty())
        {
            return;
        }

        // 设置内容链表信息
        //pCurveTable->SetContentList(newCombox.ContentList, -1);
        pCurveTable->show();
    }
}

///
/// @brief
///     行选中处理函数
///
/// @param[in]  index  行数
///
/// @par History:
/// @li 5774/WuHongTao，2020年9月23日，新建函数
///
void TPPPushButton::onSelectedRow(int index)
{
    QStringList convertTool;
    QString BtnText;

    int maxIndex = newCombox.ContentList.size() / newCombox.headList.size();

    // 如果大于最大行，说明存在问题直接返回
    if (maxIndex < index)
    {
        emit BComboxSelected(-1);
        return;
    }

    int startPos = index * newCombox.headList.size();
    for (int i = startPos; i < (startPos + newCombox.headList.size()); i++)
    {
        // 放入到列表中用于合成按钮的显示字符串
        convertTool << newCombox.ContentList[i];
    }

    int i = 0;
    for (auto msg : convertTool)
    {
        if (i >= m_LableList.size())
        {
            return;
        }
        //m_LableList[i++]->setText(msg);
    }

    // 向外部发送消息
    emit BComboxSelected(index);
}
