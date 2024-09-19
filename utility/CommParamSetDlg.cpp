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
/// @file     CommParamSetDlg.cpp
/// @brief    应用--系统--通讯设置
///
/// @author   7951/LuoXin
/// @date     2022年10月13日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2022年10月13日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "CommParamSetDlg.h"
#include "ui_CommParamSetDlg.h" 
#include <QFileDialog>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include "shared/uidef.h"
#include "shared/tipdlg.h"
#include "shared/uicommon.h"
#include "shared/CommonInformationManager.h"
#include "manager/DictionaryQueryManager.h"
#include "src/thrift/gen-cpp/defs_types.h"
#include "src/common/Mlog/mlog.h"

// 当发生错误时，弹出消息框后退出当前函数
#define ReportErr(bErr, msg) \
if ((bErr))\
{\
	TipDlg(msg).exec();\
	return false;\
}

CommParamSetDlg::CommParamSetDlg(QWidget *parent)
	: BaseDlg(parent)
	, ui(new Ui::CommParamSetDlg)
{
	ui->setupUi(this);
	InitCtrls();
}

CommParamSetDlg::~CommParamSetDlg()
{

}

void CommParamSetDlg::LoadDataToCtrls()
{
	CommParamSet cps;
	if (!DictionaryQueryManager::GetCommParamSet(cps))
	{
		ULOG(LOG_ERROR, "Failed to get communicat paramset.");
		return;
	}

	// TCP设置
	ui->tcp_set_radiobtn->setChecked(cps.bEnabledTCP);
	// 服务器ip
	ui->service_ip_edit->setText(QString::fromStdString(cps.strServiceIP));
	// 服务器Port
	if (cps.iServicePort > 0 && cps.iServicePort <= 65535)
	{
		ui->service_port_edit->setText(QString::number(cps.iServicePort));
	}

	// 串口设置
	ui->seriial_port_set_radiobtn->setChecked(cps.bEnabledSerialPort);
	// 串口
	ui->seriial_port_cbox->setCurrentText(QString::fromStdString(cps.strSerialName));
	// 数据位
	ui->data_bit_cbox->setCurrentText(QString::number(cps.iByteSize));
	// 停止位
	ui->stop_bit_cbox->setCurrentIndex(cps.iStopBits);
	// 波特率
	ui->baud_tate_cbox->setCurrentText(QString::number(cps.iBaudRate));
	// 奇偶校验
	ui->parity_check_cbox->setCurrentIndex(cps.iParity);

    // 文件路径
    ui->file_path_radiobtn->setChecked(cps.bEnabledFile);
    // 样本请求路径
    ui->sample_req_path_edit->setText(QString::fromStdString(cps.strSampleReqPath));
    ui->sample_req_path_edit->setToolTip(QString::fromStdString(cps.strSampleReqPath));
    // 结果路径)
    ui->test_rlt_path_edit->setText(QString::fromStdString(cps.strSampleRltPath));
    ui->test_rlt_path_edit->setToolTip(QString::fromStdString(cps.strSampleReqPath));
    // 样本项目路径
    ui->sample_assay_path_edit->setText(QString::fromStdString(cps.strSampleAssayPath));
    ui->sample_assay_path_edit->setToolTip(QString::fromStdString(cps.strSampleReqPath));

	// 通讯协议
    QString commProtocol = (cps.iCommProtocol == ::tf::LisProtocolType::HL7)? "HL7": "ASTM";
	ui->comm_protocol_cbox->setCurrentText(commProtocol);
	// 接收样本号
	ui->receive_sample_num_ckbox->setChecked(cps.bReceiveSampleNum);
	// 接收样本信息
	ui->receive_sample_info_ckbox->setChecked(cps.bIntroduceSample);
	// 自动上传结果
	ui->auto_upload_results_gbox->setChecked(cps.bUploadResults);
	ui->radioButton->setChecked(!cps.bUploadAfterAudit);	// 检测完成上传更新界面状态 add by chenjialin 20230815
    ui->radioButton_2->setChecked(cps.bUploadAfterAudit);   // 审核完成上传更新界面状态
	// 样本编号自动上传
	ui->auto_upload_sample_num_ckbox->setChecked(cps.bUploadSampleNum);
	// 接收复查样本
	ui->receive_recheck_sample_ckbox->setChecked(cps.bReceiveLis);
    // 通讯名称
    ui->comm_params_name->setText(QString::fromStdString(cps.strSendName));
    // 通讯id
    ui->comm_params_id->setText(QString::fromStdString(cps.strSendId));

	// 等待时间
	if (cps.iWaitTime >= 0)
	{
		ui->wait_time_edit->setText(QString::number(cps.iWaitTime));
	}

	// 最大传输次数
	if (cps.iMaxTransTimes >= 0)
	{
		ui->max_trans_times_edit->setText(QString::number(cps.iMaxTransTimes));
	}

    // 通讯连接打开，禁止编辑
    for (auto& box : findChildren<QGroupBox*>())
    {
        box->setEnabled(!cps.bCommLink);
    }

    for (auto& btn : findChildren<QRadioButton*>())
    {
        btn->setEnabled(!cps.bCommLink);
    }

    // 通讯连接
    ui->comm_link_ckbox->setChecked(cps.bCommLink);
    ui->comm_link_ckbox->setEnabled(true);
}

