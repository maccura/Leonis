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
/// @file     utilityWidget.cpp
/// @brief    应用界面
///
/// @author   7951/LuoXin
/// @date     2022年12月02日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2022年12月02日，新建函数
///
///////////////////////////////////////////////////////////////////////////
#include "utilityWidget.h"
#include "ui_utilityWidget.h"
#include <memory>
#include <QDir>
#include <QUrl>
#include <QDesktopServices>
#include <QGridLayout>
#include <QPushButton>
#include <QSysInfo>
#include "assayconfigwidget.h"
#include "QAssayIndexSetPage.h"
#include "BackupOrderWidget.h"
#include "SpecialWashDlg.h"
#include "QLogModule.h"
#include "QStatusModule.h"
#include "AiAnalysisWgt.h"
#include "model/QAlarmReagentSetModel.h"
#include "model/QAlarmSupplySetModel.h"

#include "usermngdlg.h"
#include "nodedbgdlg.h"
#include "processcheckdlg.h"
#include "sensorstatedlg.h"
#include "dbgprocessdlg.h"
#include "ConsumablesManageDlg.h"
#include "ClearSetDlg.h"
#include "DetectionSetDlg.h"
#include "SoftwareVersionDlg.h"
#include "SampleReceiveModeDlg.h"
#include "functionManageDlg.h"
#include "AssayAllocationDlg.h"

#include "QDisplayShow.h"
#include "DevManageDlg.h"
#include "CommParamSetDlg.h"
#include "SampleRackAllocationDlg.h"
#include "StartupSetDlg.h"
#include "PromptSetWidget.h"
#include "QDisplayShow.h"
#include "QAssayShowPostion.h"
#include "CalcAssayDlg.h"
#include "QStatisticAnalysisWgt.h"
#include "ExportSetDlg.h" 
#include "UserManualWidget.h"
#include "imparamwidget.h"
#include "uidcsadapter/uidcsadapter.h"

#include "maintainwidget.h"
#include "maintaindatamng.h"
#include "src/common/Mlog/mlog.h"  
#include "shared/CommonInformationManager.h"
#include "shared/tipdlg.h"
#include "shared/msgiddef.h"
#include "shared/messagebus.h"
#include "manager/UserInfoManager.h"
#include "manager/DictionaryQueryManager.h"
#include "printcom.hpp"
#include "PdfPretreatment.h"
#include "thrift/DcsControlProxy.h"

// 页面索引
enum TAB_INDEX
{
    TAB_INDEX_MAIN,								// 主界面
    TAB_INDEX_ASSAY_CONFIG,						// 项目配置界面
    TAB_INDEX_BACKUP_ORDER_INDEX,				// 备用订单
    TAB_INDEX_ASSAY_INDEX,						// 项目位置
    TAB_INDEX_SPECIAL_WASH,						// 交叉污染
    TAB_INDEX_LOG,								// 日志
    TAB_INDEX_STATUS,                           // 状态
    TAB_INDEX_DISPLAY,                          // 显示设置
    TAB_INDEX_MAINTAIN,							// 维护保养
    TAB_INDEX_PROMPT,                           // 提示设置
    TAB_INDEX_CALC,                             // 计算项目
    TAB_INDEX_STATISTIC, 					    // 统计分析
    TAB_INDEX_AI_ANALYSIS, 					    // AI识别设置
    TAB_INDEX_USER_MANUAL,                      // 用户手册
    TAB_INDEX_PARAM_IMMUNE                      // 参数（免疫）
};


UtilityWidget::UtilityWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::UtilityWidget)
    , m_pDisplayShowDlg(Q_NULLPTR)
    , m_pSampleRackAllocationDlg(nullptr)
    , m_pCommParamSetDlg(nullptr)
    , m_pDevManageDlg(nullptr)
    , m_pSoftwareVersionDlg(nullptr)
    , m_pDetectionSetDlg(nullptr)
    , m_pClearSetDlg(nullptr)
    , m_pConsumablesManageDlg(nullptr)
    , m_pSampleReceiveModeDlg(nullptr)
    , m_assayOrder(Q_NULLPTR)
    , m_pfunctionManageDlg(nullptr)
    , m_pAssayAllocationDlg(nullptr)
    , m_pNodeDbgDlg(nullptr)
    , m_pProcessCheckDlg(nullptr)
{
    ui->setupUi(this);
    Init();
}

UtilityWidget::~UtilityWidget()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
}

void UtilityWidget::SetCurrentWidgetIsMain()
{
    ui->stackedWidget->setCurrentIndex(TAB_INDEX_MAIN);
}

bool UtilityWidget::CurrentWidgetHasUnSaveData()
{
    // 显示设置
    if (ui->stackedWidget->currentIndex() == TAB_INDEX_DISPLAY)
    {
        auto* wid = qobject_cast<QDisplayShow*>(ui->stackedWidget->currentWidget());
        return (wid != nullptr && wid->isExistChangedData()
                && TipDlg(tr("当前页面存在未保存的信息，切换页面后将丢失这些信息，是否切换页面"),
                    TipDlgType::TWO_BUTTON).exec() == QDialog::Rejected);
    }
    // 提示设置
    else if (ui->stackedWidget->currentIndex() == TAB_INDEX_PROMPT)
    {
        auto* wid = qobject_cast<PromptSetWidget*>(ui->stackedWidget->currentWidget());
        return (wid != nullptr && wid->isExistChangedData()
                && TipDlg(tr("当前页面存在未保存的信息，切换页面后将丢失这些信息，是否切换页面"),
                    TipDlgType::TWO_BUTTON).exec() == QDialog::Rejected);
    }
    // 项目设置
    else if (ui->stackedWidget->currentIndex() == TAB_INDEX_ASSAY_CONFIG)
    {
        bool isExistUnSaveSample = false;
        auto* wid = qobject_cast<AssayConfigWidget*>(ui->stackedWidget->currentWidget());    

        if (wid != nullptr && wid->HasUnSaveChangedData(isExistUnSaveSample))
        {
            QString tipText(isExistUnSaveSample ? tr("当前项目存在未保存的样本类型") : tr("当前项目存在未保存的数据"));
            return TipDlg(tipText + tr("，切换页面后将丢失这些信息，是否切换页面"),
                TipDlgType::TWO_BUTTON).exec() == QDialog::Rejected;
        }
    }

    else if (ui->stackedWidget->currentIndex() == TAB_INDEX_LOG)
    {
        auto* wid = qobject_cast<QLogModule*>(ui->stackedWidget->currentWidget());
        if (wid != nullptr)
        {
            wid->StopQueryAlarmLog();
        }
    }

    return false;
}

