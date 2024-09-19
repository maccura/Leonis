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
/// @file     QProgressBarShell.h
/// @brief    酸性碱性清洗液的进度条
///
/// @author   5774/WuHongTao
/// @date     2022年1月24日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年1月24日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QProgressBarShell.h"
#include "ui_QProgressSmpClnBar.h"
#include "ui_QProgressBarShell.h"
#include "ui_QProgressBarShellIse.h"
#include <QPixmap>
#include "src/common/Mlog/mlog.h"
#include "shared/CommonInformationManager.h"
#include "manager/DictionaryQueryManager.h"
#include "src/thrift/ch/c1005/gen-cpp/c1005_constants.h"

#define FORMULA 10000.0

QProgressBarShell::QProgressBarShell(QWidget *parent)
	: QPushButton(parent)
{
    ui = new Ui::QProgressBarShell();
    ui->setupUi(this);
    m_init = false;

    // 待完善余量检测
    connect(ui->detect_btn, &QPushButton::clicked, this, [&]()
    {
        emit detectVolumn();
    });
}

QProgressBarShell::~QProgressBarShell()
{
}

///
/// @brief 获取耗材状态
///
///
/// @return 耗材状态
///
/// @par History:
/// @li 5774/WuHongTao，2022年9月22日，新建函数
///
QProgressBarShell::SupplyStatus QProgressBarShell::GetStatus()
{
    return m_supplyStatus;
}

///
/// @brief 设置耗材状态
///
/// @param[in]  status  状态
///
/// @par History:
/// @li 5774/WuHongTao，2022年9月23日，新建函数
///
void QProgressBarShell::SetSupplyStatus(SupplyStatus status)
{
    ui->detect_btn->hide();
    m_supplyStatus = status;

    switch (status.supplyType)
    {
        // 酸性
        case QProgressBarShell::ACIDITY:
        {
            m_bootleRemind = -1;
            auto supplyAcidityAttribute = CommonInformationManager::GetInstance()->GetSupplyAttributeByTypeCode(ch::tf::SuppliesType::SUPPLIES_TYPE_CUP_DETERGENT_ACIDITY, false);
            if (supplyAcidityAttribute)
            {
                m_bootleRemind = supplyAcidityAttribute.value()->bottleAlarmThreshold;
            }

            ComputAcidityStatusSupply(status);
            if (m_init == false)
            {
                ui->balance->move(ui->balance->x() + 17, ui->balance->y());
                m_init = true;
            }
        }
            break;
        // 碱性
        case QProgressBarShell::ALKALINITY:
        {
            m_bootleRemind = -1;
            auto supplyAlkalinityAttribute = CommonInformationManager::GetInstance()->GetSupplyAttributeByTypeCode(ch::tf::SuppliesType::SUPPLIES_TYPE_CUP_DETERGENT_ALKALINITY, false);
            if (supplyAlkalinityAttribute)
            {
                m_bootleRemind = supplyAlkalinityAttribute.value()->bottleAlarmThreshold;
            }

            ComputAlkalinityStatusSupply(status);
            if (m_init == false)
            {
                ui->balance->move(ui->balance->x() + 17, ui->balance->y());
                m_init = true;
            }
        }
            break;
        // 样本针清洗
        case QProgressBarShell::SAMPLEWASH:
        {
            // 样本针酸性
            if (status.supplyInfo.type == ch::tf::SuppliesType::type::SUPPLIES_TYPE_DETERGENT_ACIDITY)
            {
                m_bootleRemind = -1;
                auto supplySampleAcidityAttribute = CommonInformationManager::GetInstance()->GetSupplyAttributeByTypeCode(ch::tf::SuppliesType::SUPPLIES_TYPE_DETERGENT_ACIDITY, true);
                if (supplySampleAcidityAttribute)
                {
                    m_bootleRemind = supplySampleAcidityAttribute.value()->bottleAlarmThreshold;
                }
            }
            // 样本针碱性
            else
            {
                m_bootleRemind = -1;
                auto supplySampleAlkalinityAttribute = CommonInformationManager::GetInstance()->GetSupplyAttributeByTypeCode(ch::tf::SuppliesType::SUPPLIES_TYPE_DETERGENT_ALKALINITY, true);
                if (supplySampleAlkalinityAttribute)
                {
                    m_bootleRemind = supplySampleAlkalinityAttribute.value()->bottleAlarmThreshold;
                }
            }

            ComputSampleWashAcidityStatusSupply(status);
            ui->detect_btn->show();
        }
            break;
        // 废液桶
        case QProgressBarShell::WASTELIQUID:
            // 废液桶报警值
            m_bootleRemind = DictionaryQueryManager::GetInstance()->GetChWasteAlarmVolumn();

			ULOG(LOG_INFO, u8"获取废液桶报警值(0.1ul) : %d, 废液桶余量值(0.1ul): %d", m_bootleRemind, status.blance);

            ComputWasteLiquidStatusSupply(status);
            break;
        // 空
        case QProgressBarShell::EMPTY:
            ui->progress->setVisible(false);
            ui->shell->setVisible(false);
            ui->balance->hide();
            break;
        default:
            ui->progress->setVisible(false);
            ui->shell->setVisible(false);
            ui->balance->hide();
            break;
    }
}

