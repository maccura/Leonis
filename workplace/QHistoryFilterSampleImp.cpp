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
/// @file     QHistoryFilterSampleImp.cpp
/// @brief    筛选对话框
///
/// @author   5774/WuHongTao
/// @date     2022年5月31日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月31日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QHistoryFilterSampleImp.h"
#include "ui_QHistoryFilterSampleImp.h"
#include "src/public/SerializeUtil.hpp"
#include "src/public/DictionaryKeyName.h"
#include "src/public/ConfigSerialize.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/common.h"

#include "utility/QGroupAssayPositionSettings.h"
#include "thrift/DcsControlProxy.h"
#include "manager/DictionaryQueryManager.h"
#include "shared/CommonInformationManager.h"
#include "shared/SwitchButton.h"
#include "shared/tipdlg.h"
#include "shared/msgiddef.h"
#include "shared/messagebus.h"
#include "shared/ThriftEnumTransform.h"
#include "WorkpageCommon.h"

QHistoryFilterSampleImp::QHistoryFilterSampleImp(QWidget *parent, int pageIdx)
	: QWidget(parent), m_pageIdx(pageIdx)
{
    ui = new Ui::QHistoryFilterSampleImp();
	ui->setupUi(this);
	Init();

	// Modify bug0012035  by wuht
	ui->lineEdit_name->setMaxLength(10);

    // 根据标志决定是否显示名称编辑框
    bool bShowNameEdit = (pageIdx != 0);
    ui->widget_fillname->setVisible(bShowNameEdit);
    ui->save_1->setVisible(bShowNameEdit);
	// modify for bug12330/12326 
	ui->save_1->setVisible(false);

    if (bShowNameEdit)
    {
        ui->lineEdit_name->setText(tr("快捷筛选设置%1").arg(pageIdx));
    }
}

QHistoryFilterSampleImp::~QHistoryFilterSampleImp()
{
}

///
/// @brief 初始化
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月31日，新建函数
///
void QHistoryFilterSampleImp::Init()
{
	// ui->assay_select_btn->setEnabled(false);
	m_assaySelectDialog = nullptr;
	// modify bug3449 by wuht
	ui->assay_edit->setReadOnly(true);
	// 设置样本号的范围;(BUG0024116：不需要限制格式)
    //ui->sample_no_edit->setValidator(new QRegExpValidator(QRegExp(UI_REG_SAMPLE_NUM), this));
    //ui->sample_no_edit_2->setValidator(new QRegExpValidator(QRegExp(UI_REG_SAMPLE_NUM), this));
    ui->sample_no_edit->setMaxLength(12);
    ui->sample_no_edit_2->setMaxLength(12);
	// 设置样本条码的范围;(BUG0024116：不需要限制格式)
	//ui->sample_bar_edit->setValidator(new QRegExpValidator(QRegExp(UI_REG_SAMPLE_BAR_ASCII), this));
	//ui->sample_bar_edit_2->setValidator(new QRegExpValidator(QRegExp(UI_REG_SAMPLE_BAR_ASCII), this));
    ui->sample_bar_edit->setMaxLength(25);
    ui->sample_bar_edit_2->setMaxLength(25);

	// bug2567 modify by wuhongtao，限制大小长度
	ui->patient_record_edit->setMaxLength(50);
	ui->patient_name_edit->setMaxLength(50);
	ui->lineEditReagentLot->setMaxLength(50);

	// 如果是免疫单机，则不显示包膜腔积液样本类型
	if (CommonInformationManager::GetInstance()->GetSoftWareType() == CHEMISTRY)
	{
		ui->pushButton_positive->hide();
		ui->pushButton_negtive->hide();
		ui->label_determine->hide();

	}
	// 生化不需要显示的情况
	else
	{
		ui->pushButton_positive->show();
		ui->pushButton_negtive->show();
		ui->label_determine->show();
	}

	if (CommonInformationManager::GetInstance()->GetSoftWareType() != IMMUNE)
	{
		ui->pushButton_innerincrease->show();
	}
	else
	{
		ui->pushButton_innerincrease->hide();
	}

	bool isAi = false;
	// 只有在联机模式下才有AI
	if (DictionaryQueryManager::GetInstance()->GetPipeLine())
	{
		isAi = m_workSet.aiRecognition;
	}

	if (isAi)
	{
		ui->pushButton_except->show();
		ui->pushButton_noexcept->show();
		ui->label_airecognition->show();
	}
	else
	{
		ui->pushButton_except->hide();
		ui->pushButton_noexcept->hide();
		ui->label_airecognition->hide();
	}

	// 保存信号
	connect(ui->save_1, &QPushButton::clicked, this, &QHistoryFilterSampleImp::SignalSaveFastFilter);

	// 工作页面的设置更新
	REGISTER_HANDLER(MSG_ID_WORK_PAGE_SET_DISPLAY_UPDATE, this, OnUpdateButtonStatus);

	OnUpdateButtonStatus();
	// 项目选择对话框
	connect(ui->assay_select_btn, SIGNAL(clicked()), this, SLOT(OnAssaySelect()));
	OnReset();

    // 记录默认条件,用于是否变化的判断
    GetQueryConditon(m_cond);
}