void UtilityWidget::Init()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    ui->stackedWidget->addWidget(new AssayConfigWidget(this));
    ui->stackedWidget->addWidget(new BackupOrderWidget(this));
    ui->stackedWidget->addWidget(new QAssayIndexSetPage(this));
    ui->stackedWidget->addWidget(new SpecialWashDlg(this));
    ui->stackedWidget->addWidget(new QLogModule(this));
    ui->stackedWidget->addWidget(new QStatusModule(this));
    ui->stackedWidget->addWidget(new QDisplayShow(this));
    ui->stackedWidget->addWidget(new MaintainWidget(this));
    ui->stackedWidget->addWidget(new PromptSetWidget(this));
    ui->stackedWidget->addWidget(new CalcAssayDlg(this));
    ui->stackedWidget->addWidget(new QStatisticAnalysisWgt(this));
    ui->stackedWidget->addWidget(new AiAnalysisWgt(this));
    ui->stackedWidget->addWidget(new UserManualWidget(this));
    ui->stackedWidget->addWidget(new ImParamWidget(this));

    // 调试
    std::vector<ButtonInfo> debugBtn;
    debugBtn.push_back(ButtonInfo(tr("节点调试"), "btn_node_debug", ":/Leonis/resource/image/icon-debug-part.png", &UtilityWidget::OnNodeDebugBtnClicked));
    debugBtn.push_back(ButtonInfo(tr("传感器状态"), "btn_sensor_state", ":/Leonis/resource/image/icon-sensor-state.png", &UtilityWidget::OnSensorStateBtnCliked));
    debugBtn.push_back(ButtonInfo(tr("调试流程(免疫)"), "btn_diagnose", ":/Leonis/resource/image/im/icon-debug-process.png", &UtilityWidget::OnDiagnoseBtnCliked));
    debugBtn.push_back(ButtonInfo(tr("流程检查"), "btn_process_check", ":/Leonis/resource/image/im/icon-process-check.png", &UtilityWidget::OnProcessCheckBtnClicked));
    debugBtn.push_back(ButtonInfo(tr("参数(免疫)"), "btn_param", ":/Leonis/resource/image/im/icon-param-im.png", &UtilityWidget::OnParamBtnClicked));
    InitButtonArea(ui->DebugBtnWgt, tr("工程师"), 1, 5, debugBtn);

    // 项目块
    std::vector<ButtonInfo> assayBtn;
    assayBtn.push_back(ButtonInfo(tr("项目设置"), "btn_assay_set", ":/Leonis/resource/image/icon-software-version.png", &UtilityWidget::OnAssaySettings));
    assayBtn.push_back(ButtonInfo(tr("计算项目"), "btn_calc_assay", ":/Leonis/resource/image/icon-special-wash.png", &UtilityWidget::OnCalculateAssay));
    assayBtn.push_back(ButtonInfo(tr("加样顺序"), "btn_sampleadd_sequence", ":/Leonis/resource/image/icon-sample-rack-allocation.png", &UtilityWidget::OnAssayOrder));
    assayBtn.push_back(ButtonInfo(tr("项目位置"), "btn_assay_postion", ":/Leonis/resource/image/icon-assay-order.png", &UtilityWidget::OnAssayPositionSettings));
    assayBtn.push_back(ButtonInfo(tr("特殊清洗设置"), "btn_special_wash", ":/Leonis/resource/image/icon-assay-postion.png", &UtilityWidget::OnSpecialWash));
    assayBtn.push_back(ButtonInfo(tr("项目分配"), "btn_assay_allocation", ":/Leonis/resource/image/icon-assay-allocation.png", &UtilityWidget::OnAssayAllocationClicked));
    InitButtonArea(ui->widgetAssay, tr("项目"), 3, 2, assayBtn);

    // 系统
    std::vector<ButtonInfo> systemBtn;
    systemBtn.push_back(ButtonInfo(tr("显示设置"), "btn_display_set", ":/Leonis/resource/image/icon-display-set.png", &UtilityWidget::OnDisplay));
    systemBtn.push_back(ButtonInfo(tr("提示设置"), "btn_prompt_set", ":/Leonis/resource/image/icon-prompt-set.png", &UtilityWidget::OnPrompt));
    systemBtn.push_back(ButtonInfo(tr("日志"), "log_btn", ":/Leonis/resource/image/icon-log.png", &UtilityWidget::OnLog));
    systemBtn.push_back(ButtonInfo(tr("清空设置"), "btn_clear_set", ":/Leonis/resource/image/icon-clear-set.png", &UtilityWidget::OnClearSetClicked));
    systemBtn.push_back(ButtonInfo(tr("打印设置"), "btn_print_set", ":/Leonis/resource/image/icon-print-set.png", &UtilityWidget::OnPrintSetClicked));
	systemBtn.push_back(ButtonInfo(tr("导出设置"), "btn_export_set", ":/Leonis/resource/image/icon-export-set.png", &UtilityWidget::OnExportSetClicked));
    systemBtn.push_back(ButtonInfo(tr("用户管理"), "btn_user_mng", ":/Leonis/resource/image/icon-user-mng.png", &UtilityWidget::OnUserMngBtnClicked));
    systemBtn.push_back(ButtonInfo(tr("通讯设置"), "btn_commparam_set", ":/Leonis/resource/image/icon-commparam-set.png", &UtilityWidget::OnCommParamSetClicked));
    systemBtn.push_back(ButtonInfo(tr("统计分析"), "statistic_analysis_btn", ":/Leonis/resource/image/icon-statistic-analysis.png", &UtilityWidget::OnStatistic));
    //systemBtn.push_back(ButtonInfo(tr("视觉识别设置"), "btn_ai_analysis", ":/Leonis/resource/image/icon-ai-analysis.png", &UtilityWidget::OnAiAnalysisCfgClicked)); ——mod_tcx 硬件暂不支持，暂时隐藏该功能
    systemBtn.push_back(ButtonInfo(tr("版本信息"), "software_version_btn", ":/Leonis/resource/image/icon-software-versionc.png", &UtilityWidget::OnSoftwareVersionClicked));
    systemBtn.push_back(ButtonInfo(tr("用户手册"), "btn_help", ":/Leonis/resource/image/icon-help.png", &UtilityWidget::OnHelpFileClicked));
    InitButtonArea(ui->widgetSystem, tr("系统"), 3, 4, systemBtn);

    // 检测
    std::vector<ButtonInfo> checkBtn;
    checkBtn.push_back(ButtonInfo(tr("检测设置"), "btn_injection_model", ":/Leonis/resource/image/icon-injection-model.png", &UtilityWidget::OnInjectionModelClicked));
    checkBtn.push_back(ButtonInfo(tr("备用订单"), "btn_default_test", ":/Leonis/resource/image/icon-sample-recive-model.png", &UtilityWidget::OnStandbyOrder));
    checkBtn.push_back(ButtonInfo(tr("样本接收模式"), "btn_sample_recive_model", ":/Leonis/resource/image/icon-quick-model.png", &UtilityWidget::OnSampleReceiveModeClicked));
    checkBtn.push_back(ButtonInfo(tr("样本架分配"), "btn_sample_rack_allocation", ":/Leonis/resource/image/icon-sample-rack.png", &UtilityWidget::OnSampleRackClicked));
    InitButtonArea(ui->widgetCheck, tr("检测"), 2, 2, checkBtn);

    // 仪器
    std::vector<ButtonInfo> instrumentBtn;
    instrumentBtn.push_back(ButtonInfo(tr("设备管理"), "btn_dev_manage", ":/Leonis/resource/image/icon-dev-manage.png", &UtilityWidget::OnDevManageClicked));
    instrumentBtn.push_back(ButtonInfo(tr("维护保养"), "maintain_btn", ":/Leonis/resource/image/icon-maintain.png", &UtilityWidget::OnMaintain));
    instrumentBtn.push_back(ButtonInfo(tr("自动开机"), "btn_startup_set", ":/Leonis/resource/image/icon-startup-set.png", &UtilityWidget::OnStartupSetClicked));
    instrumentBtn.push_back(ButtonInfo(tr("状态"), "btn_status", ":/Leonis/resource/image/icon-status.png", &UtilityWidget::OnStatus));
	instrumentBtn.push_back(ButtonInfo(tr("耗材管理"), "btn_consumables_manage", ":/Leonis/resource/image/icon-consumables-manage.png", &UtilityWidget::OnConsumablesManageClicked));
	instrumentBtn.push_back(ButtonInfo(tr("功能管理"), "btn_modulemgr", ":/Leonis/resource/image/icon-auto-load-reagent.png", &UtilityWidget::OnFunctionManageClicked));
	InitButtonArea(ui->widgetInstrument, tr("仪器"), 2, 4, instrumentBtn);

    // 项目分配对特殊机型的处理，含有c2000设备时才显示该按钮
    auto& CIM = CommonInformationManager::GetInstance();
    bool hasC2000Dev = false;
    for (auto& iter : CIM->GetDeviceMaps())
    {
		if (iter.second->deviceClassify == ::tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY
                    && !iter.second->groupName.empty())
        {
            hasC2000Dev = true;
			break;
        }
    }

    SetButtonVisiable(ui->widgetAssay, "btn_assay_allocation", hasC2000Dev); 

    // 返回按钮
    for (auto btn : QWidget::findChildren<QPushButton*>("flat_return"))
    {
        connect(btn, &QPushButton::clicked, this, [&] 
        {
            if (!CurrentWidgetHasUnSaveData())
            {
                SetCurrentWidgetIsMain();
                POST_MESSAGE(MSG_ID_CURRENT_MODEL_NAME, tr("> 应用"));
            }
        });
    }

    // 刷新样式
    for (auto btn : ui->page_main->findChildren<QPushButton*>())
    {
        btn->setStyle(QApplication::style());
    }

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);

	// 设置过滤的按钮
	InitFilterMenuLocLab();

    // 设置界面左下角当前模块显示标签文字
    for (auto btn : ui->page_main->findChildren<QPushButton*>())
    {
        connect(btn, &QPushButton::clicked, this, [&] {
            QPushButton* btn = qobject_cast<QPushButton*>(sender());
			if (nullptr == btn)
			{
				return;
			}
			// 如果是需要过滤的按钮（子界面显示二级菜单，弹出对话框不显示导航二级菜单，保持与其他主模块的功能一致）
			if (m_setFilterMenuLocLab.find(btn->objectName())!=m_setFilterMenuLocLab.end())
			{
				return;
			}
			// 查找标签
            for (auto lab : btn->findChildren<QLabel*>())
            {
                if (!lab->text().isEmpty())
                {
					// 显示导航的二级菜单
                    POST_MESSAGE(MSG_ID_CURRENT_MODEL_NAME, (tr("> 应用 > ") + lab->text()));
                    return;
                }
            }
        });
    }
}