///
/// @brief 处理焦点失去的事件
///
/// @param[in]  event  事件信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年9月23日，新建函数
///
void QProgressBarShell::focusOutEvent(QFocusEvent *event)
{
    emit focusOut();
}

///
/// @brief 酸性清洗液的状态
///
/// @param[in]  status  状态
///
/// @par History:
/// @li 5774/WuHongTao，2022年9月20日，新建函数
///
void QProgressBarShell::ComputAcidityStatusSupply(SupplyStatus status)
{
    QPixmap pixmap;

    switch (status.status)
    {
        // 主用
        case QProgressBarShell::CURRENT:
        {
            // 告警状态，红色
            if (status.blance <= 0)
            {
                ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ch/c1005/border-acidity-alarm.png);");
                pixmap.load(":/Leonis/resource/image/ch/c1005/water-acidity-alarm.png");
            }
            // 提醒状态，橙色
            else if (m_bootleRemind > 0 && status.blance <= m_bootleRemind)
            {
                ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ch/c1005/border-acidity-remind.png);");
                pixmap.load(":/Leonis/resource/image/ch/c1005/water-acidity-remind.png");
            }
            // 正常状态，蓝色
            else
            {
                ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ch/c1005/border-acidity-inuse.png);");
                pixmap.load(":/Leonis/resource/image/ch/c1005/water-acidity-inuse.png");
            }
			ui->balance->setText(QString("%1ml").arg(std::ceil(status.blance / FORMULA)));
            ui->balance->show();
        }
            break;
        // 备用
        case QProgressBarShell::BACKUP:
        {
            // 告警状态，红色
            if (status.blance <= 0)
            {
                ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ch/c1005/border-acidity-alarm.png);");
                pixmap.load(":/Leonis/resource/image/ch/c1005/water-acidity-alarm.png");
            }
            // 提醒状态，橙色
            else if (m_bootleRemind > 0 && status.blance <= m_bootleRemind)
            {
                ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ch/c1005/border-acidity-remind.png);");
                pixmap.load(":/Leonis/resource/image/ch/c1005/water-acidity-remind.png");
            }
            // 正常状态，蓝灰色
            else
            {
                ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ch/c1005/border-acidity-spare.png);");
                pixmap.load(":/Leonis/resource/image/ch/c1005/water-acidity-spare.png");
            }
			ui->balance->setText(QString("%1ml").arg(std::ceil(status.blance / FORMULA)));
            ui->balance->show();
        }
            break;
        // 扫描失败
        case QProgressBarShell::SCANFAILED:
        {
            ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ch/c1005/acidity-fail.png);");
            ui->balance->hide();
        }
            break;
        // 过期(目前状态与告警状态相似)
        case QProgressBarShell::EXPIRED:
        {
            ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ch/c1005/border-acidity-alarm.png);");
            pixmap.load(":/Leonis/resource/image/ch/c1005/water-acidity-alarm.png");
			ui->balance->setText(QString("%1ml").arg(std::ceil(status.blance / FORMULA)));
            ui->balance->show();
        }
            break;
        // 空位
        case QProgressBarShell::UNKONW:
        {
            ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ch/c1005/acidity-alkalinity-empty.png);");
            ui->balance->hide();
        }
            break;
        default:
            break;
    }

	// 当数据太小，显示效果太差
	if (status.progress < 9 && status.blance > 0)
	{
		status.progress = 9;
	}

    ui->progress->SetProgress(status.progress, pixmap);
    ui->progress->setVisible(true);
    ui->shell->setVisible(true);
    setStyleSheet("");
}

