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
/// @file     imaddreactcupdlg.cpp
/// @brief    免疫添加反应杯对话框
///
/// @author   4170/TangChuXian
/// @date     2022年9月27日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月27日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "imaddreactcupdlg.h"
#include "ui_imaddreactcupdlg.h"
#include "src/common/Mlog/mlog.h"
#include <QTimer>

#define SCAN_TIMER_INTERVAL                        (100)                // 扫码定时器间隔500(ms)
#define SCAN_TIMER_MAX_OVER_TIME_COUNT             (5)                  // 扫码定时器最大超时次数

ImAddReactCupDlg::ImAddReactCupDlg(QWidget *parent)
    : BaseDlg(parent),
      m_bInit(false),
      m_iOverTimeCnt(SCAN_TIMER_MAX_OVER_TIME_COUNT)
{
    ui = new Ui::ImAddReactCupDlg();
    ui->setupUi(this);

    // 显示前初始化
    InitBeforeShow();
}

ImAddReactCupDlg::~ImAddReactCupDlg()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
    delete ui;
}

///
/// @brief
///     设置扫描到的耗材数据
///
/// @param[in]  varSplData  
///
/// @return 
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月23日，新建函数
///
void ImAddReactCupDlg::SetScanSplData(const QVariant& varSplData)
{
    m_varSplData = varSplData;
}

///
/// @brief
///     获取扫描到的耗材数据
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月23日，新建函数
///
const QVariant& ImAddReactCupDlg::GetScanSplData()
{
    return m_varSplData;
}

///
/// @brief
///     显示之前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月27日，新建函数
///
void ImAddReactCupDlg::InitBeforeShow()
{
    // 保存按钮和关闭按钮无焦点，保存按钮禁能
    ui->OkBtn->setEnabled(false);
    ui->OkBtn->setFocusPolicy(Qt::NoFocus);
    ui->CancelBtn->setFocusPolicy(Qt::NoFocus);
    ui->ScanRltTipLab->clear();
}

///
/// @brief
///     显示之后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月27日，新建函数
///
void ImAddReactCupDlg::InitAfterShow()
{
    // 初始化子控件
    InitChildCtrl();

    // 初始化信号槽连接
    InitConnect();
}

///
/// @brief
///     初始化信号槽连接
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月23日，新建函数
///
void ImAddReactCupDlg::InitConnect()
{
    // 连接信号槽
    connect(ui->OkBtn, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->CancelBtn, SIGNAL(clicked()), this, SLOT(reject()));

    // 条码编辑框文本改变
    connect(ui->BarcodeInputEdit, SIGNAL(textChanged(const QString)), this, SLOT(OnBarcodeChanged()));

    // 定时器触发
    connect(m_pScanTimer, SIGNAL(timeout()), this, SLOT(OnScanTimer()));
}

///
/// @brief
///     初始化子控件
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月23日，新建函数
///
void ImAddReactCupDlg::InitChildCtrl()
{
    // 设置标题
    SetTitleName(tr("添加反应杯"));

    // 初始化扫码定时器
    m_pScanTimer = new QTimer(this);
    m_pScanTimer->setInterval(SCAN_TIMER_INTERVAL);

    // 条码输入框禁用输入法
    ui->BarcodeInputEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
}

///
/// @brief
///     重写显示事件
///
/// @param[in]  event  事件
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月27日，新建函数
///
void ImAddReactCupDlg::showEvent(QShowEvent* event)
{
    // 基类处理
    QWidget::showEvent(event);

    // 如果是第一次显示则初始化
    if (!m_bInit)
    {
        // 显示后初始化
        InitAfterShow();
        m_bInit = true;
    }
}

///
/// @brief
///     扫码定时器触发
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月23日，新建函数
///
void ImAddReactCupDlg::OnScanTimer()
{
    // 倒计时结束仍未检测到输入
    if ((--m_iOverTimeCnt) <= 0)
    {
        m_pScanTimer->stop();
        emit SigBarcodeScanFinished();
        ui->BarcodeInputEdit->clear();
        ui->CancelBtn->setEnabled(true);
    }
}

///
/// @brief
///     条码改变（表明正在输入条码）
///
/// @par History:
/// @li 4170/TangChuXian，2022年12月23日，新建函数
///
void ImAddReactCupDlg::OnBarcodeChanged()
{
    // 重置超时次数
    m_iOverTimeCnt = SCAN_TIMER_MAX_OVER_TIME_COUNT;

    // 如果定时器未开启，则代表条码输入第一个字符
    if (!m_pScanTimer->isActive() && !ui->BarcodeInputEdit->text().isEmpty())
    {
        // 开启定时器
        m_pScanTimer->start();
        ui->CancelBtn->setEnabled(false);
    }
}