void UtilityWidget::OnAssayOrder()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 初始化
    if (m_assayOrder == nullptr)
    {
        m_assayOrder = new QAssayShowPostion(this);
    }

    m_assayOrder->show();
}

///
/// @brief
///     用户管理按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月17日，新建函数
///
void UtilityWidget::OnUserMngBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 初始化
    std::shared_ptr<UserMngDlg> spUserMngDlg = std::make_shared<UserMngDlg>(this);
    spUserMngDlg->exec();
}

void UtilityWidget::OnCommParamSetClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 初始化
    if (m_pCommParamSetDlg == nullptr)
    {
        m_pCommParamSetDlg = new CommParamSetDlg(this);
    }

    m_pCommParamSetDlg->LoadDataToCtrls();

    m_pCommParamSetDlg->show();
}

void UtilityWidget::OnStartupSetClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    StartupSetDlg autoPower(this);
    autoPower.exec();
}

///
/// @brief 显示设置的被点击
///
///
/// @par History:
/// @li 5774/WuHongTao，2023年2月2日，新建函数
///
void UtilityWidget::OnDisplaySetClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 初始化
    if (m_pDisplayShowDlg == nullptr)
    {
        m_pDisplayShowDlg = new QDisplayShow(this);
    }

    m_pDisplayShowDlg->show();
}

