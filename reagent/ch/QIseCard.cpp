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

#include "QIseCard.h"
#include "ui_QIseCard.h"
#include "shared/ThriftEnumTransform.h"
#include "shared/ReagentCommon.h"
#include "src/common/TimeUtil.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/common.h"
#include "src/thrift/ise/gen-cpp/ise_constants.h"
#include "shared/datetimefmttool.h"
#include "shared/CommonInformationManager.h"
#include <QDateTime>

#define		IC_TAG_NAME			"Internal calibrator"
#define		BS_TAG_NAME			"Buffer solution"
#define		ACI_TAG_NAME		"Detergent NO.11"
#define		ALKA_TAG_NAME		"Detergent NO.12"

QIseCard::QIseCard(QWidget *parent /*= Q_NULLPTR*/)
{
    ui = new Ui::QIseCard();
    ui->setupUi(this);
    QProgressIseBarShell::SupplyStatus status;
    ui->progress->SetSupplyStatus(status);
    // 余量检测
	connect(ui->detecte_Volumn, &QPushButton::clicked, this, [&]() {
		emit signalRemainDetect();
	});
}

///
/// @brief 设置ISE耗材信息
///
/// @param[in]  supplyInfo  耗材信息
///
/// @par History:
/// @li 5774/WuHongTao，2023年2月8日，新建函数
///
void QIseCard::SetIseSupply(ise::tf::SuppliesInfo supplyInfo)
{
	ULOG(LOG_INFO, "%s(suppliesCode : )", __FUNCTION__, supplyInfo.suppliesCode);

    // 标题
	QString tagName;
	// 内标液
	if (supplyInfo.suppliesCode == ise::tf::SuppliesType::SUPPLIES_TYPE_IS)
	{
		tagName = IC_TAG_NAME;
	}
	// 缓冲液
	else if (supplyInfo.suppliesCode == ise::tf::SuppliesType::SUPPLIES_TYPE_BUFFER)
	{
		tagName = BS_TAG_NAME;
	}
	ui->groupBox->setTitle(tagName);

    // 名称
	QString suplyName = ThriftEnumTrans::GetSupplyName((ise::tf::SuppliesType::type)supplyInfo.suppliesCode);
    ui->name_label->setText(suplyName);

    // 批号
	QString lot;
	if (supplyInfo.lotSnMode == tf::EnumLotSnMode::ELSM_LOT
		|| supplyInfo.lotSnMode == tf::EnumLotSnMode::ELSM_LOT_SN)
	{
		lot = QString::fromStdString(supplyInfo.lot);
	}
	ui->lot_label->setText(lot);

	QProgressIseBarShell::SupplyStatus status;
    status.status = QProgressIseBarShell::SUPPLSTATUS::CURRENT;

	// 若当前时间>失效时间
	if (!supplyInfo.expiryTime.empty() && ChSuppliesExpire(supplyInfo.expiryTime))
	{
		ui->label_expiredText->setStyleSheet("color: #ff6669;");
		ui->label_expired->setStyleSheet("color: #ff6669;");
		status.status = QProgressIseBarShell::SUPPLSTATUS::EXPIRED;
	}
	else
	{
		ui->label_expiredText->setStyleSheet("color: #565656;");
		ui->label_expired->setStyleSheet("color: #565656;");
	}

	// 开瓶有效期过期，瓶子图标显示红色
	if (!supplyInfo.openBottleExpiryTime.empty() && GetOpenRestdays(supplyInfo.openBottleExpiryTime) <= 0)
	{
		status.status = QProgressIseBarShell::SUPPLSTATUS::EXPIRED;
	}

	// 失效时间
	if (supplyInfo.expiryTime.empty())
	{
		ui->label_expired->clear();
	}
	else
	{
		ui->label_expired->setText(ToCfgFmtDateTime(QString::fromStdString(ConverStdStringToDateString(supplyInfo.expiryTime))));
	}

	// 液位状态(探测失败余量置为0)
	if (supplyInfo.liquidStatus == ::tf::LiquidStatus::LIQUID_STATUS_DETECT_FAIL)
	{
		supplyInfo.residual = 0;
	}

    // 余量
    status.blance = supplyInfo.residual;

    // 缓冲液
    if (supplyInfo.suppliesCode == ise::tf::SuppliesType::SUPPLIES_TYPE_BUFFER)
    {
        status.supplyType = QProgressIseBarShell::ISEBUF;
		// ise缓冲液余量百分比
		status.progress = (100 * double(supplyInfo.residual) / double(::ise::tf::g_ise_constants.ISE_BUFF_CAVITY_SIZE));
    }
    // 内部标准液
    else
    {
        status.supplyType = QProgressIseBarShell::ISEIS;
		// ise内标液余量百分比
		status.progress = (100 * double(supplyInfo.residual) / double(::ise::tf::g_ise_constants.ISE_IS_CAVITY_SIZE));
    }

	auto remainCountColor = getReamainCountColor(supplyInfo.suppliesCode, supplyInfo.residual);
	ui->label_remainCountText->setStyleSheet(remainCountColor);
	ui->label_remainCount->setStyleSheet(remainCountColor);

    // 设置余量
	ui->label_remainCount->setText(QString("%1ml").arg(std::ceil(status.blance / 10000.0)));

    // 设置ise耗材
    ui->progress->SetSupplyStatus(status);

	// 隐藏余量探测按钮
    ui->widget_tc->hide();
}

