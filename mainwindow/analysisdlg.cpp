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
/// @file     analysisdlg.cpp
/// @brief    主界面->开始分析对话框
///
/// @author   4170/TangChuXian
/// @date     2020年8月14日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2020年8月14日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "analysisdlg.h"
#include "AssayShiledDlg.h"
#include "QSampleAssayModel.h"

#include "ui_analysisdlg.h"
#include "shared/tipdlg.h"
#include "shared/uidef.h"
#include "shared/uicommon.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/QFilterComboBox.h"
#include "shared/CommonInformationManager.h"

#include "thrift/DcsControlProxy.h"
#include "thrift/im/i6000/I6000LogicControlProxy.h"
#include "manager/DictionaryQueryManager.h"
#include "src/common/common.h"
#include "src/common/Mlog/mlog.h"
#include "src/public/ConfigDefine.h"
#include "src/public/ConfigSerialize.h"
#include "src/public/SerializeUtil.hpp"
#include "src/thrift/im/i6000/gen-cpp/i6000_types.h"

#include <QMouseEvent>

AnalysisDlg::AnalysisDlg(QWidget *parent)
    : BaseDlg(parent),
      m_bInit(false),
	  m_bIsPipeLine(false),
	  m_pAssayShiledDlg(new AssayShiledDlg(this))
{
    ui = new Ui::AnalysisDlg();
    ui->setupUi(this);
	InitAfterShow();
}

AnalysisDlg::~AnalysisDlg()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
    delete ui;
}

///
/// @brief 界面显示后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2020年8月17日，新建函数
///
void AnalysisDlg::InitAfterShow()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 设置对话框标题
    SetTitleName(tr("开始"));

    // 设置正则表达式
    ui->SeqNoEdit->setValidator(new QRegExpValidator(QRegExp(UI_REG_SAMPLE_NUM), this));

    // 连接信号槽
	connect(ui->shield_btn, SIGNAL(clicked()), this, SLOT(OnAssayShieldClicked()));
    connect(ui->OkBtn, SIGNAL(clicked()), this, SLOT(OnStartBtnClicked()));
    connect(ui->CancelBtn, SIGNAL(clicked()), this, SLOT(OnCloseBtnClicked()));
	connect(m_pAssayShiledDlg, SIGNAL(sigAssayShiledChanged()), this, SLOT(OnAssayShiledChanged()));

	// 当前是否为流水线模式
	m_bIsPipeLine = DictionaryQueryManager::GetInstance()->GetPipeLine();

    // 初始化分析方法下拉框
    ui->comboBox->setView(new QTipListView(ui->comboBox));
    connect(ui->comboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        [&](int index)
    {
        // 测试前维护名称长度超过12，显示tooltip
        auto& text = ui->comboBox->currentText();
        if (text.size() > 12)
        {
            ui->comboBox->setToolTip(text);
        }
        else
        {
            ui->comboBox->setToolTip("");
        }
    });
}

///
/// @brief 窗口显示事件
///     
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 4170/TangChuXian，2020年8月17日，新建函数
///
void AnalysisDlg::showEvent(QShowEvent *event)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 第一次显示时初始化
    if (!m_bInit)
    {
        m_bInit = true;
        //InitAfterShow();
    }

	// 加载主机通讯设置
    CommParamSet spCpc;
    if (DictionaryQueryManager::GetCommParamSet(spCpc))
    {
		// 断开信号槽，setChecked时会触发槽，导致LIS多次连接
		ui->communication_ckbox->blockSignals(true);
		ui->communication_ckbox->setChecked(spCpc.bCommLink);
		// 重新连接槽
		ui->communication_ckbox->blockSignals(false);
	}

	// 加载自动复查配置
	ui->auto_recheck_ckbox->setChecked(DictionaryQueryManager::QueryAutoRecheckConfig());

    // 设置起始样本号
    SetSeqDefaultNumber();
	bool isSeqModel = DictionaryQueryManager::GetInjectionModel() == ::tf::TestMode::SEQNO_MODE;

	if (isSeqModel)
	{
		ui->SeqNoLab->setText(tr("起始样本号"));
	}
	else
	{
		ui->SeqNoLab->setText("");
	}


	ui->SeqNoEdit->setVisible(isSeqModel);

	OnAssayShiledChanged();
    UpdateMaintainGroupCommbox();

	// 让基类处理事件
    move(1080, 105);
    QDialog::showEvent(event);
}