void UtilityWidget::OnSampleRackClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 初始化
    if (m_pSampleRackAllocationDlg == nullptr)
    {
        m_pSampleRackAllocationDlg = new SampleRackAllocationDlg(this);
    }
    m_pSampleRackAllocationDlg->LoadDataToDlg();
    m_pSampleRackAllocationDlg->show();
}

void UtilityWidget::OnDevManageClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 初始化
    if (m_pDevManageDlg == nullptr)
    {
        m_pDevManageDlg = new DevManageDlg(this);
    }

    m_pDevManageDlg->LoadDataToCtrls();

    m_pDevManageDlg->show();
}

void UtilityWidget::OnSoftwareVersionClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 初始化
    if (m_pSoftwareVersionDlg == nullptr)
    {
        m_pSoftwareVersionDlg = new SoftwareVersionDlg(this);
    }

    m_pSoftwareVersionDlg->exec();
}

void UtilityWidget::OnInjectionModelClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 初始化
    if (m_pDetectionSetDlg == nullptr)
    {
        m_pDetectionSetDlg = new DetectionSetDlg(this);
    }

    m_pDetectionSetDlg->LoadDataToCtrls();

    m_pDetectionSetDlg->show();
}

void UtilityWidget::OnClearSetClicked()
{
    // 初始化
    if (m_pClearSetDlg == nullptr)
    {
        m_pClearSetDlg = new ClearSetDlg(this);
    }

    m_pClearSetDlg->LoadDataToCtrls();

    m_pClearSetDlg->show();
}

///
/// @brief  耗材管理被点击
///
/// @param[in]    void
///
/// @return void
///
/// @par History:
/// @li 7915/LeiDingXiang，2023年2月2日，新建函数
///
void UtilityWidget::OnConsumablesManageClicked()
{
    //初始化耗材管理弹窗
    if (m_pConsumablesManageDlg == nullptr)
    {
        m_pConsumablesManageDlg = new ConsumablesManageDlg(this);
    }

    m_pConsumablesManageDlg->show();
}

void UtilityWidget::OnSampleReceiveModeClicked()
{
    //初始化耗材管理弹窗
    if (m_pSampleReceiveModeDlg == nullptr)
    {
        m_pSampleReceiveModeDlg = new SampleReceiveModeDlg(this);
    }

    m_pSampleReceiveModeDlg->LoadDataToDlg();

    m_pSampleReceiveModeDlg->show();
}

void UtilityWidget::OnHelpFileClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    QString strPVersion = QSysInfo::productVersion();
    int iPVersion = strPVersion.toDouble();
    ULOG(LOG_INFO, "productVersion is %d", iPVersion);
    if (iPVersion < 10)
    {
        auto softwareType = CommonInformationManager::GetInstance()->GetSoftWareType();
        // 生免联机
        if (softwareType == SOFTWARE_TYPE::CHEMISTRY_AND_IMMUNE)
        {
            ULOG(LOG_INFO, "using QDesktopServices");
            QString strDirPath = QCoreApplication::applicationDirPath();
            QString strFilePath = strDirPath + "/ui_cfg/handbook";
            QString strSuffix = tr("*.") + QString::fromStdString(DictionaryQueryManager::GetCurrentLanuageType()) + tr(".pdf");
            QStringList nameFilters;
            nameFilters << strSuffix;
            QDir dir(strFilePath);
            QFileInfoList FilesDir = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
            for (auto info : FilesDir)
            {
                if (info.isDir())
                {
                    QDir subDir(info.filePath());
                    QFileInfoList files = subDir.entryInfoList(nameFilters, QDir::Files | QDir::Readable, QDir::Name);
                    if (files.isEmpty())
                    {
                        continue;
                    }

                    QFileInfo info = files.first();
                    QUrl url = QUrl::fromLocalFile(info.filePath());
                    QDesktopServices::openUrl(url);
                    QTime time;
                    time.start();
                    while (time.elapsed() < 2500)                   // 等待 2.5s 再打开下一个文档
                        QCoreApplication::processEvents();
                }
            }
        }
        else if (softwareType == SOFTWARE_TYPE::IMMUNE)
        {
            // 免疫
            QString strDirPath = QCoreApplication::applicationDirPath();
            QString strFilePath = strDirPath + "/ui_cfg/handbook/im";
            QString strSuffix = tr("*.") + QString::fromStdString(DictionaryQueryManager::GetCurrentLanuageType()) + tr(".pdf");
            QDir dir(strFilePath);

            QStringList nameFilters;
            nameFilters << strSuffix;
            QFileInfoList files = dir.entryInfoList(nameFilters, QDir::Files | QDir::Readable, QDir::Name);
            if (files.isEmpty())
            {
                return ;
            }

            QFileInfo info = files.first();
            QUrl url = QUrl::fromLocalFile(info.filePath());
            QDesktopServices::openUrl(url);
        }
        else
        {
            // 生化
            QString strDirPath = QCoreApplication::applicationDirPath();
            QString strFilePath = strDirPath + "/ui_cfg/handbook/ch";
            QString strSuffix = tr("*.") + QString::fromStdString(DictionaryQueryManager::GetCurrentLanuageType()) + tr(".pdf");
            QDir dir(strFilePath);

            QStringList nameFilters;
            nameFilters << strSuffix;
            QFileInfoList files = dir.entryInfoList(nameFilters, QDir::Files | QDir::Readable, QDir::Name);
            if (files.isEmpty())
            {
                return;
            }

            QFileInfo info = files.first();
            QUrl url = QUrl::fromLocalFile(info.filePath());
            QDesktopServices::openUrl(url);
        }
    }
    else
    {
        ULOG(LOG_INFO, "using QWidget");
        bool bRect = PdfPretreatment::GetInstance()->InitPdfLibrary();
        ui->stackedWidget->setCurrentIndex(TAB_INDEX_USER_MANUAL);
    }
    
}

