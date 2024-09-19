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
/// @file     mainwidget.cpp
/// @brief    主窗口界面
///
/// @author   4170/TangChuXian
/// @date     2020年4月29日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2020年4月29日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "mainwidget.h"
#include "ui_mainwidget.h"

#include <QDateTime>
#include <QSignalMapper>
#include <QMenu>
#include <QDebug>
#include <QMovie>
#include <QTimer>
#include <QMouseEvent>
#include <QProcess>
#include "QGridLayout"

#include "printcom.hpp"
#include "analysisdlg.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/tipdlg.h"
#include "shared/CommonInformationManager.h"
#include "shared/mcprogressdlg.h"
#include "shared/basedlg.h"

#include "manager/SystemConfigManager.h"
#include "manager/UserInfoManager.h"
#include "manager/DictionaryQueryManager.h"
#include "manager/SystemPowerManager.h"

#include "thrift/DcsControlProxy.h"
#include "menu/menuwidget.h"
#include "calibration/QSwitchCalibrateWidget.h"
#include "QC/qctabwidget.h"
#include "reagent/QSwitchReagentWidget.h"
#include "utility/utilityWidget.h"
#include "workplace/QWorkDeskNavigation.h"
#include "systemoverview/SystemOverviewWgt.h"

#include "ShutDownDlg.h"
#include "DeviceStatusWidget.h"

#include "src/leonis/Controller/alarmDialog.h"
#include "src/public/ConfigDefine.h"
#include "src/public/SerializeUtil.hpp"
#include "src/public/track/TrackConfigSerialize.h"
#include "src/common/common.h"
#include "src/common/Mlog/mlog.h"

#define SYS_DATETIME_UPDATE_INTERVAL                (1000)                    // 系统时间更新间隔(单位：毫秒)

#define TAB_INDEX_WORK                              (0)                       // 工作索引
#define TAB_INDEX_REGENT                            (1)                       // 试剂索引
#define TAB_INDEX_CAL                               (2)                       // 校准索引
#define TAB_INDEX_QC                                (3)                       // 质控索引
#define TAB_INDEX_UTILITY                           (4)                       // 应用索引

#define SYSTEM_TIME_MANUAL_CHANGED_FLAG             (10)                      // 几秒触发系统时间手动更改判定

#define ALAARM_BTN_QSS                            ("background:url(\":/Leonis/resource/image/btn-alarm-normal.png\");")       // 报警按钮样式

#define DEVICE_STATE_LABEL_HIGHI                   (42)                       // 设备状态标签的高度
#define DEVICE_STATE_LABEL_WIDTH                   (72)                       // 设备状态标签的宽度
#define DEVICE_STATE_SUB_LABEL_HIGHI               (13)                       // 设备状态子标签的高度
#define DEVICE_STATE_SUB_LABEL_WIDTH               (60)                       // 设备状态子标签的宽度
#define DEVICE_STATE_SUB_LABEL_X                   (6)                        // 设备状态子标签的X坐标
#define DEVICE_STATE_SUB_LABEL_Y                   (4)                        // 设备状态子标签的Y坐标

#define ALARM_PROPERTY                             ("level")                  // 报警图标动态属性

// 报警图标动态属性值
#define ALARM_PROPERTY_NONE                        ("none")                   // 报警图标动态属性:无
#define ALARM_PROPERTY_TRANSPARENT                 ("transparent")            // 报警图标动态属性:透明
#define ALARM_PROPERTY_ATTENTION                   ("attention")              // 报警图标动态属性:注意
#define ALARM_PROPERTY_SAMPLE_STOP                 ("samplestop")             // 报警图标动态属性:加样停
#define ALARM_PROPERTY_STOP                        ("stop")                   // 报警图标动态属性:停机

// 暂停和停止按钮动态属性
#define CAN_CLICK_PROPERTY                         ("canClick")               // 是否可以点击动态属性
#define ERR_MSG_PROPERTY                           ("errMsg")                 // 错误信息动态属性

MainWidget::MainWidget(std::function<void()> cbExit, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWidget)
    , m_cbExit(cbExit)
	, m_bIsInit(false)
	, m_menuWidget(nullptr)
	, m_pAnalysisDlg(nullptr)
	, m_pSystemOverviewWgt(nullptr)
{
	m_pSystemOverviewWgt = new SystemOverviewWgt(this);
	m_pSystemOverviewWgt->reject();

    ui->setupUi(this);
	setWindowIcon(QIcon(":/Leonis/resource/image/Mc64.ico"));
	
    // 设置报警动图显示标签鼠标事件穿透
    ui->AlarmLabel->hide();
    ui->AlarmLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->alarm_font_label->setAttribute(Qt::WA_TransparentForMouseEvents);

    // 初始化报警图标
    SetCtrlPropertyAndUpdateQss(ui->AlarmBtn, ALARM_PROPERTY, ALARM_PROPERTY_NONE);
    SetCtrlPropertyAndUpdateQss(ui->alarm_font_label, ALARM_PROPERTY, ALARM_PROPERTY_NONE);

    // 初始化报警标签动图
    m_pSampleStopMovie = new QMovie(":/Leonis/resource/image/btn-alarm.gif");
    m_pStopMovie = new QMovie(":/Leonis/resource/image/btn-alarm-stop.gif");
    m_pAttentionMovie = new QMovie(":/Leonis/resource/image/btn-alarm-attention.gif");

    // 注册消息处理槽函数
    RegsterMsgHandlerBeforeShow();

    m_pAlarmDialog = new QAlarmDialog(this);
    m_pAnalysisDlg = new AnalysisDlg(this);

    // 初始化设备状态标签
    InitDeviceStateLabel();

    // 初始化堆栈窗口
    InitStackWidget();

    m_pAlarmDialog->hide();

    // 报警等级改变，刷新报警按钮图标
    connect(m_pAlarmDialog, &QAlarmDialog::AlarmLevelChanged, this, &MainWidget::OnAlarmLevelChanged);
    
    // 初始化查询一次当前告警记录，同步dcs在UI未初始化完成时产生的报警
    m_pAlarmDialog->LoadAlarmBeforeInit(DcsControlProxy::GetInstance()->QueryCurAlarm());

    // 刷新打印机状态
    connect(this, &MainWidget::PrintStatusChanged, this, [&](bool isConnected)
    {
        // 打印机
        SetCtrlPropertyAndUpdateQss(ui->printf_label, "bks", isConnected ? "print_connect" : "print_connect_dis");

		// 更新系统总览打印机状态
		m_pSystemOverviewWgt->updatePrintConnection(isConnected);
    });

    // 初始化历史状态
    SetCtrlPropertyAndUpdateQss(ui->lis_label, "bks", "lis_connect_dis");

    // 初始化右上方按钮
    UpdateSampleStopAndStopBtn();
    OnUpdateAnalysisBtnStatus();
}