/// @brief
///
/// @param[in]  qryCond
///
/// @return
///
/// @par History:
/// @li 7702/WangZhongXin，2023年3月14日，新建函数
///
bool QHistoryFilterSampleImp::GetQueryConditon(::tf::HistoryBaseDataQueryCond &qryCond) const
{
	qryCond.__set_filterIndex(m_pageIdx);
	// 是否启用
    bool isUse = (m_pageIdx != 0 && ui->checkBox_filter_enable->isChecked());
    qryCond.__set_isUsed(isUse);

	if (!ui->lineEdit_name->text().isEmpty())
	{
		qryCond.__set_filterName(ui->lineEdit_name->text().toStdString());
	}

    // 样本号
    QString strSeqNoS = ui->sample_no_edit->text();
    QString strSeqNoE = ui->sample_no_edit_2->text();
    if (!strSeqNoS.isEmpty() || !strSeqNoE.isEmpty())
    {
        qryCond.__set_seqNoList(std::vector<std::string>{ strSeqNoS.toStdString(), strSeqNoE.toStdString()});
    }

    // 样本条码
    QString strSampleBarS = ui->sample_bar_edit->text();
    QString strSampleBarE = ui->sample_bar_edit_2->text();
    if (!strSampleBarS.isEmpty() || !strSampleBarE.isEmpty())
    {
        qryCond.__set_sampleBarcodeList(std::vector<std::string>{ strSampleBarS.toStdString(), strSampleBarE.toStdString()});
    }
	
	if (!ui->comboBoxDevName->currentText().isEmpty())
	{
		// 暂时认为模块号和设备编号等同
		qryCond.__set_moduleList(std::vector<std::string>{ui->comboBoxDevName->currentData().toString().toStdString()});
	}
	if (!ui->patient_record_edit->text().isEmpty())
	{
		qryCond.__set_caseNo(ui->patient_record_edit->text().toStdString());
	}
	if (!ui->patient_name_edit->text().isEmpty())
	{
		qryCond.__set_patientName(ui->patient_name_edit->text().toStdString());
	}
	if (!ui->lineEditReagentLot->text().isEmpty())
	{
		qryCond.__set_reagentLot(ui->lineEditReagentLot->text().toStdString());
	}
	// 项目通道号
	if (!m_vecAssayCode.empty())
	{
		qryCond.__set_assayCodeList(m_vecAssayCode);
	}
	// 订单类型
	{
		std::vector<::tf::HisSampleType::type> sampleTypeList;
		if (ui->pushButton_normal->isChecked()) // 常规
		{
			sampleTypeList.push_back(::tf::HisSampleType::type::SAMPLE_SOURCE_NM);
		}
		if (ui->pushButton_urg->isChecked()) // 急诊
		{
			sampleTypeList.push_back(::tf::HisSampleType::type::SAMPLE_SOURCE_EM);
		}
		if (ui->pushButton_cali->isChecked()) // 校准
		{
			sampleTypeList.push_back(::tf::HisSampleType::type::SAMPLE_SOURCE_CL);
		}
		if (ui->pushButton_qc->isChecked()) // 质控
		{
			sampleTypeList.push_back(::tf::HisSampleType::type::SAMPLE_SOURCE_QC);
		}
		if (!sampleTypeList.empty())
		{
			qryCond.__set_sampleTypeList(sampleTypeList);
		}
	}
	// 样本类型
	{
		std::vector<::tf::SampleSourceType::type> sampleSourceTypeList;
		if (ui->pushButton_xqxj->isChecked()) // 血清/血浆
		{
			sampleSourceTypeList.push_back(::tf::SampleSourceType::type::SAMPLE_SOURCE_TYPE_XQXJ);
		}
		if (ui->pushButton_qx->isChecked()) // 全血
		{
			sampleSourceTypeList.push_back(::tf::SampleSourceType::type::SAMPLE_SOURCE_TYPE_QX);
		}
		if (ui->pushButton_ny->isChecked()) // 尿液
		{
			sampleSourceTypeList.push_back(::tf::SampleSourceType::type::SAMPLE_SOURCE_TYPE_NY);
		}
		if (ui->pushButton_jmqjy->isChecked()) // 浆膜腔积液
		{
			sampleSourceTypeList.push_back(::tf::SampleSourceType::type::SAMPLE_SOURCE_TYPE_JMQJY);
		}
		if (ui->pushButton_njy->isChecked()) // 脑脊液
		{
			sampleSourceTypeList.push_back(::tf::SampleSourceType::type::SAMPLE_SOURCE_TYPE_NJY);
		}
		if (ui->pushButton_qt->isChecked()) // 其他
		{
			sampleSourceTypeList.push_back(::tf::SampleSourceType::type::SAMPLE_SOURCE_TYPE_OTHER);
		}
		if (!sampleSourceTypeList.empty())
		{
			qryCond.__set_sampleSourceTypeList(sampleSourceTypeList);
		}
	}
	// 复查
	{
		std::vector<int> retestList;
		if (ui->pushButton_haveretest->isChecked())
		{
			retestList.push_back(1);
		}
		if (ui->pushButton_noretest->isChecked())
		{
			retestList.push_back(0);
		}
		if (!retestList.empty())
		{
			qryCond.__set_retestList(retestList);
		}
        if (!ui->test_start_time->text().isEmpty() && !ui->test_stop_time->text().isEmpty())
        {
            bool haveRestest = ui->pushButton_haveretest->isChecked();
            bool noRetest = ui->pushButton_noretest->isChecked();
            std::string timeStart = ui->test_start_time->dateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString();
            std::string timeStop = ui->test_stop_time->dateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString();

            // 用于保存快捷筛选到数据库
            qryCond.__set_reTestStartTime(timeStart);
            qryCond.__set_startTime(timeStart);
            qryCond.__set_endtime(timeStop);
            // 仅勾选复查时时间为复查时间
            if (haveRestest && !noRetest)
            {
                qryCond.__set_reTestStartTime(timeStart);
                qryCond.__set_reTestEndtime(timeStop);
            }
            // 都勾选或都不勾选时间为开始测试时间到复查结束时间
            else if (!(haveRestest ^ noRetest))
            {
                qryCond.__set_startTime(timeStart);
                qryCond.__set_reTestEndtime(timeStop);
            }
        }
	}
	// 报警
	{
		std::vector<int> warnList;
		if (ui->pushButton_havealarm->isChecked())
		{
			warnList.push_back(1);
		}
		if (ui->pushButton_noalarm->isChecked())
		{
			warnList.push_back(0);
		}
		if (!warnList.empty())
		{
			qryCond.__set_warnList(warnList);
		}
	}
	// 审核
	{
		std::vector<int> checkList;
		if (ui->pushButton_verified->isChecked())
		{
			checkList.push_back(1);
		}
		if (ui->pushButton_notverify->isChecked())
		{
			checkList.push_back(0);
		}
		if (!checkList.empty())
		{
			qryCond.__set_checkList(checkList);
		}
	}
	// 手工稀释
	{
		std::vector<::tf::HisDataDiluteStatu::type> diluteStatuList;
		if (ui->pushButton_notdilute->isChecked()) // 未稀释
		{
			diluteStatuList.push_back(::tf::HisDataDiluteStatu::type::SAMPLE_SOURCE_NONE);
		}
		if (ui->pushButton_innerdilute->isChecked()) // 机内稀释
		{
			diluteStatuList.push_back(::tf::HisDataDiluteStatu::type::SAMPLE_SOURCE_AUTO);
		}
		if (ui->pushButton_innerincrease->isChecked()) // 机内增量
		{
			diluteStatuList.push_back(::tf::HisDataDiluteStatu::type::SAMPLE_SOURCE_INCREASE);
		}
		if (ui->pushButton_manualdilute->isChecked()) // 手工稀释
		{
			diluteStatuList.push_back(::tf::HisDataDiluteStatu::type::DILUTE_STATU_MANUAL);
		}
		if (!diluteStatuList.empty())
		{
			qryCond.__set_diluteStatuList(diluteStatuList);
		}
	}
	// 定性结果
	{
		std::vector<int> qualitativeResultList;
		if (ui->pushButton_positive->isChecked()) // 阳性
		{
			qualitativeResultList.push_back(::tf::QualJudge::Positive);
		}
		if (ui->pushButton_negtive->isChecked()) // 阴性
		{
			qualitativeResultList.push_back(::tf::QualJudge::Negative);
		}
		if (!qualitativeResultList.empty())
		{
			qryCond.__set_qualitativeResultList(qualitativeResultList);
		}
	}
	// 是否上传LIS
	{
		std::vector<int> sendLISList;
		if (ui->pushButton_nottrans->isChecked()) // 未传输
		{
			sendLISList.push_back(0);
		}
		if (ui->pushButton_trans->isChecked()) // 已传输
		{
			sendLISList.push_back(1);
		}
		if (!sendLISList.empty())
		{
			qryCond.__set_sendLISList(sendLISList);
		}
	}
	// 打印
	{
		std::vector<int> printList;
		if (ui->pushButton_alreadyprint->isChecked()) // 已打印
		{
			printList.push_back(1);
		}
		if (ui->pushButton_notprint->isChecked()) // 未打印
		{
			printList.push_back(0);
		}
		if (!printList.empty())
		{
			qryCond.__set_printList(printList);
		}
	}
	return true;
}

