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
/// @file     SystemOverviewWgt.cpp
/// @brief    系统总览界面源文件
///
/// @author   7997/XuXiaoLong
/// @date     2023年7月11日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7997/XuXiaoLong，2023年7月11日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "SystemOverviewWgt.h"
#include "ui_SystemOverviewWgt.h"

#include <QScrollBar>
#include <QMovie>

#include "TrackIomDeviceWgt.h"
#include "TrackDqiRwcIseDeviceWgt.h"
#include "C1005V1000DeviceWgt.h"
#include "C1005V2000DeviceWgt.h"
#include "C1005SingleDeviceWgt.h"
#include "I6000DeviceWgt.h"
#include "I6000SingleDeviceWgt.h"
#include "WarningSetWgt.h"
#include "GIFLabel.h"

#include "thrift/DcsControlProxy.h"
#include "manager/DictionaryQueryManager.h"
#include "manager/UserInfoManager.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/uicommon.h"
#include "shared/CommonInformationManager.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/common.h"
#include "src/public/SerializeUtil.hpp"
#include "src/public/track/TrackConfigDefine.h"
#include "src/public/track/TrackConfigSerialize.h"

#define			COMPUTER_W				260																// 电脑宽度
#define			COMPUTER_H				312																// 电脑高度
#define			BLANK_W					130																// 空白宽度
#define			BLANK_H					312																// 空白高度
#define			COMPUTER_PICTURE		(":/Leonis/resource/image/system-overview-computer.png")		// 电脑图片
#define			OVERLAP_PX				2																// 设备间重叠像素
#define			ONE_SCREEN_LEN			1820															// 一屏的长度	
#define			TIMER_INTERVAL			5																// 倒计时自动刷新定时器时长（s）

SystemOverviewWgt::SystemOverviewWgt(QWidget *parent)
	: QDialog(parent)
{
    ui = new Ui::SystemOverviewWgt();
	ui->setupUi(this);

	initUI();
	initConnect();

	// 初始化所有设备
	initDevices();

	// 初始化样本
	initSamples();

	// 查询更新一次已有报警瓶子
	slotUpadteBottles(WarnSetManager::GetInstance()->filterGenerateWarns());
}

SystemOverviewWgt::~SystemOverviewWgt()
{
}

void SystemOverviewWgt::updateCurrentTime(const QDateTime &time)
{
	m_date = time;

	ui->label_time->setText(time.toString("hh:mm"));
	ui->label_date->setText(time.toString(m_strDateTimeFormat));

	const QStringList weeks{ tr("星期一"),tr("星期二"),tr("星期三"),tr("星期四"),tr("星期五"),tr("星期六"),tr("星期日") };
	int idx = time.date().dayOfWeek();
    if (0 == idx)
    {
        // 将星期天置为7
        idx = 7;
    }

	ui->label_week->setText(weeks[idx - 1]);
}

void SystemOverviewWgt::updateWarningBtn(const bool flicker, const int level)
{
	ULOG(LOG_INFO, "%s(flicker=%d, level=%d).", __FUNCTION__, flicker, level);

	if (flicker) // 闪烁
	{
		//	红色闪烁
		if (level == tf::AlarmLevel::ALARM_LEVEL_TYPE_STOP)
		{
			ui->label_warn->changeGIF(WARN_STATES_RED_FLICKER);
			ui->label_warn->changeTextColor(QColor(251, 51, 67));
		}
		//	黄色闪烁
		else if(level == tf::AlarmLevel::ALARM_LEVEL_TYPE_STOPSAMPLING)
		{
			ui->label_warn->changeGIF(WARN_STATES_YELLOW_FLICKER);
			ui->label_warn->changeTextColor(QColor(251, 147, 3));
		}
		// 白色闪烁
		else
		{
			ui->label_warn->changeGIF(WARN_STATES_WHITE_FLICKER);
			ui->label_warn->changeTextColor(QColor(255, 255, 255));
		}
	}
	else // 不闪烁
	{
		//	红色
		if (level == tf::AlarmLevel::ALARM_LEVEL_TYPE_STOP)
		{
			ui->label_warn->changeGIF(WARN_STATES_RED);
			ui->label_warn->changeTextColor(QColor(251, 51, 67));
		}
		//	黄色
		else if(level == tf::AlarmLevel::ALARM_LEVEL_TYPE_STOPSAMPLING)
		{
			ui->label_warn->changeGIF(WARN_STATES_YELLOW);
			ui->label_warn->changeTextColor(QColor(251, 147, 3));
		}
		// 正常-白色
		else
		{
			ui->label_warn->changeGIF(WARN_STATES_GRAY);
			ui->label_warn->changeTextColor(QColor(255, 255, 255));
		}
	}
}

void SystemOverviewWgt::updateLisConnection(const bool isConnect)
{
	QString status = isConnect ? "lis_connect" : "lis_connect_dis";
	SetCtrlPropertyAndUpdateQss(ui->label_LIS, "bks", status);
}

void SystemOverviewWgt::updatePrintConnection(const bool isConnect)
{
	QString status = isConnect ? "print_connect" : "print_connect_dis";
	SetCtrlPropertyAndUpdateQss(ui->label_print, "bks", status);
}

void SystemOverviewWgt::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();
    if (userPms->IsPermissionShow(PSM_MONITOR_ALARMSETTINGS))
    {
        ui->btn_warnSet->show();
    }
    else
    {
        ui->btn_warnSet->hide();
    }
}