MainWidget::~MainWidget()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
    // 退出前需执行的回调函数
    if (m_cbExit != nullptr)
    {
        m_cbExit();
    }
	ULOG(LOG_INFO, "%s end.", __FUNCTION__);
}

void MainWidget::showSystemOverviewWidget()
{
	if (m_pSystemOverviewWgt == nullptr)
	{
		ULOG(LOG_ERROR, "m_pSystemOverviewWgt is nullptr");
		return;
	}

	m_pSystemOverviewWgt->showFullScreen();
}

void MainWidget::UpdatePrintDevStatus(bool isConnected)
{  
    ULOG(LOG_INFO, "%s(IsConnect:%d)", __FUNCTION__, isConnected);

    // 只在主线程中更新UI
    emit PrintStatusChanged(isConnected);    
}

void MainWidget::OnSwitchTab(int iTabIndex)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    QString text;

    // 检查应用页面是否有未保存的数据
    bool cancelOpt = false;
    for (int i = 0; i < m_vTabBtns.size(); i++)
    {
        // 原来选中应用页面，新选中应用页面
        if (m_vTabBtns[i]->isChecked() && i == TAB_INDEX_UTILITY
            || iTabIndex == TAB_INDEX_UTILITY)
        {
            if (m_pUtilityWidget->CurrentWidgetHasUnSaveData())
            {
                cancelOpt = true;
            }

            break;
        }
    }

    // 将选中索引按钮设置为按下状态，其他按钮置为松开状态
    for (int i = 0; i < m_vTabBtns.size(); i++)
    {
        if (i == iTabIndex)
        {
            m_vTabBtns[i]->setChecked(true);
            text = m_vTabBtns[i]->text();
        }
        else
        {
            m_vTabBtns[i]->setChecked(false);
        }
    }

    // 未保存数据，取消切换操作
    if (cancelOpt)
    {
        return;
    }

    // 将堆栈窗口切换到对应索引
    ui->CentralStackedWidget->setCurrentIndex(iTabIndex);

	QString showText = "> " + text;

	// 根据页面显示相应的二级菜单
	switch (iTabIndex)
	{
	case TAB_INDEX_WORK:
		break;
	case TAB_INDEX_REGENT:
	{
		QGridLayout *layout = qobject_cast<QGridLayout*>(ui->CentralStackedWidget->widget(TAB_INDEX_REGENT)->layout());
		if (layout)
		{
			QSwitchReagentWidget* regSpyWgt = qobject_cast<QSwitchReagentWidget*>(layout->itemAt(0)->widget());
			if (regSpyWgt)
			{
				showText += " > " + regSpyWgt->getTabText();
			}
		}
		break;
	}
	case TAB_INDEX_CAL:
		break;
	case TAB_INDEX_QC:
		break;
	case TAB_INDEX_UTILITY:
		break;
	default:
		break;
	}

    m_pUtilityWidget->SetCurrentWidgetIsMain();

    // 左下角，显示当前所在模块
    ui->MenuLocLab->setText(showText);
}

void MainWidget::OnExitBtnClicked()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (SystemPowerManager::GetInstance() != nullptr)
	{
		connect(SystemPowerManager::GetInstance().get(), &SystemPowerManager::ReadyToExit, this, [&] {

			// 关闭打开的顶层窗口
			this->CloseTopLevelWindow();
			this->setVisible(false);
		},Qt::UniqueConnection);
	}
	
	std::shared_ptr<ShutDownDlg> spShutDownDlg(new ShutDownDlg(this));
	spShutDownDlg->exec();

}

void MainWidget::OnAnalysisBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 弹出开始分析对话框
	if (m_pAnalysisDlg == nullptr)
	{
		m_pAnalysisDlg = new AnalysisDlg(this);
	}

	if (ui->AnalysisBtn->property(ASSAY_TEST_ENABLE).toBool())
	{
        // 处理开始弹窗弹出时，监控页面点击无法弹出开始弹窗
		m_pAnalysisDlg->hide();
		m_pAnalysisDlg->show();
	}
    else
    {
        TipDlg(m_pAnalysisDlg->GeterrText()).exec();
    }
}

///
/// @brief 自动进架按钮按下消息
///
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2024年2月6日，新建函数
///
void MainWidget::OnAnalysisBtnClicked_continueRun()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 弹出开始分析对话框
	if (m_pAnalysisDlg == nullptr)
	{
		m_pAnalysisDlg = new AnalysisDlg(this);
	}

	if (ui->AnalysisBtn->property(ASSAY_TEST_ENABLE).toBool())
	{
		m_pAnalysisDlg->ContinueRun();
	}
	else
	{
		TipDlg(m_pAnalysisDlg->GeterrText()).exec();
	}
}

///
/// @brief 加样停按钮被点击
///
///
/// @return 
///
/// @par History:
/// @li 1556/Chenjianlin，2023年9月6日，代码修改
///
void MainWidget::OnStopSamplingBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
   
    // 检查是否有能暂停的设备，没有则提示
    if (!ui->StopSamplingBtn->property(CAN_CLICK_PROPERTY).toBool())
    {
        TipDlg (ui->StopSamplingBtn->property(ERR_MSG_PROPERTY).toString()).exec();
        return;
    }

    // 加样停弹窗确认
    TipDlg td(tr("确定执行[暂停]操作吗？"), TipDlgType::TWO_BUTTON);
    td.SetButtonText(tr("执行"), tr("取消"));
    if (td.exec() == QDialog::Rejected)
    {
        return;
    }

	// 通知各模块暂停
    DcsControlProxy::GetInstance()->PauseTest();
}

void MainWidget::OnStopBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 检查是否有能停止的设备，没有则提示
    if (!ui->StopBtn->property(CAN_CLICK_PROPERTY).toBool())
    {
        TipDlg (ui->StopBtn->property(ERR_MSG_PROPERTY).toString()).exec();
        return;
    }

    // 停机弹窗确认
    TipDlg td(tr("确定执行[停止]操作吗？"), TipDlgType::TWO_BUTTON);
    td.SetButtonText(tr("执行"), tr("取消"));
    if (td.exec() == QDialog::Rejected)
    {
        return;
    }

    DcsControlProxy::GetInstance()->StopTest();  
}

