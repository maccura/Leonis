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
/// @file     QRecordISESupply.h
/// @brief    ISE耗材的登记
///
/// @author   5774/WuHongTao
/// @date     2022年7月11日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月11日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QRecordISESupply.h"
#include "ui_QRecordISESupply.h"
#include "src/common/common.h"
#include "src/common/TimeUtil.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/RfDataCodec.h"
#include "src/public/ch/ChKeys.h"
#include "src/public/ise/IseConfigDefine.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/tipdlg.h"
#include "src/thrift/ise/gen-cpp/ise_constants.h"
#include "thrift/ise/ise1005/Ise1005LogicControlProxy.h"
#include <QIntValidator>
#include <QTimer>
#include <QtConcurrent/QtConcurrent>

#define		LOTMAXLENGTH		7		// 批号输入限制
#define		SNMAXLENGTH			6		// 瓶号输入限制
#define		BARMINLEN			29		// 条码最短长度

QRecordISESupply::QRecordISESupply()
	: BaseDlg()
{
    ui = new Ui::QRecordISESupply();
	ui->setupUi(this);

	InitUi();
	InitConnect();
}

QRecordISESupply::~QRecordISESupply()
{
}

void QRecordISESupply::StartDialog(const std::shared_ptr<const tf::DeviceInfo> &device, const int moduleIdx, const int type)
{
	if (device == nullptr)
	{
		ULOG(LOG_ERROR, "%s(the device is nullptr)", __FUNCTION__);
		close();
		return;
	}

	// 缓存数据
	m_device = device;
	m_moduleIndex = moduleIdx;
	m_iseSupplyType = static_cast<ise::tf::SuppliesType::type>(type);

	// 根据耗材类型设置输入限制
	if (m_iseSupplyType == ise::tf::SuppliesType::SUPPLIES_TYPE_IS)
	{
		// 最多输入200毫升
		m_validator->setRange(0, ::ise::tf::g_ise_constants.ISE_IS_CAVITY_SIZE / 10000);
	}
	else if (m_iseSupplyType == ise::tf::SuppliesType::SUPPLIES_TYPE_BUFFER)
	{
		// 最多输入2000毫升
		m_validator->setRange(0, ::ise::tf::g_ise_constants.ISE_BUFF_CAVITY_SIZE / 10000);
	}
	ui->residual_Edit->setValidator(m_validator);

	// 如果已有信息填充参数
	FillParamter();

	// 初始化按钮使能
	ui->ok_Button->setEnabled(false);
	ui->cancel_btn->setEnabled(true);
	ui->stackedWidget->setCurrentIndex(1);
	ui->label->setText(tr("仪器正在检测，请在<span style='color:#69c'>18s</span>后进行加载。"));
	m_isCancel = false;
	this->show();

	// 获取耗材加载前的状态
	m_isRunning = false;
	if (device->status == tf::DeviceWorkState::DEVICE_STATUS_RUNNING)
	{
		m_isRunning = true;
	}

	// 异步准备加载耗材
	auto deviceSN = device->deviceSN;
	m_future = QtConcurrent::run([deviceSN]
	{
		return ise::ise1005::LogicControlProxy::AskForLoadSupply(deviceSN);
	});
	m_watcher.setFuture(m_future);

	// 每隔1秒检测一次是否完成
	m_countdown = ISE_SUPPLY_LOAD_TIMEOUT;
	m_timer->start();
}

QRecordISESupply& QRecordISESupply::GetInstance()
{
	static QRecordISESupply dialog;
	return dialog;
}

void QRecordISESupply::InitUi()
{
	SetTitleName(tr("耗材加载"));
	ui->lot_Edit->setMaxLength(LOTMAXLENGTH);
	ui->sn_Edit->setMaxLength(SNMAXLENGTH);

	m_validator = new QIntValidator(0, ::ise::tf::g_ise_constants.ISE_BUFF_CAVITY_SIZE / 10000, ui->residual_Edit);

	// 最多3年
	auto validatorOfDay = new QIntValidator(0, 365 * 3, ui->openBottleExpiryTime_Edit);
	ui->openBottleExpiryTime_Edit->setValidator(validatorOfDay);

	// 定时器
	m_timer = new QTimer(this);
	m_timer->setInterval(1000);

	// 条码限制输入数字和字母
	QRegExp regxBar("^[a-zA-Z0-9]+$");
	QValidator *validatorBar = new QRegExpValidator(regxBar, this);
	ui->lineEdit_barcode->setValidator(validatorBar);

	//this->resize(750, 588);
}