void SystemOverviewWgt::OnRgtAlarmReadFlagChange(QString devSN, bool readed)
{
	const string strDevSN = devSN.toStdString();

	ULOG(LOG_INFO, "%s(strDevSN=%s, readed=%d)", __FUNCTION__, strDevSN, readed);

	// 生化单机
	if (m_spChSingleDev)
	{
		if (strDevSN != m_spChSingleDev->m_devCh.devSN && strDevSN != m_spChSingleDev->m_devIse.devSN)
		{
			ULOG(LOG_INFO, "not ch single care device sn.");
			return;
		}

		auto tmpReaded = m_spChSingleDev->m_rgtAlarmReaded;
		if (strDevSN == m_spChSingleDev->m_devCh.devSN)
		{
			m_spChSingleDev->m_devCh.rgtAlarmReaded = readed;
		}
		else if (strDevSN == m_spChSingleDev->m_devIse.devSN)
		{
			m_spChSingleDev->m_devIse.rgtAlarmReaded = readed;
		}

		// 更新已读标志
		m_spChSingleDev->setDeviceReagentAlarmReadFlag();

		if (tmpReaded != m_spChSingleDev->m_rgtAlarmReaded)
		{
			// 更新状态灯
			m_spChSingleDev->changeStatus();
		}

		return;
	}

	std::shared_ptr<QBaseDeviceWgt> currDev = nullptr;

	// 获取设备信息
	auto spDev = CIM_INSTANCE->GetDeviceInfo(strDevSN);
	if (spDev == nullptr)
	{
		ULOG(LOG_ERROR, "spDev is nullptr.");
		return;
	}

	if (spDev->groupName.empty())
	{
		currDev = getDeviceBySn(strDevSN);
	}
	// 2000速设备
	else
	{
		currDev = getDeviceBySn(spDev->groupName);
	}

	if (currDev == nullptr)
	{
		ULOG(LOG_INFO, "can not find groupName=%s, strDevSN=%s, no need update status.", spDev->groupName, strDevSN);
		return;
	}

	switch (currDev->m_devType)
	{
	case EDT_C1005_2000: 	// 生化2000速
	{
		auto ch2000Dev = dynamic_cast<C1005V2000DeviceWgt*>(currDev.get());
		bool tmpReaded = ch2000Dev->m_rgtAlarmReaded;
		if (ch2000Dev->m_devA.devSN == strDevSN)
		{
			ch2000Dev->m_devA.rgtAlarmReaded = readed;
		}
		else if (ch2000Dev->m_devB.devSN == strDevSN)
		{
			ch2000Dev->m_devB.rgtAlarmReaded = readed;
		}

		// 更新已读标志
		ch2000Dev->setDeviceReagentAlarmReadFlag();

		if (tmpReaded != ch2000Dev->m_rgtAlarmReaded)
		{
			// 更新状态灯
			ch2000Dev->changeStatus();
		}
		break;
	}
	default:	// 其他设备
	{
		// 更新状态
		if (currDev->m_rgtAlarmReaded != readed)
		{
			currDev->m_rgtAlarmReaded = readed;
			currDev->changeStatus();
		}
		break;
	}
	}
}

void SystemOverviewWgt::OnTimerOut()
{
	m_currTotalTime -= TIMER_INTERVAL;
	if (m_currTotalTime <= 0)
	{
		m_currTotalTime = 0;
		m_timer->stop();
	}

	// 显示分钟
	int minutes = (m_currTotalTime + 59) / 60;
	ui->label_remainTime->setText(minutes > 30 ? ">30" : QString::number(minutes));
}

void SystemOverviewWgt::initUI()
{
	// 对话框默认为模态
	setModal(true);

	// 设置窗口无边框且为对话框
	setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);

	// 设置窗口透明
	setAttribute(Qt::WA_TranslucentBackground, true);

	// 仪器状态界面透明
	ui->scrollArea->setStyleSheet("background: transparent;");
	ui->scrollAreaWidgetContents->setStyleSheet("background: transparent;");

	// 联机版添加电脑设备
	if (DictionaryQueryManager::GetInstance()->GetPipeLine())
	{
		// 添加一个computer设备
		m_computerDevice = new QLabel(ui->scrollAreaWidgetContents);
		m_computerDevice->setFixedSize(COMPUTER_W, COMPUTER_H);
		m_computerDevice->setPixmap(QPixmap(COMPUTER_PICTURE));
	}

	// 报警设置弹窗
	m_warnSetWgt = new WarningSetWgt(this);

	// 报警gif 默认正常
	ui->label_warn->setDrawText(tr("报警"));
	updateWarningBtn(false, tf::AlarmLevel::ALARM_LEVEL_TYPE_INVALID);

	// 注册事件过滤器
	ui->widget_overview->installEventFilter(this);
	ui->label_warn->installEventFilter(this);
	//ui->scrollArea->installEventFilter(this);

	ui->btn_left->setEnabled(false);
	ui->btn_right->setEnabled(true);

	// TODO 暂时先隐藏
	ui->label_new_warn_left->hide();
	ui->label_new_warn_right->hide();

	// 查询日期时间格式字符串
	BaseSet bsDateTimeFormat;
	DictionaryQueryManager::GetUiBaseSet(bsDateTimeFormat);
	OnDateTimeFormatUpdate(bsDateTimeFormat);

	m_timer = new QTimer(this);
	m_timer->setInterval(TIMER_INTERVAL * 1000);
}

