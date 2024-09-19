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
/// @file     devmaintaindetailitemwgt.cpp
/// @brief    设备维护状态控件
///
/// @author   4170/TangChuXian
/// @date     2023年3月8日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月8日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "devmaintaindetailitemwgt.h"
#include "ui_devmaintaindetailitemwgt.h"
#include "shared/datetimefmttool.h"
#include "src/common/Mlog/mlog.h"
#include <QMovie>

#define MAINTAIN_STATUS_PROPERTY_NAME                   ("status")             // 维护状态属性名

#define MAINTAIN_STATUS_MAINTAIN_WAIT                   ("maintain_wait")      // 等待
#define MAINTAIN_STATUS_MAINTAIN_SUCC                   ("maintain_succ")      // 成功
#define MAINTAIN_STATUS_MAINTAIN_FAILED                 ("maintain_failed")    // 失败
#define MAINTAIN_STATUS_MAINTAINING                     ("maintaining")        // 正在维护

DevMaintainDetailItemWgt::DevMaintainDetailItemWgt(QWidget *parent)
    : QWidget(parent),
      m_enStatus(DEV_MAINTAIN_ITEM_STATUS_INIT)
{
    ui = new Ui::DevMaintainDetailItemWgt();
    ui->setupUi(this);

    // 显示之前初始化
    InitBeforeShow();
}

DevMaintainDetailItemWgt::~DevMaintainDetailItemWgt()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
    m_pMovie->deleteLater();
    delete ui;
}

///
/// @brief
///     获取维护项名称
///
/// @return 维护项名称
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月8日，新建函数
///
QString DevMaintainDetailItemWgt::GetItemName()
{
    return ui->MaintainItemLab->text();
}

///
/// @brief
///     设置维护项名称
///
/// @param[in]  strItemName  维护项名称
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月8日，新建函数
///
void DevMaintainDetailItemWgt::SetItemName(const QString& strItemName)
{
    ui->MaintainItemLab->setText(strItemName);
    QFontMetrics fm(ui->MaintainItemLab->font());
    if (isVisible() && (fm.width(strItemName) >= ui->MaintainItemLab->width()))
    {
        // 判断是否长度过长需要显示省略号
        QString newText = fm.elidedText(strItemName, Qt::ElideRight, ui->MaintainItemLab->width());
        ui->MaintainItemLab->setToolTip(strItemName);
        ui->MaintainItemLab->setText(newText);
    }
}

///
/// @brief
///     获取设备名
///
/// @return 设备名
///
/// @par History:
/// @li 4170/TangChuXian，2023年5月22日，新建函数
///
QString DevMaintainDetailItemWgt::GetDevName()
{
    return ui->DevNameLab->text();
}

///
/// @brief
///     设置设备名
///
/// @param[in]  strDevName  设备名
///
/// @par History:
/// @li 4170/TangChuXian，2023年5月22日，新建函数
///
void DevMaintainDetailItemWgt::SetDevName(const QString& strDevName)
{
    ui->DevNameLab->setText(strDevName);
}

///
/// @brief
///     获取维护时间
///
/// @return 维护时间
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月13日，新建函数
///
QString DevMaintainDetailItemWgt::GetMaintainTime()
{
    return ui->MaintainTimeLab->text();
}

///
/// @brief
///     设置维护时间
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月13日，新建函数
///
void DevMaintainDetailItemWgt::SetMaintainTime(const QString& strMaintainTime)
{
    // 如果状态不为成功或失败，时间显示空
    if ((m_enStatus != DEV_MAINTAIN_ITEM_STATUS_SUCC) && (m_enStatus != DEV_MAINTAIN_ITEM_STATUS_FAILED))
    {
        ui->MaintainTimeLab->clear();
        return;
    }

    // 设置维护时间
    ui->MaintainTimeLab->setText(ToCfgFmtDateTime(strMaintainTime));
}

///
/// @brief
///     获取设备维护状态
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月8日，新建函数
///
DevMaintainDetailItemWgt::DevMaintainItemStatus DevMaintainDetailItemWgt::GetStatus()
{
    return m_enStatus;
}

