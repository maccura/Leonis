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
/// @file     modifypassworddlg.cpp
/// @brief    质控登记对话框
///
/// @author   4170/TangChuXian
/// @date     2023年7月12日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月12日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "modifypassworddlg.h"
#include "ui_modifypassworddlg.h"
#include "src/common/Mlog/mlog.h"
#include <QApplication>
#include <QKeyEvent>

ModifyPasswordDlg::ModifyPasswordDlg(QWidget *parent)
    : BaseDlg(parent),
      m_bInit(false)
{
    // 初始化ui对象
    ui = new Ui::ModifyPasswordDlg();
    ui->setupUi(this);

    // 显示前初始化
    InitBeforeShow();
}

ModifyPasswordDlg::~ModifyPasswordDlg()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
    delete ui;
}

///
/// @brief
///     清空内容
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月13日，新建函数
///
void ModifyPasswordDlg::Clear()
{
    // 清空内容
    ui->ModPageOldkeyEdit->clear();
    ui->ModPageNewkeyEdit->clear();
    ui->ModPageNewkeySureEdit->clear();
    ui->ErrTipLab->clear();
    ui->ModPageOldkeyEdit->setFocus();
}

///
/// @brief
///     界面显示前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月12日，新建函数
///
void ModifyPasswordDlg::InitBeforeShow()
{
    // 设置标题
    SetTitleName(tr("修改密码"));
    ui->ErrTipLab->clear();

    // 安装事件过滤器
    ui->OkBtn->setDefault(true);
    ui->CancelBtn->setFocusPolicy(Qt::NoFocus);
}

///
/// @brief
///     界面显示后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月12日，新建函数
///
void ModifyPasswordDlg::InitAfterShow()
{
    // 初始化字符串资源
    InitStrResource();

    // 初始化信号槽连接
    InitConnect();

    // 初始化子控件
    InitChildCtrl();
}

///
/// @brief
///     初始化字符串资源
///
/// @par History:
/// @li 4170/TangChuXian，2022年1月17日，新建函数
///
void ModifyPasswordDlg::InitStrResource()
{

}

///
/// @brief
///     初始化信号槽连接
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月12日，新建函数
///
void ModifyPasswordDlg::InitConnect()
{
    // 连接信号槽
    // 取消按钮被点击
    connect(ui->CancelBtn, SIGNAL(clicked()), this, SLOT(reject()));

    // 用户编辑时清空错误提示
    connect(ui->ModPageOldkeyEdit, SIGNAL(textEdited(const QString&)), ui->ErrTipLab, SLOT(clear()));
    connect(ui->ModPageNewkeyEdit, SIGNAL(textEdited(const QString&)), ui->ErrTipLab, SLOT(clear()));
    connect(ui->ModPageNewkeySureEdit, SIGNAL(textEdited(const QString&)), ui->ErrTipLab, SLOT(clear()));

    // 用户编辑时更新确定按钮使能禁能
    connect(ui->ModPageOldkeyEdit, SIGNAL(textEdited(const QString&)), this, SLOT(UpdateOkBtnEnabled()));
    connect(ui->ModPageNewkeyEdit, SIGNAL(textEdited(const QString&)), this, SLOT(UpdateOkBtnEnabled()));
    connect(ui->ModPageNewkeySureEdit, SIGNAL(textEdited(const QString&)), this, SLOT(UpdateOkBtnEnabled()));
}

///
/// @brief
///     初始化子控件
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月12日，新建函数
///
void ModifyPasswordDlg::InitChildCtrl()
{
}

///
/// @brief
///     窗口显示事件
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月12日，新建函数
///
void ModifyPasswordDlg::showEvent(QShowEvent *event)
{
    // 让基类处理事件
    BaseDlg::showEvent(event);

    // 第一次显示时初始化
    if (!m_bInit)
    {
        m_bInit = true;
        InitAfterShow();
    }

    // 显示时清空编辑内容
    Clear();

    // 更新确定按钮使能状态
    UpdateOkBtnEnabled();
}

///
/// @brief
///     键盘按下事件
///
/// @param[in]  event  键盘事件
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月29日，新建函数
///
void ModifyPasswordDlg::keyPressEvent(QKeyEvent *event)
{
    // 判断是不是【Enter】键
    Qt::Key enKey = (Qt::Key)event->key();
    if (event->key() != Qt::Key_Enter && event->key() != Qt::Key_Return)
    {
        // 基类处理
        BaseDlg::keyPressEvent(event);
        return;
    }

    // 接受
    ui->OkBtn->clicked();
}

///
/// @brief
///     更新确定按钮是否使能
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月8日，新建函数
///
void ModifyPasswordDlg::UpdateOkBtnEnabled()
{
    // 如果有编辑框为空，则确定按钮置灰
    if (ui->ModPageOldkeyEdit->text().isEmpty() ||
        ui->ModPageNewkeyEdit->text().isEmpty() ||
        ui->ModPageNewkeySureEdit->text().isEmpty())
    {
        ui->OkBtn->setEnabled(false);
        return;
    }

    // 否则按钮使能
    ui->OkBtn->setEnabled(true);
}