///
/// @brief  打印设置被点击
///
/// @param[in]    void
///
/// @return void
///
/// @par History:
/// @li 6889/ChenWei，2023年3月20日，新建函数
///
void UtilityWidget::OnPrintSetClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    int iRect = printcom::printcom_open_settings_window();
    ULOG(LOG_INFO, "%d", iRect);
    return;
}

///
/// @brief  导出设置被点击
///
/// @param[in]    void
///
/// @return void
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年11月12日，新建函数
///
void UtilityWidget::OnExportSetClicked()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	std::shared_ptr<ExportSetDlg> spExportSetDlg = std::make_shared<ExportSetDlg>(this);
	spExportSetDlg->exec();
}

void UtilityWidget::OnFunctionManageClicked()
{
    if (m_pfunctionManageDlg == nullptr)
    {
        m_pfunctionManageDlg = new functionManageDlg(this);
    }

    m_pfunctionManageDlg->show();
}

void UtilityWidget::OnAssayAllocationClicked()
{
    if (m_pAssayAllocationDlg == nullptr)
    {
        m_pAssayAllocationDlg = new AssayAllocationDlg(this);
    }

    m_pAssayAllocationDlg->show();
}

///
/// @bref
///		项目设置按钮
///
/// @par History:
/// @li 8276/huchunli, 2023年7月22日，新建函数
///
void UtilityWidget::OnAssaySettings()
{
    ui->stackedWidget->setCurrentIndex(TAB_INDEX_ASSAY_CONFIG);
}

///
/// @brief
///     节点调试
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月12日，新建函数
///
void UtilityWidget::OnNodeDebugBtnClicked()
{
	if (m_pNodeDbgDlg == nullptr)
	{
		m_pNodeDbgDlg = new NodeDbgDlg(this);
	}

	// 查找是否存在非待机或者非停止状态的设备
	bool canDebug = true;
	auto mapDevices = CommonInformationManager::GetInstance()->GetDeviceMaps();
	for (const auto &pair : mapDevices)
	{
		if (pair.second->status != ::tf::DeviceWorkState::DEVICE_STATUS_STANDBY
			&& pair.second->status != ::tf::DeviceWorkState::DEVICE_STATUS_HALT
			&& pair.second->status != ::tf::DeviceWorkState::DEVICE_STATUS_WARMUP)
		{
			canDebug = false;
			break;
		}
	}

	// 无法调试
	if (!canDebug)
	{
		std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("提示"), tr("存在非待机或者非停止状态的设备，"\
			"进行调试所有设备将停止运行，是否继续?"), TipDlgType::TWO_BUTTON));
		if (pTipDlg->exec() == QDialog::Rejected)
		{
			return;
		}

		// 如果用户点击了确认，执行停止
		DcsControlProxy::GetInstance()->StopTest();
	}

     // 显示界面
    m_pNodeDbgDlg->show();
}

///
/// @brief
///     流程检查
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月12日，新建函数
///
void UtilityWidget::OnProcessCheckBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    if (m_pProcessCheckDlg == nullptr)
    {
        m_pProcessCheckDlg = new ProcessCheckDlg(this);
    }

    m_pProcessCheckDlg->exec();
}

///
/// @brief
///     调试流程按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月12日，新建函数
///
void UtilityWidget::OnDiagnoseBtnCliked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    std::shared_ptr<DbgProcessDlg> spDbgProcessDlg = std::make_shared<DbgProcessDlg>(this);
    spDbgProcessDlg->exec();
}

///
/// @brief
///     传感器状态按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月12日，新建函数
///
void UtilityWidget::OnSensorStateBtnCliked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    std::shared_ptr<SensorStateDlg> spSensorStateDlg = std::make_shared<SensorStateDlg>(this);
    spSensorStateDlg->exec();
}

///
/// @brief
///     参数按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月12日，新建函数
///
void UtilityWidget::OnParamBtnClicked()
{
    ui->stackedWidget->setCurrentIndex(TAB_INDEX_PARAM_IMMUNE);
}

