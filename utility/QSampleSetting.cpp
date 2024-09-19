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
/// @file     QSampleSetting.cpp
/// @brief    在应用-显示设置-样本信息界面
///
/// @author   1556/Chenjianlin
/// @date     2023年8月31日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 1556/Chenjianlin，2023年8月31日，添加注释
///
///////////////////////////////////////////////////////////////////////////
#include "QSampleSetting.h"
#include "ui_QSampleSetting.h"
#include "shared/uidef.h"
#include "shared/tipdlg.h"
#include "shared/uicommon.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/CommonInformationManager.h"
#include "manager/DictionaryQueryManager.h"
#include "src/common/Mlog/mlog.h"
#include <QPushButton>

QSampleSetting::QSampleSetting(QWidget *parent)
    : QWidget(parent)
{
    ui = new Ui::QSampleSetting();
    ui->setupUi(this);
    Init();
}

QSampleSetting::~QSampleSetting()
{
}

bool QSampleSetting::isExistUnsaveData()
{
    std::vector<int> saveData;
    // 默认样本管类型
    saveData.push_back(ui->sampleTubes->currentData().toInt());
    // 默认样本类型
    saveData.push_back(ui->sampleType->currentData().toInt());

    return DictionaryQueryManager::GetInstance()->GetSampleShowSet() != saveData;   
}

///
/// @brief 界面初始化
///
///
/// @return 
///
/// @par History:
/// @li 1556/Chenjianlin，2023年8月31日，bug修改，0021915: [应用] 显示设置-样本管类型增加“样本贮存管”
///
void QSampleSetting::Init()
{
    auto vtClassify = CommonInformationManager::GetInstance()->GetAllDeviceClassify();
    bool bOnlyImm = true;
    for (auto classifi : vtClassify)
    {
        if (classifi == tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE || classifi == tf::AssayClassify::ASSAY_CLASSIFY_OTHER)
        {
            continue;
        }
        bOnlyImm = false;
        break;
    }
    //若为仅有免疫设备则不显示浆膜积腔液
    if (bOnlyImm)
    {
        AddTfEnumItemToComBoBox(ui->sampleType, ::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_XQXJ);
        AddTfEnumItemToComBoBox(ui->sampleType, ::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_QX);
        AddTfEnumItemToComBoBox(ui->sampleType, ::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_NY);
        AddTfEnumItemToComBoBox(ui->sampleType, ::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_NJY);
        AddTfEnumItemToComBoBox(ui->sampleType, ::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_OTHER);
    }
    else
    {
        // 初始化样本源类型下拉框
        AddTfEnumItemToComBoBox(ui->sampleType, ::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_XQXJ);
        AddTfEnumItemToComBoBox(ui->sampleType, ::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_QX);
        AddTfEnumItemToComBoBox(ui->sampleType, ::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_NY);
        AddTfEnumItemToComBoBox(ui->sampleType, ::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_NJY);
        AddTfEnumItemToComBoBox(ui->sampleType, ::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_JMQJY);
        AddTfEnumItemToComBoBox(ui->sampleType, ::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_OTHER);
    }

    // 初始化样本杯下拉框
    AddTfEnumItemToComBoBox(ui->sampleTubes, ::tf::TubeType::TUBE_TYPE_NORMAL);
    AddTfEnumItemToComBoBox(ui->sampleTubes, ::tf::TubeType::TUBE_TYPE_MICRO);
    AddTfEnumItemToComBoBox(ui->sampleTubes, ::tf::TubeType::TUBE_TYPE_ORIGIN);
	AddTfEnumItemToComBoBox(ui->sampleTubes, ::tf::TubeType::TUBE_TYPE_STORE);

    ui->sampleTubes->setCurrentIndex(0);
    ui->sampleType->setCurrentIndex(0);

    // 保存
    connect(ui->save_Button, &QPushButton::clicked, this, [&]()
    {
        OnSaveData();
        emit this->closeDialog();
    });

	UpdateComBoxInfo();
}

///
/// @brief 更新样本管和样本类型的默认类型
///
///
/// @par History:
/// @li 5774/WuHongTao，2023年10月20日，新建函数
///
void QSampleSetting::UpdateComBoxInfo()
{
	std::vector<int> savedata = DictionaryQueryManager::GetInstance()->GetSampleShowSet();

	// 默认样本管类型
	if (savedata.size() > 0)
	{
		if (savedata[0] > 0)
		{
			ui->sampleTubes->setCurrentIndex(savedata[0] - 1);
		}
	}

	// 默认样本类型
	if (savedata.size() > 1)
	{
		if (savedata[1] == 0)
		{
			ui->sampleType->setCurrentIndex(5);
		}
		else
		{
			ui->sampleType->setCurrentIndex(savedata[1] - 1);
		}
	}
}

// modify by  wuht for bug3453
void QSampleSetting::showEvent(QShowEvent *event)
{
	UpdateComBoxInfo();
	QWidget::showEvent(event);
}

void QSampleSetting::OnSaveData()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    const auto& CIM = CommonInformationManager::GetInstance();
    if (CIM->IsExistDeviceRuning())
    {
        TipDlg(tr("保存失败"), tr("仪器在运行中不能修改样本信息")).exec();
        return;
    }

    std::vector<int> saveData;
    // 默认样本管类型
    saveData.push_back(ui->sampleTubes->currentData().toInt());
    // 默认样本类型
    saveData.push_back(ui->sampleType->currentData().toInt());

    if (!DictionaryQueryManager::SaveSampleDefaultShowType(saveData))
    {
        ULOG(LOG_ERROR, "Failed to save sampledefault showtype.");
    }
    else
    {
        TipDlg(tr("提示"), tr("保存成功")).exec();
        POST_MESSAGE(MSG_ID_SAMPLE_SHOWSET_UPDATE);
    }
}