void CommParamSetDlg::InitCtrls()
{
	SetTitleName(tr("通讯设置"));

	// 初始化串口下拉框
	auto myList = QSerialPortInfo::availablePorts();
	for (auto ss : myList)
	{
		ui->seriial_port_cbox->addItem(ss.portName());
	}

	// 初始化数据位下拉框	
	for (auto each : ::tf::_DataBits_VALUES_TO_NAMES)
	{
		auto type = (::tf::DataBits::type)each.first;
		ui->data_bit_cbox->addItem(QString::number(type));
	}

	// 初始化停止位下拉框	
	for (auto each : ::tf::_StopBits_VALUES_TO_NAMES)
	{
		auto type = (::tf::StopBits::type)each.first;
		AddTfEnumItemToComBoBox(ui->stop_bit_cbox, type);
	}

	// 初始化波特率下拉框	
	for (auto each : ::tf::_BaudRate_VALUES_TO_NAMES)
	{
		auto type = (::tf::BaudRate::type)each.first;
		ui->baud_tate_cbox->addItem(QString::number(type));
	}

	// 初始化奇偶校验下拉框	
	for (auto each : ::tf::_Parity_VALUES_TO_NAMES)
	{
		auto type = (::tf::Parity::type)each.first;
		AddTfEnumItemToComBoBox(ui->parity_check_cbox, type);
	}

	// 设置所有下拉框
	for (auto box : QWidget::findChildren<QComboBox*>())
	{
		box->setCurrentIndex(-1);
	}

	// IP输入框设置正则表达式
	ui->service_ip_edit->setValidator(new QRegExpValidator(QRegExp(UI_IP)));
	// 端口输入框设置正则表达式
	ui->service_port_edit->setValidator(new QRegExpValidator(QRegExp(UI_PORT)));
	// 接收等待时间输入框设置正则表达式
	ui->wait_time_edit->setValidator(new QRegExpValidator(QRegExp("^\\d+$")));
	// 最大传输次数输入框设置正则表达式
	ui->max_trans_times_edit->setValidator(new QRegExpValidator(QRegExp("^\\d+$")));

	// 保存按钮
	connect(ui->save_btn, SIGNAL(clicked()), this, SLOT(OnSaveBtnClicked()));

    // 浏览按钮被点击
    connect(ui->browse1_btn, SIGNAL(clicked()), this, SLOT(OnBrowseBtnClicked()));
    connect(ui->browse2_btn, SIGNAL(clicked()), this, SLOT(OnBrowseBtnClicked()));
    connect(ui->browse3_btn, SIGNAL(clicked()), this, SLOT(OnBrowseBtnClicked()));
	
	// 串口设置按钮
	connect(ui->seriial_port_set_radiobtn, &QRadioButton::toggled, this, [=](bool status) 
	{
        if (status)
        {
            ui->tcp_set_radiobtn->setChecked(false);
            ui->file_path_radiobtn->setChecked(false);
        }

        ui->seriial_port_cbox->setEnabled(status);
        ui->data_bit_cbox->setEnabled(status);
        ui->stop_bit_cbox->setEnabled(status);
        ui->baud_tate_cbox->setEnabled(status);
        ui->parity_check_cbox->setEnabled(status);
	});

	// TCP设置按钮
	connect(ui->tcp_set_radiobtn, &QRadioButton::toggled, this, [=](bool status)
	{
        if (status)
        {
            ui->seriial_port_set_radiobtn->setChecked(false);
            ui->file_path_radiobtn->setChecked(false);
        }
		
        ui->service_ip_edit->setEnabled(status);
        ui->service_port_edit->setEnabled(status);
	});

    // 文件路径
    connect(ui->file_path_radiobtn, &QRadioButton::toggled, this, [=](bool status)
    {
        if (status)
        {
            ui->seriial_port_set_radiobtn->setChecked(false);
            ui->tcp_set_radiobtn->setChecked(false);
        }

        ui->sample_req_path_edit->setEnabled(status);
        ui->test_rlt_path_edit->setEnabled(status);
        ui->sample_assay_path_edit->setEnabled(status);
        ui->browse1_btn->setEnabled(status);
        ui->browse2_btn->setEnabled(status);
        ui->browse3_btn->setEnabled(status);
    });
}