void MainWidget::OnAlarmBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 停止报警音
	m_pAlarmDialog->StopAlarmSound();

    // 显示报警弹窗(hide再show的原因是:当m_pAlarmDialog已经显示了，系统总览自动弹出后，点击报警按钮无法正常弹出)
	m_pAlarmDialog->hide();
	m_pAlarmDialog->show();

    // 刷新报警按钮状态
    OnAlarmLevelChanged(AlarmModel::Instance().GetCurrentAlarmLevel());
}

void MainWidget::OnUpdateCurTime()
{
    // 获取上一次时间，用于判断时间是否修改
    static auto s_lastDateTime = QDateTime::currentDateTime();;

    // 当前时间显示
    QDateTime curDateTime = QDateTime::currentDateTime();
    ui->CurDateTimeLab->setText(curDateTime.toString(m_strDateTimeFormat));

	// 更新系统总览界面时间
	if (m_pSystemOverviewWgt)
	{
		m_pSystemOverviewWgt->updateCurrentTime(curDateTime);
	}

    // 日期不再同一天，则发送日期修改消息
    if (qAbs(s_lastDateTime.daysTo(curDateTime)) >= 1)
    {
        POST_MESSAGE(MSG_ID_SYSTEM_DATE_MANUAL_CHANGED);
    }

    // 赋值上一次日期时间
    s_lastDateTime = curDateTime;
}

void MainWidget::OnCurMenuLocChange(QString strCurMenuLocate)
{
    // 左下角，显示当前所在模块
    ui->MenuLocLab->setText(strCurMenuLocate);
}

void MainWidget::OnStatDlgAssayBtnClicked(int iBtnIndex)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
}

void MainWidget::OnStatDlgSampSizeComboChanged(int iIndex)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
}

void MainWidget::OnP300StatMenuItemActive()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
}

void MainWidget::OnP300StatOkBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
}

void MainWidget::OnU2000StatMenuItemActive()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
}

void MainWidget::InitAfterShow()
{
	// 初始化成员变量
	InitMemberVar();

	// 初始化信号槽连接
	InitConnect();
}

void MainWidget::InitMemberVar()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 设置登录用户
    std::shared_ptr<tf::UserInfo> pUserInfo = UserInfoManager::GetInstance()->GetLoginUserInfo();
    QString userName = (pUserInfo != nullptr && pUserInfo->__isset.nickname) ? QString::fromStdString(pUserInfo->nickname) : "Invalid";  
    if (ui->UserLab->fontMetrics().width(userName) > ui->UserLab->maximumWidth())
    {
        userName.insert(userName.size() / 2, "\n");
    }   
    ui->UserLab->setText(userName);

    // 实时更新当前时间定时器
    m_pGetCurdateTimeTimer = new QTimer(this);
    m_pGetCurdateTimeTimer->start(SYS_DATETIME_UPDATE_INTERVAL);

    // 初始化tab按钮
    ui->WorkBtn->setCheckable(true);
    ui->RegentBtn->setCheckable(true);
    ui->CalBtn->setCheckable(true);
    ui->QcBtn->setCheckable(true);
    ui->AppBtn->setCheckable(true);

    // 初始化tab按钮数组
    m_vTabBtns.push_back(ui->WorkBtn);
    m_vTabBtns.push_back(ui->RegentBtn);
    m_vTabBtns.push_back(ui->CalBtn);
    m_vTabBtns.push_back(ui->QcBtn);
    m_vTabBtns.push_back(ui->AppBtn);

    // 默认选中工作项
    ui->WorkBtn->setChecked(true);

	// 更新日期时间格式字符串
	BaseSet bsDateTimeFormat;
	DictionaryQueryManager::GetUiBaseSet(bsDateTimeFormat);
	UpdateDateTimeFormat(bsDateTimeFormat);

    // 进度条对话框
    m_pProgressDlg = std::make_shared<McProgressDlg>(this);
}

///
/// @brief 更新日期时间格式字符串
///
/// @param[in]  ds  格式信息
///
/// @return 
///
/// @par History: 0021368: [应用] 显示设置-日期格式界面修改日期格式为日月年，界面右上角日期只显示年份和时间
/// @li 1556/Chenjianlin，2023年8月15日，新建函数
///
void MainWidget::UpdateDateTimeFormat(const BaseSet& ds)
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
	// 加上时间
	strDateTimeFormat += "   hh:mm";
	// 12小时显示格式
	if (ds.iTimeForm == 1)
	{
		strDateTimeFormat += " AP";
	}
	// 更新日期时间显示格式
	m_strDateTimeFormat = strDateTimeFormat;
}

void MainWidget::InitStackWidget()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 设置对话框中心点
    BaseDlg::SetCenterPoint(ui->CentralStackedWidget->geometry().center());

    // 内部窗口全部设置布局
    for (int i = 0; i < ui->CentralStackedWidget->count(); i++)
    {
        QGridLayout* pLayout = new QGridLayout(ui->CentralStackedWidget->widget(i));
        pLayout->setSpacing(0);
        pLayout->setMargin(0);
    }

    // 添加对应的窗口
    ui->CentralStackedWidget->widget(TAB_INDEX_WORK)->layout()->addWidget(new QWorkDeskNavigation(ui->CentralStackedWidget->widget(TAB_INDEX_WORK)));
    ui->CentralStackedWidget->widget(TAB_INDEX_REGENT)->layout()->addWidget(new QSwitchReagentWidget(ui->CentralStackedWidget->widget(TAB_INDEX_REGENT)));
    ui->CentralStackedWidget->widget(TAB_INDEX_CAL)->layout()->addWidget(new QSwitchCalibrateWidget(ui->CentralStackedWidget->widget(TAB_INDEX_CAL)));
    ui->CentralStackedWidget->widget(TAB_INDEX_QC)->layout()->addWidget(new QcTabWidget(ui->CentralStackedWidget->widget(TAB_INDEX_QC)));

    m_pUtilityWidget = new UtilityWidget(ui->CentralStackedWidget->widget(TAB_INDEX_UTILITY));
    ui->CentralStackedWidget->widget(TAB_INDEX_UTILITY)->layout()->addWidget(m_pUtilityWidget);

    // 默认选中工作项
    ui->CentralStackedWidget->setCurrentIndex(TAB_INDEX_WORK);
}

