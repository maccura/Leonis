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

#include "qcprintsetdlg.h"
#include "ui_qcprintsetdlg.h"
#include "src/common/Mlog/mlog.h"
#include "shared/uicommon.h"
#include <QDate>

QcPrintSetDlg::QcPrintSetDlg(QWidget *parent)
    : BaseDlg(parent)
    , m_bInit(false)
{
    ui = new Ui::QcPrintSetDlg();
    ui->setupUi(this);

    SetTitleName(tr("打印"));
    InitStrResource();
}

QcPrintSetDlg::~QcPrintSetDlg()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
    delete ui;
}

///
/// @brief 窗口显示事件
///     
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 6889/chenwei，2022年11月7日，新建函数
///
void QcPrintSetDlg::showEvent(QShowEvent *event)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 让基类处理事件
    BaseDlg::showEvent(event);

    // 第一次显示时初始化
    if (!m_bInit)
    {
        m_bInit = true;
        InitAfterShow();
    }
}

///
/// @brief 显示后初始化
///     
/// @par History:
/// @li 6889/chenwei，2022年11月7日，新建函数
///
void QcPrintSetDlg::InitAfterShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    InitConnect();
    InitChildWidget();
    ui->QcDate->setEnabled(false);
}

///
/// @brief 初始化子控件
///     
/// @par History:
/// @li 6889/chenwei，2022年11月7日，新建函数
///
void QcPrintSetDlg::InitChildWidget()
{
    // 加载项目信息
    QDate date = QDate::currentDate();
    ui->QcDate->setDate(date);
}

///
/// @brief 初始化字符资源
///     
/// @par History:
/// @li 6889/chenwei，2022年11月7日，新建函数
///
void QcPrintSetDlg::InitStrResource()
{
    ui->OkBtn->setText(tr("确定"));
    ui->CancelBtn->setText(tr("取消"));
}

///
/// @brief 初始化信号槽连接
///     
/// @par History:
/// @li 6889/chenwei，2022年11月7日，新建函数
///
void QcPrintSetDlg::InitConnect()
{
    connect(ui->OkBtn, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->CancelBtn, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->QcLJ, SIGNAL(clicked()), this, SLOT(ChoosePrintMode()));
    connect(ui->QcSingleDay, SIGNAL(clicked()), this, SLOT(ChoosePrintMode()));
}

///
/// @brief 判断打印模式
///     
/// @par History:
/// @li 6889/chenwei，2022年11月7日，新建函数
///
bool QcPrintSetDlg::isSelectPrintLJ()
{
    if (ui->QcLJ->isChecked())
    {
        return true;
    }
    else
    {
        return false;
    }
}

///
/// @brief 获取质控日期
///     
/// @par History:
/// @li 6889/chenwei，2022年11月7日，新建函数
///
QDate QcPrintSetDlg::getQcDate()
{
    return ui->QcDate->date();
}

void QcPrintSetDlg::ChoosePrintMode()
{
    if (ui->QcLJ->isChecked())
    {
        ui->QcDate->setEnabled(false);
    }
    else if (ui->QcSingleDay->isChecked())
    {
        ui->QcDate->setEnabled(true);
    }
}