void SystemOverviewWgt::initConnect()
{
	// 返回
	connect(ui->btn_back, &QPushButton::clicked, this, [=] {
		ULOG(LOG_INFO, u8"点击监控页面返回按钮...");
		this->reject();
	});
	// 开始
	connect(ui->btn_start, &QPushButton::clicked, this, [=] {
		ULOG(LOG_INFO, u8"点击监控页面开始按钮...");
		this->reject(); 
		emit signalStart();
	});
	// 暂停
	connect(ui->btn_suspend, &QPushButton::clicked, this, &SystemOverviewWgt::signalSuspend);
	// 停止
	connect(ui->btn_stop, &QPushButton::clicked, this, &SystemOverviewWgt::signalStop);
	// 设置
	connect(ui->btn_warnSet, &QPushButton::clicked, this, &SystemOverviewWgt::slotClickSet);
	// 复位
	connect(ui->btn_reset, &QPushButton::clicked, this, [=] {
		POST_MESSAGE(MSG_ID_SYSTEM_OVERVIEW_CLICK_RESET);
	});
	// 左滑按钮
	connect(ui->btn_left, &QPushButton::clicked, this, [=] {
		ui->scrollArea->horizontalScrollBar()->setValue(ui->scrollArea->horizontalScrollBar()->minimum());
		ui->btn_left->setEnabled(false);
		ui->btn_right->setEnabled(true);
	});
	// 右滑按钮
	connect(ui->btn_right, &QPushButton::clicked, this, [=] {
		ui->scrollArea->horizontalScrollBar()->setValue(ui->scrollArea->horizontalScrollBar()->maximum());
		ui->btn_left->setEnabled(true);
		ui->btn_right->setEnabled(false);
	});

	// 监听样本信息
	REGISTER_HANDLER(MSG_ID_SAMPLE_INFO_UPDATE, this, OnSampleUpdate);

	// 监听更新报警瓶子
	REGISTER_HANDLER(MSG_ID_WARN_UPDATE_TO_SYS_OVERVIEW, this, slotUpadteBottles);

	// 监听设备是否屏蔽
	REGISTER_HANDLER(MSG_ID_DEVICE_MASK_UPDATE, this, slotUpadteDeviceMaskStatus);

	// 注册时间格式改变槽函数
	REGISTER_HANDLER(MSG_ID_DISPLAY_SET_UPDATE, this, OnDateTimeFormatUpdate);

	// 注册设备倒计时刷新槽函数
	REGISTER_HANDLER(MSG_ID_DEV_COUNT_DOWN_UPDATE, this, OnDeviceCountDownUpdate);

	// 注册设备状态改变处理槽函数
    REGISTER_HANDLER(MSG_ID_DEVS_STATUS_CHANGED, this, OnDevStateChange);

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);

	// 注册试剂耗材余量不足报警已读标志更新槽函数
	REGISTER_HANDLER(MSG_ID_RGT_ALARM_READ_FLAG_UPDATE, this, OnRgtAlarmReadFlagChange);

	// 总倒计时自减
	connect(m_timer, &QTimer::timeout, this, &SystemOverviewWgt::OnTimerOut);
}

