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

#include "QSampleResultSetting.h"
#include "ui_QSampleResultSetting.h"
#include "shared/tipdlg.h"
#include "shared/uidef.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "src/common/Mlog/mlog.h"
#include "manager/DictionaryQueryManager.h"
#include <QPushButton>

QSampleResultSetting::QSampleResultSetting(QWidget *parent)
    : QWidget(parent)
{
    ui = new Ui::QSampleResultSetting();
    ui->setupUi(this);
    Init();
}

QSampleResultSetting::~QSampleResultSetting()
{
}

bool QSampleResultSetting::isExistUnsaveData()
{
    SampleShowSet saveData;
    saveData.QCShowConc = ui->concSampleButton->isChecked();
    saveData.CaliShowConc = ui->concCalibrateButton->isChecked();
    saveData.enableFlag = ui->editFlagEnable->isChecked();
    saveData.editflag = ui->editFlagEdit->text().toStdString();

    return !(DictionaryQueryManager::GetInstance()->GetSampleResultSet() == saveData);    
}

void QSampleResultSetting::Init()
{
    // 保存
    connect(ui->save_Button, &QPushButton::clicked, this, [&]()
    {
        OnSaveData();
        emit this->closeDialog();
    });

	UpdateInfo();
}

// modfiy bug3452 by wuht
void QSampleResultSetting::showEvent(QShowEvent *event)
{
	UpdateInfo();
}

void QSampleResultSetting::UpdateInfo()
{
    SampleShowSet savedata = DictionaryQueryManager::GetInstance()->GetSampleResultSet();
	// 样本质控的结果显示样式
	{
		if (savedata.QCShowConc)
		{
			ui->concSampleButton->setChecked(true);
			ui->siganlSampleButton->setChecked(false);
		}
		else
		{
			ui->concSampleButton->setChecked(false);
			ui->siganlSampleButton->setChecked(true);
		}
	}

	// 校准样本的结果显示样式
	{
		if (savedata.CaliShowConc)
		{
			ui->concCalibrateButton->setChecked(true);
			ui->siganlCalibrateButton->setChecked(false);
		}
		else
		{
			ui->concCalibrateButton->setChecked(false);
			ui->siganlCalibrateButton->setChecked(true);
		}
	}

    // 是否启用结果修改标志
    ui->editFlagEnable->setChecked(savedata.enableFlag);

	// 修改标识
	ui->editFlagEdit->setText(QString::fromStdString(savedata.editflag));

	// 隐藏启用样结果修改标识控件
	ui->editFlagEnable->hide();
	ui->editFlagEdit->hide();
}

void QSampleResultSetting::OnSaveData()
{
    SampleShowSet saveData;
    saveData.QCShowConc = ui->concSampleButton->isChecked();
    saveData.CaliShowConc = ui->concCalibrateButton->isChecked();
    saveData.enableFlag = ui->editFlagEnable->isChecked();
    saveData.editflag = ui->editFlagEdit->text().toStdString();

    if (!DictionaryQueryManager::SaveSampleShowSet(saveData))
    {
        ULOG(LOG_ERROR, "ModifyDictionaryInfo Failed");
    }
    else
    {
        TipDlg(tr("提示"), tr("保存成功")).exec();
        POST_MESSAGE(MSG_ID_SAMPLE_SHOWSET_UPDATE);
    }
}