///
/// @brief 检查输入数据是否合法
///
///
/// @return 
///
/// @par History: 修改bug，完善消息提示 0023062: [应用] 通讯设置界面设置接受等待时间为空，保存失败但提示信息错误
/// @li 1556/Chenjianlin，2023年11月7日，添加注释
///
bool CommParamSetDlg::CheckInputData()
{

	// 串口设置
	if (ui->seriial_port_set_radiobtn->isChecked())
	{
		ReportErr(ui->seriial_port_cbox->currentIndex()	== -1,tr("请设置串口"));
		ReportErr(ui->data_bit_cbox->currentIndex()		== -1, tr("请设置数据位"));
		ReportErr(ui->stop_bit_cbox->currentIndex()		== -1, tr("请设置停止位"));
		ReportErr(ui->baud_tate_cbox->currentIndex()	== -1, tr("请设置波特率"));
		ReportErr(ui->parity_check_cbox->currentIndex() == -1, tr("请设置奇偶校验"));
	}

	// TCP设置
    if (ui->tcp_set_radiobtn->isChecked())
    {
        ReportErr(ui->service_ip_edit->text().isEmpty(), tr("请设置服务器IP，格式如：xxx.xxx.xxx.xxx"));
        ReportErr(ui->service_port_edit->text().isEmpty(), tr("请设置端口，1≤端口≤65535"));
    }

    // 文件路径
    if (ui->file_path_radiobtn->isChecked())
    {
        ReportErr(ui->sample_req_path_edit->text().isEmpty(), tr("请设置样本请求路径"));
        ReportErr(ui->test_rlt_path_edit->text().isEmpty(), tr("请设置结果路径"));
        ReportErr(ui->sample_assay_path_edit->text().isEmpty(), tr("请设置样本项目路径"));
    }

    QString value = ui->service_ip_edit->text();
    ReportErr(!QRegExp(UI_IP).exactMatch(value) && !ui->service_ip_edit->text().isEmpty(), tr("服务器IP错误，格式如：xxx.xxx.xxx.xxx"));
    int tempData = ui->service_port_edit->text().toInt();
    ReportErr(tempData < 1 || tempData > 65535, tr("端口设置错误：1≤端口≤65535"));
	
    // 通讯名称
    ReportErr(ui->comm_params_name->text().isEmpty(), tr("请设置通讯名称"));
    // 通讯id
    ReportErr(ui->comm_params_id->text().isEmpty(), tr("请设置通讯id"));
	// 通讯协议
	ReportErr(ui->comm_protocol_cbox->currentIndex() == -1, tr("请设置通讯协议"));
	// 等待时间
	ReportErr(ui->wait_time_edit->text().isEmpty(), tr("请设置接收等待时间，5≤接收等待时间≤60"));
    tempData = ui->wait_time_edit->text().toInt();
	ReportErr(tempData<5 || tempData > 60, tr("接收等待时间设置错误：5≤接收等待时间≤60"));
	// 最大传输次数
	ReportErr(ui->max_trans_times_edit->text().isEmpty(), tr("请设置最大传输次数，1≤最大传输次数≤3"));
    tempData = ui->max_trans_times_edit->text().toInt();
	ReportErr(tempData<1 || tempData > 3, tr("最大传输次数设置错误：1≤最大传输次数≤3"));

	return true;
}

