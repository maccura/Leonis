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

#include "QDialogAi.h"
#include "ui_QDialogAi.h"
#include "shared/CommonInformationManager.h"
#include "thrift/DcsControlProxy.h"
#include "bcy_ime_common.h"

QDialogAi::QDialogAi(QWidget *parent)
	: BaseDlg(parent)
{
    ui = new Ui::QDialogAi();
	ui->setupUi(this);
	SetTitleName(tr("视觉识别"));
	connect(ui->Cancel_button, &QPushButton::clicked, this, [&]() {this->close(); });
	connect(ui->ok_btn, &QPushButton::clicked, this, [&]() {this->close(); });
}

QDialogAi::~QDialogAi()
{
}

void QDialogAi::Clear()
{
	ui->label->setText(tr("图片"));
	ui->label_2->setText(tr("无"));
	ui->label_3->setText(tr("无"));
	ui->label_4->setText(tr("无"));
	ui->label_5->setText(tr("无"));
	ui->label_6->setText(tr("无"));
	ui->label_7->setText(tr("无"));
	ui->label_8->setText(tr("无"));
	ui->label_9->setText(tr("无"));
	ui->label_10->setText(tr("无"));
}

QString QDialogAi::ConvertColor(int colorType)
{
	QString colorName = "";
	if (colorType >= TubeHatColor::TUBE_HAT_UNKNOWN || colorType < 0)
	{
		return colorName;
	}

	auto hatColor = TubeHatColor(colorType);

	switch (hatColor)
	{
		case TUBE_HAT_RED:
			colorName = tr("红色");
			break;
		case TUBE_HAT_BLUE:
			colorName = tr("蓝色");
			break;
		case TUBE_HAT_BLACK:
			colorName = tr("黑色");
			break;
		case TUBE_HAT_PURPLE:
			colorName = tr("紫色");
			break;
		case TUBE_HAT_GRAY:
			colorName = tr("灰色");
			break;
		case TUBE_HAT_ORANGE:
			colorName = tr("橙色");
			break;
		case TUBE_HAT_YELLOW:
			colorName = tr("黄色");
			break;
		case TUBE_HAT_WHITE:
			colorName = tr("白色");
			break;
		case TUBE_HAT_UNKNOWN:
			colorName = tr("未知");
			break;
		default:
			colorName = tr("未知");
			break;
	}

	return colorName;
}

QString QDialogAi::ConvertPattern(int patternType)
{
	QString patternName = "";
	if (patternType > specimen_object_pattern::kSEPCIMEN_BLOOD_COLLECTION_TUBE || patternType < 0)
	{
		return patternName;
	}

	auto pattern = specimen_object_pattern(patternType);

	switch (pattern)
	{
		case kSEPCIMEN_ObjectPatternUnknown:
			patternName = tr("未知");
			break;
		case kSEPCIMEN_ObjectPattern13_100mm:
			patternName = tr("13*100");
			break;
		case kSEPCIMEN_HITACHI_MICRO_TUBE:
			patternName = tr("日立微量杯");
			break;
		case kSEPCIMEN_QUALITY_CONTROL_TUBE:
			patternName = tr("质控品管");
			break;
		case kSEPCIMEN_HITACHI_STANDARD_TUBE:
			patternName = tr("日立标准杯");
			break;
		case kSEPCIMEN_BLOOD_COLLECTION_TUBE:
			patternName = tr("采血管");
			break;
		default:
			patternName = tr("未知");
			break;
	}

	return patternName;
}

void QDialogAi::SetSample(const std::shared_ptr<tf::SampleInfo>& spSample)
{
	if (spSample == Q_NULLPTR)
	{
		return;
	}

	if (!spSample->__isset.aiRecognizeResultId)
	{
		return;
	}

	if (spSample->aiRecognizeResultId <= 0)
	{
		return;
	}

	SetAiRecognizeId(spSample->aiRecognizeResultId);
}

void QDialogAi::SetAiRecognizeId(const qint64 aiRecognizeId)
{
	Clear();
	::tf::AiRecognizeResultQueryCond qryCond;
	qryCond.__set_id(aiRecognizeId);
	::tf::AiRecognizeResultQueryResp qryResp;

	// 执行查询条件
	if (!DcsControlProxy::GetInstance()->QueryAiRecognizeResult(qryResp, qryCond)
		|| qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS
		|| qryResp.lstAiRecognizeResult.empty())
	{
		return;
	}

	auto& tubeInfo = qryResp.lstAiRecognizeResult[0];
	if (tubeInfo.__isset.isCentrifuaged)
	{
		auto centriState = tubeInfo.isCentrifuaged ? QString(tr("已离心")) : QString(tr("未离心"));
		ui->label_2->setText(centriState);
	}

	if (tubeInfo.__isset.tubeHatColor)
	{
		ui->label_3->setText(ConvertColor(tubeInfo.tubeHatColor));
	}

	if (tubeInfo.__isset.hasHat)
	{
		auto haveCap = tubeInfo.hasHat ? QString(tr("有")) : QString(tr("无"));
		ui->label_4->setText(haveCap);
	}

	if (tubeInfo.__isset.pattern)
	{
		ui->label_5->setText(ConvertPattern(tubeInfo.pattern));
	}


	if (tubeInfo.__isset.height && tubeInfo.__isset.width)
	{
		auto width = QString::number(tubeInfo.width);
		auto height = QString::number(tubeInfo.height);
		ui->label_6->setText(width + "*" + height);
	}

	if (tubeInfo.__isset.serumQuantity)
	{
		auto serumQuantity = tubeInfo.serumQuantity * 10000 + "ml";
		ui->label_7->setText(serumQuantity);
	}


	if (tubeInfo.__isset.icterusLevel)
	{
		int start = tubeInfo.icterusLevel - tf::QualJudge::CH_SIND_LEVEL1;
		auto status = CommonInformationManager::GetInstance()->GetResultStatusLHIbyCode(::ch::tf::g_ch_constants.ASSAY_CODE_L, start);
		ui->label_8->setText(status);
	}

	if (tubeInfo.__isset.hemolysisLevel)
	{
		int start = tubeInfo.icterusLevel - tf::QualJudge::CH_SIND_LEVEL1;
		auto status = CommonInformationManager::GetInstance()->GetResultStatusLHIbyCode(::ch::tf::g_ch_constants.ASSAY_CODE_H, start);
		ui->label_9->setText(QString::number(tubeInfo.hemolysisLevel));
	}

	if (tubeInfo.__isset.lipemiaLevel)
	{
		int start = tubeInfo.icterusLevel - tf::QualJudge::CH_SIND_LEVEL1;
		auto status = CommonInformationManager::GetInstance()->GetResultStatusLHIbyCode(::ch::tf::g_ch_constants.ASSAY_CODE_I, start);
		ui->label_10->setText(QString::number(tubeInfo.lipemiaLevel));
	}

	// 图片
	if (tubeInfo.__isset.picPath)
	{
		ui->label->setPixmap(QPixmap(tubeInfo.picPath.c_str()).scaled(ui->label->size(), Qt::KeepAspectRatio));
	}
}