///
/// @bref
///		项目位置按钮
///
/// @par History:
/// @li 8276/huchunli, 2023年7月22日，新建函数
///
void UtilityWidget::OnAssayPositionSettings()
{
    ui->stackedWidget->setCurrentIndex(TAB_INDEX_ASSAY_INDEX);
}

///
/// @bref
///		备用订单
///
/// @par History:
/// @li 8276/huchunli, 2023年7月22日，新建函数
///
void UtilityWidget::OnStandbyOrder()
{
    ui->stackedWidget->setCurrentIndex(TAB_INDEX_BACKUP_ORDER_INDEX);
}

///
/// @bref
///		交叉污染按钮
///
/// @par History:
/// @li 8276/huchunli, 2023年7月22日，新建函数
///
void UtilityWidget::OnSpecialWash()
{
    ui->stackedWidget->setCurrentIndex(TAB_INDEX_SPECIAL_WASH);
}

///
/// @bref
///		日志按钮
///
/// @par History:
/// @li 8276/huchunli, 2023年7月22日，新建函数
///
void UtilityWidget::OnLog()
{
    ui->stackedWidget->setCurrentIndex(TAB_INDEX_LOG);
}

///
/// @bref
///		状态按钮
///
/// @par History:
/// @li 8276/huchunli, 2023年7月22日，新建函数
///
void UtilityWidget::OnStatus()
{
    ui->stackedWidget->setCurrentIndex(TAB_INDEX_STATUS);
}

///
/// @bref
///		显示按钮
///
/// @par History:
/// @li 8276/huchunli, 2023年7月22日，新建函数
///
void UtilityWidget::OnDisplay()
{
    ui->stackedWidget->setCurrentIndex(TAB_INDEX_DISPLAY);
}

///
/// @bref
///		维护保养
///
/// @par History:
/// @li 8276/huchunli, 2023年7月22日，新建函数
///
void UtilityWidget::OnMaintain()
{
    // 初始化维护数据缓存
    MaintainDataMng::GetInstance();
    ui->stackedWidget->setCurrentIndex(TAB_INDEX_MAINTAIN);
}

///
/// @bref
///		提示按钮
///
/// @par History:
/// @li 8276/huchunli, 2023年7月22日，新建函数
///
void UtilityWidget::OnPrompt()
{
    ui->stackedWidget->setCurrentIndex(TAB_INDEX_PROMPT);
}

///
/// @bref
///		计算项目
///
/// @par History:
/// @li 8276/huchunli, 2023年7月22日，新建函数
///
void UtilityWidget::OnCalculateAssay()
{
    ui->stackedWidget->setCurrentIndex(TAB_INDEX_CALC);
}

///
/// @bref
///		统计分析
///
/// @par History:
/// @li 8276/huchunli, 2023年7月22日，新建函数
///
void UtilityWidget::OnStatistic()
{
    ui->stackedWidget->setCurrentIndex(TAB_INDEX_STATISTIC);
}

///
/// @brief
///     AI识别设置按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2023年11月30日，新建函数
///
void UtilityWidget::OnAiAnalysisCfgClicked()
{
    ULOG(LOG_INFO, __FUNCTION__);
    ui->stackedWidget->setCurrentIndex(TAB_INDEX_AI_ANALYSIS);
}

