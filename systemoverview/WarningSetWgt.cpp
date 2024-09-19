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
/// @file     WarningSetWgt.cpp
/// @brief    报警设置界面源文件
///
/// @author   7997/XuXiaoLong
/// @date     2023年7月13日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7997/XuXiaoLong，2023年7月13日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "WarningSetWgt.h"
#include "ui_WarningSetWgt.h"
#include "src/common/Mlog/mlog.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "manager/WarnSetManager.h"
#include "manager/DictionaryQueryManager.h"
#include "manager/UserInfoManager.h"
#include <QCheckBox>

#define		COLUMN		2			//< CheckBox列数

WarningSetWgt::WarningSetWgt(QWidget *parent)
	: BaseDlg(parent)
{
    ui = new Ui::WarningSetWgt();
	ui->setupUi(this);

	initUI();
	initCheckBoxs();
	initConnect();
}

WarningSetWgt::~WarningSetWgt()
{

}

void WarningSetWgt::initUI()
{
	// 设置标题
	SetTitleName(tr("报警设置"));

	// 初始化所有CheckBox
	QStringList ise_SupplyLists = QStringList() << tr("开瓶过期") << tr("未校准") << tr("耗材过期") << tr("校准失败")
		<< tr("余量不足") << tr("校准结果过期") << tr("余量为0") << tr("耗材失控");

	QStringList ch_im_ReagentLists = QStringList() << tr("开瓶过期") << tr("校准失败") << tr("试剂过期")
		<< tr("瓶校准曲线过期") << tr("项目余量不足") << tr("批校准曲线过期") << tr("项目余量为0") << tr("试剂失控") << tr("未校准");

	QStringList ch_im_SupplyLists = QStringList() << tr("开瓶过期") << tr("余量不足") << tr("耗材过期") << tr("余量为0");

	QStringList ch_im_LiquidLists = QStringList() << tr("废液量达报警值") << tr("废液桶满");

	QStringList im_TrashCanLists = QStringList() << tr("废料量达报警值") << tr("废料桶满");

	// 试剂
	for (int i = 0; i < ch_im_ReagentLists.size(); i++)
	{
		QCheckBox *ch_ckBox = new QCheckBox(ch_im_ReagentLists.at(i), this);
		QCheckBox *im_ckBox = new QCheckBox(ch_im_ReagentLists.at(i), this);
		m_chRBoxs.push_back(ch_ckBox);
		m_imRBoxs.push_back(im_ckBox);
		ui->gridLayout_CH_R->addWidget(ch_ckBox, i / COLUMN, i % COLUMN);
		ui->gridLayout_IM_R->addWidget(im_ckBox, i / COLUMN, i % COLUMN);
	}

	// 耗材
	for (int i = 0; i < ise_SupplyLists.size(); i++)
	{
		QCheckBox *ise_ckBox = new QCheckBox(ise_SupplyLists.at(i), this);
		m_iseBoxs.push_back(ise_ckBox);
		ui->gridLayout_ISE->addWidget(ise_ckBox, i / COLUMN, i % COLUMN);
	}
	for (int i = 0; i < ch_im_SupplyLists.size(); i++)
	{
		QCheckBox *ch_ckBox = new QCheckBox(ch_im_SupplyLists.at(i), this);
		QCheckBox *im_ckBox = new QCheckBox(ch_im_SupplyLists.at(i), this);
		m_chSBoxs.push_back(ch_ckBox);
		m_imSBoxs.push_back(im_ckBox);
		ui->gridLayout_CH_S->addWidget(ch_ckBox, i / COLUMN, i % COLUMN);
		ui->gridLayout_IM_S->addWidget(im_ckBox, i / COLUMN, i % COLUMN);
	}

	// 废液
	for (const auto &text : ch_im_LiquidLists)
	{
		QCheckBox *ch_ckBox = new QCheckBox(text, this);
		QCheckBox *im_ckBox = new QCheckBox(text, this);
		m_chLBoxs.push_back(ch_ckBox);
		m_imLBoxs.push_back(im_ckBox);
		ui->verticalLayout_CH_L->addWidget(ch_ckBox);
		ui->verticalLayout_IM_L->addWidget(im_ckBox);
	}

	// 废料
	for (const auto &text : im_TrashCanLists)
	{
		QCheckBox *im_ckBox = new QCheckBox(text, this);
		m_imTBoxs.push_back(im_ckBox);
		ui->verticalLayout_IM_T->addWidget(im_ckBox);
	}
}

void WarningSetWgt::initConnect()
{
	// 确定
	connect(ui->btn_sure, &QPushButton::clicked, this, &WarningSetWgt::slotClickSure);
	// 取消
	connect(ui->btn_cancel, &QPushButton::clicked, this, &WarningSetWgt::close);
}

