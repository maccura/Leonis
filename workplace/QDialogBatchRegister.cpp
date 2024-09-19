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
/// @file     QDialogBatchRegister.h
/// @brief    批量输入对话框
///
/// @author   5774/WuHongTao
/// @date     2022年5月12日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月12日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QDialogBatchRegister.h"
#include "ui_QDialogBatchRegister.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/StringUtil.h"
#include "shared/uidef.h"
#include "shared/tipdlg.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "manager/UserInfoManager.h"
#include "manager/DictionaryQueryManager.h"

QDialogBatchRegister::QDialogBatchRegister(QWidget *parent)
	: BaseDlg(parent)
    , m_barMode(false)
{
    ui = new Ui::QDialogBatchRegister();
	ui->setupUi(this);
	SetTitleName(tr("批量输入"));
	ui->ok_btn->setEnabled(false);
    ui->sample_count_radio->setChecked(true);
    ui->sample_stopno->setEnabled(false);

    // 取消按钮
    connect(ui->Cancel_button, &QPushButton::clicked, this, [&]() 
    {
        ui->sample_count_radio->setChecked(true);
        //ui->checkBox->setChecked(false);
        emit ui->sample_count_radio->clicked(); 
        this->close(); 
    });

	connect(ui->ok_btn, SIGNAL(clicked()), this, SLOT(OnOkBtnClicked()));

    auto fSetStatus = [&]()
    {
        if (ui->sample_count_radio->isChecked())
        {
            ui->sample_stopno->setEnabled(false);
            ui->sample_count->setEnabled(true);
        }
        else
        {
            ui->sample_stopno->setEnabled(true);
            ui->sample_count->setEnabled(false);
        }
    };

    // 点击样本数量
	connect(ui->sample_count_radio, &QRadioButton::clicked, this, [=]() 
    {
        fSetStatus();
        // 范围暂定1-200
        ui->sample_count->setValidator(new QRegExpValidator(QRegExp(UI_BATCH), this)); 
        // 刷新okButton
        OnSampleEditChanged("");
    });

    // 点击末位序号按钮
	connect(ui->stop_sample_radio, &QRadioButton::clicked, this, [=]() 
    {
        fSetStatus();
        // 条码模式下是25位
        if (m_barMode)
        {
            ui->sample_stopno->setValidator(new QRegExpValidator(QRegExp(UI_REG_SAMPLE_BAR_NUM), this));
        }
        // 序号模式下12位
        else
        {
            ui->sample_stopno->setValidator(new QRegExpValidator(QRegExp(UI_REG_SAMPLE_NUM), this));
        }

        // 刷新okButton
        OnSampleEditChanged("");
    });

	connect(ui->sample_stopno, SIGNAL(textChanged(const QString &)), this, SLOT(OnSampleEditChanged(const QString &)));
	connect(ui->sample_count, SIGNAL(textChanged(const QString &)), this, SLOT(OnSampleEditChanged(const QString &)));

    // 范围1-200
    ui->sample_count->setValidator(new QRegExpValidator(QRegExp(UI_BATCH), this));
    ui->assayRunTimes->setValidator(new QRegExpValidator(QRegExp(UI_BATCH), this));

    // 权限限制
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);
}

QDialogBatchRegister::~QDialogBatchRegister()
{

}

///
/// @brief 获取终止样本号和样本数量
///
/// @param[in]  seq     样本号
/// @param[in]  count   样本数量
///
/// @return 批量输入模式类型
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月13日，新建函数
///
QDialogBatchRegister::BATCHMODE QDialogBatchRegister::GetBatchInputPatater(cpp_int& stop, int& count, int& times)
{
    stop = m_stop;
	count = m_count;
    times = m_testTimes;

	if (ui->sample_count_radio->isChecked())
	{
		return SAMPLECOUNT;
	}
	else
	{
		return STOPSAMPLENO;
	}
}

