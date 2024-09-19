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
/// @file     QAssaySelectButton.h
/// @brief    项目选择按钮控件
///
/// @author   5774/WuHongTao
/// @date     2022年5月7日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月7日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QAssaySelectButton.h"
#include "ui_QAssaySelectButton.h"
#include "shared/QAssayTabWidgets.h"
#include <QMouseEvent>
#include <QFontMetrics>

const QVariant qvForbidden(QStringLiteral("disable"));
const QVariant qvForbiddenText(QStringLiteral("assay_name_dis"));
const QVariant qvNone(QStringLiteral("none"));
const QVariant qvNormal(QStringLiteral("normal"));
const QVariant qvAssayName(QStringLiteral("assay_name"));

const QVariant qvOk(QStringLiteral("ok"));
const QVariant qvReCheck(QStringLiteral("recheck"));
const QVariant qvWait(QStringLiteral("wait"));

const QVariant item_mask(QStringLiteral("item_mask"));
const QVariant reagentdis(QStringLiteral("reagentdis"));
const QVariant caldis(QStringLiteral("caldis"));
const QVariant dulitionDec(QStringLiteral("dec"));
const QVariant dulitionInc(QStringLiteral("inc"));
const QVariant textFormat(QStringLiteral("assay_name"));
const QVariant dillute(QStringLiteral("dillute"));

const QVariant item_mask_hv(QStringLiteral("item_mask_hv"));
const QVariant reagentdis_hv(QStringLiteral("reagentdis_hv"));
const QVariant caldis_hv(QStringLiteral("caldis_hv"));
const QVariant dulitionDec_hv(QStringLiteral("dec_hv"));
const QVariant dulitionInc_hv(QStringLiteral("inc_hv"));
const QVariant textFormat_hv(QStringLiteral("assay_name_hv"));
const QVariant dillute_hv(QStringLiteral("dillute_hv"));

const QVariant item_mask_dis(QStringLiteral("item_mask_dis"));
const QVariant reagentdis_dis(QStringLiteral("reagentdis_dis"));
const QVariant caldis_dis(QStringLiteral("caldis_dis"));

const QVariant qvEmpty(QStringLiteral(""));

QAssaySelectButton::QAssaySelectButton(QWidget *parent, int indexPos)
    : QWidget(parent)
    , m_indexPos(indexPos)
{
    ui = new Ui::QAssaySelectButton();
    ui->setupUi(this);
    setAttribute(Qt::WA_StyledBackground);
}

QAssaySelectButton::~QAssaySelectButton()
{
}

///
/// @brief 设置按钮属性
///
/// @param[in]  attribute  属性参数
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月7日，新建函数
///
void QAssaySelectButton::SetButtonAttribute(const AssayButtonData& attribute)
{
	/*
    auto assayStatus = QSystemMonitor::GetInstance().GetAssayStatus(attribute.assayCode);
    if (assayStatus && (attribute.assayType == ASSAYTYPE1_WORKPAGE))
    {
        attribute.bIsMask = assayStatus->maskAssay;
        attribute.bIsRegentAbn = assayStatus->abnormalReagent;
        attribute.bIsCaliAbn = assayStatus->abnormalCalibrate;
    }*/

    if (attribute == m_attribute)
    {
        return;
    }

    UpdateButtonStatus(attribute);
    m_attribute = attribute;
    style()->unpolish(this);
    style()->polish(this);
}

///
/// @brief 更新按钮属性
///
/// @param[in]  isUse  是否使用中
/// @param[in]  isProfile  是否组合项目
///
/// @par History:
/// @li 8276/huchunli，2022年10月8日，新建函数
///
void QAssaySelectButton::SetButtonAttribute(bool isUse, bool isProfile)
{
    m_attribute.bIsUse = isUse;
    m_attribute.bIsProfile = isProfile;
    UpdateButtonStatus(m_attribute);

    style()->unpolish(this);
    style()->polish(this);
}