void WarningSetWgt::initCheckBoxs()
{
    m_cacheWarns.Clear();
	if (!DictionaryQueryManager::GetInstance()->GetWarnsSet(m_cacheWarns))
	{
		ULOG(LOG_ERROR, "Failed to get warnset from DB.");
		return;
	}

	// 更新界面
	updateUI();
}

void WarningSetWgt::updateUI()
{
	// 试剂
	for (int i = 0; i < m_chRBoxs.size(); i++)
	{
		m_chRBoxs[i]->setChecked(m_cacheWarns.chReagentCheckBoxs[i]);
		m_imRBoxs[i]->setChecked(m_cacheWarns.imReagentCheckBoxs[i]);
	}

	// ISE耗材
	for (int i = 0; i < m_iseBoxs.size(); i++)
	{
		m_iseBoxs[i]->setChecked(m_cacheWarns.iseSupplyCheckBoxs[i]);
	}

	// 生化/免疫耗材
	for (int i = 0; i < m_chSBoxs.size(); i++)
	{
		m_chSBoxs[i]->setChecked(m_cacheWarns.chSupplyCheckBoxs[i]);
		m_imSBoxs[i]->setChecked(m_cacheWarns.imSupplyCheckBoxs[i]);
	}

	// 废液
	for (int i = 0; i < m_chLBoxs.size(); i++)
	{
		m_chLBoxs[i]->setChecked(m_cacheWarns.chLiquidWasteCheckBoxs[i]);
		m_imLBoxs[i]->setChecked(m_cacheWarns.imLiquidWasteCheckBoxs[i]);
	}

	// 废料
	for (int i = 0; i < m_imTBoxs.size(); i++)
	{
		m_imTBoxs[i]->setChecked(m_cacheWarns.imTrashCanCheckBoxs[i]);
	}
}

void WarningSetWgt::setWarnVisible(const bool ise, const bool ch, const bool im, const bool chWaste, const bool imWaste)
{
    int iVisibleTabCnt = 3;
	if (!im)
	{
		ui->tabWidget->removeTab(2);
        --iVisibleTabCnt;
	}

	if (!ch)
	{
		ui->tabWidget->removeTab(1);
        --iVisibleTabCnt;
	}

	if (!ise)
	{
		ui->tabWidget->removeTab(0);
        --iVisibleTabCnt;
	}

    // tab数小于等于1则不显示tabbar
    if (iVisibleTabCnt <= 1)
    {
        ui->tabWidget->tabBar()->setVisible(false);
    }

	if (!chWaste)
	{
		ui->label_chWaste->hide();
		ui->verticalWidget_chWaste->hide();
	}

	if (!imWaste)
	{
		ui->label_imWaste->hide();
		ui->verticalWidget_imWaste->hide();
	}
}

void WarningSetWgt::showEvent(QShowEvent *event)
{
	updateUI();

    bool bEditPermit = UserInfoManager::GetInstance()->IsPermisson(PSM_MONITOR_ALARMSETTINGS);

    ui->ISE->setEnabled(bEditPermit);
    ui->widgetCH->setEnabled(bEditPermit);
    ui->widgetIM->setEnabled(bEditPermit);
}

void WarningSetWgt::slotClickSure()
{
	SystemOverviewWarnSet data;

	for (int i = 0; i < m_chRBoxs.size(); i++)
	{
		data.chReagentCheckBoxs[i] = m_chRBoxs[i]->isChecked();
		data.imReagentCheckBoxs[i] = m_imRBoxs[i]->isChecked();
	}

	for (int i = 0; i < m_iseBoxs.size(); i++)
	{
		data.iseSupplyCheckBoxs[i] = m_iseBoxs[i]->isChecked();
	}

	for (int i = 0; i < m_chSBoxs.size(); i++)
	{
		data.chSupplyCheckBoxs[i] = m_chSBoxs[i]->isChecked();
		data.imSupplyCheckBoxs[i] = m_imSBoxs[i]->isChecked();
	}

	for (int i = 0; i < m_chLBoxs.size(); i++)
	{
		data.chLiquidWasteCheckBoxs[i] = m_chLBoxs[i]->isChecked();
		data.imLiquidWasteCheckBoxs[i] = m_imLBoxs[i]->isChecked();
	}

	for (int i = 0; i < m_imTBoxs.size(); i++)
	{
		data.imTrashCanCheckBoxs[i] = m_imTBoxs[i]->isChecked();
	}

    if (!DictionaryQueryManager::SaveWarnsSet(data))
    {
        ULOG(LOG_ERROR, "Failed to save warnsset config.");
        this->close();
        return;
    }

	m_cacheWarns = data;

	// 通知报警设置管理器，关注的报警设置列表更新
	POST_MESSAGE(MSG_ID_SYSTEM_OVERVIEW_WARNSET_UPDATE,data);

	this->close();
}