///
/// @brief 设置默认结束样本
///
/// @param[in]  seq  样本号
///
/// @return true表示设置成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年10月8日，新建函数
///
bool QDialogBatchRegister::SetSefaultStopNo(cpp_int keyData, bool mode)
{
    // 设置默认值
    ui->sample_count->setText("1");
    ui->assayRunTimes->setText("1");
    ui->sample_stopno->setText(QString::fromStdString(keyData.str()));
    m_barMode = mode;
	// modify bug0011803 by wuht
    m_start = keyData - 1;

    // 序号模式
    if (mode)
    {
        ui->stop_sample_radio->setText(tr("末位样本条码"));
        ui->sample_stopno->setValidator(new QRegExpValidator(QRegExp(UI_REG_SAMPLE_BAR_NUM), this));
    }
    // 条码模式
    else
    {
        ui->stop_sample_radio->setText(tr("末位样本号"));
        ui->sample_stopno->setValidator(new QRegExpValidator(QRegExp(UI_REG_SAMPLE_NUM), this));
    }

    if (!DictionaryQueryManager::GetBatchAddSampleConfig(m_batchAction))
    {
        ULOG(LOG_ERROR, "Failed to get batchadd sample config.");
        close();
        return false;
    }

	// 样本量模式
	if (m_batchAction.SampleCount)
	{
		ui->sample_count_radio->setChecked(true);
		emit ui->sample_count_radio->clicked();
	}
	// 样本末尾模式
	else
	{
		ui->stop_sample_radio->setChecked(true);
		emit ui->stop_sample_radio->clicked();
	}

    return true;
}

///
/// @brief 确定按钮槽函数
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月12日，新建函数
///
void QDialogBatchRegister::OnOkBtnClicked()
{
	auto stopNumber = ui->sample_stopno->text().toStdString();
	if (!stringutil::IsPureDigit(stopNumber))
	{
		TipDlg(tr("末位样本号(条码)不是纯数字")).exec();
		ULOG(LOG_ERROR, "the end seq/barcode is not pure number");
		return;
	}

	m_stop = cpp_int(stopNumber);
	m_count = ui->sample_count->text().toInt();
    m_testTimes = ui->assayRunTimes->text().toInt();

    if (ui->stop_sample_radio->isChecked())
    {
        if (m_stop < m_start)
        {
            TipDlg(tr("结束样本号(条码)不能小于开始样本号(条码)")).exec();
            ULOG(LOG_ERROR, "the start sample > stop sample");
            return;
        }

        auto distance = m_stop - m_start;
        if (distance >= cpp_int(200))
        {
            TipDlg(tr("最大添加样本不能超过200.")).exec();
            ULOG(LOG_ERROR, "have not selected any samples");
            return;
        }
    }

    // 保存用户选择
    m_batchAction.SampleCount = ui->sample_count_radio->isChecked();
    if (!DictionaryQueryManager::SaveBatchAddSampleConfig(m_batchAction))
    {
		ULOG(LOG_ERROR, "Failed to save batch addample config.");
		close();
		return;
	}

	close();
	emit batchParameterInputed();
}

///
/// @brief 批量输入的改变
///
/// @param[in]  text  改变字符
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月13日，新建函数
///
void QDialogBatchRegister::OnSampleEditChanged(const QString& text)
{
	ui->ok_btn->setEnabled(true);
	// 终止样本号按下
	if (ui->stop_sample_radio->isChecked())
	{
		if (ui->sample_stopno->text().isEmpty())
		{
			ui->ok_btn->setEnabled(false);
		}
        else
        {
            ui->ok_btn->setEnabled(true);
        }
	}
	else if (ui->sample_count_radio->isChecked())
	{
		if (ui->sample_count->text().isEmpty())
		{
			ui->ok_btn->setEnabled(false);
		}
        else
        {
            ui->ok_btn->setEnabled(true);
        }
	}
}

///
/// @bref
///		权限变化响应
///
/// @par History:
/// @li 8276/huchunli, 2023年9月22日，新建函数
///
void QDialogBatchRegister::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    // 是否允许重复次数
    if (UserInfoManager::GetInstance()->IsPermisson(PMS_BATCHINPUT_REPEATTIMES))
    {
        ui->label->show();
        ui->assayRunTimes->show();
    }
    else
    {
        ui->label->hide();
        ui->assayRunTimes->hide();
    }
}