void SystemOverviewWgt::initDevices()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	::tf::DeviceInfoQueryResp DeviceResp;
	::tf::DeviceInfoQueryCond QueryDeviceCond;

	// 查询设备信息
	if (!DcsControlProxy::GetInstance()->QueryDeviceInfo(DeviceResp, QueryDeviceCond)
		|| DeviceResp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
		|| DeviceResp.lstDeviceInfos.size() <= 0)
	{
		ULOG(LOG_ERROR, "QueryDeviceInfo failed.");
		return;
	}

	// 将设备按照设备序列号从小到大排序（满足拓扑图配置）
	std::sort(DeviceResp.lstDeviceInfos.begin(), DeviceResp.lstDeviceInfos.end(),
		[](const ::tf::DeviceInfo &a, const ::tf::DeviceInfo &b) {
		return stoi(a.deviceSN) < stoi(b.deviceSN);
	});

	// 判断“报警设置”界面存在哪些设备, 并且是否存在相应废液桶
	bool haveISE = false, haveCH = false, haveIM = false, haveChWaste = false, haveImWaste = false;
	// 是否联机版
	bool isOnline = DictionaryQueryManager::GetInstance()->GetPipeLine();
	// 单机版需隐藏
	if (!isOnline)
	{
		setLeftRightVisible(false);
		// 生化单机
		if (!CIM_INSTANCE->GetImisSingle())
		{
			m_spChSingleDev = make_shared<C1005SingleDeviceWgt>(EDT_C1005_SINGLE, ui->scrollAreaWidgetContents);
		}
	}

	// 依次初始化设备信息
	for (const auto& dev : DeviceResp.lstDeviceInfos)
	{
		string devSN = dev.deviceSN;

		// 判断设备体系
		switch (dev.deviceClassify)
		{
		case::tf::AssayClassify::ASSAY_CLASSIFY_OTHER:	// 其他 代表轨道
		{
			if (dev.deviceType == tf::DeviceType::DEVICE_TYPE_TRACK
				&& isOnline)
			{
				// 解析轨道模块
				track::bcyime::TrackOtherInfo tdoi;
				if (!DecodeJson(tdoi, dev.otherInfo))
				{
					ULOG(LOG_WARN, "DecodeJson Failed.");
					continue;
				}

				if (tdoi.node_infos.empty())
				{
					ULOG(LOG_INFO, "TrackOtherInfo node_infos is empty.");
					continue;
				}

				// 遍历轨道子模块，设备序列号使用轨道序列号+"-"+节点id，轨道子节点无屏蔽状态
				for (const auto& node : tdoi.node_infos)
				{
					shared_ptr<QBaseDeviceWgt> device = nullptr;

					if (node.second.nodeType == tf::DeviceType::DEVICE_TYPE_TRACK_6008_IOM01)
					{
						// IOM设备
						device.reset(new TrackIomDeviceWgt(devSN + "-" + to_string(node.first), node.second.name, EDT_TRACK_IOM,
							node.second.status, false, false, ui->scrollAreaWidgetContents));

						// 注册事件过滤器 点击IOM设备时转到样本架监视界面
						device->installEventFilter(this);
					}
					else if (node.second.nodeType == tf::DeviceType::DEVICE_TYPE_TRACK_6008_DQI)
					{
						// DQI设备
						device.reset(new TrackDqiRwcIseDeviceWgt(devSN + "-" + to_string(node.first), node.second.name, EDT_TRACK_DQI,
							node.second.status, false, false, true, ui->scrollAreaWidgetContents));
					}
					else if (node.second.nodeType == tf::DeviceType::DEVICE_TYPE_TRACK_6008_RWC)
					{
						// RWC设备
						device.reset(new TrackDqiRwcIseDeviceWgt(devSN + "-" + to_string(node.first), node.second.name, EDT_TRACK_RWC,
							node.second.status, false, false, true, ui->scrollAreaWidgetContents));
					}

					// 缓存设备
					device != nullptr ? m_vecDevices.push_back(device) : void(0);
				}
			}
			break;
		}
		case::tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE:	// 免疫
		{
			if (dev.deviceType == tf::DeviceType::DEVICE_TYPE_I6000)
			{
				if (isOnline)// 联机版
				{
					m_vecDevices.push_back(make_shared<I6000DeviceWgt>(devSN, dev.name, EDT_I6000, 
						dev.status, dev.enableWasterContainer, dev.masked, GetRgtAlarmReadFlag(devSN), ui->scrollAreaWidgetContents));
				}
				else // 免疫单机版
				{
					auto imSingleDev = make_shared<I6000SingleDeviceWgt>(devSN, dev.name, EDT_I6000_SINGLE, 
						dev.status, dev.enableWasterContainer, dev.masked, GetRgtAlarmReadFlag(devSN), ui->scrollAreaWidgetContents);
					imSingleDev->m_iomStatus = dev.iomStatus;

					// 缓存设备
					m_vecDevices.push_back(imSingleDev);
				}

				dev.enableWasterContainer ? (haveImWaste = true) : void(0);
				haveIM = true;
			}
			break;
		}
		case::tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY:	// 生化
		{
			if (dev.deviceType == tf::DeviceType::DEVICE_TYPE_C1000)
			{
				if (isOnline) // 联机版
				{
					if (dev.groupName.empty()) // 单机
					{
						m_vecDevices.push_back(make_shared<C1005V1000DeviceWgt>(devSN, dev.name, EDT_C1005_1000, dev.status, 
							dev.enableWasterContainer, dev.masked, GetRgtAlarmReadFlag(devSN), ui->scrollAreaWidgetContents));
					}
					else // 两联机
					{
						// 两联机时使用设备组名作为两联机设备序列号
						devSN = dev.groupName;

						// 第一次创建2000速 初始化设备A
						auto baseDevice = getDeviceBySn(devSN);
						if (baseDevice == nullptr)
						{
							shared_ptr<C1005V2000DeviceWgt> ch2000Dev = make_shared<C1005V2000DeviceWgt>(devSN, dev.groupName, EDT_C1005_2000,
								 dev.enableWasterContainer, ui->scrollAreaWidgetContents);

							// 更新设备A信息
							ch2000Dev->UpdateDevInfo(dev.deviceSN, dev.name, dev.status, dev.masked, GetRgtAlarmReadFlag(dev.deviceSN));

							m_vecDevices.push_back(ch2000Dev);
						}
						// 已存在2000速设备,那么初始化设备B
						else
						{
							auto ch2000Dev = dynamic_cast<C1005V2000DeviceWgt*>(baseDevice.get());

							// 更新设备B信息
							ch2000Dev->UpdateDevInfo(dev.deviceSN, dev.name, dev.status, dev.masked, GetRgtAlarmReadFlag(dev.deviceSN));

							// 设置废液桶
							ch2000Dev->setWasteLiquidVisible(dev.enableWasterContainer);

							// 设置是否有试剂报警
							ch2000Dev->setDeviceReagentAlarmReadFlag();
						}
					}
				}
				else // 生化单机版
				{
					if (m_spChSingleDev == nullptr)
					{
						ULOG(LOG_ERROR, "m_spChSingleDev is nullptr.");
						return;
					}

					// 更新生化设备信息
					m_spChSingleDev->updateChDeviceInfo(devSN, dev.name, dev.status, dev.iomStatus,
						dev.enableWasterContainer, dev.masked, GetRgtAlarmReadFlag(devSN));

					// 缓存设备
					m_vecDevices.push_back(m_spChSingleDev);
				}

				if (dev.enableWasterContainer)
				{
					haveChWaste = true;
				}

				haveCH = true;
			}
			break;
		}
		case::tf::AssayClassify::ASSAY_CLASSIFY_ISE:	// ise
		{
			if (dev.deviceType == tf::DeviceType::DEVICE_TYPE_ISE1005)
			{
				// 联机版ISE设备才独立
				if (isOnline)
				{
					m_vecDevices.push_back(make_shared<TrackDqiRwcIseDeviceWgt>(devSN, dev.name, EDT_ISE1005,
						dev.status, dev.enableWasterContainer, dev.masked, GetRgtAlarmReadFlag(devSN), ui->scrollAreaWidgetContents));
				}
				// 生化单机版
				else if (m_spChSingleDev)
				{
					// 更新ise设备信息
					m_spChSingleDev->updateIseDeviceInfo(devSN, dev.status, dev.masked, GetRgtAlarmReadFlag(devSN));
				}

				haveISE = true;
			}
			break;
		}
		default:
			break;
		}
	}

	// 连接信号槽
	for (const auto& device : m_vecDevices)
	{
		// 状态初始化
		device->changeStatus();

		// 点击单机版中的iom模块
		if (device->m_devType == EDT_C1005_SINGLE || device->m_devType == EDT_I6000_SINGLE)
		{
			connect(device.get(), &QBaseDeviceWgt::signalSampleRackMonitor, this, &SystemOverviewWgt::signalSampleRackMonitor);
		}

		if (device->m_devType == EDT_UNKNOWN
			|| device->m_devType == EDT_TRACK_IOM
			|| device->m_devType == EDT_TRACK_DQI
			|| device->m_devType == EDT_TRACK_RWC)
		{
			continue;
		}

		// 点击报警图标转到试剂页面
		connect(device.get(), &QBaseDeviceWgt::signalShowReagentPage, this, &SystemOverviewWgt::signalShowReagentPage);

		// 维护态时点击设备状态
		connect(device.get(), &QBaseDeviceWgt::signalMaintain, this, &SystemOverviewWgt::signalMaintain);
	}

	// 刷新所有设备位置
	updateDevicesPos();

	// 报警设置Tab显隐
	m_warnSetWgt->setWarnVisible(haveISE, haveCH, haveIM, haveChWaste, haveImWaste);
}