///
///
/// @brief 设置当前查询条件
///
/// @param[in]  qryCond  查询条件
///
/// @return
///
/// @par History:
/// @li 7702/WangZhongXin，2023年4月13日，新建函数
///
void QHistoryFilterSampleImp::SetQueryConditon(const ::tf::HistoryBaseDataQueryCond &qryCond)
{
	if (!qryCond.__isset.filterIndex || qryCond.filterIndex != m_pageIdx)
	{
		return;
	}

	m_cond = qryCond;

	OnReset();
	// 是否启用
	if (qryCond.__isset.isUsed)
	{
		ui->checkBox_filter_enable->setChecked(qryCond.isUsed);
	}
	// 筛选名称
	if (qryCond.__isset.filterName)
	{
		ui->lineEdit_name->setText(QString::fromStdString(qryCond.filterName));
	}
	// 样本序号
	if (qryCond.__isset.seqNoList && qryCond.seqNoList.size() >= 2)
	{
		ui->sample_no_edit->setText(QString::fromStdString(qryCond.seqNoList[0]));
		ui->sample_no_edit_2->setText(QString::fromStdString(qryCond.seqNoList[1]));
	}
	// 样本条码
	if (qryCond.__isset.sampleBarcodeList && qryCond.sampleBarcodeList.size() >= 2)
	{
		ui->sample_bar_edit->setText(QString::fromStdString(qryCond.sampleBarcodeList[0]));
		ui->sample_bar_edit_2->setText(QString::fromStdString(qryCond.sampleBarcodeList[1]));
	}
	// 模块号
	if (qryCond.__isset.moduleList && qryCond.moduleList.size() >= 1)
	{
		const auto deviceSN = qryCond.moduleList[0];
		auto it = std::find_if(m_mapDevices.cbegin(), m_mapDevices.cend(), [&](const std::pair<int, std::vector<::tf::DeviceInfo>> &p)
							   {
			for (const auto &d : p.second)
			{
				if (d.deviceSN == deviceSN)
				{
					return true;
				}
			}
			return false; });
		// 未找到该模块
		if (it == m_mapDevices.cend())
		{
			return;
		}
		ui->comboBoxDevType->setCurrentIndex(ui->comboBoxDevType->findData(it->first));
		ui->comboBoxDevName->setCurrentIndex(ui->comboBoxDevName->findData(QString::fromStdString(deviceSN)));
	}
	// 病历号
	if (qryCond.__isset.caseNo)
	{
		ui->patient_record_edit->setText(QString::fromStdString(qryCond.caseNo));
	}
	// 病人名称
	if (qryCond.__isset.patientName)
	{
		ui->patient_name_edit->setText(QString::fromStdString(qryCond.patientName));
	}
	// 试剂批号
	if (qryCond.__isset.reagentLot)
	{
		ui->lineEditReagentLot->setText(QString::fromStdString(qryCond.reagentLot));
	}
	// 项目通道号
	m_vecAssayCode.clear();
	if (qryCond.__isset.assayCodeList && qryCond.assayCodeList.size() >= 1)
	{
		QStringList strAssayNames;
		for (auto assayCode : qryCond.assayCodeList)
		{
			std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(assayCode);
			if (spAssayInfo == nullptr)
			{
				continue;
			}
			m_vecAssayCode.push_back(assayCode);
            strAssayNames.append(QString::fromStdString(spAssayInfo->assayName));
		}
        SetAssayEdit(strAssayNames.join(tr("、")));
	}
	// 订单类型
	if (qryCond.__isset.sampleTypeList)
	{
		for (auto type : qryCond.sampleTypeList)
		{
			switch ((::tf::HisSampleType::type)type)
			{
			case ::tf::HisSampleType::type::SAMPLE_SOURCE_NM:
				ui->pushButton_normal->setChecked(true);
				break;
			case ::tf::HisSampleType::type::SAMPLE_SOURCE_EM:
				ui->pushButton_urg->setChecked(true);
				break;
			case ::tf::HisSampleType::type::SAMPLE_SOURCE_CL:
				ui->pushButton_cali->setChecked(true);
				break;
			case ::tf::HisSampleType::type::SAMPLE_SOURCE_QC:
				ui->pushButton_qc->setChecked(true);
				break;
			default:
				break;
			}
		}
	}
	// 样本类型
	if (qryCond.__isset.sampleSourceTypeList)
	{
		for (auto type : qryCond.sampleSourceTypeList)
		{
			switch ((::tf::SampleSourceType::type)type)
			{
			case ::tf::SampleSourceType::type::SAMPLE_SOURCE_TYPE_XQXJ:
				ui->pushButton_xqxj->setChecked(true);
				break;
			case ::tf::SampleSourceType::type::SAMPLE_SOURCE_TYPE_QX:
				ui->pushButton_qx->setChecked(true);
				break;
			case ::tf::SampleSourceType::type::SAMPLE_SOURCE_TYPE_NY:
				ui->pushButton_ny->setChecked(true);
				break;
			case ::tf::SampleSourceType::type::SAMPLE_SOURCE_TYPE_JMQJY:
				ui->pushButton_jmqjy->setChecked(true);
				break;
			case ::tf::SampleSourceType::type::SAMPLE_SOURCE_TYPE_NJY:
				ui->pushButton_njy->setChecked(true);
				break;
			case ::tf::SampleSourceType::type::SAMPLE_SOURCE_TYPE_OTHER:
				ui->pushButton_qt->setChecked(true);
				break;
			default:
				break;
			}
		}
	}
	// 复查
	if (qryCond.__isset.retestList)
	{
		for (auto type : qryCond.retestList)
		{
			switch (type)
			{
			case 1:
				ui->pushButton_haveretest->setChecked(true);
				break;
			case 0:
				ui->pushButton_noretest->setChecked(true);
				break;
			default:
				break;
			}
		}
	}
	// 时间
    {
        bool haveRestest = qryCond.__isset.retestList && qryCond.retestList.size() > 0 && qryCond.retestList[0] == 1;
        bool noRetest = qryCond.__isset.retestList && qryCond.retestList.size() > 1 && qryCond.retestList[1] == 0;
        QString strTimeBegin = QString::fromStdString(qryCond.startTime);
        QString strTimeEnd = QString::fromStdString(qryCond.endtime);

        // 仅勾选复查时时间为复查时间
        if (haveRestest && !noRetest)
        {
            strTimeBegin = QString::fromStdString(qryCond.reTestStartTime);
            strTimeEnd = QString::fromStdString(qryCond.reTestEndtime);
        }
        // 都勾选或都不勾选时间为开始测试时间到复查结束时间
        else if (!(haveRestest ^ noRetest))
        {
            strTimeBegin = QString::fromStdString(qryCond.startTime);
            strTimeEnd = QString::fromStdString(qryCond.reTestEndtime);
        }
        if (!strTimeBegin.isEmpty())
        {
            ui->test_start_time->setDateTime(QDateTime::fromString(strTimeBegin, "yyyy-MM-dd hh:mm:ss"));
        }
        if (!strTimeEnd.isEmpty())
        {
            ui->test_stop_time->setDateTime(QDateTime::fromString(strTimeEnd, "yyyy-MM-dd hh:mm:ss"));
        }
    }
	// 报警
	if (qryCond.__isset.warnList && qryCond.warnList.size() >= 1)
	{
		for (auto type : qryCond.warnList)
		{
			switch (type)
			{
			case 1:
				ui->pushButton_havealarm->setChecked(true);
				break;
			case 0:
				ui->pushButton_noalarm->setChecked(true);
				break;
			default:
				break;
			}
		}
	}
	// 审核
	if (qryCond.__isset.checkList && qryCond.checkList.size() >= 1)
	{
		for (auto type : qryCond.checkList)
		{
			switch (type)
			{
			case 1:
				ui->pushButton_verified->setChecked(true);
				break;
			case 0:
				ui->pushButton_notverify->setChecked(true);
				break;
			default:
				break;
			}
		}
	}
	// 手工稀释
	if (qryCond.__isset.diluteStatuList)
	{
		for (auto type : qryCond.diluteStatuList)
		{
			switch ((::tf::HisDataDiluteStatu::type)type)
			{
			case ::tf::HisDataDiluteStatu::type::SAMPLE_SOURCE_NONE:
				ui->pushButton_notdilute->setChecked(true);
				break;
			case ::tf::HisDataDiluteStatu::type::SAMPLE_SOURCE_AUTO:
				ui->pushButton_innerdilute->setChecked(true);
				break;
			case ::tf::HisDataDiluteStatu::type::SAMPLE_SOURCE_INCREASE:
				ui->pushButton_innerincrease->setChecked(true);
				break;
			case ::tf::HisDataDiluteStatu::type::DILUTE_STATU_MANUAL:
				ui->pushButton_manualdilute->setChecked(true);
				break;
			default:
				break;
			}
		}
	}
	// 定性结果
	if (qryCond.__isset.qualitativeResultList)
	{
		for (int type : qryCond.qualitativeResultList)
		{
			switch (type)
			{
			case ::tf::QualJudge::Positive:
				ui->pushButton_positive->setChecked(true);
				break;
			case ::tf::QualJudge::Negative:
				ui->pushButton_negtive->setChecked(true);
				break;
			default:
				break;
			}
		}
	}
	// 是否上传LIS
	if (qryCond.__isset.sendLISList)
	{
		for (auto type : qryCond.sendLISList)
		{
			switch (type)
			{
			case 1:
				ui->pushButton_trans->setChecked(true);
				break;
			case 0:
				ui->pushButton_nottrans->setChecked(true);
				break;
			default:
				break;
			}
		}
	}
	// 打印
	if (qryCond.__isset.printList)
	{
		for (auto type : qryCond.printList)
		{
			switch (type)
			{
			case 1:
				ui->pushButton_alreadyprint->setChecked(true);
				break;
			case 0:
				ui->pushButton_notprint->setChecked(true);
				break;
			default:
				break;
			}
		}
	}
}