void AnalysisDlg::SetSeqDefaultNumber()
{
    // 所有设备待机，清空上一次测试的样本序号
    if (CommonInformationManager::GetInstance()->IsAllDeviceStandby())
    {
        m_lastSeqNo.clear();
    }

    if (DictionaryQueryManager::GetInjectionModel() == ::tf::TestMode::SEQNO_MODE)
    {
        std::string seq = DcsControlProxy::GetInstance()->GetSeqStartSampleNumber(m_lastSeqNo.toStdString());
        ui->SeqNoEdit->setText(QString::fromStdString(seq));
    }
    else
    {
        ui->SeqNoEdit->setText("");
    }
}

///
/// @brief  检测i6000设备是否有试剂仓盖打开的情况
///
/// @param[in]    void
///
/// @return bool
///
/// @par History:
/// @li 7915/LeiDingXiang，2023年1月3日，新建函数
///
bool AnalysisDlg::QueryImReagentCoverIsClose()
{
	// 查询当前所有设备状态
	tf::DeviceInfoQueryResp devInfoResp;
	tf::DeviceInfoQueryCond devInfoCond;

	// 执行查询
	if (!DcsControlProxy::GetInstance()->QueryDeviceInfo(devInfoResp, devInfoCond))
	{
		ULOG(LOG_WARN, "QueryDeviceInfo failed!");
		return false;
	}
	// 结果为空则返回
	if (devInfoResp.lstDeviceInfos.empty())
	{
		ULOG(LOG_INFO, "devInfoResp.lstDeviceInfos.empty()!");
		return false;
	}

	for (const auto& m_DeviceInfoVec : devInfoResp.lstDeviceInfos)
	{
		// 获取选中设备信息
		const tf::DeviceInfo& stuTfDevInfo = m_DeviceInfoVec;

		//若为i 6000 判断试剂仓盖状态
		if (stuTfDevInfo.deviceType == ::tf::DeviceType::DEVICE_TYPE_I6000)
		{
			//判断在待机状态或加样停状态试剂仓盖是否打开
			if (im::i6000::LogicControlProxy::QueryMaterialStatus(stuTfDevInfo.deviceSN, im::i6000::tf::MonitorMessageType::MONITOR_TYPE_REAGENTCOVER))
			{
				ULOG(LOG_INFO, "The device can't start test,cause its reagent cover is open!");
				TipDlg(tr("试剂仓盖开启不能做样本测试！")).exec();
				return false;
			}
		}
	}
	return true;
}

///
/// @brief  检测i 6000设备是否有废液桶溢出情况
///
/// @param[in]    
///
/// @return 
///
/// @par History:
/// @li 7915/LeiDingXiang，2023年1月12日，新建函数
///
bool AnalysisDlg::QueryI6000WasteLiquidFull()
{
	// 查询当前所有设备状态
	tf::DeviceInfoQueryResp devInfoResp;
	tf::DeviceInfoQueryCond devInfoCond;

	// 执行查询
	if (!DcsControlProxy::GetInstance()->QueryDeviceInfo(devInfoResp, devInfoCond))
	{
		ULOG(LOG_WARN, "QueryDeviceInfo failed!");
		return false;
	}
	// 结果为空则返回
	if (devInfoResp.lstDeviceInfos.empty())
	{
		ULOG(LOG_INFO, "devInfoResp.lstDeviceInfos.empty()!");
		return false;
	}

	for (const auto& m_DeviceInfoVec : devInfoResp.lstDeviceInfos)
	{
		// 获取选中设备信息
		const tf::DeviceInfo& stuTfDevInfo = m_DeviceInfoVec;

		//若为i 6000 判断废液桶状态
		if (stuTfDevInfo.deviceType == ::tf::DeviceType::DEVICE_TYPE_I6000)
		{
			//判断废液桶是否满
			if (im::i6000::LogicControlProxy::QueryMaterialStatus(stuTfDevInfo.deviceSN, im::i6000::tf::MonitorMessageType::MONITOR_TYPE_WASTEQUILD))
			{
				ULOG(LOG_INFO, "The device can't start test,cause its waste liquid is full!");
				TipDlg(tr("废液桶满不能开启样本测试！")).exec();
				return false;
			}
		}
	}
	return true;
}