void SystemOverviewWgt::initSamples()
{
	// 查询所有样本 只查询病人样本，排除质控品和校准品
	::tf::SampleInfoQueryCond querySampleCond;
	querySampleCond.__set_sampleTypes({ tf::SampleType::SAMPLE_TYPE_PATIENT });
	::tf::SampleInfoQueryResp sampleResults;
	if (!DcsControlProxy::GetInstance()->QuerySampleInfo(querySampleCond, sampleResults)
		|| sampleResults.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ULOG(LOG_ERROR, "QuerySampleInfo Failed");
		return;
	}

    if (sampleResults.lstSampleInfos.empty())
    {
        ULOG(LOG_INFO, "lstSampleInfos is empty");
        return;
    }

	// 遍历样本
	for (const auto& sample : sampleResults.lstSampleInfos)
	{
		m_mapSamples[sample.id] = sample;
		if (sample.status == tf::SampleStatus::SAMPLE_STATUS_PENDING)
		{
			m_toTestSampleCount++;
		}
	}

	// 总样本数量
	ui->label_sampleTotal->setText(QString("%1").arg(m_mapSamples.size()/*, 4, 10, QLatin1Char('0')*/));
	// 待测样本
	ui->label_testSample->setText(QString("%1").arg(m_toTestSampleCount/*, 4, 10, QLatin1Char('0')*/));
}

std::shared_ptr<QBaseDeviceWgt> SystemOverviewWgt::getDeviceBySn(const string &sn)
{
	// 查找指定设备序列号设备
	auto iter = find_if(m_vecDevices.begin(), m_vecDevices.end(), [&](const std::shared_ptr<QBaseDeviceWgt> &bdw){
		return bdw->m_devSN == sn;
	});

	if (iter != m_vecDevices.end())
	{
		return *iter;
	}
	else
	{
		return nullptr;
	}
}

void SystemOverviewWgt::updateDevicesPos()
{
	// 总宽度
	int totalWidth = 0;

	// 包含电脑设备
	if (m_computerDevice != nullptr)
	{
		m_computerDevice->move(totalWidth, 0);
		totalWidth += m_computerDevice->width() - OVERLAP_PX;
	}

	// 遍历设备 设置位置
	for (const auto& device : m_vecDevices)
	{
		device->move(totalWidth, 0);
		totalWidth += device->width() - OVERLAP_PX;
	}

	// 包含电脑设备需在末尾添加空白
	if (m_computerDevice != nullptr)
	{
		// 添加一个空白
		QLabel *blank = new QLabel(ui->scrollAreaWidgetContents);
		blank->setFixedSize(BLANK_W, BLANK_H);
		totalWidth += blank->width();
	}

	// 设置显示宽度
	ui->scrollAreaWidgetContents->setFixedWidth(totalWidth + OVERLAP_PX);

	// 如果长度没超过一屏 隐藏按钮
	if (ui->scrollAreaWidgetContents->width() < ONE_SCREEN_LEN)
	{
		setLeftRightVisible(false);
	}
}

void SystemOverviewWgt::setLeftRightVisible(const bool show)
{
	ui->btn_left->setVisible(show);
	ui->btn_right->setVisible(show);
}

bool SystemOverviewWgt::GetRgtAlarmReadFlag(const string& devSN)
{
	// 查询是否存在试剂耗材余量不足未读
	auto ret = DcsControlProxy::GetInstance()->QueryReagentAlarmReaded(devSN);
	if (ret.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ULOG(LOG_ERROR, "QueryReagentAlarmReaded(devSN=%s) failed, ret=%s", devSN, ToString(ret));

		// 查询失败，置为已读
		ret.value = true;
	}

	return ret.value;
}

bool SystemOverviewWgt::eventFilter(QObject *watched, QEvent *event)
{
	// 鼠标按下
	if (event->type() != QEvent::MouseButtonPress)
	{
		return false;
	}

	// 鼠标左键
	if (static_cast<QMouseEvent*>(event)->button() != Qt::LeftButton)
	{
		return false;
	}

	// 点击区域为ui->widget_overview
	if (watched == ui->widget_overview)
	{
		emit signalShowDataBrowse();
	}
	// 点击区域为ui->label_warn
	else if (watched == ui->label_warn)
	{
		emit signalWarning();
	}
	// 点击区域为ui->scrollArea
	//else if (watched == ui->scrollArea)
	//{
	//	m_isScrollArea = true;
	//	m_isScrolling = true;
	//	m_prevX = static_cast<QMouseEvent*>(event)->x();
	//}
	// 点击区域为IOM设备
	else
	{
		for (const auto &dev : m_vecDevices)
		{
			// 找到IOM设备
			if (dev->m_devType == EDT_TRACK_IOM
				&& dev.get() == watched)
			{
				emit signalSampleRackMonitor();
				break;
			}
		}
	}

	return QObject::eventFilter(watched, event);
}

