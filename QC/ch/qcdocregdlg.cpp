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
/// @file     qcdocregdlg.cpp
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

#include "qcdocregdlg.h"
#include "ui_qcdocregdlg.h"
#include "src/common/Mlog/mlog.h"

QcDocRegDlg::QcDocRegDlg(QWidget *parent)
    : BaseDlg(parent),
      m_bInit(false),
      m_bModifyQcMode(false),
      m_ciModifyStrechHeight(50)
{
    // 初始化ui对象
    ui = new Ui::QcDocRegDlg();
    ui->setupUi(this);

    // 显示前初始化
    InitBeforeShow();
}

QcDocRegDlg::~QcDocRegDlg()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
    delete ui;
}

///
/// @brief
///     设置内容页列表
///
/// @param[in]  wgtList  窗口列表
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月12日，新建函数
///
void QcDocRegDlg::SetContentPageList(const QList<QWidget*>& wgtList)
{
    // 移除之前的界面
    for (int i = 0; i < ui->QcRegStkWgt->count(); i++)
    {
        QWidget* pWgt = ui->QcRegStkWgt->widget(i);
        disconnect(pWgt, SIGNAL(SigClosed()), this, SLOT(close()));
        ui->QcRegStkWgt->removeWidget(pWgt);
    }

    // 逐个添加界面
    for (const auto& pWgt : wgtList)
    {
        // 指针检查
        if (pWgt == Q_NULLPTR)
        {
            continue;
        }

        // 添加界面
        ui->QcRegStkWgt->addWidget(pWgt);

        // 监听关闭信号
        connect(pWgt, SIGNAL(SigClosed()), this, SLOT(close()));
    }
}

///
/// @brief
///     设置当前页
///
/// @param[in]  pWidget  界面
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月12日，新建函数
///
void QcDocRegDlg::SetCurrentPage(QWidget* pWidget)
{
    // 切换到对应页
    ui->QcRegStkWgt->setCurrentWidget(pWidget);
}

///
/// @brief
///     设置修改质控模式
///
/// @param[in]  bModifyQcMode  修改质控模式
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月12日，新建函数
///
void QcDocRegDlg::SetModifyQcMode(bool bModifyQcMode)
{
    if (m_bModifyQcMode == bModifyQcMode)
    {
        // 模式未变不需操作
        return;
    }

    // 更新模式
    m_bModifyQcMode = bModifyQcMode;

    // 获取高度
    int iHeight = ui->TopFrame->height();
    ui->TopFrame->setVisible(!m_bModifyQcMode);
    if (m_bModifyQcMode)
    {
        ui->QcRegStkWgt->move(ui->QcRegStkWgt->x(), ui->QcRegStkWgt->y() - m_ciModifyStrechHeight);
        this->setFixedHeight(this->height() - m_ciModifyStrechHeight);
        SetTitleName(tr("修改"));
    }
    else
    {
        ui->QcRegStkWgt->move(ui->QcRegStkWgt->x(), ui->QcRegStkWgt->y() + m_ciModifyStrechHeight);
        this->setFixedHeight(this->height() + m_ciModifyStrechHeight);
        SetTitleName(tr("登记"));
    }
}

///
/// @brief
///     设置忽略试剂图标是否显示
///
/// @param[in]  bVisible  是否显示
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月31日，新建函数
///
void QcDocRegDlg::SetIgnRgntIconVisible(bool bVisible)
{
    ui->label_symbol->setVisible(bVisible);
    ui->label_symbolName->setVisible(bVisible);
}

///
/// @brief
///     界面显示前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月12日，新建函数
///
void QcDocRegDlg::InitBeforeShow()
{
    SetTitleName(tr("登记"));
}

///
/// @brief
///     界面显示后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月12日，新建函数
///
void QcDocRegDlg::InitAfterShow()
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
void QcDocRegDlg::InitStrResource()
{

}

///
/// @brief
///     初始化信号槽连接
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月12日，新建函数
///
void QcDocRegDlg::InitConnect()
{
    // 扫描登记单选框被点击
    connect(ui->ScanRegRBtn, SIGNAL(clicked()), this, SIGNAL(SigSwitchScanRegPage()));

    // 扫描登记单选框被点击
    connect(ui->ManualRegRBtn, SIGNAL(clicked()), this, SIGNAL(SigSwitchManualRegPage()));

    // 扫描登记单选框被点击
    connect(ui->DownloadRegRBtn, SIGNAL(clicked()), this, SIGNAL(SigSwitchQcDownloadPage()));
}

///
/// @brief
///     初始化子控件
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月12日，新建函数
///
void QcDocRegDlg::InitChildCtrl()
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
void QcDocRegDlg::showEvent(QShowEvent *event)
{
    // 让基类处理事件
    BaseDlg::showEvent(event);

    // 第一次显示时初始化
    if (!m_bInit)
    {
        m_bInit = true;
        InitAfterShow();
    }

    // 显示时自动选中扫描登记单选框
    ui->ScanRegRBtn->setChecked(true);
}
