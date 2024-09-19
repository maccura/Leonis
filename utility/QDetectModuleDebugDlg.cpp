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
/// @file     QDetectModuleDebugDlg.h
/// @brief 	 发光剂流程
///
/// @author   7656/zhang.changjiang
/// @date      2022年12月9日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2022年12月9日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QDetectModuleDebugDlg.h"
#include "ui_QDetectModuleDebugDlg.h"
#include <QRadioButton>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QDateTime>
#include <boost/math/tools/stats.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <boost/accumulators/statistics/density.hpp>
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/CommonInformationManager.h"
#include "manager/DictionaryQueryManager.h"
#include "./model/DeviceInfoModel.h"
#include "../thrift/im/ImLogicControlProxy.h"
#include "uidcsadapter/uidcsadapter.h"
#include "uidcsadapter/abstractdevice.h"
#include "src/common/Mlog/mlog.h"
#include "shared/FileExporter.h"
#include "src/thrift/im/gen-cpp/im_types.h"
#include "shared/tipdlg.h"

#define ATTEN_FACTOR_PERCISION 1000 // 分光系数精度(因为分光系数是浮点数，传参数组是整形数组，将分光系数放大1000倍传给dcs，在使用的时候除以1000)

using Accumulators = boost::accumulators::accumulator_set<double, boost::accumulators::features<boost::accumulators::tag::mean, boost::accumulators::tag::variance>>;
QDetectModuleDebugDlg::QDetectModuleDebugDlg(QWidget *parent)
	: BaseDlg(parent),
	m_dMaxValue(0),
	m_iRow(0)
{
    ui = new Ui::QDetectModuleDebugDlg();
	ui->setupUi(this);
	Init();
}

QDetectModuleDebugDlg::~QDetectModuleDebugDlg()
{
}

///
///  @brief 初始化
///
void QDetectModuleDebugDlg::Init()
{
	SetTitleName(tr("发光剂流程"));
	// 注册发光剂消息
	REGISTER_HANDLER(MSG_ID_MAINTAIN_DETECT_UPDATE, this, ShowDetectSignals);
	// 开始按钮
	connect(ui->start_btn, &QPushButton::clicked, this, &QDetectModuleDebugDlg::OnStartBtnClicked);
	// 清除按钮
	connect(ui->clean_btn, &QPushButton::clicked, this, &QDetectModuleDebugDlg::OnCleanBtnClicked);
	// 关闭按钮
	connect(ui->close_btn, &QPushButton::clicked, this, &QDetectModuleDebugDlg::OnCloseBtnClicked);
    // 导出按钮
    connect(ui->export_btn, &QPushButton::clicked, this, &QDetectModuleDebugDlg::OnExportBtnClicked);

	auto& devInfos = CommonInformationManager::GetInstance()->GetDeviceMaps();
	if (devInfos.empty())
	{
		ULOG(LOG_ERROR, "%s(), devInfos.empty()", __FUNCTION__);
		return;
	}

	// 根据设备列表创建单选框
	QHBoxLayout* pHlayout = new QHBoxLayout(ui->devListFrame);
	pHlayout->setMargin(0);
	pHlayout->setSpacing(30);
	pHlayout->addStretch(1);
	QVector<QRadioButton*> vecRBtn;
	bool isPipeLine = DictionaryQueryManager::GetInstance()->GetPipeLine();

	// 依次添加具体设备
	for (const auto& dev : devInfos)
	{
		if (dev.second->deviceType == ::tf::DeviceType::type::DEVICE_TYPE_INVALID)
		{
			continue;
		}

		if (dev.second->deviceType == ::tf::DeviceType::type::DEVICE_TYPE_TRACK && !isPipeLine)
		{
			continue;
		}

		QRadioButton* pRBtn;
		pRBtn = new QRadioButton(QString::fromStdString(dev.second->name));
		connect(pRBtn, SIGNAL(clicked()), this, SLOT(OnDevRBtnClicked()));
		pHlayout->addWidget(pRBtn);
		vecRBtn.append(pRBtn);
	}

	if (!vecRBtn.isEmpty())
	{
		vecRBtn.first()->setChecked(true);
	}

	// 如果只有一个设备，则隐藏设备列表
	if (vecRBtn.size() == 1)
	{
		ui->devListFrame->hide();
	}
	
	std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(vecRBtn.first()->text());
	if (spIDev != Q_NULLPTR)
	{
		m_curDeviceSN = spIDev->DeviceSn().toStdString();
	}
}