///
/// @brief 清除卡片当前信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月11日，新建函数
///
void QAssaySelectButton::ClearStatus()
{
    ClearButtonStatus();
    m_attribute.assayCode = -1;
    m_attribute.bIsAnalyzed = false;
    m_attribute.bIsRecheck = false;
    m_attribute.bIsRequest = false;
    m_attribute.bIsMask = false;
    m_attribute.bIsRegentAbn = false;
    m_attribute.bIsCaliAbn = false;
    m_attribute.bIsForbidden = false;
}

///
/// @bref
///		置位非选中
///
/// @par History:
/// @li 8276/huchunli, 2023年10月13日，新建函数
///
void QAssaySelectButton::ClearSelected()
{
    if (m_attribute.bIsRequest)
    {
        m_attribute.bIsRequest = false;
        setProperty("state_bks", qvNormal);
        style()->unpolish(this);
        style()->polish(this);
    }
}

int  QAssaySelectButton::ClearContent()
{
    int origAssayCode = m_attribute.assayCode;

    m_attribute.assayCode = -1;
    m_attribute.bIsRequest = false;
    m_attribute.strAssayName.clear();

    ui->assayName_label->setText(m_attribute.strAssayName);
    this->update();

    return origAssayCode;
}

void QAssaySelectButton::ResetText(const QString& strText)
{
    m_attribute.strAssayName = strText;

    ui->assayName_label->setText(m_attribute.strAssayName);
    this->update();
}

///
/// @brief 更新按钮状态
///
/// @param[in]  attribute  按钮状态参数
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月7日，新建函数
/// @li 8276/huchunli, 2022年10月9日，减少QVariant的频繁构造
///
void QAssaySelectButton::UpdateButtonStatus(const AssayButtonData& attribute)
{
    // 清除状态，然后更新
    ClearButtonStatus();
    // 禁用
    if (attribute.bIsForbidden)
    {
        setProperty("state_bks", qvForbidden);	
    }
    // 申请
    else if (attribute.bIsAnalyzed)
    {
        setProperty("state_bks", qvOk);
    }
    // 复查
    else if (attribute.bIsRecheck)
    {
        setProperty("state_bks", qvReCheck);
    }
    // 待测
    else if (attribute.bIsRequest)
    {
        setProperty("state_bks", qvWait);
    }

	// modify bug(3081)
	bool use_hv = (attribute.bIsRecheck || attribute.bIsRequest || attribute.bIsAnalyzed);
	if (attribute.bIsForbidden)
	{
		ui->assayName_label->setProperty("state_texts", qvForbiddenText);
	}
	else
	{
		ui->assayName_label->setProperty("state_texts", use_hv ? textFormat_hv : textFormat);
	}

    ui->assayName_label->style()->unpolish(ui->assayName_label);
    ui->assayName_label->style()->polish(ui->assayName_label);

    // 项目遮蔽
    if (attribute.bIsMask)
    {
		if (attribute.bIsForbidden)
		{
			ui->item_mask->setProperty("states", item_mask_dis);
		}
		else
		{
			ui->item_mask->setProperty("states", use_hv ? item_mask_hv : item_mask);
		}
    }

    if (m_attribute.bIsMask != attribute.bIsMask || m_attribute.bIsForbidden != attribute.bIsForbidden)
    {
        ui->item_mask->style()->unpolish(ui->item_mask);
        ui->item_mask->style()->polish(ui->item_mask);
    }

    // 试剂异常
    if (attribute.bIsRegentAbn)
    {
		if (attribute.bIsForbidden)
		{
			ui->reagentdis->setProperty("states", reagentdis_dis);
		}
		else
		{
			ui->reagentdis->setProperty("states", use_hv ? reagentdis_hv : reagentdis);
		}
    }

    if (m_attribute.bIsRegentAbn != attribute.bIsRegentAbn || m_attribute.bIsForbidden != attribute.bIsForbidden)
    {
        ui->reagentdis->style()->unpolish(ui->reagentdis);
        ui->reagentdis->style()->polish(ui->reagentdis);
    }

    // 校准异常
    if (attribute.bIsCaliAbn)
    {
		if (attribute.bIsForbidden)
		{
			ui->caldis->setProperty("states", caldis_dis);
		}
		else
		{
			ui->caldis->setProperty("states", use_hv ? caldis_hv : caldis);
		}
    }

    if (m_attribute.bIsCaliAbn != attribute.bIsCaliAbn || m_attribute.bIsForbidden != attribute.bIsForbidden)
    {
        ui->caldis->style()->unpolish(ui->caldis);
        ui->caldis->style()->polish(ui->caldis);
    }

    // 减量
    if (attribute.enSampleSize == DEC_SIZE)
    {
        ui->dillute_label->setProperty("states", use_hv ? dulitionDec_hv : dulitionDec);
    }
    // 增量
    else if (attribute.enSampleSize == INC_SIZE)
    {
        ui->dillute_label->setProperty("states", use_hv ? dulitionInc_hv : dulitionInc);
    }
    // 自动稀释和手动稀释
    else if (attribute.enSampleSize == USER_SIZE
        || attribute.enSampleSize == MANUAL_SIZE)
    {
        ui->dillute_label->setProperty("state_texts", use_hv ? textFormat_hv : textFormat);
        if (attribute.iDilution != 1)
        {
            ui->dillute_label->setText(QString::number(attribute.iDilution));
        }
    }
    else
    {
        ui->dillute_label->setProperty("states", qvEmpty);
    }
    ui->dillute_label->style()->unpolish(ui->dillute_label);
    ui->dillute_label->style()->polish(ui->dillute_label);
	// modify for bug2185 by wuht
	int width = ui->assayName_label->fontMetrics().width(attribute.strAssayName);
	if (width > ui->assayName_label->width())
	{
		ui->assayName_label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	}
	else
	{
		ui->assayName_label->setAlignment(Qt::AlignCenter);
	}

	// 设置名称
	ui->assayName_label->setText(attribute.strAssayName);

    // 如果名字过长，添加提示
    QFontMetrics fm(ui->assayName_label->fontMetrics());
    if (fm.width(attribute.strAssayName) > ui->assayName_label->width())
    {
        this->setToolTip(attribute.strAssayName);
    }
}