void SystemOverviewWgt::showEvent(QShowEvent *event)
{
	// 现在已添加更新时间相关函数，是否SetIgnoreImSplUpdate函数可删除？
    WarnSetManager::GetInstance()->SetIgnoreImSplUpdate(false);

	// 更新瓶子状态（可能开瓶过期了）
	WarnSetManager::GetInstance()->OnUpdateTimeDependParam();
	WarnSetManager::GetInstance()->SetSysVisble(true);

    QWidget::showEvent(event);
}

void SystemOverviewWgt::hideEvent(QHideEvent *event)
{
    WarnSetManager::GetInstance()->SetIgnoreImSplUpdate(true);
	WarnSetManager::GetInstance()->SetSysVisble(false);

    QWidget::hideEvent(event);
}

void SystemOverviewWgt::slotClickSet()
{
	m_warnSetWgt->show();
}

void SystemOverviewWgt::OnSampleUpdate(tf::UpdateType::type enUpdateType, std::vector<tf::SampleInfo, std::allocator<tf::SampleInfo>> lstSIs)
{
	for (const auto &s : lstSIs)
	{
		// 只关注病人样本，排除质控品和校准品
		if (s.sampleType != tf::SampleType::SAMPLE_TYPE_PATIENT)
		{
			continue;
		}

		switch (enUpdateType)
		{
		case ::tf::UpdateType::type::UPDATE_TYPE_ADD: // 增加
		{
			if (!m_mapSamples.contains(s.id))
			{
				m_mapSamples[s.id] = s;
				if (s.status == tf::SampleStatus::SAMPLE_STATUS_PENDING)
				{
					m_toTestSampleCount++;
				}
			}
			break;
		}
		case ::tf::UpdateType::type::UPDATE_TYPE_DELETE: // 删除
		{
			if (m_mapSamples.contains(s.id))
			{
				if (m_mapSamples[s.id].status == tf::SampleStatus::SAMPLE_STATUS_PENDING)
				{
					m_toTestSampleCount--;
				}
				m_mapSamples.remove(s.id);
			}
			break;
		}
		case ::tf::UpdateType::type::UPDATE_TYPE_MODIFY: // 更新
		{
			if (m_mapSamples.contains(s.id))
			{
				// 原来是待测 现在变为待测 待测样本则减少
				if (m_mapSamples[s.id].status == tf::SampleStatus::SAMPLE_STATUS_PENDING
					&& s.status != tf::SampleStatus::SAMPLE_STATUS_PENDING)
				{
					m_toTestSampleCount--;
				}

				// 原来不是待测 现在变为待测 待测样本则增加
				if (m_mapSamples[s.id].status != tf::SampleStatus::SAMPLE_STATUS_PENDING
					&& s.status == tf::SampleStatus::SAMPLE_STATUS_PENDING)
				{
					m_toTestSampleCount++;
				}

				m_mapSamples[s.id] = s;
			}
			break;
		}
		default:
			break;
		}
	}

	// 总样本数量
	ui->label_sampleTotal->setText(QString("%1").arg(m_mapSamples.size()/*, 4, 10, QLatin1Char('0')*/));
	// 待测样本
	ui->label_testSample->setText(QString("%1").arg(m_toTestSampleCount/*, 4, 10, QLatin1Char('0')*/));
}

void SystemOverviewWgt::slotUpadteBottles(const QMap<QString, QMap<EnumBottlePos, EnumWarnLevel>> &bottleInfos)
{
	// 生化单机
	if (m_spChSingleDev)
	{
		// 统计ch、ise设备的报警
		auto devCh = QString::fromStdString(m_spChSingleDev->m_devCh.devSN);
		auto devIse = QString::fromStdString(m_spChSingleDev->m_devIse.devSN);
		QMap<EnumBottlePos, EnumWarnLevel> totalDatas;
		if (bottleInfos.contains(devCh))
		{
			totalDatas = bottleInfos[devCh];
		}

		// 生化单机时，将ise的报警归类到生化耗材中
		if (bottleInfos.contains(devIse))
		{
			EnumWarnLevel iseLevel = EWL_NOTICE;
			for (auto it = bottleInfos[devIse].begin(); it != bottleInfos[devIse].end(); ++it)
			{
				iseLevel = max(iseLevel, it.value());
			}

			if (totalDatas.contains(EBP_CH_SUPPLY))
			{
				EnumWarnLevel chLevel = totalDatas[EBP_CH_SUPPLY];
				totalDatas[EBP_CH_SUPPLY] = max(iseLevel, chLevel);
			}
			else
			{
				totalDatas[EBP_CH_SUPPLY] = iseLevel;
			}
		}

		m_spChSingleDev->upadteBottle(totalDatas);

		return;
	}

	// 遍历设备
	for (auto &dev : m_vecDevices)
	{
		// 不用更新瓶子的设备
		if (dev->m_devType == EDT_UNKNOWN
			|| dev->m_devType == EDT_TRACK_IOM
			|| dev->m_devType == EDT_TRACK_DQI
			|| dev->m_devType == EDT_TRACK_RWC)
		{
			continue;
		}

		// 生化2000速特殊处理
		if (dev->m_devType == EDT_C1005_2000)
		{
			auto c10052000dev = dynamic_cast<C1005V2000DeviceWgt*>(dev.get());
			if (c10052000dev)
			{
				auto devA = QString::fromStdString(c10052000dev->m_devA.devSN);
				auto devB = QString::fromStdString(c10052000dev->m_devB.devSN);
				// 判断2000速是否包含两个设备
				if (devA.isEmpty() || devB.isEmpty())
				{
					ULOG(LOG_ERROR, "CH 2000s devSn error");
					continue;
				}
				// 统计A、B设备的报警
				QMap<EnumBottlePos, EnumWarnLevel> totalDatas;
				if (bottleInfos.contains(devA))
				{
					totalDatas = bottleInfos[devA];
				}
				if (bottleInfos.contains(devB))
				{
					for (auto it = bottleInfos[devB].begin(); it != bottleInfos[devB].end(); ++it)
					{
						// 高级别报警 跳过
						if (totalDatas.contains(it.key()) && totalDatas[it.key()] == EWL_WARNING)
						{
							continue;
						}
						totalDatas[it.key()] = it.value();
					}
				}

				c10052000dev->upadteBottle(totalDatas);
			}

			continue;
		}

		// 其他的设备
		auto devSN = QString::fromStdString(dev->m_devSN);
		bottleInfos.contains(devSN) ? dev->upadteBottle(bottleInfos[devSN]) : dev->upadteBottle({});
	}
}