///
///  @brief:	显示信号值
///
///  @param[in]   mit    维护项类型
///  @param[in]   sigValue  信号值
///
///  @return	:
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年7月27日，新建函数
///
void QDetectModuleDebugDlg::ShowDetectSignals(tf::MaintainItemType::type mit, double sigValue)
{
	// 如果不是发光剂流程直接返回
	if (mit != tf::MaintainItemType::type::MAINTAIN_ITEM_DETECT_TEST)
	{
		return;
	}

	// 获取最大的检测信号值
	if (m_dMaxValue < sigValue)
	{
		m_dMaxValue = sigValue;
		ui->doubleSpinBox_max->setValue(m_dMaxValue);
	}

	// 将信号值存入信号值列表，以便于统计均值和标准差
	auto time = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
	m_sigValues.push_back(sigValue);
	m_sigFinTime.push_back(time);
	// 显示第m_iRow次检测信号值
	ui->textEdit->append(tr("第") + QString::number(++m_iRow) + tr("次检测信号值为：") + QString::number(sigValue, 'f', 0) + tr(" 时间:") + time);

	// 如果信号值只有一个，不计算均值和标准差
	if (m_sigValues.size() - 1 <= 0)
	{
		return;
	}
	
	// 初始化求和框架
	Accumulators acc;
	for_each(m_sigValues.begin(), m_sigValues.end(), std::ref(acc));

	// 显示均值
	ui->doubleSpinBox_avr->setValue(boost::accumulators::mean(acc));

	// 显示标准差
	ui->doubleSpinBox_std_dev->setValue(std::sqrt(boost::accumulators::variance(acc)));
}

///
///  @brief:	点击开始按钮
///
void QDetectModuleDebugDlg::OnStartBtnClicked()
{
#if 0
	std::vector<int32_t> lstParam;
	lstParam.push_back(ui->comboBox_mode->currentIndex());
	lstParam.push_back(ui->comboBox_steps->currentText().toInt());
	lstParam.push_back(ui->spinBox_val->value());
	lstParam.push_back(ui->doubleSpinBox_AttenFactor->value() * ATTEN_FACTOR_PERCISION);
	lstParam.push_back(ui->comboBox_sub_group->currentIndex());
	std::vector<std::string> deviceSNs;
	deviceSNs.push_back(m_curDeviceSN);
	im::LogicControlProxy::DetectModuleDebug(::tf::MaintainItemType::type::MAINTAIN_ITEM_DETECT_TEST, deviceSNs, lstParam);

#else

	std::vector<int32_t> lstParam;
	auto mtit = ::tf::MaintainItemType::type::MAINTAIN_ITEM_DETECT_TEST;
	auto mpcst = ::im::tf::MaintProcCheckSubType::type::MODULE_DETECT_NO_CUP;
	int iIndex = ui->comboBox_mode->currentIndex();
	if (0 == iIndex || 1 == iIndex)
	{
		mpcst = (::im::tf::MaintProcCheckSubType::type)(iIndex);
	}
	else if (2 == iIndex)
	{
		mtit = ::tf::MaintainItemType::type::MAINTAIN_ITEM_INJECT_NEEDLE_TEST;
		mpcst = ::im::tf::MaintProcCheckSubType::type::MODULE_SAMPLE_PMT;
	}

	lstParam.push_back(mpcst);
	lstParam.push_back(ui->comboBox_steps->currentText().toInt());
	lstParam.push_back(ui->spinBox_val->value());

	std::vector<std::string> deviceSNs;
	deviceSNs.push_back(m_curDeviceSN);
	im::LogicControlProxy::ProcessCheck(mtit, deviceSNs, lstParam);

#endif

}