void AnalysisDlg::UpdateMaintainGroupCommbox()
{
    ui->comboBox->clear();

    ::tf::MaintainGroupQueryResp mgqr; 
    ::tf::MaintainGroupQueryCond mgqc;

    if (!DcsControlProxy::GetInstance()->QueryMaintainGroup(mgqr, mgqc))
    {
        ULOG(LOG_ERROR, "QueryMaintainGroups Failed !");
        return;
    }

    for (auto& group : mgqr.lstMaintainGroups)
    {
        // 开始前维护下拉框包含特定的单项维护
        if (group.groupType == ::tf::MaintainGroupType::MAINTAIN_GROUP_SINGLE
            && (group.items[0].itemType == tf::MaintainItemType::MAINTAIN_ITEM_RESET
                || group.items[0].itemType == tf::MaintainItemType::MAINTAIN_ITEM_AUTO_SHUTDOWN
                || group.items[0].itemType == tf::MaintainItemType::MAINTAIN_ITEM_CLEAN_TRACK

                // 生化维护项
                || group.items[0].itemType == tf::MaintainItemType::MAINTAIN_ITEM_CH_ISE_AIR_PURGE
                || group.items[0].itemType == tf::MaintainItemType::MAINTAIN_ITEM_CH_PHOTOMETER_CHECK
                || group.items[0].itemType == tf::MaintainItemType::MAINTAIN_ITEM_CH_FLUID_PERFUSION
                || group.items[0].itemType == tf::MaintainItemType::MAINTAIN_ITEM_CH_REAGENT_SCAN
                || group.items[0].itemType == tf::MaintainItemType::MAINTAIN_ITEM_ISE_FLUID_PERFUSION

                // 免疫维护项
                || group.items[0].itemType == tf::MaintainItemType::MAINTAIN_ITEM_REA_VORTEX
                || group.items[0].itemType == tf::MaintainItemType::MAINTAIN_ITEM_FILL_PIP
                || group.items[0].itemType == tf::MaintainItemType::MAINTAIN_ITEM_RESET_AFTER_TEST
                || group.items[0].itemType == tf::MaintainItemType::MAINTAIN_ITEM_SAMPLE_HANDLE_SYS_RESET
                || group.items[0].itemType == tf::MaintainItemType::MAINTAIN_ITEM_REAG_ONLINE_LOAD_RESET
                || group.items[0].itemType == tf::MaintainItemType::MAINTAIN_ITEM_LOAD_FILL_SUBS
                || group.items[0].itemType == tf::MaintainItemType::MAINTAIN_ITEM_MAG_FILL_PIP
                || group.items[0].itemType == tf::MaintainItemType::MAINTAIN_ITEM_CLEAN_CUP_TRACK
                || group.items[0].itemType == tf::MaintainItemType::MAINTAIN_ITEM_CHANGE_USESTATUSE_FILL_SUBS
                || group.items[0].itemType == tf::MaintainItemType::MAINTAIN_ITEM_CLEAR_FILL_PIPE_CUPS
                || group.items[0].itemType == tf::MaintainItemType::MAINTAIN_ITEM_REAGENT_SCAN))
        {
            ui->comboBox->addItem(ConvertTfEnumToQString(group.items[0].itemType), QString::number(group.id));
        }
        // 开始前维护下拉框包含所有的用户自定义维护和开机维护
        else if (group.groupType == ::tf::MaintainGroupType::MAINTAIN_GROUP_CUSTOM)
        {
            ui->comboBox->addItem(QString::fromStdString(group.groupName), QString::number(group.id));
        }
        else if (group.groupType == ::tf::MaintainGroupType::MAINTAIN_GROUP_START)
        {
            ui->comboBox->addItem(ConvertTfEnumToQString(group.groupType), QString::number(group.id));
        }
    }

    bool bCheck = false;
    std::string strMain;
    if (!DictionaryQueryManager::GetStarttestMaintainConfig(bCheck, strMain))
    {
        return;
    }
    ui->comboBox->setCurrentIndex(ui->comboBox->findData(QString::fromStdString(strMain)));
    ui->maintain_checkbox->setChecked(bCheck);
}