///
/// @brief 碱性清洗液的状态
///
/// @param[in]  status  状态
///
/// @par History:
/// @li 5774/WuHongTao，2022年9月21日，新建函数
///
void QProgressBarShell::ComputAlkalinityStatusSupply(SupplyStatus status)
{
    QPixmap pixmap;

    switch (status.status)
    {
        // 主用
        case QProgressBarShell::CURRENT:
        {
            // 告警状态，红色
            if (status.blance <= 0)
            {
                ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ch/c1005/border-alkalinity-alarm.png);");
                pixmap.load(":/Leonis/resource/image/ch/c1005/water-alkalinity-alarm.png");
            }
            // 提醒状态，橙色
            else if (m_bootleRemind > 0 && status.blance <= m_bootleRemind)
            {
                ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ch/c1005/border-alkalinity-remind.png);");
                pixmap.load(":/Leonis/resource/image/ch/c1005/water-alkalinity-remind.png");
            }
            // 正常状态，蓝色
            else
            {
                ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ch/c1005/border-alkalinity-inuse.png);");
                pixmap.load(":/Leonis/resource/image/ch/c1005/water-alkalinity-inuse.png");
            }
			ui->balance->setText(QString("%1ml").arg(std::ceil(status.blance / FORMULA)));
            ui->balance->show();
        }
        break;
        // 备用
        case QProgressBarShell::BACKUP:
        {
            // 告警状态，红色
            if (status.blance <= 0)
            {
                ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ch/c1005/border-alkalinity-alarm.png);");
                pixmap.load(":/Leonis/resource/image/ch/c1005/water-alkalinity-alarm.png");
            }
            // 提醒状态，橙色
            else if (m_bootleRemind > 0 && status.blance <= m_bootleRemind)
            {
                ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ch/c1005/border-alkalinity-remind.png);");
                pixmap.load(":/Leonis/resource/image/ch/c1005/water-alkalinity-remind.png");
            }
            // 正常状态，蓝灰色
            else
            {
                ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ch/c1005/border-alkalinity-spare.png);");
                pixmap.load(":/Leonis/resource/image/ch/c1005/water-alkalinity-spare.png");
            }
			ui->balance->setText(QString("%1ml").arg(std::ceil(status.blance / FORMULA)));
            ui->balance->show();
        }
        break;
        // 扫描失败
        case QProgressBarShell::SCANFAILED:
        {
            ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ch/c1005/alkalinity-fail.png);");
            ui->balance->hide();
        }
        break;
        // 过期
        case QProgressBarShell::EXPIRED:
        {
            ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ch/c1005/border-alkalinity-alarm.png);");
            pixmap.load(":/Leonis/resource/image/ch/c1005/water-alkalinity-alarm.png");
			ui->balance->setText(QString("%1ml").arg(std::ceil(status.blance / FORMULA)));
            ui->balance->show();
        }
        break;
        // 空位
        case QProgressBarShell::UNKONW:
        {
            ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ch/c1005/acidity-alkalinity-empty.png);");
            ui->balance->hide();
        }
        break;
        default:
            break;
    }

	// 当数据太小，显示效果太差
	if (status.progress < 9 && status.blance > 0)
	{
		status.progress = 9;
	}

    ui->progress->SetProgress(status.progress, pixmap);
    ui->progress->setVisible(true);
    ui->shell->setVisible(true);
    setStyleSheet("");
}