///
/// @bref
///		权限变更响应函数
///
/// @par History:
/// @li 8276/huchunli, 2023年7月22日，新建函数
///
void UtilityWidget::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();

    // 特殊清洗设置
    SOFTWARE_TYPE curType = CommonInformationManager::GetInstance()->GetSoftWareType();
    bool bShowSpecial(false);
    bool bShowSRegent = userPms->IsPermissionShow(PMS_SETTINGS_SPECIALWASH_REAGET);
    bool bShowSCup = userPms->IsPermissionShow(PMS_SETTINGS_SPECIALWASH_CUP);
    bool bShowSample = userPms->IsPermissionShow(PMS_SETTINGS_SPECIALWASH_SAMPLE);
    switch (curType)
    {
    case CHEMISTRY:
        bShowSpecial = (bShowSRegent || bShowSCup);
        break;
    case IMMUNE:
        bShowSpecial = bShowSample;
        break;
    case CHEMISTRY_AND_IMMUNE:
        bShowSpecial = (bShowSRegent || bShowSCup || bShowSample);
        break;
    default:
        break;
    }
    SetButtonVisiable(ui->widgetAssay, "btn_special_wash", bShowSpecial);

    // 样本接收模式
    SetButtonVisiable(ui->widgetCheck, "btn_sample_recive_model", userPms->IsPermissionShow(PSM_IM_TESTSET_RECEIVEMODE));

    // 打印设置(暂时操作员不可范围)
    SetButtonVisiable(ui->widgetSystem, "btn_print_set", userPms->IsPermissionShow(PSM_PRINT_SETTINGS));
    // 通讯设置
    SetButtonVisiable(ui->widgetSystem, "btn_commparam_set", userPms->IsPermissionShow(PSM_IM_LIS_PARAM));
    // 导出设置
    SetButtonVisiable(ui->widgetSystem, "btn_export_set", userPms->IsPermissionShow(PSM_IM_EXPORT_SET));
    // 统计分析
    bool bSampleStat = userPms->IsPermissionShow(PSM_IM_STATISTICS_SAMPLEVOL);
    bool bReagentStat = userPms->IsPermissionShow(PSM_IM_STATISTICS_REAGENT);
    bool bCaliStat = userPms->IsPermissionShow(PSM_IM_STATISTICS_CALI);
    SetButtonVisiable(ui->widgetSystem, "statistic_analysis_btn", (bSampleStat || bReagentStat || bCaliStat));
    // AI设置
    SetButtonVisiable(ui->widgetSystem, "btn_ai_analysis", userPms->IsPermissionShow(PSM_IM_AI_RECONGLIZE));

    // 耗材管理
    bool bShowSupp(false);
    switch (curType)
    {
    case CHEMISTRY:
        bShowSupp = userPms->IsPermissionShow(PSM_ISE_SAMPLENEEDLE_WASHCFG);
        break;
    case IMMUNE:
        bShowSupp = userPms->IsPermissionShow(PSM_IM_SUPPLYMGR);
        break;
    case CHEMISTRY_AND_IMMUNE:
        bShowSupp = userPms->IsPermissionShow(PSM_ISE_SAMPLENEEDLE_WASHCFG) || userPms->IsPermissionShow(PSM_IM_SUPPLYMGR);
        break;
    default:
        break;
    }
    SetButtonVisiable(ui->widgetInstrument, "btn_consumables_manage", bShowSupp);

    // 功能管理
    SetButtonVisiable(ui->widgetInstrument, "btn_modulemgr", userPms->IsPermissionShow(PSM_IM_FUNCTION_MANAGE));

    // 自动开机设置
    SetButtonVisiable(ui->widgetInstrument, "btn_startup_set", userPms->IsPermissionShow(PSM_IM_AUTO_POWERON));

    // 工程师调试-节点调试
    SetButtonVisiable(ui->DebugBtnWgt, "btn_node_debug", userPms->IsPermissionShow(PSM_ENGDEBUG_NODEDEBUG));
    // 工程师调试-传感器状态
    SetButtonVisiable(ui->DebugBtnWgt, "btn_sensor_state", userPms->IsPermissionShow(PSM_ENGDEBUG_SENSORSTUTA));
    // 工程师调试-调试流程（免疫）
    bool bDebugGram = (curType == CHEMISTRY) ? false : userPms->IsPermissionShow(PSM_ENGDEBUG_DEBUGPROGRAM);
    SetButtonVisiable(ui->DebugBtnWgt, "btn_diagnose", bDebugGram);
    // 工程师调试-流程检测（免疫）
    bool bProgramCheck = (curType == CHEMISTRY) ? false : userPms->IsPermissionShow(PSM_ENGDEBUG_PROGRAMCHECK);
    SetButtonVisiable(ui->DebugBtnWgt, "btn_process_check", bProgramCheck);
    // 工程师调试-参数（免疫）
    bool bParameter = (curType == CHEMISTRY) ? false : userPms->IsPermissionShow(PSM_ENGDEBUG_PARAMETER);
    SetButtonVisiable(ui->DebugBtnWgt, "btn_param", bParameter);

    bool bShowLabel = (userPms->IsPermissionShow(PSM_ENGDEBUG_NODEDEBUG) || userPms->IsPermissionShow(PSM_ENGDEBUG_SENSORSTUTA) || \
        bDebugGram || bProgramCheck || bParameter);
    ui->DebugBtnWgt->setVisible(bShowLabel);
}

///
/// @bref
///		往一个Widget框中添加按钮列表
///
/// @param[in] pRootWidget 目标框
/// @param[in] title 框标题
/// @param[in] rowCnt 框的行限制
/// @param[in] colCnt 框的列限制
/// @param[in] btnInfo 按钮信息列表
///
/// @par History:
/// @li 8276/huchunli, 2023年7月22日，新建函数
///
void UtilityWidget::InitButtonArea(QWidget* pRootWidge, const QString& titlet, int rowCnt, int colCnt, const std::vector<ButtonInfo>& btnInfo)
{
    // 特殊处理：如果是调试按钮，则与其他按钮的样式不同
    std::string propertyName = (pRootWidge == ui->DebugBtnWgt) ? "systemDebug" : "system";

    QVBoxLayout *qVLayout = new QVBoxLayout(pRootWidge);

    // 标题标签
    QHBoxLayout *qHeadLayout = new QHBoxLayout(pRootWidge);
    qHeadLayout->addWidget(new QLabel(titlet));
    qHeadLayout->addStretch();
    qVLayout->addLayout(qHeadLayout);

    // 添加按钮
    QGridLayout *pGridLayout = new QGridLayout(pRootWidge);
    std::vector<ButtonInfo>::const_iterator it = btnInfo.begin();
    int row = 0;
    int col = 0;
    for (; it != btnInfo.end(); ++it)
    {
        if (row + 1 > rowCnt)
        {
            ULOG(LOG_ERROR, "Row amount over the except.");
            break;
        }

        QPushButton *pBtn = new QPushButton(pRootWidge);
        pBtn->setProperty("buttons", QVariant(propertyName.c_str()));
        pBtn->setObjectName(it->m_objectName);
        pBtn->setMaximumWidth(256);
        pBtn->setMinimumWidth(256);
        pBtn->setMaximumHeight(66);
        pBtn->setMinimumHeight(60);
        pGridLayout->addWidget(pBtn, row, col, Qt::AlignLeft);
        col++;

        // 在按钮上添加图标Label和文字Label，并进行水平布局
        QLabel* labeIcon = new QLabel(pBtn);
        labeIcon->setPixmap(QPixmap(it->m_iconName));

        QLabel* labeText = new QLabel(it->m_showName, pBtn);
        labeText->setAlignment(Qt::AlignmentFlag::AlignVCenter | Qt::AlignmentFlag::AlignRight);
        labeText->setWordWrap(true);
        QHBoxLayout* hLayout = new QHBoxLayout();
        hLayout->addSpacing(15);
        hLayout->addWidget(labeIcon);
        hLayout->addWidget(labeText);
        hLayout->addSpacing(20);

        // 添加事件处理
        connect(pBtn, &QPushButton::clicked, this, it->m_memberSlot);

        pBtn->setLayout(hLayout);

        if (col + 1 > colCnt)
        {
            row++;
            col = 0;
        }
    }
    qVLayout->addLayout(pGridLayout);
    qVLayout->addStretch();
}