bool AnalysisDlg::UpdateOkBtnStatus()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 添加显示的错误文本
    auto AppendErrText = [](QString&errText, const QString& tempStr, int& count, int& errDevCount, int lastErrDevCount)
    {
        if (lastErrDevCount == errDevCount)
        {
            errDevCount++;
        }

        // 错误信息最多5行
        if (count < 5)
        {
            errText += errText.isEmpty() ? tempStr : "\n" + tempStr;
            count++;
        }
    };

    // 清空错误提示文本
    m_errText.clear();

    // 联机先判断轨道
    int errCount = 0;
    auto CIM = CommonInformationManager::GetInstance();
    if (DictionaryQueryManager::GetInstance()->GetPipeLine())
    {
        for (const auto& dev : CIM->GetDeviceFromType({ ::tf::DeviceType::DEVICE_TYPE_TRACK }))
        {
            // 轨道待机、运行、进架停才能开始测试
            QString devName = QString::fromStdString("[" + dev->groupName + dev->name + "]");
            if (tf::DeviceWorkState::DEVICE_STATUS_STANDBY == dev->status
                || tf::DeviceWorkState::DEVICE_STATUS_STOP_PUSH_RACK == dev->status
                || tf::DeviceWorkState::DEVICE_STATUS_RUNNING == dev->status)
            {
                // 检查是否有影响开始测试的原因
                for (const auto& err : dev->faats)
                {
                     QString tempStr = devName + ConvertTfEnumToQString(err);
                     m_errText += m_errText.isEmpty() ? tempStr : "\n" + tempStr;
                     errCount++;

                     // 错误原因最多显示五行
                     if (errCount >= 5)
                     {
                         break;
                     }
                }
            } 
            else
            {
                // 设备的当前状态不能开始测试
                QString tempStr = devName + tr("的状态是：") + ConvertTfEnumToQString(
                                    (::tf::DeviceWorkState::type)dev->status) + tr("，不能开始测试");
                m_errText = tempStr;
                errCount++;
            }
        }
    }

    // 轨道能否开始的标志
    bool trackIsAppend = m_errText.isEmpty();

    // 错误原因最多显示五行
    if (errCount >= 5)
    {
        SetCtrlPropertyAndUpdateQss(ui->OkBtn, ASSAY_TEST_ENABLE, false);
        return false;
    }

    int	errDevCount = 0;
    int lastErrDevCount = 0;
    const auto devMaps = CIM->GetDeviceMaps();
	for (const auto& iter : devMaps)
	{
        const auto& dev = iter.second;
        lastErrDevCount = errDevCount;

        // 不检查轨道
        if (tf::DeviceType::DEVICE_TYPE_TRACK == dev->deviceType)
        {
            continue;
        }

        // 仪器屏蔽不提示，bug:13678
        if (dev->masked)
        {
            errDevCount++;
        }

        QString devName = QString::fromStdString("[" + dev->groupName + dev->name + "]");

        // 仪器处于待机、进架停、加样停、运行、才能开始测试。
        // 免疫设备开始逻辑特殊，由faats带出，不在此处处理，缺陷30728
        if (!(tf::DeviceWorkState::DEVICE_STATUS_STANDBY == dev->status
            || tf::DeviceWorkState::DEVICE_STATUS_STOP_PUSH_RACK == dev->status
            || tf::DeviceWorkState::DEVICE_STATUS_SAMPSTOP == dev->status
            || tf::DeviceWorkState::DEVICE_STATUS_RUNNING == dev->status)
           && ::tf::DeviceType::DEVICE_TYPE_I6000 != dev->deviceType)
        {
            QString tempStr = devName + tr("的状态是：") + ConvertTfEnumToQString(
                (::tf::DeviceWorkState::type)dev->status) + tr("，不能开始测试");
            AppendErrText(m_errText, tempStr, errCount, errDevCount, lastErrDevCount);
        }

        // 显示影响设备不能开始测试的原因
        for (const auto& err : dev->faats)
        {
            QString tempStr = devName + ConvertTfEnumToQString(err);
            AppendErrText(m_errText, tempStr, errCount, errDevCount, lastErrDevCount);
        }

        // 电解质设备需要其绑定的比色设备可以测试才能测试
//         if (dev->deviceType == ::tf::DeviceType::DEVICE_TYPE_ISE1005)
//         {
//             const auto& spDev = CIM->GetBindDeviceInfoBySn(dev->deviceSN);
//             if (spDev == nullptr)
//             {
//                 continue;
//             }
// 
//             if (!spDev->faats.empty()
//                 || tf::DeviceWorkState::DEVICE_STATUS_DISCONNECT == spDev->status
//                 || tf::DeviceWorkState::DEVICE_STATUS_HALT == spDev->status
//                 || tf::DeviceWorkState::DEVICE_STATUS_MAINTAIN == spDev->status)
//             {
//                 QString tempStr = devName + tr("不能开始测试，因为%1不能开始测试")
//                    .arg(QString::fromStdString(spDev->groupName + spDev->name));
// 
//                 AppendErrText(m_errText, tempStr, errCount, errDevCount, lastErrDevCount);
//             }
//         }
	}

    // 轨道不能开始测试、没有设备能开始测试就不能点击开始
    if (!trackIsAppend || (errDevCount == devMaps.size() -1))
    {
        if (!trackIsAppend)
        {
            QString tempStr(tr("轨道当前的状态不能开始测试"));
            m_errText += m_errText.isEmpty() ? tempStr : "\n" + tempStr;
        }

        if (errDevCount == devMaps.size() - 1)
        {
            QString tempStr(tr("当前无可用测试设备"));
            m_errText += m_errText.isEmpty() ? tempStr : "\n" + tempStr;
        }

        SetCtrlPropertyAndUpdateQss(ui->OkBtn, ASSAY_TEST_ENABLE, false);
        return false;
    }

    SetCtrlPropertyAndUpdateQss(ui->OkBtn, ASSAY_TEST_ENABLE, true);
    return true;
}