void CommParamSetDlg::OnSaveBtnClicked()
{
    CommParamSet cps;
    if (!DictionaryQueryManager::GetCommParamSet(cps))
    {
        ULOG(LOG_ERROR, "Query CommParamSet Failed");
        TipDlg(tr("保存失败"), tr("保存失败")).exec();
        return;
    }

    // 只有仪器是待机才能修改通讯设置
    if (cps.bCommLink != ui->comm_link_ckbox->isChecked())
    {
        for (auto& dev : CommonInformationManager::GetInstance()->GetDeviceMaps())
        {
            // 来自需求文档，仪器待机才能修改通讯设置的开关
            if (devIsRun(*(dev.second)))
            {
                TipDlg(tr("存在正在运行的仪器，不能修改通讯设置")).exec();
                return;
            }
        }
    }

	// 检查输入数据是否合法
	if (!CheckInputData())
	{
		return;
	}


	// 串口设置
	cps.bEnabledSerialPort  = ui->seriial_port_set_radiobtn->isChecked();
	// TCP设置
	cps.bEnabledTCP			= ui->tcp_set_radiobtn->isChecked();
    // 文件路径
    cps.bEnabledFile        = ui->file_path_radiobtn->isChecked();

	if (cps.bEnabledSerialPort)
	{
		// 串口名称
		cps.strSerialName	= ui->seriial_port_cbox->currentText().toStdString();
		// 数据位
		cps.iByteSize = ui->data_bit_cbox->currentText().toInt();
		// 停止位
		cps.iStopBits = ui->stop_bit_cbox->currentIndex();
		// 波特率
		cps.iBaudRate = ui->baud_tate_cbox->currentText().toInt();
		// 奇偶校验
		cps.iParity = ui->parity_check_cbox->currentIndex();
	}

	if (cps.bEnabledTCP)
	{
		//ip
		cps.strServiceIP = ui->service_ip_edit->text().toStdString();
		// 端口
		cps.iServicePort = ui->service_port_edit->text().toInt();
	}

    if (cps.bEnabledFile)
    {
        // 样本请求路径
        cps.strSampleReqPath = ui->sample_req_path_edit->text().toStdString();
        // 结果路径
        cps.strSampleRltPath = ui->test_rlt_path_edit->text().toStdString();
        // 样本项目路径
        cps.strSampleAssayPath = ui->sample_assay_path_edit->text().toStdString();
    }

	// 通讯协议
    QString commProtocol = ui->comm_protocol_cbox->currentText();
	cps.iCommProtocol    = ((commProtocol == "HL7")? tf::LisProtocolType::HL7: tf::LisProtocolType::ASTM);
	// 通讯连接
	cps.bCommLink		 = ui->comm_link_ckbox->isChecked();
	// 接收样本号
	cps.bReceiveSampleNum = ui->receive_sample_num_ckbox->isChecked();
	// 接收样本信息
	cps.bIntroduceSample = ui->receive_sample_info_ckbox->isChecked();
	// 自动上传结果
	cps.bUploadResults	 = ui->auto_upload_results_gbox->isChecked();
    cps.bUploadAfterAudit = ui->radioButton_2->isChecked();
    // 通讯名称
    cps.strSendName = ui->comm_params_name->text().toStdString();
    // 通讯ID
    cps.strSendId = ui->comm_params_id->text().toStdString();
	// 样本编号自动上传
	cps.bUploadSampleNum = ui->auto_upload_sample_num_ckbox->isChecked();
	// 接收LIS复查样本
	cps.bReceiveLis		 = ui->receive_recheck_sample_ckbox->isChecked();
	// 接收等待时间
	cps.iWaitTime		 = ui->wait_time_edit->text().toInt();
	// 最大传输次数
	cps.iMaxTransTimes	 = ui->max_trans_times_edit->text().toInt();

    if (!DictionaryQueryManager::SaveCommParamSet(cps))
    {
        ULOG(LOG_ERROR, "Failed to save commparmset.");
        TipDlg(tr("保存失败"), tr("保存失败")).exec();
        return;
    }

    close();
}

///
/// @brief
///     浏览按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2024年5月27日，新建函数
///
void CommParamSetDlg::OnBrowseBtnClicked()
{
    // 获取信号发送者
    QPushButton* pClickedBtn = qobject_cast<QPushButton*>(sender());
    if (pClickedBtn == Q_NULLPTR)
    {
        ULOG(LOG_ERROR, "pClickedBtn == Q_NULLPTR.");
        return;
    }

    // 判断是哪个路径
    QString filePtah("");
    if (pClickedBtn == ui->browse1_btn)
    {
        filePtah = QFileDialog::getExistingDirectory(this, tr("样本请求路径"), ("./"));
        if (filePtah.isEmpty())
        {
            ULOG(LOG_INFO, "filePtah.isEmpty()");
            return;
        }
        ui->sample_req_path_edit->setText(filePtah);
    }
    else if (pClickedBtn == ui->browse2_btn)
    {
        filePtah = QFileDialog::getExistingDirectory(this, tr("结果路径"), ("./"));
        if (filePtah.isEmpty())
        {
            ULOG(LOG_INFO, "filePtah.isEmpty()");
            return;
        }
        ui->test_rlt_path_edit->setText(filePtah);
    }
    else if (pClickedBtn == ui->browse3_btn)
    {
        filePtah = QFileDialog::getExistingDirectory(this, tr("样本项目路径"), ("./"));
        if (filePtah.isEmpty())
        {
            ULOG(LOG_INFO, "filePtah.isEmpty()");
            return;
        }
        ui->sample_assay_path_edit->setText(filePtah);
    }
    else
    {
        ULOG(LOG_WARN, "Unknown button.");
        return;
    }
}