///
/// @brief 清除项目按钮的状态
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月7日，新建函数
///
void QAssaySelectButton::ClearButtonStatus()
{
    setProperty("state_bks", qvNormal);
    setProperty("state_texts", qvAssayName);

    ui->item_mask->setProperty("states", qvNone);
    ui->reagentdis->setProperty("states", qvNone);
    ui->caldis->setProperty("states", qvNone);
    ui->dillute_label->setProperty("states", qvNone);
    ui->dillute_label->setText(QStringLiteral(""));
}

///
/// @brief 重写鼠标按下事件
///
/// @param[in]  event  鼠标事件
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月11日，新建函数
///
void QAssaySelectButton::mousePressEvent(QMouseEvent *event)
{
    // 若是未使用，则返回
	/*
    if (!m_attribute.bIsUse)
    {
        return;
    }
	*/

    // 按下左键，发送当前位置
    if (event->button() == Qt::LeftButton)
    {
        emit select(m_indexPos);
		emit select(m_indexPos, m_attribute.assayCode);
    }

    QWidget::mousePressEvent(event);
}

AssayButtonData::AssayButtonData()
{
    bIsProfile = false;
    bIsRegentAbn = false;
    bIsCaliAbn = false;
    bIsMask = false;
    bIsAnalyzed = false;
    bIsRecheck = false;
    bIsRequest = false;
    bIsUse = false;
    bIsForbidden = false;
    buttonType = NORAML_BUTTON;
    iDilution = 1;
    assayCode = -1;
    testTimes = 0;
    enSampleSize = STD_SIZE;
    preSampleSize = STD_SIZE;
    assayType = ASSAYTYPE3_POSITION_SET;
    preiDilution = 0;
    guidList = {};
    strAssayName = "";
}