///
/// @brief 更新工作页面的按钮状态
///
///
/// @par History:
/// @li 7702/WangZhongXin，2023年9月5日，新建函数
///
void QHistoryFilterSampleImp::UpdateButtonStatus(const PageSet& workSet)
{
	// 审核
	ui->label_verify->setVisible(workSet.audit);
	for (int index = 0; index < ui->horizontalLayout_verify->count(); ++index)
	{
		auto item = ui->horizontalLayout_verify->itemAt(index);
		auto widget = item->widget();
		if (widget == nullptr)
		{
			continue;
		}
		widget->setVisible(workSet.audit);
	}
	// 打印
	ui->label_selectprint->setVisible(workSet.print);
	for (int index = 0; index < ui->horizontalLayout_selectprint->count(); ++index)
	{
		auto item = ui->horizontalLayout_selectprint->itemAt(index);
		auto widget = item->widget();
		if (widget == nullptr)
		{
			continue;
		}
		widget->setVisible(workSet.print);
	}
}

bool QHistoryFilterSampleImp::IsWannaSaveChanged() const
{
    tf::HistoryBaseDataQueryCond tempCond;
    if (GetQueryConditon(tempCond))
    {
        if (!CompearFilterCondition(tempCond, m_cond))
        {
            TipDlg noticeDlg(tr("是否保存当前设置？"), TipDlgType::TWO_BUTTON);
            if (noticeDlg.exec() != QDialog::Rejected)
            {
                return true;
            }
        }
    }

    return false;
}