void MainWidget::InitDeviceStateLabel()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 当前是否为单机版
    bool isPipeLine = DictionaryQueryManager::GetInstance()->GetPipeLine();
    auto  devMap = CommonInformationManager::GetInstance()->GetDeviceMaps();

    if (isPipeLine)
    {
        // 联机设备按序列号排序
        std::map<int, std::shared_ptr<const tf::DeviceInfo>> tempMap;
        for (auto& iter : devMap)
        {
            try
            {
                tempMap[std::stoi(iter.first)] = iter.second;
            }
            catch (const std::invalid_argument& e)
            {
                ULOG(LOG_INFO, "%s()", __FUNCTION__);
                continue;
            }
            catch (const std::out_of_range& e)
            {
                ULOG(LOG_INFO, "%s()", __FUNCTION__);
                continue;
            }
        }

        InitPipeDeviceStateLabel(tempMap);
    } 
    else
    {
        // 单机
        InitSingalDeviceStateLabel(devMap);
    }
}

void MainWidget::InitSingalDeviceStateLabel(std::map<std::string, std::shared_ptr<const tf::DeviceInfo>>& devMap)
{
    // 三个设备就是比色、ISE和轨道，不存在只有比色的情况
    if (devMap.size() == 3)
    {
        auto devWid = new DeviceStatusWidget(DeviceStatusWidget::ANALYSIS_TWO_DEVICE,
            "", { ::tf::DeviceType::DEVICE_TYPE_ISE1005, ::tf::DeviceType::DEVICE_TYPE_C1000 }, ui->device_status_widget);

        // 单机版设备状态标签固定位置为1
        ui->horizontalLayout->insertWidget(1, devWid);

        for (auto& iter : devMap)
        {
            auto& dev = iter.second;

            if (dev->deviceType == ::tf::DeviceType::DEVICE_TYPE_C1000)
            {
                devWid->SetName(QString::fromStdString(dev->name), true);
                devWid->AddDeviceSn(QString::fromStdString(dev->deviceSN));
                devWid->UpdateStatus(dev->status, true);
                devWid->UpdateMasked(dev->masked, true);
            }
            else if (dev->deviceType == ::tf::DeviceType::DEVICE_TYPE_ISE1005)
            {
                devWid->SetName(QString::fromStdString(dev->name), false);
                devWid->AddDeviceSn(QString::fromStdString(dev->deviceSN));
                devWid->UpdateStatus(dev->status, false);
                devWid->UpdateMasked(dev->masked, false);
            }
        }

        return;
    }
    // 两个设备就是免疫和轨道
    else if (devMap.size() == 2)
    {
        for (auto& iter : devMap)
        {
            auto& dev = iter.second;
            if (dev->deviceType == ::tf::DeviceType::DEVICE_TYPE_TRACK)
            {
                continue;
            }

            auto devWid = new DeviceStatusWidget(DeviceStatusWidget::ANALYSIS_ONE_DEVICE,
                QString::fromStdString(dev->name), { ::tf::DeviceType::DEVICE_TYPE_I6000 }, ui->device_status_widget);

            // 单机版设备状态标签固定位置为1
            ui->horizontalLayout->insertWidget(1, devWid);
            devWid->AddDeviceSn(QString::fromStdString(dev->deviceSN));
            devWid->UpdateStatus(dev->status, true);
            devWid->UpdateMasked(dev->masked, true);           
        }
    }
}

void MainWidget::InitPipeDeviceStateLabel(std::map<int, std::shared_ptr<const tf::DeviceInfo>>& devMap)
{
    int devLabCount = 1; 
    int analysisCount = 0;
    for (auto& iter : devMap)
    {
        const auto& dev = *(iter.second);
        if (dev.deviceType == ::tf::DeviceType::DEVICE_TYPE_TRACK)
        {
            // 联机板显示轨道各个子模块状态
            track::bcyime::TrackOtherInfo tdoi;
            if (!DecodeJson(tdoi, dev.otherInfo))
            {
                ULOG(LOG_ERROR, "Decode track other info failed!");
                return;
            }

            for (auto& item : tdoi.node_infos)
            {
                auto devLab = new DeviceStatusWidget(DeviceStatusWidget::OPTIONAL_DEVICE,
                    QString::fromStdString(item.second.name), { dev.deviceType }, ui->device_status_widget);

                ui->horizontalLayout->insertWidget(devLabCount, devLab);
                devLabCount++;

                devLab->UpdateStatus(item.second.status, true);
                devLab->AddDeviceSn(QString::number(item.second.nodeid));
            }
        }
        else if (dev.deviceType != ::tf::DeviceType::DEVICE_TYPE_TRACK)
        {
            // 根据设备类型设置不同的状态标签类型
            auto widType = DeviceStatusWidget::ANALYSIS_ONE_DEVICE;
            if (dev.subDeviceType == ::tf::DeviceType::DEVICE_TYPE_C2000)
            {
                widType = DeviceStatusWidget::ANALYSIS_TWO_DEVICE;
            }
            else if (dev.subDeviceType == ::tf::DeviceType::DEVICE_TYPE_ISE1005)
            {
                widType = DeviceStatusWidget::OPTIONAL_DEVICE;
            }

            // 统计分析仪数量
            if (widType != DeviceStatusWidget::OPTIONAL_DEVICE)
            {
                analysisCount++;
            }

            // 标签显示两个设备的时候特殊处理
            DeviceStatusWidget* devWid = nullptr;
            if (widType == DeviceStatusWidget::ANALYSIS_TWO_DEVICE)
            {
                bool isDeal = false;
                for (auto wid : ui->device_status_widget->findChildren<DeviceStatusWidget*>())
                {
                    if (wid->GetGroupName().toStdString() == dev.groupName && !dev.groupName.empty())
                    {
                        wid->SetName(QString::fromStdString(dev.groupName + dev.name), false);
                        wid->AddDeviceSn(QString::fromStdString(dev.deviceSN));
                        wid->UpdateStatus(dev.status, false);
                        wid->UpdateMasked(dev.masked, false);
                        isDeal = true;
                        break;
                    }
                }

                if (isDeal)
                {
                    continue;
                }

                devWid = new DeviceStatusWidget(widType,QString::fromStdString(dev.groupName + dev.name), 
                { dev.deviceType }, ui->device_status_widget);

                devWid->AddDeviceSn(QString::fromStdString(dev.deviceSN));
                devWid->SetGroupName(QString::fromStdString(dev.groupName));
                devWid->UpdateStatus(dev.status, true);
                devWid->UpdateMasked(dev.masked, true);
            }
            else
            {
                devWid = new DeviceStatusWidget(widType, QString::fromStdString(dev.name),
                { dev.deviceType }, ui->device_status_widget);

                devWid->AddDeviceSn(QString::fromStdString(dev.deviceSN));
                devWid->UpdateStatus(dev.status, true);
                devWid->UpdateMasked(dev.masked, true);
            }

            ui->horizontalLayout->insertWidget(devLabCount, devWid);
            devLabCount++;
        }
    }

    // 分析仪超过3个不显示导航窗口
    ui->MenuLocLab->setVisible(analysisCount <= 3);
}