///
///  @brief:	点击清除
///
void QDetectModuleDebugDlg::OnCleanBtnClicked()
{
	ui->textEdit->clear();
	m_sigValues.clear();
	m_sigFinTime.clear();
	ui->doubleSpinBox_max->setValue(0);
	ui->doubleSpinBox_NS->setValue(0);
	ui->doubleSpinBox_avr->setValue(0);
	ui->doubleSpinBox_std_dev->setValue(0);
	m_dMaxValue = 0;
	m_iRow = 0;
}

///
///  @brief:	点击关闭
///
void QDetectModuleDebugDlg::OnCloseBtnClicked()
{
	close();
}

///
///  @brief     点击设备单选按钮
///
void QDetectModuleDebugDlg::OnDevRBtnClicked()
{
	// 获取信号发送者
	QRadioButton* pRBtn = qobject_cast<QRadioButton*>(sender());
	if (pRBtn == Q_NULLPTR)
	{
		return;
	}
	std::shared_ptr<AbstractDevice> spIDev = DeviceFactory::GetDevice(pRBtn->text());
	if (spIDev == Q_NULLPTR)
	{
		return;
	}
	m_curDeviceSN = spIDev->DeviceSn().toStdString();
}

///
///  @brief     导出按钮
///
void QDetectModuleDebugDlg::OnExportBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 弹出保存文件对话框
    QString strFileName = QFileDialog::getSaveFileName(this, tr("保存为..."), QString(), tr("CSV files (*.csv)"));
    if (strFileName.isEmpty())
    {
        ULOG(LOG_INFO, "File path is empty!");
        return;
    }

    // 记录导出文本
    QStringList strExportTextList;
    QString strInfo;
    strInfo = ui->label->text() + "\t" + ui->comboBox_mode->currentText() + "\t" + ui->label_3->text() + "\t" + ui->doubleSpinBox_max->text();
    strExportTextList.push_back(strInfo);
    strInfo = ui->label_2->text() + "\t" + ui->comboBox_steps->currentText() + "\t" + ui->label_4->text() + "\t" + ui->doubleSpinBox_NS->text();
    strExportTextList.push_back(strInfo);
    strInfo = ui->label_5->text() + "\t" + ui->spinBox_val->text() + "\t" + ui->label_9->text() + "\t" + ui->doubleSpinBox_avr->text();
    strExportTextList.push_back(strInfo);
    strInfo = ui->label_6->text() + "\t" + ui->doubleSpinBox_AttenFactor->text() + "\t" + ui->label_8->text() + "\t" + ui->doubleSpinBox_std_dev->text();
    strExportTextList.push_back(strInfo);
    strInfo = ui->label_7->text() + "\t" + ui->comboBox_sub_group->currentText();
    strExportTextList.push_back(strInfo);

    for(int iIndex = 0; iIndex < m_sigValues.size() && iIndex < m_sigFinTime.size(); iIndex++)
    {
		auto sigValue =m_sigValues.at(iIndex);
		auto time = m_sigFinTime.at(iIndex);
        strInfo = tr("第") + QString::number(iIndex + 1) + tr("次检测信号值为：") + "\t" + QString::number(sigValue, 'f', 0) + "\t" + tr(" 时间:") + "\t" + time;
        strExportTextList.push_back(strInfo);
    }

    std::shared_ptr<FileExporter> pFileEpt(new FileExporter());
    bool bRect = pFileEpt->ExportInfoToFile(strExportTextList, strFileName);

    // 弹框提示导出
    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(bRect ? tr("导出完成!") : tr("导出失败！")));
    pTipDlg->exec();
}