///
/// @brief 设备类型改变的槽函数
///
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年8月16日，新建函数
///
void QHistoryFilterSampleImp::OnDeviceChanged()
{
	ui->comboBoxDevName->clear();
	auto userData = ui->comboBoxDevType->currentData().toInt();
	if (m_mapDevices.count(userData) == 0)
	{
		return;
	}

    // 定性按钮显示与否
    auto funcDetermineRsStat = [this](bool showDetCtrl) {
        ui->label_determine->setVisible(showDetCtrl);
        for (int i = 0; i < ui->horizontalLayout_determine->count(); ++i)
        {
            auto item = ui->horizontalLayout_determine->itemAt(i);
            auto widget = item->widget();
            if (widget != nullptr)
            {
                widget->setVisible(showDetCtrl);
                if (!showDetCtrl && qobject_cast<QPushButton*>(widget) != nullptr)
                {
                    qobject_cast<QPushButton*>(widget)->setChecked(false);
                }
            }
        }
    };

	// 免疫的样本类型无浆膜腔积液
	if (ui->comboBoxDevType->currentData().toInt() == ::tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE)
	{
        SetSampleTypeBtnVisble(false);

		// 免疫有定性结果
        funcDetermineRsStat(true);
	}
	else
    {
        SetSampleTypeBtnVisble(true);

		// 生化无定性结果
        funcDetermineRsStat(false);
	}

	auto &devices = m_mapDevices[ui->comboBoxDevType->currentData().toInt()];
	for (auto device : devices)
	{
		if (device.deviceClassify == ui->comboBoxDevType->currentData().toInt())
		{
			auto name = QString::fromStdString(device.groupName + device.name);
			ui->comboBoxDevName->addItem(name, QString::fromStdString(device.deviceSN));
		}
	}
	if (ui->comboBoxDevName->count() > 0)
	{
		ui->comboBoxDevName->insertItem(0, "", "");
	}
}