///
/// @brief 样本针清洗液状态
///
/// @param[in]  status  状态
///
/// @par History:
/// @li 5774/WuHongTao，2022年9月21日，新建函数
///
void QProgressBarShell::ComputSampleWashAcidityStatusSupply(SupplyStatus status)
{
    QPixmap pixmap;

    switch (status.status)
    {
        // 主用
        case QProgressBarShell::CURRENT:
        {
            // 告警状态，红色
            if (status.blance <= 0)
            {
                ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ch/c1005/border-needle-alarm.png);");
                pixmap.load(":/Leonis/resource/image/ch/c1005/water-needle-alarm.png");
            }
            // 提醒状态，橙色
            else if (m_bootleRemind > 0 && status.blance <= m_bootleRemind)
            {
                ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ch/c1005/border-needle-remind.png);");
                pixmap.load(":/Leonis/resource/image/ch/c1005/water-needle-remind.png");
            }
            // 正常状态，蓝色
            else
            {
                ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ch/c1005/border-needle-inuse.png);");
                pixmap.load(":/Leonis/resource/image/ch/c1005/water-needle-inuse.png");
            }
			ui->balance->setText(QString("%1ml").arg(std::ceil(status.blance / FORMULA)));
            ui->balance->show();
        }
        break;
        // 备用
        case QProgressBarShell::BACKUP:
        {
            // 告警状态，红色
            if (status.blance <= 0)
            {
                ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ch/c1005/border-needle-alarm.png);");
                pixmap.load(":/Leonis/resource/image/ch/c1005/water-needle-alarm.png");
            }
            // 提醒状态，橙色
            else if (m_bootleRemind > 0 && status.blance <= m_bootleRemind)
            {
                ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ch/c1005/border-needle-remind.png);");
                pixmap.load(":/Leonis/resource/image/ch/c1005/water-needle-remind.png");
            }
            // 正常状态，蓝灰色
            else
            {
                ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ch/c1005/border-needle-spare.png);");
                pixmap.load(":/Leonis/resource/image/ch/c1005/water-needle-spare.png");
            }
			ui->balance->setText(QString("%1ml").arg(std::ceil(status.blance / FORMULA)));
            ui->balance->show();
        }
        break;
        // 扫描失败
        case QProgressBarShell::SCANFAILED:
        {
            ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ch/c1005/needle-fail.png);");
            ui->balance->hide();
        }
        break;
        // 过期
        case QProgressBarShell::EXPIRED:
        {
            ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ch/c1005/border-needle-alarm.png);");
            pixmap.load(":/Leonis/resource/image/ch/c1005/water-needle-alarm.png");
			ui->balance->setText(QString("%1ml").arg(std::ceil(status.blance / FORMULA)));
            ui->balance->show();
        }
        break;
        // 空位
        case QProgressBarShell::UNKONW:
        {
            ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ch/c1005/needle-empty.png);");
            ui->balance->hide();
        }
        break;
        default:
            break;
    }

	// 当数据太小，显示效果太差
	//if (status.progress < 9 && status.blance > 0)
	//{
	//	status.progress = 9;
	//}

    ui->progress->SetProgress(status.progress, pixmap);
    ui->progress->setVisible(true);
    ui->shell->setVisible(true);
    setStyleSheet("");
}

///
/// @brief 废液桶状态
///
/// @param[in]  status  状态
///
/// @par History:
/// @li 5774/WuHongTao，2022年9月21日，新建函数
///
void QProgressBarShell::ComputWasteLiquidStatusSupply(SupplyStatus status)
{
    QPixmap pixmap;
    // 告警状态（废液满了，高位浮子触发）
    if (status.blance >= ::ch::c1005::tf::g_c1005_constants.MAX_WASTE)
    {
        ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ch/c1005/border-discard-alarm.png);");
        pixmap.load(":/Leonis/resource/image/ch/c1005/water-discard-alarm.png");
    }
	// 提醒状态（废液达到提醒值）
	else if (m_bootleRemind > 0 && status.blance >= m_bootleRemind)
	{
		ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ch/c1005/border-discard-notice.png);");
		pixmap.load(":/Leonis/resource/image/ch/c1005/water-discard-notice.png");
	}
    // 正常状态，蓝色
    else
    {
        ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ch/c1005/border-discard-inuse.png);");
        pixmap.load(":/Leonis/resource/image/ch/c1005/water-discard-inuse.png");
    }

    if (m_init == false)
    {
        ui->balance->move(ui->balance->x() + 44, ui->balance->y());
        m_init = true;
    }

	// 当数据太小，显示效果太差
	if (status.progress < 6 && status.blance > 0)
	{
		status.progress = 6;
	}

	ui->balance->setText(QString("%1ml").arg(std::ceil(status.blance / FORMULA)));
    ui->balance->show();
    ui->progress->SetProgress(status.progress, pixmap);
    ui->progress->setVisible(true);
    ui->shell->setVisible(true);
    setStyleSheet("");
}

QProgressSmpClnBarShell::QProgressSmpClnBarShell(QWidget *parent /*= Q_NULLPTR*/)
    : QSupplyProgress(parent)
{
}


QSupplyProgress::QSupplyProgress(QWidget *parent /*= Q_NULLPTR*/)
    : QWidget(parent)
{
    ui = new Ui::QProgressSmpClnBar();
    ui->setupUi(this);
    // 待完善余量检测
    connect(ui->detect_btn, &QPushButton::clicked, this, [=]()
    {
        return;
    });
}