///
/// @bref
///		隐藏一个Widget中的按钮，后续按钮往前移动
///
/// @param[in] pRootWidget 目标框
/// @param[in] btnObjName 按钮类名
///
/// @par History:
/// @li 8276/huchunli, 2023年7月22日，新建函数
///
void UtilityWidget::SetButtonHide(QWidget* pRootWidget, const QString& btnObjName)
{
    QGridLayout* pGridLayout = pRootWidget->findChild<QGridLayout*>();
    if (pGridLayout == Q_NULLPTR)
    {
        return;
    }
    int maxCol = pGridLayout->columnCount();
    QList<QPushButton*> allButn = pRootWidget->findChildren<QPushButton*>();

    // 筛选出显示的按钮
    QList<QPushButton*> displayBtn;
    for (QPushButton* btn : allButn)
    {
        if (!btn->isHidden())
        {
            displayBtn.append(btn);
        }
    }

    // 移除目标按钮以及之后的widget.
    int removeIdx = -1;
    for (int i = 0; i < displayBtn.size(); ++i)
    {
        if (displayBtn[i]->objectName() == btnObjName)
        {
            removeIdx = i;
            displayBtn[i]->hide();
            for (; i < displayBtn.size(); ++i)
            {
                pGridLayout->removeWidget(displayBtn[i]);
            }
            break;;
        }
    }

    // 当按钮没有变化时，不做按钮位置变更
    if (removeIdx == -1)
    {
        return;
    }

    // 重新填充布局
    int row = removeIdx / maxCol;
    int col = removeIdx % maxCol;
    for (int i = removeIdx + 1; i < displayBtn.size(); ++i)
    {
        pGridLayout->addWidget(displayBtn[i], row, col, Qt::AlignLeft);

        col++;
        if (col + 1 > maxCol)
        {
            col = 0;
            row++;
        }
    }
    if (displayBtn.size() - 1 < maxCol)
    {
        QWidget* qHolderWidget = new QWidget(this);
        qHolderWidget->setMinimumWidth(240);
        qHolderWidget->setObjectName("place_holder");
        pGridLayout->addWidget(qHolderWidget, row, col, Qt::AlignLeft);
    }
}

///
/// @bref
///		显示一个Widget中的按钮，后续按钮往后移动
///
/// @param[in] pRootWidget 目标框
/// @param[in] btnObjName 按钮类名
///
/// @par History:
/// @li 8276/huchunli, 2023年7月22日，新建函数
///
void UtilityWidget::SetButtonShow(QWidget* pRootWidget, const QString& btnObjName)
{
    QPushButton* btn = pRootWidget->findChild<QPushButton*>(btnObjName);
    if (btn == Q_NULLPTR || !btn->isHidden())
    {
        return;
    }
    btn->show();

    // 移除所有布局
    QGridLayout* pGridLayout = pRootWidget->findChild<QGridLayout*>();
    if (pGridLayout == Q_NULLPTR)
    {
        return;
    }
    QList<QWidget*> allWidget = pRootWidget->findChildren<QWidget*>("place_holder");
    for (QWidget* pwidget : allWidget)
    {
        pGridLayout->removeWidget(pwidget);
        delete pwidget;
        pwidget = Q_NULLPTR;
    }

    int maxCol = pGridLayout->columnCount();
    QList<QPushButton*> allButn = pRootWidget->findChildren<QPushButton*>();

    // 获取显示按钮
    QList<QPushButton*> showBtn;
    for (QPushButton* pCurBtn : allButn)
    {
        if (!pCurBtn->isHidden())
        {
            showBtn.append(pCurBtn);
            pGridLayout->removeWidget(pCurBtn);
        }
    }

    // 重新填充布局
    int row = 0;
    int col = 0;
    for (QPushButton* pCurBtn : showBtn)
    {
        pGridLayout->addWidget(pCurBtn, row, col, Qt::AlignLeft);

        col++;
        if (col + 1 > maxCol)
        {
            col = 0;
            row++;
        }
    }
    if (showBtn.size() < maxCol)
    {
        for (; col < maxCol; ++col)
        {
            QWidget* qHolderWidget = new QWidget(this);
            qHolderWidget->setMinimumWidth(240);
            qHolderWidget->setObjectName("place_holder");
            pGridLayout->addWidget(qHolderWidget, row, col, Qt::AlignLeft);
        }
    }
}

///
/// @bref
///		设置按钮的显示与隐藏状态，并调整按钮的布局中的位置
///
/// @param[in] pRootWidget 目标框
/// @param[in] btnObjName 目标按钮对象名字
/// @param[in] isShow 显示或隐藏
///
/// @par History:
/// @li 8276/huchunli, 2023年7月22日，新建函数
///
void UtilityWidget::SetButtonVisiable(QWidget* pRootWidget, const QString& btnObjName, bool isShow)
{
    if (isShow)
    {
        SetButtonShow(pRootWidget, btnObjName);
    }
    else
    {
        SetButtonHide(pRootWidget, btnObjName);
    }
}

///
/// @brief 初始化应用模块在主界面左下角导航菜单过滤
///
///
/// @return 
///
/// @par History:
/// @li 1556/Chenjianlin，2024年3月19日，新建函数
///
void UtilityWidget::InitFilterMenuLocLab()
{
	m_setFilterMenuLocLab = { "btn_sampleadd_sequence",	"btn_assay_allocation",	// 项目
		"btn_clear_set", "btn_print_set", "btn_export_set", "btn_user_mng", "btn_commparam_set", "software_version_btn",	// 系统
		"btn_injection_model", "btn_sample_recive_model", "btn_sample_rack_allocation",	// 检测
		"btn_dev_manage", "btn_startup_set", "btn_consumables_manage", "btn_modulemgr",	// 仪器
		"btn_node_debug", "btn_process_check", "btn_diagnose", "btn_sensor_state", // 调试
	};
}