void AnalysisDlg::ContinueRun()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	OnStartBtnClicked();
}

void AnalysisDlg::OnAssayShieldClicked()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	m_pAssayShiledDlg->show();
} 

void AnalysisDlg::OnStartBtnClicked()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 更新当前能否开始测试
    bool canTest = UpdateOkBtnStatus();
     
    if (!m_errText.isEmpty())
    {
        auto type =  TipDlgType::SINGLE_BUTTON;
        if (canTest)
        {
            m_errText += tr("\n是否开始测试");
            type = TipDlgType::TWO_BUTTON;
        }
        
        if (TipDlg(m_errText, type).exec() == QDialog::Rejected)
        {
            return;
        }
    }

    //判断开始流程前的限制条件
    if (ui->SeqNoEdit->isVisible() && ui->SeqNoEdit->text().isEmpty())
    {
        TipDlg(tr("请输入开始测试的样本序号")).exec();
        return;
    }

    if (!SaveCommParamConfig()
        || !SaveAutoRecheckConfig()
        || !SaveMaintainConfig())
    {
        ULOG(LOG_ERROR, "save analysis config info failed !");
        return;
    }

    ::tf::StartTestParams params;
    if (ui->SeqNoEdit->isVisible())
    {
        params.__set_strSeqNo(ui->SeqNoEdit->text().toLocal8Bit().data());
    }
	// 开始前维护组ID号
    if (ui->maintain_checkbox->isChecked() && ui->comboBox->currentIndex() >= 0)
    {
        params.__set_iMaintainGroupId(ui->comboBox->currentData().toInt());
    }

    params.__set_bAutoRetest(ui->auto_recheck_ckbox->isChecked());
    params.__set_bCommunicate(ui->communication_ckbox->isChecked());
    
