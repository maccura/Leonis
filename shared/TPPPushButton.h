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
#pragma once
#include <QPushButton>
#include "shared/comboxtabledlg.h"
#include "ui_TPPPushButton.h"

class QMouseEvent;
class QLabel;


// 自定义combox的信息
struct ComboxInfo
{
    // 表头的信息
    QStringList headList;
    // 内容信息
    QStringList ContentList;
};

class TPPPushButton :
    public QPushButton
{
    Q_OBJECT
public:
    ///
    /// @brief
    ///     构造函数
    ///
    /// @param[in]  parent  父控件
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年9月23日，新建函数
    ///
    explicit TPPPushButton(QWidget *parent = 0);

    ///
    /// @brief
    ///     析构函数
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年9月23日，新建函数
    ///
    ~TPPPushButton();

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
    void setPicName(QString pic_name);

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
    void setComboxInfo(ComboxInfo& dataCombox);

    ///
    /// @brief 清空标签中的信息
    ///
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月1日，新建函数
    ///
    void ClearLabelsInfo();

    ///
    /// @brief 设置当前曲线所在表格行
    ///
    /// @param[in]  row 表格中的行号  
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月1日，新建函数
    ///
    void SetCurrentCurveNo(int row);

signals:
    ///
    /// @brief
    ///     选中信号
    ///
    /// @param[in]  index  信号的参数，具体选中行数
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年9月23日，新建函数
    ///
    void BComboxSelected(int index);

protected:
    ///
    /// @brief
    ///     进入事件
    ///
    /// @param[in]  *  具体事件句柄
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年9月23日，新建函数
    ///
    void enterEvent(QEvent *);

    ///
    /// @brief
    ///     离开事件
    ///
    /// @param[in]  *  具体事件句柄
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年9月23日，新建函数
    ///
    void leaveEvent(QEvent *);

    ///
    /// @brief
    ///     鼠标按下的事件
    ///
    /// @param[in]  event  具体事件句柄
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年9月23日，新建函数
    ///
    void mousePressEvent(QMouseEvent *event);

    ///
    /// @brief
    ///     事件释放
    ///
    /// @param[in]  event  具体事件句柄
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年9月23日，新建函数
    ///
    void mouseReleaseEvent(QMouseEvent *event);

private slots:
    ///
    /// @brief
    ///     行选中处理函数
    ///
    /// @param[in]  index  行数
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年9月23日，新建函数
    ///
    void onSelectedRow(int index);

private:
    // 枚举按钮的几种状态
    enum ButtonStatus { NORMAL, ENTER, PRESS, NOSTATUS };
    ButtonStatus                    status;
    QString                         pic_name;

    int                             btn_width;                  //< 按钮宽度
    int                             btn_height;                 //< 按钮高度
    bool                            mouse_press;                //< 按钮左键是否按下
    ComboxTableDlg*                 pCurveTable;                //< 对比校准曲线的表
    QWidget*                        pparent;                    //< 父控件指针
    ComboxInfo                      newCombox;                  //< 新类型combox控件的对象
    Ui::Form                        ui;
    std::vector<QLabel*>            m_LableList;                //< 显示控件
};