void MainWidget::InitConnect()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 功能按钮
    connect(ui->AnalysisBtn, SIGNAL(clicked()), this, SLOT(OnAnalysisBtnClicked()));
    connect(ui->StopSamplingBtn, SIGNAL(clicked()), this, SLOT(OnStopSamplingBtnClicked()));
    connect(ui->StopBtn, SIGNAL(clicked()), this, SLOT(OnStopBtnClicked()));
    connect(ui->AlarmBtn, SIGNAL(clicked()), this, SLOT(OnAlarmBtnClicked()));
	connect(ui->OptionBtn, SIGNAL(clicked()), this, SLOT(OnExitBtnClicked()));

    // 定时器
    connect(m_pGetCurdateTimeTimer, SIGNAL(timeout()), this, SLOT(OnUpdateCurTime()));

    // tab页面跳转索引按钮
    QSignalMapper *pSignalMapper = new QSignalMapper(this);
    for (int i = 0; i < m_vTabBtns.size(); i++)
    {
        pSignalMapper->setMapping(m_vTabBtns[i], i);
        connect(m_vTabBtns[i], SIGNAL(clicked()), pSignalMapper, SLOT(map()));
    }
    connect(pSignalMapper, SIGNAL(mapped(int)), this, SLOT(OnSwitchTab(int)));

	// 系统总览界面传来的显示数据浏览信号
	connect(m_pSystemOverviewWgt, &SystemOverviewWgt::signalShowDataBrowse, this, &MainWidget::OnShowDataBrowsePage);
	// 系统总览界面传来的开始信号
	connect(m_pSystemOverviewWgt, &SystemOverviewWgt::signalStart, this, &MainWidget::OnAnalysisBtnClicked);
	// 系统总览界面传来的暂停信号
	connect(m_pSystemOverviewWgt, &SystemOverviewWgt::signalSuspend, this, &MainWidget::OnStopSamplingBtnClicked);
	// 系统总览界面传来的停止信号
	connect(m_pSystemOverviewWgt, &SystemOverviewWgt::signalStop, this, &MainWidget::OnStopBtnClicked);
	// 系统总览界面传来的报警信号
	connect(m_pSystemOverviewWgt, &SystemOverviewWgt::signalWarning, this, &MainWidget::OnAlarmBtnClicked);
	// 系统总览界面传来的样本架监视信号
	connect(m_pSystemOverviewWgt, &SystemOverviewWgt::signalSampleRackMonitor, this, &MainWidget::OnShowSampleRackMonitor);
	// 系统总览界面传来的显示试剂盘界面
	connect(m_pSystemOverviewWgt, &SystemOverviewWgt::signalShowReagentPage, this, &MainWidget::OnShowShowReagentPage);
	// 系统总览界面传来的显示维护界面
	connect(m_pSystemOverviewWgt, &SystemOverviewWgt::signalMaintain, this, &MainWidget::OnShowMaintainPage);

	// 监听继续测试消息
	REGISTER_HANDLER(MSG_ID_CONTINUE_RUN, this, OnAnalysisBtnClicked_continueRun);
}

void MainWidget::RegsterMsgHandlerBeforeShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 注册登录成功处理函数
	if (SystemConfigManager::GetInstance()->IsShowFullScreen())
	{
		REGISTER_HANDLER(MSG_ID_LOGIN_SUCCESSED, this, showFullScreen);
	}
	else
	{
		REGISTER_HANDLER(MSG_ID_LOGIN_SUCCESSED, this, showMaximized);
	}

	// 用户登录
	REGISTER_HANDLER(MSG_ID_LOGIN_SUCCESSED, this, UpdateLoginUser);
    // 注册设备状态改变处理槽函数
    REGISTER_HANDLER(MSG_ID_DEVS_STATUS_CHANGED, this, OnDevStateChange);
    // 注册设备当前状态倒计时改变
    REGISTER_HANDLER(MSG_ID_DEV_COUNT_DOWN_UPDATE, this, OnDevStateTimeChange);
	// 注册时间格式改变槽函数
	REGISTER_HANDLER(MSG_ID_DISPLAY_SET_UPDATE, this, OnDateTimeFormatUpDate);
    // 打开进度条对话框
    REGISTER_HANDLER(MSG_ID_PROGRESS_DIALOG_OPEN, this, OpenProgressDlg);
    // 关闭进度条对话框
    REGISTER_HANDLER(MSG_ID_PROGRESS_DIALOG_CLOSE, this, CloseProgressDlg);
    // 更新进度条进度
    REGISTER_HANDLER(MSG_ID_PROGRESS_DIALOG_CHANGE, this, UpdateProgressDlg);
    // 监听开始测试按钮状态
    REGISTER_HANDLER(MSG_ID_UPDATE_ASSAY_TEST_BTN, this, OnUpdateAnalysisBtnStatus);
    // 监听Lis状态
    REGISTER_HANDLER(MSG_ID_LIS_CONNECTION_STATUS, this, OnUpdateLisStatus);
    // 监听设备是否屏蔽
    REGISTER_HANDLER(MSG_ID_DEVICE_MASK_UPDATE, this, OnUpadteDeviceMaskStatus);
    // 设置界面左下角当前模块显示标签文字
    REGISTER_HANDLER(MSG_ID_CURRENT_MODEL_NAME, this, OnCurMenuLocChange);
    // 监听错误提示，用于多线程操作时在主界面提示用户
    REGISTER_HANDLER(MSG_ID_SHOW_ERROR_TEXT, this, OnShowErrText);
}

void MainWidget::showEvent(QShowEvent *event)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 基类先处理
    QWidget::showEvent(event);

    // 没有初始化则初始化
    if (!m_bIsInit)
    {
        // 置初始化标志并初始化
        m_bIsInit = true;
        InitAfterShow();

        // 更新当前时间
        OnUpdateCurTime();
    }
}