///
/// @brief
///     设置设备维护状态
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月8日，新建函数
///
void DevMaintainDetailItemWgt::SetStatus(DevMaintainItemStatus enStatus)
{
    // 如果状态未改变，则直接返回
    if (enStatus == m_enStatus)
    {
        return;
    }

    // 设置状态
    m_enStatus = enStatus;

    // 根据状态构造对应字符串（用于qss动态属性
    QString strStatus("");
    switch (enStatus)
    {
    case DevMaintainDetailItemWgt::DEV_MAINTAIN_ITEM_STATUS_WAIT:
        strStatus = MAINTAIN_STATUS_MAINTAIN_WAIT;
        ui->ReMaintainBtn->setVisible(false);
        ui->StatusTextLab->setStyleSheet("");
        ui->StatusTextLab->setText(tr("等待"));
        ui->ExeGifLab->setMovie(Q_NULLPTR);
        m_pMovie->stop();
        break;
    case DevMaintainDetailItemWgt::DEV_MAINTAIN_ITEM_STATUS_SUCC:
        strStatus = MAINTAIN_STATUS_MAINTAIN_SUCC;
        ui->ReMaintainBtn->setVisible(false);
        ui->StatusTextLab->setStyleSheet("color:#05b30d;");
        ui->StatusTextLab->setText(tr("成功"));
        ui->ExeGifLab->setMovie(Q_NULLPTR);
        m_pMovie->stop();
        break;
    case DevMaintainDetailItemWgt::DEV_MAINTAIN_ITEM_STATUS_FAILED:
        strStatus = MAINTAIN_STATUS_MAINTAIN_FAILED;
        ui->ReMaintainBtn->setVisible(true);
        ui->StatusTextLab->setStyleSheet("color:#e41313;");
        ui->StatusTextLab->setText(tr("失败"));
        ui->ExeGifLab->setMovie(Q_NULLPTR);
        m_pMovie->stop();
        break;
    case DevMaintainDetailItemWgt::DEV_MAINTAIN_ITEM_STATUS_EXCUTE:
        strStatus = MAINTAIN_STATUS_MAINTAINING;
        ui->ReMaintainBtn->setVisible(false);
        ui->StatusTextLab->setStyleSheet("color:#05b30d;");
        ui->StatusTextLab->setText(tr("正在维护"));
        ui->MaintainTimeLab->clear();
        ui->ExeGifLab->setMovie(m_pMovie);
        if (m_pMovie->state() != QMovie::Running)
        {
            m_pMovie->start();
        }
        break;
    default:
        break;
    }

    ui->StatusTextLab->setToolTip(ui->StatusTextLab->text());
    ui->StatusTextLab->setText(ui->StatusTextLab->fontMetrics().elidedText(ui->StatusTextLab->text(), Qt::ElideRight, 80));

    // 更新动态属性
    ui->BkFrame->setProperty(MAINTAIN_STATUS_PROPERTY_NAME, strStatus);
    ui->StatusIconLab->setProperty(MAINTAIN_STATUS_PROPERTY_NAME, strStatus);

    // 刷新界面显示
    UpdateStatus();
}

///
/// @brief
///     显示之前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月13日，新建函数
///
void DevMaintainDetailItemWgt::InitBeforeShow()
{
    // 初始化动画
    m_pMovie = new QMovie(":/Leonis/resource/image/gif-green-circle.gif");

    // 默认状态为等待
    SetStatus(DEV_MAINTAIN_ITEM_STATUS_WAIT);

    // 清空维护名和维护时间
    ui->StatusTextLab->setText(tr("等待"));
    ui->MaintainItemLab->clear();
    ui->MaintainTimeLab->clear();

    // 连接信号槽
    // 重新维护按钮被点击
    connect(ui->ReMaintainBtn, SIGNAL(clicked()), this, SLOT(OnReMaintainBtnClicked()));
}

///
/// @brief
///     更新状态
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月8日，新建函数
///
void DevMaintainDetailItemWgt::UpdateStatus()
{
    // 更新状态(背景)
    ui->BkFrame->style()->unpolish(ui->BkFrame);
    ui->BkFrame->style()->polish(ui->BkFrame);
    ui->BkFrame->update();

    // 更新状态(背景)
    ui->StatusIconLab->style()->unpolish(ui->StatusIconLab);
    ui->StatusIconLab->style()->polish(ui->StatusIconLab);
    ui->StatusIconLab->update();
}

///
/// @brief
///     显示事件
///
/// @param[in]  pEvt  事件对象
///
/// @par History:
/// @li 4170/TangChuXian，2024年8月22日，新建函数
///
void DevMaintainDetailItemWgt::showEvent(QShowEvent* pEvt)
{
    QWidget::showEvent(pEvt);
    QString strItemName = ui->MaintainItemLab->text();
    if (!strItemName.isEmpty())
    {
        // 触发一次显示后的文本自适应调整
        SetItemName(strItemName);
    }
}

///
/// @brief
///     重新维护按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月13日，新建函数
///
void DevMaintainDetailItemWgt::OnReMaintainBtnClicked()
{
    // 发送重新维护信号
    emit SigReMaintain(ui->DevNameLab->text());
}