void SystemOverviewWgt::slotUpadteDeviceMaskStatus(const QString& groupName, const QString& devSn, bool masked)
{
	auto strDevSN = devSn.toStdString();

	ULOG(LOG_INFO, "%s(groupName=%s, devSn=%s, masked=%d)", __FUNCTION__, groupName.toStdString(), strDevSN, masked);

	// 生化单机特殊处理
	if (m_spChSingleDev)
	{
		bool tmpChMask = m_spChSingleDev->m_devCh.mask;
		bool tmpIseMask = m_spChSingleDev->m_devIse.mask;
		if (m_spChSingleDev->m_devCh.devSN == strDevSN)
		{
			m_spChSingleDev->m_devCh.mask = masked;
		}
		else if (m_spChSingleDev->m_devIse.devSN == strDevSN)
		{
			m_spChSingleDev->m_devIse.mask = masked;
		}

		if (tmpChMask != m_spChSingleDev->m_devCh.mask
			|| tmpIseMask != m_spChSingleDev->m_devIse.mask)
		{
			// 更新状态
			m_spChSingleDev->changeStatus();
		}

		return;
	}

	std::shared_ptr<QBaseDeviceWgt> currDev = nullptr;

	// 生化2000速
	if (!groupName.isEmpty())
	{
		currDev = getDeviceBySn(groupName.toStdString());
	}
	// 其他
	else
	{
		currDev = getDeviceBySn(strDevSN);
	}

	if (currDev == nullptr)
	{
		ULOG(LOG_INFO, "can not find groupName=%s, devSN=%s, no need update mask.", groupName.toStdString(), strDevSN);
		return;
	}

	switch (currDev->m_devType)
	{
	case EDT_C1005_2000:	// 生化2000速
	{
		auto ch2000Dev = dynamic_cast<C1005V2000DeviceWgt*>(currDev.get());
		bool tmpAMask = ch2000Dev->m_devA.mask;
		bool tmpBMask = ch2000Dev->m_devB.mask;

		if (ch2000Dev->m_devA.devSN == strDevSN)
		{
			ch2000Dev->m_devA.mask = masked;
		}
		else if (ch2000Dev->m_devB.devSN == strDevSN)
		{
			ch2000Dev->m_devB.mask = masked;
		}

		if (tmpAMask != ch2000Dev->m_devA.mask
			|| tmpBMask != ch2000Dev->m_devB.mask)
		{
			// 更新状态
			ch2000Dev->changeStatus();
		}

		break;
	}
	default: // 其他设备
	{
		if (currDev->m_mask != masked)
		{
			currDev->m_mask = masked;

			// 更新状态
			currDev->changeStatus();
		}
		break;
	}
	}
}

void SystemOverviewWgt::OnDateTimeFormatUpdate(const BaseSet& ds)
{
	QString strDateTimeFormat;
	// 如果没有获取到时间日期格式信息
	if (ds.strDateForm.empty())
	{
		strDateTimeFormat = "MM/dd";
	}
	else
	{
		strDateTimeFormat = QString::fromStdString(ds.strDateForm).replace("yyyy/", "");
		strDateTimeFormat = strDateTimeFormat.replace("/yyyy", "");
	}

	// 更新日期时间显示格式
	m_strDateTimeFormat = strDateTimeFormat;

	updateCurrentTime(m_date);
}

void SystemOverviewWgt::OnDeviceCountDownUpdate(int totalSeconds, const QMap<QString, int> &sn2seconds)
{
	// 需判断等于0的情况，将倒计时直接置为0
	if (totalSeconds >= 0)
	{
		m_currTotalTime = totalSeconds;
		if (!m_timer->isActive())
		{
			m_timer->start();
		}
	}

	// 显示分钟
	int minutes = (m_currTotalTime + 59) / 60;
	ui->label_remainTime->setText(minutes > 30 ? ">30" : QString::number(minutes));

	// 生化单机特殊处理
	if (m_spChSingleDev)
	{
		// 生化、ise设备序列号
		auto devSnCh = QString::fromStdString(m_spChSingleDev->m_devCh.devSN);
		auto devSnIse = QString::fromStdString(m_spChSingleDev->m_devIse.devSN);
		if (sn2seconds.contains(devSnCh))
		{
			m_spChSingleDev->updateTime(m_spChSingleDev->m_devCh.devSN, sn2seconds[devSnCh]);
		}
		if (sn2seconds.contains(devSnIse))
		{
			m_spChSingleDev->updateTime(m_spChSingleDev->m_devIse.devSN, sn2seconds[devSnIse]);
		}

		return;
	}

	// 遍历设备
	for (auto &dev : m_vecDevices)
	{
		// 不用更新时间的设备
		if (dev->m_devType == EDT_UNKNOWN
			|| dev->m_devType == EDT_TRACK_IOM
			|| dev->m_devType == EDT_TRACK_DQI
			|| dev->m_devType == EDT_TRACK_RWC)
		{
			continue;
		}

		// 生化2000速特殊处理
		if (dev->m_devType == EDT_C1005_2000)
		{
			auto c10052000dev = dynamic_cast<C1005V2000DeviceWgt*>(dev.get());
			if (c10052000dev)
			{
				// A、B序列号
				auto devA = QString::fromStdString(c10052000dev->m_devA.devSN);
				auto devB = QString::fromStdString(c10052000dev->m_devB.devSN);

				// 判断2000速是否包含两个设备
				if (devA.isEmpty() || devB.isEmpty())
				{
					ULOG(LOG_ERROR, "CH 2000s devSn error");
					continue;
				}
				if (sn2seconds.contains(devA))
				{
					c10052000dev->updateTime(c10052000dev->m_devA.devSN, sn2seconds[devA]);
				}
				if (sn2seconds.contains(devB))
				{
					c10052000dev->updateTime(c10052000dev->m_devB.devSN, sn2seconds[devB]);
				}
			}

			continue;
		}

		// 其他的设备
		auto devSN = QString::fromStdString(dev->m_devSN);
		if (sn2seconds.contains(devSN))
		{
			dev->updateTime(sn2seconds[devSN]);
		}
	}
}