///
/// @brief 设置仓外耗材状态
///
/// @param[in]  status  耗材状态
///
/// @return true 返回成功
///
/// @par History:
/// @li 5774/WuHongTao，2023年1月17日，新建函数
///
bool QSupplyProgress::SetSupplyStatus(SupplyStatus status)
{
    m_supplyStatus = status;
    ShowSupplyStatus(status);
    return true;
}

///
/// @brief 显示耗材状态
///
/// @param[in]  status  耗材状态
///
/// @par History:
/// @li 5774/WuHongTao，2023年1月17日，新建函数
///
void QSupplyProgress::ShowSupplyStatus(SupplyStatus status)
{
    // 显示毫升
	ui->balance->setText(QString("%1ml").arg(std::ceil(status.blance / FORMULA)));
    ui->balance->show();

	// 当数据太小，显示效果太差
	//if (status.progress < 5 && status.blance > 0)
	//{
	//	status.progress = 5;
	//}

    ui->progress->SetProgress(status.progress, status.pixmap);
    ui->progress->setVisible(true);
    ui->shell->setStyleSheet(status.shellPixmap);
    ui->shell->setVisible(true);

    // 按钮是否显示
    if (status.isShowBtn)
    {
        ui->detect_btn->setVisible(true);
    }
    else
    {
        ui->detect_btn->setVisible(false);
    }

    setStyleSheet("");
}

QProgressIseBarShell::QProgressIseBarShell(QWidget *parent /*= Q_NULLPTR*/)
{
    ui = new Ui::IseProgressBar();
    ui->setupUi(this);

    // 待完善余量检测
    connect(ui->detect_btn, &QPushButton::clicked, this, [=]()
    {
        return;
    });
}

void QProgressIseBarShell::SetSupplyStatus(SupplyStatus status)
{
    ui->detect_btn->hide();
    m_supplyStatus = status;

    switch (status.supplyType)
    {
        // 样本针清洗
        case QProgressIseBarShell::SAMPLEWASH:
        {
            // 样本针酸性
            if (status.supplyInfo.type == ch::tf::SuppliesType::type::SUPPLIES_TYPE_DETERGENT_ACIDITY)
            {
                m_bootleRemind = -1;
                auto supplySampleAcidityAttribute = CommonInformationManager::GetInstance()->GetSupplyAttributeByTypeCode(ch::tf::SuppliesType::SUPPLIES_TYPE_DETERGENT_ACIDITY, true);
                if (supplySampleAcidityAttribute)
                {
                    m_bootleRemind = supplySampleAcidityAttribute.value()->bottleAlarmThreshold;
                }
                ui->detect_btn->show();
            }
            // 样本针碱性
            else if(status.supplyInfo.type == ch::tf::SuppliesType::type::SUPPLIES_TYPE_DETERGENT_ACIDITY)
            {
                m_bootleRemind = -1;
                auto supplySampleAlkalinityAttribute = CommonInformationManager::GetInstance()->GetSupplyAttributeByTypeCode(ch::tf::SuppliesType::SUPPLIES_TYPE_DETERGENT_ALKALINITY, true);
                if (supplySampleAlkalinityAttribute)
                {
                    m_bootleRemind = supplySampleAlkalinityAttribute.value()->bottleAlarmThreshold;
                }
                ui->detect_btn->show();
            }
            // 样本针酸性(ise)
            else if (status.supplyIseInfo.suppliesCode == ise::tf::SuppliesType::SUPPLIES_TYPE_DETERGENT_ACIDITY)
            {
                m_bootleRemind = -1;
                auto supplySampleAlkalinityAttribute = CommonInformationManager::GetInstance()->GetISESupplyAttributeByTypeCode(status.supplyIseInfo.suppliesCode);
                if (supplySampleAlkalinityAttribute)
                {
                    m_bootleRemind = supplySampleAlkalinityAttribute.value()->bottleAlarmThreshold;
                }
            }
            // 样本针碱性(ise)
            else
            {
                m_bootleRemind = -1;
                auto supplySampleAlkalinityAttribute = CommonInformationManager::GetInstance()->GetISESupplyAttributeByTypeCode(status.supplyIseInfo.suppliesCode);
                if (supplySampleAlkalinityAttribute)
                {
                    m_bootleRemind = supplySampleAlkalinityAttribute.value()->bottleAlarmThreshold;
                }
            }

            ComputSampleWashAcidityStatusSupply(status);
        }
        break;
        // 内部标准液
        case QProgressIseBarShell::ISEIS:
        {
            m_bootleRemind = -1;
            auto attribute = CommonInformationManager::GetInstance()->GetISESupplyAttributeByTypeCode(ise::tf::SuppliesType::SUPPLIES_TYPE_IS);
            if (attribute)
            {
                m_bootleRemind = attribute.value()->bottleAlarmThreshold;
            }

            IseISStatusSupply(status);
        }
        break;
        // 缓释液
        case QProgressIseBarShell::ISEBUF:
        {
            m_bootleRemind = -1;
            auto attribute = CommonInformationManager::GetInstance()->GetISESupplyAttributeByTypeCode(ise::tf::SuppliesType::SUPPLIES_TYPE_BUFFER);
            if (attribute)
            {
                m_bootleRemind = attribute.value()->bottleAlarmThreshold;
            }

            IseBufStatusSupply(status);
        }
        break;
            // 空
        case QProgressIseBarShell::EMPTY:
            ui->progress->setVisible(false);
            ui->shell->setVisible(false);
            ui->balance->hide();
            break;
        default:
            ui->progress->setVisible(false);
            ui->shell->setVisible(false);
            ui->balance->hide();
            break;
    }
}