void MainWidget::OnDevStateChange(tf::DeviceInfo deviceInfo)
{
    ULOG(LOG_INFO, "%s(%s)", __FUNCTION__, ToString(deviceInfo));
	
    OnUpdateAnalysisBtnStatus();
    UpdateSampleStopAndStopBtn();

    if (!deviceInfo.__isset.status)
    {
        ULOG(LOG_INFO, "This device status is null");
        return;
    }
    else if (deviceInfo.status == ::tf::DeviceWorkState::DEVICE_STATUS_STANDBY)
    {
        // 仪器转为待机，当前所有注意以上的报警设为已处理
        AlarmLevelChangedAfterDevReset(QString::fromStdString(deviceInfo.deviceSN));
    }

	if (m_menuWidget)
	{
		m_menuWidget->SetExitBtnEnable(false);
	}

    if (!deviceInfo.__isset.deviceSN)
    {
        ULOG(LOG_ERROR, "Input Params Invalid!"); 
        return;
    }

    // 找到对应的设备标签
    const auto& CIM = CommonInformationManager::GetInstance();
    auto spDeviceInfo = CIM->GetDeviceInfo(deviceInfo.deviceSN);
    if (spDeviceInfo == nullptr)
    {
        ULOG(LOG_ERROR, "Get Device Info Failed By Sn:%s", deviceInfo.deviceSN);
        return;
    }

    if (DictionaryQueryManager::GetInstance()->GetPipeLine())
    {
        // 联机轨道特殊处理
        if (spDeviceInfo->deviceType == ::tf::DeviceType::DEVICE_TYPE_TRACK)
        {
            // 联机板显示轨道各个子模块状态
            track::bcyime::TrackOtherInfo tdoi;
            if (!DecodeJson(tdoi, spDeviceInfo->otherInfo))
            {
                ULOG(LOG_ERROR, "Decode track other info failed!");
                return;
            }

            for (auto& item : tdoi.node_infos)
            {
                for (auto lab : ui->device_status_widget->findChildren<DeviceStatusWidget*>())
                {
                    if (lab->GetDeviceSns(spDeviceInfo->deviceType).contains(QString::number(item.second.nodeid)))
                    {
                        lab->UpdateStatus(item.second.status, true);
                    }
                }
            }
        }
        else 
        {
            for (auto lab : ui->device_status_widget->findChildren<DeviceStatusWidget*>())
            {
                if (lab->GetDeviceSns(spDeviceInfo->deviceType).contains(QString::fromStdString(spDeviceInfo->deviceSN)))
                {
                    // 联机B设备显示到第二个
                    lab->UpdateStatus(spDeviceInfo->status, spDeviceInfo->name != "B");
                }
            }
        }
    } 
    else
    {
        // 单机不处理轨道
        if (spDeviceInfo->deviceType == ::tf::DeviceType::DEVICE_TYPE_TRACK)
        {
            return;
        }

        //单机版只有一个设备状态标签，ISE显示在第二个
        auto lab = ui->device_status_widget->findChild<DeviceStatusWidget*>();
        lab->UpdateStatus(spDeviceInfo->status, spDeviceInfo->deviceType != ::tf::DeviceType::DEVICE_TYPE_ISE1005);
        //lab->UpdateMasked(spDeviceInfo->masked, spDeviceInfo->deviceType != ::tf::DeviceType::DEVICE_TYPE_ISE1005);
    }
}

void MainWidget::OnDevStateTimeChange(int totalSeconds, const QMap<QString, int> &sn2seconds)
{
    bool isPipe = DictionaryQueryManager::GetInstance()->GetPipeLine();
    const auto& CIM = CommonInformationManager::GetInstance();
    for (auto iter = sn2seconds.begin(); iter != sn2seconds.end(); iter++)
    {
        for (auto& lab : ui->device_status_widget->findChildren<DeviceStatusWidget*>())
        {
            auto sn = iter.key();
            auto spDev = CIM->GetDeviceInfo(sn.toStdString());
            if (spDev == nullptr)
            {
                ULOG(LOG_ERROR, "Get device info failed by sn[%s]", sn.toStdString());
                continue;
            }

            if (lab->GetDeviceSns(spDev->deviceType).contains(sn))
            {
                // 联机B设备显示在第二个，单机ISE显示在第二个
                bool isFirst = isPipe ? spDev->name != "B" : spDev->deviceType != ::tf::DeviceType::DEVICE_TYPE_ISE1005;
                lab->UpdateTime(iter.value(), isFirst);
            }
        }
    }
}

void MainWidget::mousePressEvent(QMouseEvent *event)
{
	// 如果鼠标左键按下
	if (event->button() == Qt::LeftButton)
	{
		// 将logo现在的坐标转化为屏幕全局坐标
		auto logoPos = ui->LogoLabel->mapToGlobal(QPoint(0, 0));
		QRect globalRect = QRect(logoPos.x(), logoPos.y(), ui->LogoLabel->rect().width(), ui->LogoLabel->rect().height());
		// 点击区域为ui->LogoLabel
		if (globalRect.contains(event->globalPos()))
		{
			m_pSystemOverviewWgt->showFullScreen();
		}
	}

	QWidget::mousePressEvent(event);
}

void MainWidget::UpdateSampleStopAndStopBtn()
{    
    const auto& CIM = CommonInformationManager::GetInstance();
    auto devs = CIM->GetDeviceMaps();

    // 单机版移除轨道
    if (!DictionaryQueryManager::GetInstance()->GetPipeLine())
    {
        auto iter = std::find_if(devs.begin(), devs.end(), [](auto& dev)
        { return dev.second->deviceType == ::tf::DeviceType::DEVICE_TYPE_TRACK; });
        if (iter != devs.end())
        {
            devs.erase(iter);
        }
    }

    // 检查仪器状态，是否存在非加样停状态的仪器
    int count = std::count_if(devs.begin(), devs.end(), [](auto& dev) {
        return dev.second->status != tf::DeviceWorkState::DEVICE_STATUS_SAMPSTOP
               && dev.second->status != tf::DeviceWorkState::DEVICE_STATUS_STOP_PUSH_RACK
                && dev.second->status != tf::DeviceWorkState::DEVICE_STATUS_SAMPSTOP_FAULT
                && !dev.second->masked;
    });
    if (count <= 0)
    {
        ui->StopSamplingBtn->setProperty(ERR_MSG_PROPERTY, tr("当前所有未屏蔽的仪器已是暂停状态"));
        SetCtrlPropertyAndUpdateQss(ui->StopSamplingBtn, CAN_CLICK_PROPERTY, false);
    }
    else
    {
        // 只能在运行状态下才能切换到暂停状态 0022114: [控制器] 仪器处于待机或维护状态下，手动点击“暂停”按钮，弹出确认暂停的提示弹窗
        int iRunningStatusDev = std::count_if(devs.begin(), devs.end(), [](const auto& dev) {
            return dev.second->status == tf::DeviceWorkState::DEVICE_STATUS_RUNNING;
        });
        if (iRunningStatusDev <= 0)	// 若没有设备处于运行状态
        {
            ui->StopSamplingBtn->setProperty(ERR_MSG_PROPERTY, tr("仪器必须是运行状态才能执行[暂停]操作"));
            SetCtrlPropertyAndUpdateQss(ui->StopSamplingBtn, CAN_CLICK_PROPERTY, false);
        }
        else
        {
            SetCtrlPropertyAndUpdateQss(ui->StopSamplingBtn, CAN_CLICK_PROPERTY, true);
        }
    }

    // 检查仪器状态，是否存在可停止的仪器
    int stopCount = std::count_if(devs.begin(), devs.end(), [](auto& dev) {
        return dev.second->status != tf::DeviceWorkState::DEVICE_STATUS_DISCONNECT
            && dev.second->status != tf::DeviceWorkState::DEVICE_STATUS_HALT
            && !dev.second->masked;
    });
    if (stopCount <= 0)
    {
        ui->StopBtn->setProperty(ERR_MSG_PROPERTY, tr("当前所有未屏蔽的仪器已是停止或者断开连接状态，不能执行[停止]操作"));
        SetCtrlPropertyAndUpdateQss(ui->StopBtn, CAN_CLICK_PROPERTY, false);
    }
    else
    {
        SetCtrlPropertyAndUpdateQss(ui->StopBtn, CAN_CLICK_PROPERTY, true);
    }
}