///
/// @brief 项目选择对应的槽函数
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月31日，新建函数
///
void QHistoryFilterSampleImp::OnAssaySelect()
{
	if (m_assaySelectDialog != nullptr)
	{
        delete m_assaySelectDialog;
        m_assaySelectDialog = nullptr;
	}

    m_assaySelectDialog = new QGroupAssayPositionSettings(Q_NULLPTR, DlgTypeAssaySelect);
    connect(m_assaySelectDialog, SIGNAL(selected()), this, SLOT(OnSelectAssay()));

    QString strAssays = ui->assay_edit->toPlainText();
    if (!strAssays.isEmpty())
    {
        m_assaySelectDialog->UpdateButtonCheckStat(strAssays.split(tr("、")));
    }

	m_assaySelectDialog->show();
}

///
/// @brief 处理项目选择信息
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月1日，新建函数
///
void QHistoryFilterSampleImp::OnSelectAssay()
{
	if (m_assaySelectDialog == nullptr)
	{
		return;
	}

    QStringList lstAssayNames;
    std::set<int> setAssayCodes;
    m_assaySelectDialog->GetSelectedAssayNames(lstAssayNames, setAssayCodes);

	// 依次显示控件
	m_vecAssayCode.clear();
	for (auto item : setAssayCodes)
	{
		m_vecAssayCode.push_back(item);
    }
    SetAssayEdit(lstAssayNames.join(tr("、")));
}