void QRecordISESupply::InitConnect()
{
	// 保存或者修改耗材信息
	connect(ui->ok_Button, &QPushButton::clicked, this, [&]()
	{
		this->SaveSupply();
	});

	// 关闭按钮
	connect(ui->cancel_btn, &QPushButton::clicked, this, [&]()
	{
		// 在加载界面则通知下位机取消加载，在详细信息界面则直接关闭窗口
		if (ui->stackedWidget->currentIndex() == 1 && !m_isCancel)
		{
			// TODO 中途取消，告知下位机不加载了
			CancelTask();
		}

		this->close();
	});

	// 定时器时间到
	connect(m_timer, &QTimer::timeout, [&]()
	{
		if (m_countdown > 0)
		{
			m_countdown--;
			// 更新倒计时
			ui->label->setText(tr("仪器正在检测，请在<span style='color:#69c'>%1s</span>后进行加载。").arg(m_countdown));
		}
	});

	// 监听异步任务
	connect(&m_watcher, &QFutureWatcher<bool>::finished, [&]()
	{
		// 停止计时
		m_timer->stop();

		// 中途取消不能获取结果
		if (!m_isCancel)
		{
			bool ret = m_watcher.future().result();
			if (ret)
			{
				ui->ok_Button->setEnabled(true);
				ui->stackedWidget->setCurrentIndex(0);
				ui->lineEdit_barcode->setFocus();
			}
			else
			{
				if (m_countdown <= 0)
				{
					ULOG(LOG_WARN, "AskForLoadSupply timeout.");
					ui->label->setText(tr("切换到待机态超时。"));
				}
				else
				{
					ui->label->setText(tr("切换到待机态失败。"));
				}
			}

			// 表示已完成
			m_isCancel = true;
		}
	});

	// 勾选条码扫描才允许输入条码
	connect(ui->rb_scan, &QRadioButton::clicked, this, &QRecordISESupply::OnRadioButtonClicked);
	connect(ui->rb_manual, &QRadioButton::clicked, this, &QRecordISESupply::OnRadioButtonClicked);
}

void QRecordISESupply::FillParamter()
{
	// 获取ISE设备名称
	auto getModuleName = [&](int idx)->QString
	{
		if (idx == 1)
		{
			return "A";
		}
		else
		{
			return "B";
		}
	};
	QString moduleName = m_device->moduleCount == 1 ? QString::fromStdString(m_device->name)
		: QString::fromStdString(m_device->name) + getModuleName(m_moduleIndex);
	ui->label_module->setText(moduleName);

	QString supplyName = m_iseSupplyType == ise::tf::SuppliesType::SUPPLIES_TYPE_IS
		? ISE_IS_NAME : ISE_BUF_NAME;
	 ui->label_name->setText(supplyName);

	// 查询耗材信息
	::ise::tf::SuppliesInfoQueryResp _return;
	::ise::tf::SuppliesInfoQueryCond siqc;
	siqc.__set_deviceSN({m_device->deviceSN});
	siqc.__set_moduleIndex(m_moduleIndex);
	siqc.__set_suppliesCode(m_iseSupplyType);
	ise::ise1005::LogicControlProxy::QuerySuppliesInfo(_return, siqc);
	if (_return.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
		|| _return.lstSuppliesInfo.empty())
	{
		return;
	}

	// 只能查到唯一一个数据
    auto supplyIseInfo = _return.lstSuppliesInfo[0];

	// 当前耗材的数据库主键
	m_id = supplyIseInfo.id;

	// 试剂批号 瓶号(需根据lotSnMode进行判断，因为会自动生成瓶号，当未输入瓶号时需要隐藏)
	QString strLot, strSn;
	if (supplyIseInfo.lotSnMode == tf::EnumLotSnMode::ELSM_LOT_SN)
	{
		strLot = supplyIseInfo.__isset.lot ? QString::fromStdString(supplyIseInfo.lot) : QStringLiteral("");
		strSn = supplyIseInfo.__isset.sn ? QString::fromStdString(supplyIseInfo.sn) : QStringLiteral("");
	}
	else if (supplyIseInfo.lotSnMode == tf::EnumLotSnMode::ELSM_LOT)
	{
		strLot = supplyIseInfo.__isset.lot ? QString::fromStdString(supplyIseInfo.lot) : QStringLiteral("");
	}
	else if (supplyIseInfo.lotSnMode == tf::EnumLotSnMode::ELSM_SN)
	{
		strSn = supplyIseInfo.__isset.sn ? QString::fromStdString(supplyIseInfo.sn) : QStringLiteral("");
	}
	// 批号
	ui->lot_Edit->setText(strLot);
	// 瓶号
	ui->sn_Edit->setText(strSn);

	// 余量
	(supplyIseInfo.__isset.residual && supplyIseInfo.residual > 0) ? ui->residual_Edit->setText(QString::number(supplyIseInfo.residual/10000)):ui->residual_Edit->clear();
	// 有效期
	if (supplyIseInfo.__isset.expiryTime && !supplyIseInfo.expiryTime.empty())
	{
		ui->dateEdit->setDate(QDateTime::fromString(QString::fromStdString(supplyIseInfo.expiryTime), "yyyy-MM-dd hh:mm:ss").date());
	}
	else
	{
		ui->dateEdit->setDate(QDate::currentDate());
	}

	if (supplyIseInfo.__isset.openBottleExpiryTime)
	{
		auto durationTime = TimeStringToPosixTime(supplyIseInfo.openBottleExpiryTime) - boost::posix_time::second_clock::local_time();
		auto durationHours = durationTime.hours();
		auto days = durationHours / 24;
		if ((durationHours % 24) > 0)
		{
			days++;
		}

		// 过期显示0
		ui->openBottleExpiryTime_Edit->setText(days > 0 ? QString::number(days) : "0" );
	}
	else
	{
		ui->openBottleExpiryTime_Edit->setText("0");
	}

	// 位置信息
	m_supplyPos.__set_deviceSN(supplyIseInfo.deviceSN);
	m_supplyPos.__set_moduleIndex(supplyIseInfo.moduleIndex);
	(supplyIseInfo.suppliesCode == ise::tf::SuppliesType::SUPPLIES_TYPE_IS)
		? m_supplyPos.__set_pos(::ise::ise1005::tf::CabinetPos::POS_IS)
		: m_supplyPos.__set_pos(::ise::ise1005::tf::CabinetPos::POS_BUF);

	// 清空之前条码信息
	ui->lineEdit_barcode->clear();

	// 默认扫码录入
	ui->rb_scan->setChecked(true);
	emit ui->rb_scan->clicked(true);
}