//     if (!CheckReagentRemainCount())
//     {
//         return;
//     }

	// 开始分析
	DcsControlProxy::GetInstance()->StartTest(params);
    m_lastSeqNo = ui->SeqNoEdit->text();
	
    close();
}

void AnalysisDlg::OnCloseBtnClicked()
{
	// 关闭后重新打开该界面导致ui->CancelBtn处于hover状态，bug 0013003
	QMouseEvent *leaveEvent = new QMouseEvent(QEvent::Leave, QPointF(0, 0), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
	QApplication::postEvent(ui->CancelBtn, leaveEvent);

	close();
}

bool AnalysisDlg::SaveCommParamConfig()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

    CommParamSet communicateCfg;
    if (!DictionaryQueryManager::GetCommParamSet(communicateCfg))
    {
        TipDlg(tr("保存主机通讯设置失败！")).exec();
        return false;
    }

    if (communicateCfg.bCommLink == ui->communication_ckbox->isChecked())
    {
        ULOG(LOG_INFO, "QueryCommParamConfig not modify");
        return true;
    }

    for (auto& dev : CommonInformationManager::GetInstance()->GetDeviceMaps())
    {
        // 来自需求文档，仪器待机才能修改通讯设置的开关
        if (devIsRun(*(dev.second)))
        {
            TipDlg(tr("存在正在运行的仪器，不能修改通讯设置")).exec();
            return false;
        }
    }
    communicateCfg.bCommLink = ui->communication_ckbox->isChecked();

    if (!DictionaryQueryManager::SaveCommParamSet(communicateCfg))
    {
        ULOG(LOG_ERROR, "Encode2Xml CommParamConfig Failed");
        TipDlg(tr("保存主机通讯设置失败！")).exec();
        return false;
    }

    return true;
}

bool AnalysisDlg::CheckReagentRemainCount()
{
    DetectionSetting detecConfg;
    if (!DictionaryQueryManager::GetDetectionConfig(detecConfg))
    {
        ULOG(LOG_INFO, "Get detection config failed!");
        return false;
    }

    std::map<int, int> assayAndTimes;
    if (!DataPrivate::Instance().GetAssayAndTestTimes(
        (tf::TestMode::type)detecConfg.testMode, assayAndTimes))
    {
        ULOG(LOG_INFO, "Get assay test times failed!");
        return false;
    }

    if (!CommonInformationManager::GetInstance()->CheckReagentRemainCount(assayAndTimes)
        && TipDlg(tr("当前试剂不足以完成此批次的项目测试，是否开始测试"),
            TipDlgType::TWO_BUTTON).exec() == QDialog::Rejected)
    {
        return false;
    }

    return true;
}

bool AnalysisDlg::SaveAutoRecheckConfig()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (!DictionaryQueryManager::SaveAutoRecheckConfig(ui->auto_recheck_ckbox->isChecked()))
    {
        ULOG(LOG_ERROR, "ModifyDictionaryInfo AutoRecheckConfig Failed");
        TipDlg(tr("保存自动复查设置失败！")).exec();
        return false;
    }

    return true;
}

bool AnalysisDlg::SaveMaintainConfig()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    bool bCheck = ui->maintain_checkbox->isChecked();
    std::string strData = ui->comboBox->currentData().toString().toStdString();
    if (!DictionaryQueryManager::SaveStarttestMaintainConfig(bCheck, strData))
    {
        ULOG(LOG_ERROR, "ModifyDictionaryInfo maintain config failed");
        TipDlg(tr("保存维护配置失败！")).exec();
        return false;
    }

    return true;
}