///
/// @brief 保存筛选参数
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月31日，新建函数
///
bool QHistoryFilterSampleImp::VerifyParameter()
{
	// 时刻在未知时候被改成0了，这里先再次重置
	ui->test_start_time->setTime(QTime(0, 0, 0));
	ui->test_stop_time->setTime(QTime(23, 59, 59));

	if (ui->widget_fillname->isVisible() && ui->lineEdit_name->text().isEmpty())
	{
		TipDlg(tr("提示"), tr("快捷筛选名称不能为空，请重新设置！")).exec();
		return false;
	}

    if (ui->test_start_time->text().isEmpty() || ui->test_stop_time->text().isEmpty())
    {
        TipDlg(tr("提示"), tr("必须同时设置开始和结束时间！")).exec();
        return false;
    }
	if (ui->test_start_time->dateTime() > ui->test_stop_time->dateTime())
	{
		TipDlg(tr("提示"), tr("检测日期结束时间早于开始时间，请重新设置！")).exec();
		return false;
	}
	auto ContainsNonDigitFunc = [](const QString& str)->bool {
		QRegularExpression regex("[^0-9]");
		return str.contains(regex);
	};

	// 输入范围时只能输入数字
	if (!ui->sample_no_edit->text().isEmpty() && !ui->sample_no_edit_2->text().isEmpty())
	{
		if (ContainsNonDigitFunc(ui->sample_no_edit->text()) || ContainsNonDigitFunc(ui->sample_no_edit_2->text()))
		{
			// 最新方案不再需要该限制 add 备by wzx-20240124
			//TipDlg(tr("提示"), tr("范围查询时样本条码只能为数字")).exec();
			//return false;
		}
		else
		{
			auto bar1 = ui->sample_no_edit->text().toUInt();
			auto bar2 = ui->sample_no_edit_2->text().toUInt();
			if (bar1 > bar2)
			{
				TipDlg(tr("提示"), tr("开始样本号大于结束样本号")).exec();

                // bugfix：0026451，清空填写的值
                ui->sample_no_edit->clear();
                ui->sample_no_edit_2->clear();

				return false;
			}
		}
	}
	if (!ui->sample_bar_edit->text().isEmpty() && !ui->sample_bar_edit_2->text().isEmpty())
	{
		if (ContainsNonDigitFunc(ui->sample_bar_edit->text()) || ContainsNonDigitFunc(ui->sample_bar_edit_2->text()))
		{
			// 最新方案不再需要该限制 add 备by wzx-20240124
			//TipDlg(tr("提示"), tr("范围查询时样本条码只能为数字")).exec();
			//return false;
		}
		else
		{
			auto bar1 = ui->sample_bar_edit->text().toUInt();
			auto bar2 = ui->sample_bar_edit_2->text().toUInt();
			if (bar1 > bar2)
			{
				TipDlg(tr("提示"), tr("开始样本条码大于结束样本条码")).exec();

                ui->sample_bar_edit->clear();
                ui->sample_bar_edit_2->clear();

				return false;
			}
		}
	}
#if 0 // bugfix: 0012211: [工作-1000速]“启用筛选”框未启用，设置筛选条件后点击“确定”按钮，界面自动关闭，需要进行筛选
	// 不是快捷查询时才发送信号查询数据库
	if (m_pageIdx == 0 || ui->checkBox_filter_enable->isChecked())
#endif
	{
		emit finished();
	}
	return true;
}

QString QHistoryFilterSampleImp::GetFilterName() const
{
    return ui->lineEdit_name->text();
}

///
/// @brief 重置按钮槽函数
///
///
/// @return
///
/// @par History:
/// @li 7702/WangZhongXin，2023年3月14日，新建函数
///
void QHistoryFilterSampleImp::OnReset()
{
	auto tab = this;
	// 恢复界面可勾选按钮状态
	QList<QPushButton *> childButtons = tab->findChildren<QPushButton *>(QString(), Qt::FindChildrenRecursively);
	for (auto btn : childButtons)
	{
		if (btn->isCheckable())
		{
			btn->setChecked(false);
		}
	}
	// 恢复界面输入框
	QList<QLineEdit *> childLineEdits = tab->findChildren<QLineEdit *>(QString(), Qt::FindChildrenRecursively);
	for (auto ed : childLineEdits)
	{
		// 避免清除时间输入框
		if (qobject_cast<QDateTimeEdit *>(ed->parent()))
		{
			continue;
		}
		ed->clear();
	}
	// 时间输入框设置为当前时间
	QList<QDateTimeEdit *> childs = tab->findChildren<QDateTimeEdit *>(QString(), Qt::FindChildrenRecursively);
	for (auto ed : childs)
	{
		ed->setDate(QDate::currentDate());
	}
	ui->test_start_time->setTime(QTime(0, 0, 0));
	ui->test_stop_time->setTime(QTime(23, 59, 59));

	// 恢复模块下拉框
	m_mapDevices.clear();
	// 恢复查询勾选框
	ui->checkBox_filter_enable->setChecked(false);
	::tf::DeviceInfoQueryResp retDevs;
	::tf::DeviceInfoQueryCond dIqcDevs;
	// 获取DCS客户端对象
	std::shared_ptr<DcsControlProxy> spCilent = DcsControlProxy::GetInstance();
	if (spCilent == nullptr)
	{
		return;
	}
	if (!spCilent->QueryDeviceInfo(retDevs, dIqcDevs))
	{
		return;
	}
	std::set<int> deviceTypes;
	for (const tf::DeviceInfo& device : retDevs.lstDeviceInfos)
	{
		m_mapDevices[device.deviceClassify].push_back(device);
		deviceTypes.insert(device.deviceClassify);
	}

	disconnect(ui->comboBoxDevType, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &QHistoryFilterSampleImp::OnDeviceChanged);
	ui->comboBoxDevType->clear();
	for (int deviceType : deviceTypes)
	{
        ::tf::AssayClassify::type curAssayType = (::tf::AssayClassify::type)deviceType;
        if (curAssayType == ::tf::AssayClassify::type::ASSAY_CLASSIFY_OTHER)
        {
            continue;
        }
        ui->comboBoxDevType->addItem(ThriftEnumTrans::GetAssayClassfiyName(curAssayType), curAssayType);
	}

	// 存在设备时添加空白设备项，当只有一种设备时默认选中该设备，多种设备时选中空白项
	if (ui->comboBoxDevType->count() == 1)
	{
		ui->comboBoxDevType->insertItem(0, "", -1);
		ui->comboBoxDevType->setCurrentIndex(1);
	}
	else if (ui->comboBoxDevType->count() > 1)
	{
		ui->comboBoxDevType->insertItem(0, "", -1);
		ui->comboBoxDevType->setCurrentIndex(0);
	}

	connect(ui->comboBoxDevType, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &QHistoryFilterSampleImp::OnDeviceChanged);
	OnDeviceChanged();

    QString strFilterName = m_cond.__isset.filterName ? QString::fromStdString(m_cond.filterName) : tr("快捷筛选设置%1").arg(m_pageIdx);
    ui->lineEdit_name->setText(strFilterName);

	// 清除项目信息中的项目
	if (m_assaySelectDialog != Q_NULLPTR)
	{
		m_vecAssayCode.clear();
		m_assaySelectDialog->ClearSelectedAssays();
        SetAssayEdit("");
	}
}