bool QRecordISESupply::SaveSupply()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (ui->residual_Edit->text().isEmpty())
	{
		TipDlg(tr("提示"), tr("耗材余量不能为空")).exec();
		return false;
	}

	if (ui->openBottleExpiryTime_Edit->text().isEmpty())
	{
		TipDlg(tr("提示"), tr("开瓶有效期不能为空")).exec();
		return false;
	}

	// 耗材信息
	ise::tf::SuppliesInfo supplyInfo;
	supplyInfo.__set_manualRegister(true);
	supplyInfo.__set_version(1);
	supplyInfo.__set_deviceSN(m_device->deviceSN);
    supplyInfo.__set_moduleIndex(m_moduleIndex);
	supplyInfo.__set_suppliesCode(m_iseSupplyType);

	// 批号 瓶号
	QString lot = ui->lot_Edit->text();
	QString sn = ui->sn_Edit->text();
	supplyInfo.__set_lot(lot.toStdString());
	supplyInfo.__set_sn(sn.toStdString());

	// 判断批号瓶号模式
	if (lot.isEmpty() && sn.isEmpty())
	{
		supplyInfo.__set_lotSnMode(tf::EnumLotSnMode::ELSM_NO_LOT_SN);
	}
	else if (!lot.isEmpty() && !sn.isEmpty())
	{
		supplyInfo.__set_lotSnMode(tf::EnumLotSnMode::ELSM_LOT_SN);
	}
	else if (!lot.isEmpty())
	{
		supplyInfo.__set_lotSnMode(tf::EnumLotSnMode::ELSM_LOT);
	}
	else if (!sn.isEmpty())
	{
		supplyInfo.__set_lotSnMode(tf::EnumLotSnMode::ELSM_SN);
	}

	supplyInfo.__set_residual(ui->residual_Edit->text().toInt() * 10000);
	QString format = "yyyy-MM-dd 00:00:00";
	if (ui->dateEdit->date().isValid())
	{
		supplyInfo.__set_expiryTime(ui->dateEdit->date().toString(format).toStdString());
	}

	// 上机时间
	auto currentDateTime = QDateTime::currentDateTime();
	supplyInfo.__set_registerTime(currentDateTime.toString("yyyy-MM-dd hh:mm:ss").toStdString());

	// 开瓶有效期
	int	openExpire = ui->openBottleExpiryTime_Edit->text().toInt();

	std::string expireDateTimeString = currentDateTime.addDays(openExpire).toString(format).toStdString();
	supplyInfo.__set_openBottleExpiryTime(expireDateTimeString);

	// 耗材的类型限制了耗材的位置
	supplyInfo.__set_pos(m_supplyPos.pos);
	m_id > 0 ? supplyInfo.__set_id(m_id) : (void(0));

	// 添加耗材
	auto ret = ise::ise1005::LogicControlProxy::ManualRegisterSupplies(supplyInfo);
	if (ret != ::ise::tf::ManualRegisterResult::type::MRR_SUCCESS)
	{
		ULOG(LOG_ERROR, "ManualRegisterSupplies Failed.");
		TipDlg(tr("提示"), tr("耗材录入失败")).exec();
		return false;
	}

	// 更新耗材信息成功以后，开始进行管路填充(取消次数后1代表需要管路填充，0不需要)
	if (!ise::ise1005::LogicControlProxy::LoadSupply(m_supplyPos, ui->ckBox_fillPipe->isChecked(), m_isRunning))
	{
		ULOG(LOG_ERROR, "LoadSupply Failed.");
	}

	close();
	return true;
}