void MainWidget::CloseTopLevelWindow()
{
	auto topLevelWidgets = QApplication::topLevelWidgets();
	for (auto w : topLevelWidgets)
	{
		if (w != this && w->isVisible())
		{
			w->close();
		}
	}
}

void MainWidget::OnDateTimeFormatUpDate(const BaseSet& ds)
{
	// 更新日期时间格式
	UpdateDateTimeFormat(ds);
	// 刷新时间
	OnUpdateCurTime();
}

///
/// @brief
///     打开进度条对话框
///
/// @par History:
/// @li 6889/ChenWei，2023年3月27日，新建函数
///
void MainWidget::OpenProgressDlg(QString strTitle, bool bIndicator)
{
    m_pProgressDlg->SetProgressTitle(strTitle);
    m_pProgressDlg->SetModule(bIndicator);
    m_pProgressDlg->exec();
}

///
/// @brief
///     关闭进度条对话框
///
/// @par History:
/// @li 6889/ChenWei，2023年3月27日，新建函数
///
void MainWidget::CloseProgressDlg(bool)
{
    m_pProgressDlg->close();
}

///
/// @brief
///     更新进度条
///
/// @param[in]  ProgressInfo  进度信息
///
/// @par History:
/// @li 6889/ChenWei，2023年3月27日，新建函数
///
void MainWidget::UpdateProgressDlg(int iPro)
{
    m_pProgressDlg->UpdateProgressRate(iPro);
}

void MainWidget::OnAlarmLevelChanged(int level)
{
    bool isShow = m_pAlarmDialog->isVisible();
    bool unRead = m_pAlarmDialog->IsExistUnReadAlarm();
    bool existReagent = AlarmModel::Instance().IsExistReagentAlarmBySn();

	// 系统总览报警等级
	int sysWarnLevel = tf::AlarmLevel::ALARM_LEVEL_TYPE_INVALID;

    // 弹窗未打开且存在未读的报警，报警图标闪烁
    // 无报警或者注意级别，不存在试剂/耗材的报警，显示蓝色
    if ((level == tf::AlarmLevel::ALARM_LEVEL_TYPE_CAUTION && !existReagent)
        || level == tf::AlarmLevel::ALARM_LEVEL_TYPE_INVALID)
    {
        if (isShow || !unRead || level == tf::AlarmLevel::ALARM_LEVEL_TYPE_INVALID)
        {
            ui->AlarmLabel->hide();
            // 设置报警按钮蓝色
            SetCtrlPropertyAndUpdateQss(ui->AlarmBtn, ALARM_PROPERTY, ALARM_PROPERTY_NONE);
        }
        else
        {
            ui->AlarmLabel->setMovie(m_pAttentionMovie);
            m_pAttentionMovie->start();
            ui->AlarmLabel->show();

            // 设置报警按钮透明
            SetCtrlPropertyAndUpdateQss(ui->AlarmBtn, ALARM_PROPERTY, ALARM_PROPERTY_TRANSPARENT);
        }

        // 设置蓝色字体
        SetCtrlPropertyAndUpdateQss(ui->alarm_font_label, ALARM_PROPERTY, ALARM_PROPERTY_NONE);

		sysWarnLevel = tf::AlarmLevel::ALARM_LEVEL_TYPE_INVALID;
    }

    // 注意级别存在试剂/耗材的报警和暂停级别，显示黄色
    else if ((level == tf::AlarmLevel::ALARM_LEVEL_TYPE_CAUTION && existReagent)
        || level == tf::AlarmLevel::ALARM_LEVEL_TYPE_STOPSAMPLING)
    {
        if (isShow || !unRead)
        {
            ui->AlarmLabel->hide();
            // 设置报警按钮黄色
            SetCtrlPropertyAndUpdateQss(ui->AlarmBtn, ALARM_PROPERTY, ALARM_PROPERTY_SAMPLE_STOP);
        }
        else
        {
            ui->AlarmLabel->setMovie(m_pSampleStopMovie);
            m_pSampleStopMovie->start();
            ui->AlarmLabel->show();
            // 设置报警按钮透明
            SetCtrlPropertyAndUpdateQss(ui->AlarmBtn, ALARM_PROPERTY, ALARM_PROPERTY_TRANSPARENT);
        }

        // 设置黄色字体
        SetCtrlPropertyAndUpdateQss(ui->alarm_font_label, ALARM_PROPERTY, ALARM_PROPERTY_SAMPLE_STOP);

		sysWarnLevel = tf::AlarmLevel::ALARM_LEVEL_TYPE_STOPSAMPLING;
    }
    // 停机，弹窗打开时显示红色，弹窗关闭红色闪烁
    else if (level == tf::AlarmLevel::ALARM_LEVEL_TYPE_STOP)
    {
        if (isShow || !unRead)
        {
            ui->AlarmLabel->hide();
            // 设置报警按钮红色
            SetCtrlPropertyAndUpdateQss(ui->AlarmBtn, ALARM_PROPERTY, ALARM_PROPERTY_STOP);
        }
        else
        {
            ui->AlarmLabel->setMovie(m_pStopMovie);
            m_pStopMovie->start();
            ui->AlarmLabel->show();
            // 设置报警按钮透明
            SetCtrlPropertyAndUpdateQss(ui->AlarmBtn, ALARM_PROPERTY, ALARM_PROPERTY_TRANSPARENT);
        }

        // 设置红色字体
        SetCtrlPropertyAndUpdateQss(ui->alarm_font_label, ALARM_PROPERTY, ALARM_PROPERTY_STOP);

		sysWarnLevel = tf::AlarmLevel::ALARM_LEVEL_TYPE_STOP;
    }

    // 产生试剂/耗材的报警设备状态栏显示黄色
    for (DeviceStatusWidget* devLab : ui->device_status_widget->findChildren<DeviceStatusWidget*>())
    {
        bool reagentAlarm = false;

        // 轨道没有试剂耗材报警
        auto devTypes = devLab->GetDeviceType();
        if (devTypes.contains(::tf::DeviceType::DEVICE_TYPE_TRACK))
        {
            continue;
        }

        // 检查设备是否存在试剂耗材报警
        for (QString sn : devLab->GetDeviceSns(devTypes[0]))
        {
            if (AlarmModel::Instance().IsExistReagentAlarmBySn(sn.toStdString()))
            {
                reagentAlarm = true;
                break;
            }           
        }

        devLab->UpdateReagentAlarm(reagentAlarm);
    }
    
	// 同步更新系统总览页面-报警按钮
	m_pSystemOverviewWgt->updateWarningBtn(!ui->AlarmLabel->isHidden(), sysWarnLevel);
}