void QProgressIseBarShell::SetRemind(int remind)
{
    m_bootleRemind = remind;
}

void QProgressIseBarShell::focusOutEvent(QFocusEvent *event)
{
    emit focusOut();
}

void QProgressIseBarShell::ComputSampleWashAcidityStatusSupply(SupplyStatus status)
{
    QPixmap pixmap;

    switch (status.status)
    {
        // 主用
        case QProgressIseBarShell::CURRENT:
        {
            // 告警状态，红色
            if (status.blance <= 0)
            {
                ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ise/border-smp-cln-alarm.png);");
                pixmap.load(":/Leonis/resource/image/ise/water-smp-cln-alarm.png");
            }
            // 提醒状态，橙色
            else if (m_bootleRemind > 0 && status.blance <= m_bootleRemind)
            {
                ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ise/border-smp-cln-remind.png);");
                pixmap.load(":/Leonis/resource/image/ise/water-smp-cln-remind.png");
            }
            // 正常状态，蓝色
            else
            {
                ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ise/border-smp-cln.png);");
                pixmap.load(":/Leonis/resource/image/ise/water-smp-cln.png");
            }
			ui->balance->setText(QString("%1ml").arg(std::ceil(status.blance / FORMULA)));
            ui->balance->hide();
        }
        break;
        // 备用
        case QProgressIseBarShell::BACKUP:
        {
            // 告警状态，红色
            if (status.blance <= 0)
            {
                ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ise/border-smp-cln-alarm.png);");
                pixmap.load(":/Leonis/resource/image/ise/water-smp-cln-alarm.png");
            }
            // 提醒状态，橙色
            else if (m_bootleRemind > 0 && status.blance <= m_bootleRemind)
            {
                ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ise/border-smp-cln-remind.png);");
                pixmap.load(":/Leonis/resource/image/ise/water-smp-cln-remind.png");
            }
            // 正常状态，蓝灰色
            else
            {
                ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ise/border-smp-cln.png);");
                pixmap.load(":/Leonis/resource/image/ise/water-smp-cln.png");
            }
			ui->balance->setText(QString("%1ml").arg(std::ceil(status.blance / FORMULA)));
            ui->balance->hide();
        }
        break;
        // 扫描失败
        case QProgressIseBarShell::SCANFAILED:
        {
            ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ch/ise/smp-cln-fail.png);");
            ui->balance->hide();
        }
        break;
        // 过期
        case QProgressIseBarShell::EXPIRED:
        {
            ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ise/border-smp-cln-alarm.png);");
            pixmap.load(":/Leonis/resource/image/ise/water-smp-cln-alarm.png");
			ui->balance->setText(QString("%1ml").arg(std::ceil(status.blance / FORMULA)));
            ui->balance->hide();
        }
        break;
        // 空位
        case QProgressIseBarShell::UNKONW:
        {
            ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ch/ise/smp-cln-empty.png);");
            ui->balance->hide();
        }
        break;
        default:
            break;
    }

	// 当数据太小，显示效果太差
	if (status.progress < 3 && status.blance > 0)
	{
		status.progress = 3;
	}

    ui->progress->SetProgress(status.progress, pixmap);
    ui->progress->setVisible(true);
    ui->shell->setVisible(true);
    setStyleSheet("");
}