void QRecordISESupply::CancelTask()
{
	m_isCancel = true;

	if (!m_future.isCanceled())
	{
		m_future.cancel();
	}
	if (!m_watcher.isCanceled())
	{
		m_watcher.cancel();
	}

	m_timer->stop();
}

void QRecordISESupply::OnParseBarcode()
{
	if (ui->rb_manual->isChecked())
	{
		return;
	}

	QString BarCode = ui->lineEdit_barcode->text();

	if (BarCode.isEmpty())
	{
		TipDlg(tr("提示"), tr("请输入条码号.")).exec();
		return;
	}

	if (BARMINLEN > BarCode.length())
	{
		TipDlg(tr("提示"), tr("解析失败，条码长度不对.")).exec();
		return;
	}

	// 当前输入条码的最后四位为CRC校验码
	auto currTextCrc = BarCode.right(4);
	// 去掉后4位得到前面的条码
	BarCode.chop(4);
	// 校验CRC
	RfDataCodec<16, BARCODE_TruncPoly, BARCODE_InitRem, BARCODE_FinalXor, BARCODE_ReflectIn, BARCODE_ReflectRem> rfdc;
	auto crc = rfdc.GenCheckCodeHexString(BarCode.toStdString());
	if (crc != currTextCrc.toStdString())
	{
		TipDlg(tr("提示"), tr("解析失败，CRC校验失败.")).exec();
		return;
	}

	// 耗材编号 占3位
	int startPos = 0;
	int len = 3;
	int code = BarCode.mid(startPos, len).toInt();
	if (code != m_iseSupplyType)
	{
		TipDlg(tr("提示"), tr("解析失败，耗材编号不匹配.")).exec();
		return;
	}

	// 批号，7位
	startPos += len;
	len = 7;
	QString lot = BarCode.mid(startPos, len);
	ui->lot_Edit->setText(lot);

	// 瓶号长度，1位
	startPos += len;
	len = 1;
	int snLen = BarCode.mid(startPos, len).toInt();

	// 瓶号，snLen位
	startPos += len;
	len = snLen;
	int sn = BarCode.mid(startPos, len).toInt();
	ui->sn_Edit->setText(QString::number(sn));

	// 过期时间，6位
	startPos += len;
	len = 6;
	QString expireDate = "20" + BarCode.mid(startPos, len);
	ui->dateEdit->setDate(QDate::fromString(expireDate, "yyyyMMdd"));

	// 开瓶有效期，3位
	startPos += len;
	len = 3;
	int openDays = BarCode.mid(startPos, len).toInt();
	ui->openBottleExpiryTime_Edit->setText(QString::number(openDays));

	// 余量
	startPos += len;
	len = 4;
	int residual = BarCode.mid(startPos, len).toInt();
	ui->residual_Edit->setText(QString::number(residual));
}

void QRecordISESupply::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
	{
		OnParseBarcode();
		event->ignore();
		return;
	}

	QDialog::keyPressEvent(event);
}

void QRecordISESupply::OnRadioButtonClicked()
{
	auto btn = qobject_cast<QRadioButton*>(sender());
	if (btn == nullptr)
	{
		return;
	}

	bool enable = (btn == ui->rb_manual);
	ui->lineEdit_barcode->setEnabled(!enable);
	ui->lot_Edit->setEnabled(enable);
	ui->sn_Edit->setEnabled(enable);
	ui->openBottleExpiryTime_Edit->setEnabled(enable);
	ui->dateEdit->setEnabled(enable);
}