///
/// @brief 根据应用设置更新按钮状态
///
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2024年1月31日，新建函数
///
void QHistoryFilterSampleImp::OnUpdateButtonStatus()
{
	// 构造查询条件
	::tf::DictionaryInfoQueryCond   qryCond;
	::tf::DictionaryInfoQueryResp   qryResp;
	qryCond.__set_keyName(DKN_WORKPAGE_SET);

	// 查询
	if (!DcsControlProxy::GetInstance()->QueryDictionaryInfo(qryResp, qryCond)
		|| qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS
		|| qryResp.lstDictionaryInfos.empty())
	{
		ULOG(LOG_ERROR, "QueryDictionaryInfo Failed");
		return;
	}

	if (!DecodeJson(m_workSet, qryResp.lstDictionaryInfos[0].value))
	{
		ULOG(LOG_ERROR, "DecodeJson workSet Failed");
		return;
	}

	// 根据审核、打印是否启用决定是否显示
	// 打印
	for (auto item : ui->verticalLayout_selectprint->children())
	{
		auto widget = qobject_cast<QWidget*>(item);
		if (widget == nullptr)
		{
			continue;
		}
		if (m_workSet.print)
		{
			widget->show();
		}
		else
		{
			widget->hide();
		}
	}
	// 审核
	for (auto item : ui->verticalLayout_verify->children())
	{
		auto widget = qobject_cast<QWidget*>(item);
		if (widget == nullptr)
		{
			continue;
		}
		if (m_workSet.audit)
		{
			widget->show();
		}
		else
		{
			widget->hide();
		}
	}
}

///
/// @bref
///		设置‘项目名’编辑框
///
/// @param[in] strAssays 项目名字符串
///
/// @par History:
/// @li 8276/huchunli, 2023年9月4日，新建函数
///
void QHistoryFilterSampleImp::SetAssayEdit(const QString& strAssays)
{
    ui->assay_edit->setText(strAssays);
    ui->assay_edit->setToolTip(strAssays);

    if (m_assaySelectDialog != nullptr && strAssays.isEmpty())
    {
        m_assaySelectDialog->ClearSelectedAssays();
    }
}

bool QHistoryFilterSampleImp::CompearFilterCondition(tf::HistoryBaseDataQueryCond src1, tf::HistoryBaseDataQueryCond src2) const
{
    const int dataLen = 10; // yyyy-MM-dd 长度
    src1.startTime.resize(dataLen);
    src1.endtime.resize(dataLen);
    src1.reTestStartTime.resize(dataLen);
    src1.reTestEndtime.resize(dataLen);

    src2.startTime.resize(dataLen);
    src2.endtime.resize(dataLen);
    src2.reTestStartTime.resize(dataLen);
    src2.reTestEndtime.resize(dataLen);

    return src1 == src2;
}

void QHistoryFilterSampleImp::SetSampleTypeBtnVisble(bool showBmqjy)
{
    std::vector<QPushButton*> sampleTypeBtns = {
        ui->pushButton_xqxj,
        ui->pushButton_qx,
        ui->pushButton_ny,
        ui->pushButton_jmqjy,
        ui->pushButton_njy,
        ui->pushButton_qt
    };

    WorkpageCommon::SetSampleTypeGridBtnsVisble(sampleTypeBtns, ui->gridLayout_samplytype, 3, showBmqjy);
}

void QHistoryFilterSampleImp::showEvent(QShowEvent *event)
{
    // 如果是免疫单机，则不显示包膜腔积液样本类型
    if (CommonInformationManager::GetInstance()->GetSoftWareType() == IMMUNE)
    {
        SetSampleTypeBtnVisble(false);
    }

	bool isAi = false;
	// 只有在联机模式下才有AI
	if (DictionaryQueryManager::GetInstance()->GetPipeLine())
	{
		isAi = m_workSet.aiRecognition;
	}

	if (isAi)
	{
		ui->pushButton_except->show();
		ui->pushButton_noexcept->show();
		ui->label_airecognition->show();
	}
	else
	{
		ui->pushButton_except->hide();
		ui->pushButton_noexcept->hide();
		ui->label_airecognition->hide();
	}

	QWidget::showEvent(event);
}

void QHistoryFilterSampleImp::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
}
