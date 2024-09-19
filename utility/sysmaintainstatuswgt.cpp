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
/// @file     sysmaintainstatuswgt.cpp
/// @brief    系统维护状态控件
///
/// @author   4170/TangChuXian
/// @date     2023年2月15日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月15日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "sysmaintainstatuswgt.h"
#include "ui_sysmaintainstatuswgt.h"
#include "src/common/Mlog/mlog.h"

#define MAINTAIN_STATUS_PROPERTY_NAME                   ("status")             // 维护状态属性名

#define MAINTAIN_STATUS_NOT_MAINTAIN                    ("not_maintain")       // 未维护
#define MAINTAIN_STATUS_MAINTAINING                     ("maintaining")        // 正在维护
#define MAINTAIN_STATUS_MAINTAIN_FINISHED               ("maintain_finished")  // 维护完成
#define MAINTAIN_STATUS_MAINTAIN_FAILED                 ("maintain_failed")    // 失败
#define MAINTAIN_STATUS_MAINTAINING_FAILED              ("maintaining_failed") // 正在维护并存在失败

SysMaintainStatusWgt::SysMaintainStatusWgt(QWidget *parent)
    : QWidget(parent),
      m_enStatus(SYS_MAINTAIN_STATUS_NOT_MAINTAIN)
{
    ui = new Ui::SysMaintainStatusWgt();
    ui->setupUi(this);

    // 显示之前初始化
    InitBeforeShow();
}

SysMaintainStatusWgt::~SysMaintainStatusWgt()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
    delete ui;
}

///
/// @brief
///     获取进度
///
/// @return 进度（分数表示）
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月15日，新建函数
///
QString SysMaintainStatusWgt::GetProgress()
{
    return ui->ProgressLab->text();
}

///
/// @brief
///     设置进度
///
/// @param[in]  dProgress  进度百分比：1表示100%，0表示0%
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月15日，新建函数
///
void SysMaintainStatusWgt::SetProgress(QString strProgress)
{
    ui->ProgressLab->setText(strProgress);
}

///
/// @brief
///     获取按钮维护状态
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月15日，新建函数
///
SysMaintainStatusWgt::SysMaintainStatus SysMaintainStatusWgt::GetStatus()
{
    return m_enStatus;
}

///
/// @brief
///     设置按钮维护状态
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月15日，新建函数
///
void SysMaintainStatusWgt::SetStatus(SysMaintainStatus enStatus)
{
    // 如果状态未改变，则返回
    if (m_enStatus == enStatus)
    {
        // 刷新界面显示
        return;
    }

    // 设置状态
    m_enStatus = enStatus;

    // 根据状态构造对应字符串（用于qss动态属性
    QString strStatusVal("");
    QString strStatusText("");
    switch (enStatus)
    {
    case SysMaintainStatusWgt::SYS_MAINTAIN_STATUS_NOT_MAINTAIN:
        strStatusVal = MAINTAIN_STATUS_NOT_MAINTAIN;
        strStatusText = tr("未维护");
        break;
    case SysMaintainStatusWgt::SYS_MAINTAIN_STATUS_MAINTAINING:
        strStatusVal = MAINTAIN_STATUS_MAINTAINING;
        strStatusText = tr("正在维护");
        break;
    case SysMaintainStatusWgt::SYS_MAINTAIN_STATUS_MAINTAIN_FINISHED:
        strStatusVal = MAINTAIN_STATUS_MAINTAIN_FINISHED;
        strStatusText = tr("维护完成");
        break;
    case SysMaintainStatusWgt::SYS_MAINTAIN_STATUS_FAILED:
        strStatusVal = MAINTAIN_STATUS_MAINTAIN_FAILED;
        strStatusText = tr("维护失败");
        break;
    case SysMaintainStatusWgt::SYS_MAINTAIN_STATUS_MAINTAINING_FAILED:
        strStatusVal = MAINTAIN_STATUS_MAINTAINING_FAILED;
        strStatusText = tr("维护失败");
        break;
    default:
        break;
    }

    // 更新动态属性
    ui->MaintainStatusLab->setProperty(MAINTAIN_STATUS_PROPERTY_NAME, strStatusVal);
    ui->BkMovieLab->setProperty(MAINTAIN_STATUS_PROPERTY_NAME, strStatusVal);

    // 更新维护状态文本
    ui->MaintainStatusLab->setText(strStatusText);

    // 刷新界面显示
    UpdateStatusDisplay();
}

///
/// @brief
///     更新状态显示
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月15日，新建函数
///
void SysMaintainStatusWgt::UpdateStatusDisplay()
{
    // 更新状态(进度)
    ui->MaintainStatusLab->style()->unpolish(ui->MaintainStatusLab);
    ui->MaintainStatusLab->style()->polish(ui->MaintainStatusLab);
    ui->MaintainStatusLab->update();

    // 根据状态更新背景
    ui->BkMovieLab->style()->unpolish(ui->BkMovieLab);
    ui->BkMovieLab->style()->polish(ui->BkMovieLab);
    ui->BkMovieLab->update();
}

///
/// @brief
///     显示前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月15日，新建函数
///
void SysMaintainStatusWgt::InitBeforeShow()
{
    // 初始化状态为未维护
    SetStatus(SYS_MAINTAIN_STATUS_NOT_MAINTAIN);
}