void QProgressIseBarShell::IseISStatusSupply(SupplyStatus status)
{
    QPixmap pixmap;
    switch (status.status)
    {
        // 主用
        case QProgressIseBarShell::CURRENT:
        {
            // 告警状态，红色
            if (status.blance <= 0)
            {
                ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ise/border-is-alarm.png);");
                pixmap.load(":/Leonis/resource/image/ise/water-is-alarm.png");
            }
            // 提醒状态，橙色
            else if (m_bootleRemind > 0 && status.blance <= m_bootleRemind)
            {
                ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ise/border-is-remind.png);");
                pixmap.load(":/Leonis/resource/image/ise/water-is-remind.png");
            }
            // 正常状态，蓝色
            else
            {
                ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ise/border-is.png);");
                pixmap.load(":/Leonis/resource/image/ise/water-is.png");
            }
			ui->balance->setText(QString("%1ml").arg(std::ceil(status.blance / FORMULA)));
            ui->balance->hide();
        }
        break;
        // 扫描失败
        case QProgressIseBarShell::SCANFAILED:
        {
            ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ch/ise/is-fail.png);");
            ui->balance->hide();
        }
        break;
        // 过期
        case QProgressIseBarShell::EXPIRED:
        {
            ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ise/border-is-alarm.png);");
            pixmap.load(":/Leonis/resource/image/ise/water-is-alarm.png");
			ui->balance->setText(QString("%1ml").arg(std::ceil(status.blance / FORMULA)));
            ui->balance->hide();
        }
        break;
        // 空位
        case QProgressIseBarShell::UNKONW:
        {
            ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ch/ise/is-empty.png);");
            ui->balance->hide();
        }
        break;
        default:
            break;
    }

    // 当数据太小，显示效果太差
	if (status.progress < 3 && status.blance > 0)
	{
		status.progress = 3;
	}

    ui->progress->SetProgress(status.progress, pixmap);
    ui->progress->setVisible(true);
    ui->shell->setVisible(true);
    setStyleSheet("");
}

void QProgressIseBarShell::IseBufStatusSupply(SupplyStatus status)
{
    QPixmap pixmap;

    switch (status.status)
    {
        // 主用
        case QProgressIseBarShell::CURRENT:
        {
            // 告警状态，红色
            if (status.blance <= 0)
            {
                ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ise/border-buf-alarm.png);");
                pixmap.load(":/Leonis/resource/image/ise/water-buf-alarm.png");
            }
            // 提醒状态，橙色
            else if (m_bootleRemind > 0 && status.blance <= m_bootleRemind)
            {
                ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ise/border-buf-remind.png);");
                pixmap.load(":/Leonis/resource/image/ise/water-buf-remind.png");
            }
            // 正常状态，蓝色
            else
            {
                ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ise/border-buf.png);");
                pixmap.load(":/Leonis/resource/image/ise/water-buf.png");
            }
			ui->balance->setText(QString("%1ml").arg(std::ceil(status.blance / FORMULA)));
            ui->balance->hide();
        }
        break;
        // 扫描失败
        case QProgressIseBarShell::SCANFAILED:
        {
            ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ch/ise/buf-fail.png);");
            ui->balance->hide();
        }
        break;
        // 过期
        case QProgressIseBarShell::EXPIRED:
        {
            ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ise/border-buf-alarm.png);");
            pixmap.load(":/Leonis/resource/image/ise/water-buf-alarm.png");
			ui->balance->setText(QString("%1ml").arg(std::ceil(status.blance / FORMULA)));
            ui->balance->hide();
        }
        break;
        // 空位
        case QProgressIseBarShell::UNKONW:
        {
            ui->shell->setStyleSheet("border-image:url(:/Leonis/resource/image/ch/ise/buf-empty.png);");
            ui->balance->hide();
        }
        break;
        default:
            break;
    }

    // 当数据太小，显示效果太差
	if (status.progress < 3 && status.blance > 0)
	{
		status.progress = 3;
	}

    ui->progress->SetProgress(status.progress, pixmap);
    ui->progress->setVisible(true);
    ui->shell->setVisible(true);
    setStyleSheet("");
}