void MainWidget::AlarmLevelChangedAfterDevReset(QString devSn)
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    AlarmModel::Instance().SetAlarmIsDealByDevSn(devSn.toStdString());
    OnAlarmLevelChanged(AlarmModel::Instance().GetCurrentAlarmLevel());
}

void MainWidget::OnUpadteDeviceMaskStatus(const QString& groupName, const QString& devSn, bool masked)
{  
    const auto& CIM = CommonInformationManager::GetInstance();
    auto spDev = CIM->GetDeviceInfo(devSn.toStdString());
    if (spDev == nullptr)
    {
        ULOG(LOG_ERROR, "find device[%s] failed", devSn.toStdString());
        return;
    }

    bool isPipe = DictionaryQueryManager::GetInstance()->GetPipeLine();
    for (auto& lab : ui->device_status_widget->findChildren<DeviceStatusWidget*>())
    {
        if (lab->GetDeviceSns(spDev->deviceType).contains(devSn))
        {
            // 联机B设备显示在第二个，单机ISE显示在第二个
            bool isFirst = isPipe ? spDev->name != "B" : spDev->deviceType != ::tf::DeviceType::DEVICE_TYPE_ISE1005;
            lab->UpdateMasked(masked, isFirst);
        }
    }

    OnUpdateAnalysisBtnStatus();
    UpdateSampleStopAndStopBtn();
}

void MainWidget::OnUpdateAnalysisBtnStatus()
{
    SetCtrlPropertyAndUpdateQss(ui->AnalysisBtn, ASSAY_TEST_ENABLE, m_pAnalysisDlg->UpdateOkBtnStatus());
}

void MainWidget::OnUpdateLisStatus(const bool status)
{
    ULOG(LOG_INFO, "%s(IsConnect:%d)", __FUNCTION__, status);

    SetCtrlPropertyAndUpdateQss(ui->lis_label, "bks", status ? "lis_connect" : "lis_connect_dis");

	// 同步更新系统总览界面的LIS状态
	m_pSystemOverviewWgt->updateLisConnection(status);
}

void MainWidget::OnShowDataBrowsePage()
{
	// 选中数据浏览
	auto layout = qobject_cast<QGridLayout*>(ui->CentralStackedWidget->widget(TAB_INDEX_WORK)->layout());
	if (layout)
	{
		auto workWgt = qobject_cast<QWorkDeskNavigation*>(layout->itemAt(0)->widget());
		if (workWgt)
		{
			workWgt->showPageDataBrowse();
		}
	}

	// 选中工作项
	OnSwitchTab(TAB_INDEX_WORK);

	CloseTopLevelWindow();
	m_pSystemOverviewWgt->reject();
}

void MainWidget::OnShowSampleRackMonitor()
{
	// 选中样本架监视
	auto layout = qobject_cast<QGridLayout*>(ui->CentralStackedWidget->widget(TAB_INDEX_WORK)->layout());
	if (layout)
	{
		auto workWgt = qobject_cast<QWorkDeskNavigation*>(layout->itemAt(0)->widget());
		if (workWgt)
		{
			workWgt->showSampleRackMonitor();
		}
	}

	// 选中工作项
	OnSwitchTab(TAB_INDEX_WORK);

	CloseTopLevelWindow();
	m_pSystemOverviewWgt->reject();
}

///
/// @brief 登录用户切换
///     
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月8日，新建函数
///
void MainWidget::UpdateLoginUser()
{
	// 设置登录用户
	std::shared_ptr<tf::UserInfo> pUserInfo = UserInfoManager::GetInstance()->GetLoginUserInfo();
	QString userName = (pUserInfo != nullptr && pUserInfo->__isset.nickname) ? QString::fromStdString(pUserInfo->nickname) : "Invalid";
    if (ui->UserLab->fontMetrics().width(userName) > ui->UserLab->maximumWidth())
    {
        userName.insert(userName.size() / 2, "\n");
    }
    ui->UserLab->setText(userName);
}

void MainWidget::OnShowShowReagentPage(const QString &devSn)
{
	ULOG(LOG_INFO, "%s(): devSn: %s", __FUNCTION__, devSn);

	// 选中试剂/耗材页面
	auto layout = qobject_cast<QGridLayout*>(ui->CentralStackedWidget->widget(TAB_INDEX_REGENT)->layout());
	if (layout)
	{
		auto regSpyWgt = qobject_cast<QSwitchReagentWidget*>(layout->itemAt(0)->widget());
		if (regSpyWgt)
		{
			regSpyWgt->showReagentPlatePage(devSn.toStdString());
		}
	}

	// 选中试剂项
	OnSwitchTab(TAB_INDEX_REGENT);

	CloseTopLevelWindow();
	m_pSystemOverviewWgt->reject();
}

void MainWidget::OnShowMaintainPage()
{
	if (m_pUtilityWidget == nullptr)
	{
		return;
	}

	// 选中应用项
	OnSwitchTab(TAB_INDEX_UTILITY);

	// 选中维护保养
	m_pUtilityWidget->OnMaintain();

	CloseTopLevelWindow();
	m_pSystemOverviewWgt->reject();
}

void MainWidget::OnShowErrText(QString text)
{
    // 关闭进度条
    POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE);

    // 显示提示信息
    TipDlg(text).exec();
}
