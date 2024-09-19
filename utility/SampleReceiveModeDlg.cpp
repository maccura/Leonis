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
/// @file     SampleReceiveModeDlg.h
/// @brief    样本接收模式弹窗
///
/// @author   7951/LuoXin
/// @date     2023年2月20日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2023年2月20日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "SampleReceiveModeDlg.h"
#include "ui_SampleReceiveModeDlg.h"
#include "shared/tipdlg.h"
#include "shared/CommonInformationManager.h"
#include "manager/DictionaryQueryManager.h"
#include "src/common/Mlog/mlog.h"

SampleReceiveModeDlg::SampleReceiveModeDlg(QWidget *parent)
	: BaseDlg(parent)
	, ui(new Ui::SampleReceiveModeDlg)
{
	ui->setupUi(this);
	BaseDlg::SetTitleName(tr("样本接收模式"));

	connect(ui->save_btn, SIGNAL(clicked()), this, SLOT(OnSaveBtnClicked()));
}

SampleReceiveModeDlg::~SampleReceiveModeDlg()
{

}

void SampleReceiveModeDlg::LoadDataToDlg()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);

	ui->checkBox->setChecked(false);
	ui->comboBox->setCurrentIndex(-1);

    ConfigSampleRecvMode csrm;
    if (!DictionaryQueryManager::GetSampleRecvModelConfig(csrm))
    {
        ULOG(LOG_ERROR, "Failed to get sampler recv model config.");
        return;
    }

	ui->checkBox->setChecked(csrm.bEn);
	// 格式化字符串信息（解决不能显示.5的行）modify by chenjianlin 20230815
	QString strHour = csrm.iIdleTime % 3600==0 ? QString::number(csrm.iIdleTime / 3600) : QString::number(csrm.iIdleTime / 3600.0, 'f', 1);
	// 选择对应的行
    ui->comboBox->setCurrentText(strHour);
}

void SampleReceiveModeDlg::OnSaveBtnClicked()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);

    if (CommonInformationManager::GetInstance()->IsExistDeviceRuning())
    {
        TipDlg(tr("保存失败"), tr("存在运行中的仪器，不能修改样本接收模式")).exec();
        return;
    }

	ConfigSampleRecvMode csrm;
	csrm.bEn = ui->checkBox->isChecked();
	csrm.iIdleTime = ui->comboBox->currentText().toDouble() * 3600;

    // 保存样本架分配
    if (!DictionaryQueryManager::SaveSampleRecvModelConfig(csrm))
    {
        ULOG(LOG_ERROR, "Failed to save sample recv model config.");
        TipDlg(tr("保存失败"), tr("保存失败")).exec();
        return;
    }

	close();
}