void SystemOverviewWgt::OnDevStateChange(tf::DeviceInfo deviceInfo)
{
	ULOG(LOG_INFO, "%s(deviceInfo=[ %s ])", __FUNCTION__, ToString(deviceInfo));

	// 生化单机
	if (m_spChSingleDev)
	{
		if (deviceInfo.deviceSN != m_spChSingleDev->m_devCh.devSN && deviceInfo.deviceSN != m_spChSingleDev->m_devIse.devSN)
		{
			ULOG(LOG_INFO, "not ch single care device sn.");
			return;
		}

		// 缓存老的状态
		int tmpChStatus = m_spChSingleDev->m_devCh.status;
		int tmpIseStatus = m_spChSingleDev->m_devIse.status;
		int tmpIomStatus = m_spChSingleDev->m_iomStatus;

		if (deviceInfo.deviceSN == m_spChSingleDev->m_devCh.devSN)
		{
			m_spChSingleDev->m_devCh.status = deviceInfo.status;
			m_spChSingleDev->m_iomStatus = deviceInfo.iomStatus;
		}
		else if (deviceInfo.deviceSN == m_spChSingleDev->m_devIse.devSN)
		{
			m_spChSingleDev->m_devIse.status = deviceInfo.status;
		}

		// 状态有变化
		if (m_spChSingleDev->m_devCh.status != tmpChStatus 
			|| m_spChSingleDev->m_devIse.status != tmpIseStatus
			|| m_spChSingleDev->m_iomStatus != tmpIomStatus)
		{
			m_spChSingleDev->changeStatus();
		}

		return;
	}

	std::shared_ptr<QBaseDeviceWgt> currDev = nullptr;

	if (deviceInfo.groupName.empty())
	{
		// 轨道设备 且联机
		if (deviceInfo.deviceType == tf::DeviceType::DEVICE_TYPE_TRACK 
			&& DictionaryQueryManager::GetInstance()->GetPipeLine())
		{
			// 解析轨道模块
			track::bcyime::TrackOtherInfo tdoi;
			if (!DecodeJson(tdoi, deviceInfo.otherInfo))
			{
				ULOG(LOG_ERROR, "DecodeJson Failed.");
				return;
			}

			for (const auto& node : tdoi.node_infos)
			{
				auto dev = getDeviceBySn(deviceInfo.deviceSN + "-" + to_string(node.first));
				if (dev && dev->m_devStatus != node.second.status)
				{
					dev->m_devStatus = node.second.status;
					dev->changeStatus();
				}
			}

			return;
		}

		currDev = getDeviceBySn(deviceInfo.deviceSN);
	}
	// 2000速设备
	else
	{
		currDev = getDeviceBySn(deviceInfo.groupName);
	}

	if (currDev == nullptr)
	{
		ULOG(LOG_INFO, "can not find groupName=%s, devSN=%s, no need update status.", deviceInfo.groupName, deviceInfo.deviceSN);
		return;
	}

	switch (currDev->m_devType)
	{
	case EDT_I6000_SINGLE: 	// 免疫单机
	{
		auto imSingleDev = dynamic_cast<I6000SingleDeviceWgt*>(currDev.get());
		if (imSingleDev->m_devStatus != deviceInfo.status || imSingleDev->m_iomStatus != deviceInfo.iomStatus)
		{
			imSingleDev->m_devStatus = deviceInfo.status;
			imSingleDev->m_iomStatus = deviceInfo.iomStatus;
			imSingleDev->changeStatus();
		}
		break;
	}
	case EDT_C1005_2000: 	// 生化2000速
	{
		auto ch2000Dev = dynamic_cast<C1005V2000DeviceWgt*>(currDev.get());

		// 缓存老的状态
		int tmpChAStatus = ch2000Dev->m_devA.status;
		int tmpChBStatus = ch2000Dev->m_devB.status;

		if (deviceInfo.deviceSN == ch2000Dev->m_devA.devSN)
		{
			ch2000Dev->m_devA.status = deviceInfo.status;
		}
		else if (deviceInfo.deviceSN == ch2000Dev->m_devB.devSN)
		{
			ch2000Dev->m_devB.status = deviceInfo.status;
		}

		// 状态有变化
		if (ch2000Dev->m_devA.status != tmpChAStatus
			|| ch2000Dev->m_devB.status != tmpChBStatus)
		{
			ch2000Dev->changeStatus();
		}

		break;
	}
	default:	// 其他设备
	{
		// 更新状态
		if (currDev->m_devStatus != deviceInfo.status)
		{
			currDev->m_devStatus = deviceInfo.status;
			currDev->changeStatus();
		}
		break;
	}
	}
}