///
/// @brief 设置台面耗材状态
///
/// @param[in]  supplyInfo  耗材信息
///
/// @par History:
/// @li 5774/WuHongTao，2023年2月8日，新建函数
///
void QIseCard::SetSmpClnSupply(ise::tf::SuppliesInfo supplyInfo)
{
	// 隐藏批号、失效日期
	ui->widget_lot->hide();
	ui->widget_time->hide();

	// 设置上下高度 左、上、右、下
	ui->verticalLayout_3->setContentsMargins(-1, -1, -1, 5);

	m_devSn = supplyInfo.deviceSN;

	// 标题
	QString tagName;
	// 样本针酸性清洗液
	if (supplyInfo.suppliesCode == ise::tf::SuppliesType::SUPPLIES_TYPE_DETERGENT_ACIDITY)
	{
		tagName = ACI_TAG_NAME;
	}
	// 样本针碱性清洗液
	else if (supplyInfo.suppliesCode == ise::tf::SuppliesType::SUPPLIES_TYPE_DETERGENT_ALKALINITY)
	{
		tagName = ALKA_TAG_NAME;
	}
	ui->groupBox->setTitle(tagName);

	// 名称
    QString suplyName = ThriftEnumTrans::GetSupplyName((ise::tf::SuppliesType::type)supplyInfo.suppliesCode);
    ui->name_label->setText(suplyName);

	// ise酸碱清洗液没有射频卡也没法录入，不显示开瓶有效期、失效日期等，永不过期，不显示红色
	QProgressIseBarShell::SupplyStatus status;
	status.status = QProgressIseBarShell::SUPPLSTATUS::CURRENT;
    status.supplyType = QProgressIseBarShell::SUPPLYTYPE::SAMPLEWASH;

	// 液位状态(探测失败余量置为0)
	if (supplyInfo.liquidStatus == ::tf::LiquidStatus::LIQUID_STATUS_DETECT_FAIL)
	{
		supplyInfo.residual = 0;
	}

	// 余量
    status.blance = supplyInfo.residual;
    status.supplyIseInfo = supplyInfo;

    // ise样本针酸碱清洗液余量百分比
    status.progress = (100 * double(supplyInfo.residual) / double(::ise::tf::g_ise_constants.ISE_ACI_ALK_CAVITY_SIZE));

	auto remainCountColor = getReamainCountColor(supplyInfo.suppliesCode, supplyInfo.residual);
	ui->label_remainCountText->setStyleSheet(remainCountColor);
	ui->label_remainCount->setStyleSheet(remainCountColor);

	// 设置余量
	ui->label_remainCount->setText(QString("%1ml").arg(std::ceil(status.blance / 10000.0)));

    ui->progress->SetSupplyStatus(status);
}

void QIseCard::SetChecked(bool ck)
{
	if (ck)
	{
		ui->groupBox->setProperty("borders", "group-select");
	}
	else
	{
		ui->groupBox->setProperty("borders", "group");
	}

	ui->groupBox->style()->unpolish(ui->groupBox);
	ui->groupBox->style()->polish(ui->groupBox);
}

QProgressIseBarShell* QIseCard::getIcon() const
{
    return ui->progress;
};

QString QIseCard::getReamainCountColor(const int supplyCode, const double residual)
{
	// 耗材属性信息
	int bottleAlarmThreshold = 0;
	auto supplyAttribute = CommonInformationManager::GetInstance()->GetISESupplyAttributeByTypeCode(supplyCode);
	if (supplyAttribute)
	{
		bottleAlarmThreshold = supplyAttribute.value()->bottleAlarmThreshold;
	}

	// 警告
	if (residual <= 0)
	{
		return "color: #ff6669;";
	}
	// 提醒
	else if (bottleAlarmThreshold > 0 && residual <= bottleAlarmThreshold)
	{
		return "color: #FA9016;";
	}

	// 正常
	return "color: #565656;";
}