void AnalysisDlg::OnAssayShiledChanged()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 查询项目屏蔽信息
	tf::DeviceInfoQueryCond qc;
	tf::DeviceInfoQueryResp qr;
	if (!DcsControlProxy::GetInstance()->QueryDeviceInfo(qr, qc)
		|| qr.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ULOG(LOG_ERROR, "Query AssayShiled Failed!");
		return;
	}

    // 按项目编号获取项目名称
    QString textTemp;
    for (const auto& dev : qr.lstDeviceInfos)
    {
        std::map<int, std::set<int>> assayCodes;
        if (!dev.maskedAssayCode.empty() && !DecodeJson(assayCodes, dev.maskedAssayCode))
        {
            ULOG(LOG_ERROR, "DecodeJson maskedAssayCode Failed!");
            continue;
        }

        auto& CIM = CommonInformationManager::GetInstance();
        for (auto& iter : assayCodes)
        {
            QString devName = QString::fromStdString(dev.groupName + dev.name);

            // ISE多模块显示-A,-B
            if (dev.deviceType == ::tf::DeviceType::DEVICE_TYPE_ISE1005
                && assayCodes.size() > 1)
            {
                devName += iter.first == 1 ? "-A" : "-B";
            }
            devName += ":";

            // 获取屏蔽的设备名字
            QString codeName;
            for (int code : iter.second)
            {
                QString name = QString::fromStdString(CIM->GetAssayNameByCode(code));
                codeName += codeName.isEmpty() ? name : tr("、") + name;
            }

            // 没有屏蔽的项目就跳过
            if (codeName.isEmpty())
            {
                continue;
            }

            textTemp += devName + codeName + "\n";
        }
    }

	// 显示到界面
	ui->textEdit->setPlainText(textTemp);
}

///
/// @brief  主机通讯按钮限制
///
/// @param[in]    void
///
/// @return void
///
/// @par History:
/// @li 7915/LeiDingXiang，2023年11月7日，新建函数
///
void AnalysisDlg::OnCommunicationChanged()
{
    bool bCheck = true;

    // 查询当前所有设备状态
    tf::DeviceInfoQueryResp devInfoResp;
    tf::DeviceInfoQueryCond devInfoCond;

    // 执行查询
    if (!DcsControlProxy::GetInstance()->QueryDeviceInfo(devInfoResp, devInfoCond))
    {
        ULOG(LOG_WARN, "QueryDeviceInfo failed!");
    }
    // 结果为空则返回
    if (devInfoResp.lstDeviceInfos.empty())
    {
        ULOG(LOG_WARN, "devInfoResp.lstDeviceInfos.empty()!");
    }

    for (auto DeviceInfoVec : devInfoResp.lstDeviceInfos)
    {
        if (::tf::DeviceType::DEVICE_TYPE_TRACK == DeviceInfoVec.deviceType
            || ::tf::DeviceType::DEVICE_TYPE_INVALID == DeviceInfoVec.deviceType)
        {
            continue;
        }

        //设备为待机或维护状态才可以修改
        if (::tf::DeviceWorkState::DEVICE_STATUS_MAINTAIN != DeviceInfoVec.status 
            && ::tf::DeviceWorkState::DEVICE_STATUS_STANDBY != DeviceInfoVec.status
            && ::tf::DeviceWorkState::DEVICE_STATUS_DISCONNECT != DeviceInfoVec.status)
        {
            bCheck = false;
        }
    }

    if (!bCheck)
    {
        //有一台免疫设备非待机和加样停状态给出提示
        ULOG(LOG_WARN, "The device can't start test,cause its status is not standyby or sample stoping!");
        TipDlg(tr("仪器在待机或维护时才可以修改通讯设置！")).exec();
        ui->communication_ckbox->setChecked(!ui->communication_ckbox->checkState());
    }
}
