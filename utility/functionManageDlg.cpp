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
/// @file     functionManageDlg.cpp
/// @brief    应用--仪器--功能管理
///
/// @author   7951/LuoXin
/// @date     2023年8月1日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2023年8月1日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include <QStandardItemModel>
#include "ui_functionManageDlg.h"
#include "functionSetDlg.h"
#include "functionConfigDlg.h"
#include "functionManageDlg.h"
#include "QHeadSortDisplay.h"
#include "thrift/DcsControlProxy.h"
#include "manager/UserInfoManager.h"
#include "manager/DictionaryQueryManager.h"

#include "src/common/common.h"
#include "src/common/Mlog/mlog.h"
#include "src/public/ch/ChConfigDefine.h"
#include "src/public/ise/IseConfigDefine.h"
#include "src/public/ch/ChConfigSerialize.h"
#include "src/public/im/ImConfigSerialize.h"
#include "src/public/ise/IseConfigSerialize.h"
#include "src/public/SerializeUtil.hpp"
#include "shared/uidef.h"
#include "shared/tipdlg.h"
#include "shared/uicommon.h"
#include "shared/msgiddef.h"
#include "shared/messagebus.h"
#include "shared/QComDelegate.h"
#include "shared/CReadOnlyDelegate.h"
#include "shared/CommonInformationManager.h"

#define             statusRole                Qt::UserRole +1
#define             indexRole                 Qt::UserRole +2

#define             CH_FLAG                   "ch"
#define             IM_FLAG                   "im"

// 表格各信息所在列
enum TableViewColunm
{
    COLUNM_NUM,                             // 序号
    COLUNM_NAME,				            // 功能
};

functionManageDlg::functionManageDlg(QWidget *parent /*= Q_NULLPTR*/)
    : BaseDlg(parent)
    , ui(new Ui::functionManageDlg)
    , m_pChTabViewModel(new QColorHeaderModel)
    , m_pImTabViewModel(new QColorHeaderModel)
    , m_pFunctionSetDlg(new functionSetDlg(this))
    , m_pFunctionConfigDlg(new functionConfigDlg(this))
{
    ui->setupUi(this);

    // 设置标题
    BaseDlg::SetTitleName(tr("功能管理"));

    InitCtrls();
}

functionManageDlg::~functionManageDlg()
{

}

void functionManageDlg::LoadDataToCtrls()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    ::tf::DeviceInfoQueryResp qr;
    ::tf::DeviceInfoQueryCond qc;

    if (!DcsControlProxy::GetInstance()->QueryDeviceInfo(qr, qc)
        || qr.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "QueryDeviceInfo Failed !");
        return;
    }

    for (const tf::DeviceInfo& dev : qr.lstDeviceInfos)
    {
        if (dev.deviceType == ::tf::DeviceType::DEVICE_TYPE_I6000
          ||dev.deviceType == ::tf::DeviceType::DEVICE_TYPE_C1000
          ||dev.deviceType == ::tf::DeviceType::DEVICE_TYPE_ISE1005)
        {
            SetViewItemSelect(dev);
        }
    }
}

void functionManageDlg::showEvent(QShowEvent *event)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 基类先处理
    QWidget::showEvent(event);

    LoadDataToCtrls();

    // 选中第一行
    ui->tabWidget->setCurrentIndex(0);
    if (ui->tabWidget->currentWidget()->objectName().contains(CH_FLAG))
    {
        ui->ch_tableView->selectRow(0);
    }
}

void functionManageDlg::InitCtrls()
{
    auto type = CommonInformationManager::GetInstance()->GetSoftWareType();
    if (type == CHEMISTRY)
    {
        ui->tabWidget->removeTab(1);
    } 
    else if (type == IMMUNE)
    {
        ui->tabWidget->removeTab(0);
        ui->function_set_btn->hide();
    }

    // 保存按钮
    connect(ui->save_btn, &QPushButton::clicked, this, &functionManageDlg::OnSaveBtnClicked);
    // 保存功能管理设置信息
    connect(m_pFunctionConfigDlg->GetSaveBtnPtr(), &QPushButton::clicked, this, &functionManageDlg::OnFunctionConfigSave);
    // 保存功能设置弹窗
    connect(ui->function_set_btn, &QPushButton::clicked, this, &functionManageDlg::OnFunctionSetBtnClicked);
    // 功能管理弹窗
//     connect(ui->btn_function_config, &QPushButton::clicked, this, [&]
//     {
//         if (ui->tabWidget->currentWidget()->objectName().contains(CH_FLAG))
//         {
//             m_pFunctionConfigDlg->LoadDataToView(m_functionManageData[CH_FLAG]);
//         }
//         else
//         {
//             m_pFunctionConfigDlg->LoadDataToView(m_functionManageData[IM_FLAG]);
//         }
// 
//         m_pFunctionConfigDlg->show();
//     });

    // 切换当前页
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, [&] 
    {
        ui->function_set_btn->setVisible(ui->tabWidget->currentWidget()->objectName().contains(CH_FLAG));
    });

    // 勾选框
    connect(ui->ch_tableView, &QTableView::doubleClicked, this, [&](const QModelIndex& index)
    {
        if (index.column() >= COLUNM_NAME)
        {
            m_pChTabViewModel->item(index.row(), index.column())->setData(
                !m_pChTabViewModel->item(index.row(), index.column())->data().toBool(), statusRole);
        }
    });

    // 勾选框
    connect(ui->im_tableView, &QTableView::doubleClicked, this, [&](const QModelIndex& index)
    {
        if (index.column() >= COLUNM_NAME)
        {
            m_pImTabViewModel->item(index.row(), index.column())->setData(
                !m_pImTabViewModel->item(index.row(), index.column())->data().toBool(), statusRole);
        }
    });

    InitTabView();

    // 表格选中行改变
    connect(ui->ch_tableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &functionManageDlg::OnCurrentRowChanged);

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);
}

void functionManageDlg::InitTabView()
{
    ui->ch_tableView->setModel(m_pChTabViewModel);
    ui->im_tableView->setModel(m_pImTabViewModel);

    // 设置表头
    QStringList chHeaderList({ tr("序号"), tr("功能") });
    QStringList imHeaderList({ tr("序号"), tr("功能") });
    const auto& CIM = CommonInformationManager::GetInstance();
    for (auto dev : CIM->GetDeviceMaps())
    {
        if (dev.second->deviceClassify == ::tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY
            || dev.second->deviceClassify == ::tf::AssayClassify::ASSAY_CLASSIFY_ISE)
        {
            chHeaderList.push_back(QString::fromStdString(dev.second->groupName + dev.second->name));
        }
        else if (dev.second->deviceClassify == ::tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE)
        {
            imHeaderList.push_back(QString::fromStdString(dev.second->name));
        }
    }

    m_pChTabViewModel->setHorizontalHeaderLabels(chHeaderList);
    m_pImTabViewModel->setHorizontalHeaderLabels(imHeaderList);

	ui->ch_tableView->setColumnWidth(0, 80);
	ui->ch_tableView->setColumnWidth(1, 260);
    ui->im_tableView->setColumnWidth(0, 80);
    ui->im_tableView->setColumnWidth(1, 260);

    for (int i=2; i<m_pChTabViewModel->columnCount(); i++)
    {
        // 设置表头颜色
        m_pChTabViewModel->SetColunmColor(i, UI_HEADER_FONT_COLOR);
        // 勾选代理
        ui->ch_tableView->setItemDelegateForColumn(i, new CheckBoxDelegate(this));
    }

    for (int i = 2; i < m_pImTabViewModel->columnCount(); i++)
    {
        // 设置表头颜色
        m_pImTabViewModel->SetColunmColor(i, UI_HEADER_FONT_COLOR);
        // 勾选代理
        ui->im_tableView->setItemDelegateForColumn(i, new CheckBoxDelegate(this));
    }
    //悬浮显示文本代理
    ui->im_tableView->setMouseTracking(true);
    ui->im_tableView->setItemDelegateForColumn(1, new QShowTextDelegate(this));

    UpdateRowDataToView();
}

void functionManageDlg::UpdateRowDataToView()
{   
    m_pChTabViewModel->removeRows(0, m_pChTabViewModel->rowCount());
    m_pImTabViewModel->removeRows(0, m_pImTabViewModel->rowCount());

    if (!DictionaryQueryManager::GetFunctionManageConfig(m_functionManageData))
    {
        ULOG(LOG_ERROR, "Failed to get functionmanage config.");
        return;
    }

    // 生化
    for (auto& item : m_functionManageData[CH_FLAG])
    {
        // 序号
        AddTextToTableView(m_pChTabViewModel, item.postion, COLUNM_NUM, QString::number(item.postion + 1));
        // 模块
        AddTextToTableView(m_pChTabViewModel, item.postion, COLUNM_NAME, m_pFunctionConfigDlg->GetStringByIndex(item.index));

        for (int i = 2; i < m_pChTabViewModel->columnCount(); i++)
        {
            auto si = new QStandardItem("");
            si->setData(item.index, indexRole);
            m_pChTabViewModel->setItem(item.postion, i, si);
            //ui->ch_tableView->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
            ui->ch_tableView->horizontalHeader()->setStretchLastSection(true);
        }
    }

    // 免疫
    int iHideRowCnt = 0;
    for (auto& item : m_functionManageData[IM_FLAG])
    {
        if (item.enable)
        {
            // 序号
            AddTextToTableView(m_pImTabViewModel, item.postion, COLUNM_NUM, QString::number(item.postion + 1 - iHideRowCnt))
            // 模块
            AddTextToTableView(m_pImTabViewModel, item.postion, COLUNM_NAME, m_pFunctionConfigDlg->GetStringByIndex(item.index))

            for (int i = 2; i < m_pImTabViewModel->columnCount(); i++)
            {
                auto si = new QStandardItem("");
                si->setData(item.index, indexRole);
                m_pImTabViewModel->setItem(item.postion, i, si);
                //ui->im_tableView->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
				ui->im_tableView->horizontalHeader()->setStretchLastSection(true);
            }

            // 隐藏气泡检测-清洗缓冲液管路、气泡检测-试剂管路、超声洗针
            if (item.index == IM_CONC_BULLER_SENSOR || item.index == IM_REAG_WASH_PIPETTOR_BULLER_SENSOR || item.index == IM_ULTRASONIC_WASH_PIPETTOR)
            {
                ui->im_tableView->hideRow(item.postion);
                ++iHideRowCnt;
            }
        }
    }

    LoadDataToCtrls();
}

int functionManageDlg::GetColunmByDevName(QColorHeaderModel* model, QString devName)
{
    for (int i = 2; i < model->columnCount(); i++)
    {
        if (model->headerData(i, Qt::Horizontal).toString() == devName)
        {
            return i;
        }
    }

    return -1;
}

void functionManageDlg::SetViewItemSelect(const ::tf::DeviceInfo& dev) 
{
    bool isCh = false;
    if (dev.deviceClassify == ::tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY
        || dev.deviceClassify == ::tf::AssayClassify::ASSAY_CLASSIFY_ISE)
    {
        isCh = true;
    } 

    // 获得所在列
    QColorHeaderModel* model = isCh ? m_pChTabViewModel : m_pImTabViewModel;
    int col = GetColunmByDevName(model, QString::fromStdString(dev.groupName + dev.name));
    if (col < 0)
    {
        ULOG(LOG_INFO, "GetColunmByDevName Failed Name:%s ", dev.groupName + dev.name);
        return;
    }

    for (int i=0; i<model->rowCount(); i++)
    {
        auto si = model->item(i, col);
        int index = si->data(indexRole).toInt();
        bool isExist = true;
        si->setData(GetStatusByIndex(dev, index, isExist), statusRole);
        si->setData(isExist ? "" : "#f6f6f6", VIEW_ITEM_DISABLE_ROLE);
    }
}

bool functionManageDlg::GetStatusByIndex(const ::tf::DeviceInfo& dev, int index, bool& isExist)
{
    if (dev.deviceClassify == ::tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY)
    {
        ChDeviceOtherInfo chdoi;
        if (!DecodeJson(chdoi, dev.otherInfo))
        {
            ULOG(LOG_ERROR, "DecodeJson ChDeviceOtherInfo Failed");
            return false;
        }

        switch (index)
        {
            case ChFunctionItemId::CH_CLOT_DETECTION:
                return chdoi.clotDetection.enable;
            case ChFunctionItemId::LOAD_REAGENT_ONLINE:
                isExist = chdoi.loadReagentOnline.exist;
                return chdoi.loadReagentOnline.enable && chdoi.loadReagentOnline.exist;
            case ChFunctionItemId::START_CUP_NO_CHANGED:
                return chdoi.startCupNoChanged;
            case ChFunctionItemId::INCUBATION_TP:
                return chdoi.incubationTp.enable;
            case ChFunctionItemId::ULTRASONIC_MIXED:
                return chdoi.ultrasonicMixed.enable;
            case ChFunctionItemId::REAGENT_PROBE_RUN:
                return chdoi.reagentProbeRun.enable;
            case ChFunctionItemId::SAMPLE_PROBE_RUN:
                return chdoi.sampleProbeRun;
            case ChFunctionItemId::PIPELINE_BUBBLES:
                return chdoi.pipelineBubbles.enable;
            case ChFunctionItemId::ELECTRONIC_COVER:
                return chdoi.electronicCover;
            case ChFunctionItemId::DEGAS_CHECK:
                return chdoi.degasCheck;
            case ChFunctionItemId::DEVICE_SLEEP:
                return chdoi.deviceSleepInfo.enable;
            case ChFunctionItemId::CLEAN_STATION:
                return chdoi.cleanStation;
            case ChFunctionItemId::MODULE_ENABLE:
                return chdoi.enable;
            default:
                return false;
        }

    }
    else if (dev.deviceClassify == ::tf::AssayClassify::ASSAY_CLASSIFY_ISE)
    {
        IseDeviceOtherInfo isedoi;
        if (!DecodeJson(isedoi, dev.otherInfo))
        {
            ULOG(LOG_ERROR, "DecodeJson IseDeviceOtherInfo Failed");
            return false;
        }

        switch (index)
        {
            case ChFunctionItemId::CH_CLOT_DETECTION:
                return isedoi.clotDetection.enable;
            case ChFunctionItemId::SAMPLE_PROBE_RUN:
                return isedoi.sampleProbeRun;
            case ChFunctionItemId::PIPELINE_BUBBLES:
                return isedoi.pipelineBubbles;
            case ChFunctionItemId::ELECTRONIC_COVER:
                return isedoi.electronicCover;
            case ChFunctionItemId::MODULE_ENABLE:
                return isedoi.enable;
            default:
                isExist = false;
                return false;
        }
    }
    else if (dev.deviceClassify == ::tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE)
    {
        ImDeviceOtherInfo imdoi;
        if (!DecodeJson(imdoi, dev.otherInfo))
        {
            ULOG(LOG_ERROR, "DecodeJson ImDeviceOtherInfo Failed");
            return false;
        }

        switch (index)
        {
            case ImFunctionItemId::IM_CLOT_DETECTION:
                return imdoi.bClotDetection;
            case ImFunctionItemId::IM_BACK_CLAMSHELL_FAN_DETECTION:
                return imdoi.bBackClamshellFanDetection;
            case ImFunctionItemId::IM_BACK_COVER_FAN_DETECTION:
                return imdoi.bBackCoverFanDetection;
            case ImFunctionItemId::IM_CONTROL_BOX_FAN_DETECTION:
                return imdoi.bControlBoxFanDetection;
            case ImFunctionItemId::IM_WATER_FAN_DETECTION:
                return imdoi.bWaterFanDetection;
            case ImFunctionItemId::IM_POWER_BOX_FAN_DETECTION:
                return imdoi.bPowerBoxFanDetection;
            case ImFunctionItemId::IM_SUB_BULLER_SENSOR:
                return imdoi.bSubBullerSensor;
            case ImFunctionItemId::IM_CONC_BULLER_SENSOR:
                return imdoi.bConcBullerSensor;
            case ImFunctionItemId::IM_REAG_WASH_PIPETTOR_BULLER_SENSOR:
                return imdoi.bReagWashPipettorBullerSensor;
			case ImFunctionItemId::IM_SUCK_SAMPLE_WASH_POOL_LEAKAGE_DETECTION:
				return imdoi.bSuckSampleWashPoolLeakageDetection;
			case ImFunctionItemId::IM_REAG_ARM_2ND_WASH_POOL_LEAKAGE_DETECTION:
				return imdoi.bArm2ndWashPoolLeakageDetection;
            case ImFunctionItemId::IM_MIDDLE_BORD_LEAKAGE_DETECTION:
                return imdoi.bMiddleBordLeakageDetection;
            case ImFunctionItemId::IM_FLUID_COMPONENT_LEAKAGE_DETECTION:
                return imdoi.bFluidComponentLeakageDetection;
            case ImFunctionItemId::IM_CONFECT_LEAKAGE_DETECTION:
                return imdoi.bConfectLeakageDetection;
            case ImFunctionItemId::IM_SAMPLE_TUPE_DETECTION:
                return imdoi.bSampleTupeDetection;
            case ImFunctionItemId::IM_PRESSURE_DETECTION:
                return imdoi.bPressureDetection;
            case ImFunctionItemId::IM_BACK_COVER_SWITCH_DETECTION:
                return imdoi.bBackCoverSwitchDetection;
			case ImFunctionItemId::IM_AUTO_LOAD_REAGENT:
				return imdoi.bAutoLoadReagent;
			case ImFunctionItemId::IM_BUZZER_VOICE:
				return imdoi.bBuzzerVoice;
			case ImFunctionItemId::IM_ULTRASONIC_WASH_PIPETTOR:
				return imdoi.bUltrasonicWashPipettor;
            default:
                return false;
        }
    }

    return false;
}

bool functionManageDlg::UpdateDeviceOtherInfo(QColorHeaderModel* model)
{
    for (int col = 2; col < model->columnCount(); col++)
    {
        // 获取设备信息
        std::string name = model->headerData(col, Qt::Horizontal).toString().toStdString();
        auto spDev = CommonInformationManager::GetInstance()->GetDeviceInfoByDeviceName(name);
        if (spDev == nullptr)
        {
            ULOG(LOG_ERROR, "GetDeviceInfoByDeviceName Failed Name:%s", name);
            return false;
        }

        // 生化
        if (spDev->deviceClassify == ::tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY)
        {
            if (!UpdateChDeviceOtherInfo(*spDev, col))
            {
                ULOG(LOG_ERROR, "UpdateChDeviceOtherInfo Failed");
                return false;
            }
        } 
        // ISE
        else if (spDev->deviceClassify == ::tf::AssayClassify::ASSAY_CLASSIFY_ISE)
        {
            if (!UpdateIseDeviceOtherInfo(*spDev, col))
            {
                ULOG(LOG_ERROR, "UpdateIseDeviceOtherInfo Failed");
                return false;
            }
        }

        // 免疫
        else if (spDev->deviceClassify == ::tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE)
        {
            if (!UpdateImDeviceOtherInfo(*spDev, col))
            {
                ULOG(LOG_ERROR, "UpdateImDeviceOtherInfo Failed");
                return false;
            }
        }
    }

    return true;
}

bool functionManageDlg::UpdateChDeviceOtherInfo(const ::tf::DeviceInfo& dev, int col)
{
    ChDeviceOtherInfo chdoi;
    if (!DecodeJson(chdoi, dev.otherInfo))
    {
        ULOG(LOG_ERROR, "DecodeJson ChDeviceOtherInfo Failed");
        return false;
    }

    // 获取索引
    for (int iRow = 0; iRow < m_pChTabViewModel->rowCount(); iRow++)
    {
        int index = m_pChTabViewModel->item(iRow, col)->data(indexRole).toInt();
        bool status = m_pChTabViewModel->item(iRow, col)->data(statusRole).toBool();

        // 按索引修改状态
        switch (index)
        {
            case ChFunctionItemId::CH_CLOT_DETECTION:
                chdoi.clotDetection.enable = status;
                break;
            case ChFunctionItemId::LOAD_REAGENT_ONLINE:
                chdoi.loadReagentOnline.enable = status;
                break;
            case ChFunctionItemId::START_CUP_NO_CHANGED:
                chdoi.startCupNoChanged = status;
                break;
            case ChFunctionItemId::INCUBATION_TP:
                chdoi.incubationTp.enable = status;
                break;
            case ChFunctionItemId::ULTRASONIC_MIXED:
                chdoi.ultrasonicMixed.enable = status;
                break;
            case ChFunctionItemId::REAGENT_PROBE_RUN:
                chdoi.reagentProbeRun.enable = status;
                break;
            case ChFunctionItemId::SAMPLE_PROBE_RUN:
                chdoi.sampleProbeRun = status;
                break;
            case ChFunctionItemId::PIPELINE_BUBBLES:
                chdoi.pipelineBubbles.enable = status;
                break;
            case ChFunctionItemId::ELECTRONIC_COVER:
                chdoi.electronicCover = status;
                break;
            case ChFunctionItemId::DEGAS_CHECK:
                chdoi.degasCheck = status;
                break;
            case ChFunctionItemId::DEVICE_SLEEP:
                chdoi.deviceSleepInfo.enable = status;
                break;
            case ChFunctionItemId::CLEAN_STATION:
                chdoi.cleanStation = status;
                break;
            case ChFunctionItemId::MODULE_ENABLE:
                chdoi.enable = status;
            default:
                break;
        }
    }

    std::string xml;
    if (!Encode2Json(xml,chdoi))
    {
        ULOG(LOG_ERROR, "Encode2Json ChDeviceOtherInfo Failed");
        return false;
    }

    ::tf::DeviceInfo newDev;
    newDev.__set_deviceSN(dev.deviceSN);
    newDev.__set_otherInfo(xml);

    if (!CommonInformationManager::GetInstance()->ModifyDeviceInfo(newDev))
    {
        ULOG(LOG_ERROR, "ModifyDeviceInfo Failed");
        return false;
    }

    return true;
}

bool functionManageDlg::UpdateIseDeviceOtherInfo(const ::tf::DeviceInfo& dev, int col)
{
    IseDeviceOtherInfo isedoi;
    if (!DecodeJson(isedoi, dev.otherInfo))
    {
        ULOG(LOG_ERROR, "DecodeJson IseDeviceOtherInfo Failed");
        return false;
    }

    // 获取索引
    for (int iRow = 0; iRow < m_pChTabViewModel->rowCount(); iRow++)
    {
        int index = m_pChTabViewModel->item(iRow, col)->data(indexRole).toInt();
        bool status = m_pChTabViewModel->item(iRow, col)->data(statusRole).toBool();

        // 按索引修改状态
        switch (index)
        {
            case ChFunctionItemId::CH_CLOT_DETECTION:
                isedoi.clotDetection.enable = status;
                break;
            case ChFunctionItemId::PIPELINE_BUBBLES:
                isedoi.pipelineBubbles= status;
                break;
            case ChFunctionItemId::SAMPLE_PROBE_RUN:
                isedoi.sampleProbeRun = status;
                break;
            case ChFunctionItemId::ELECTRONIC_COVER:
                isedoi.electronicCover = status;
                break;
            case ChFunctionItemId::MODULE_ENABLE:
                isedoi.enable = status;
                break;
            default:
                break;
        }
    }

    std::string xml;
    if (!Encode2Json(xml, isedoi))
    {
        ULOG(LOG_ERROR, "Encode2Json IseDeviceOtherInfo Failed");
        return false;
    }

    ::tf::DeviceInfo newDev;
    newDev.__set_deviceSN(dev.deviceSN);
    newDev.__set_otherInfo(xml);

    if (!CommonInformationManager::GetInstance()->ModifyDeviceInfo(newDev))
    {
        ULOG(LOG_ERROR, "ModifyDeviceInfo Failed");
        return false;
    }

    return true;
}

bool functionManageDlg::UpdateImDeviceOtherInfo(const ::tf::DeviceInfo& dev, int col)
{
    ImDeviceOtherInfo imdoi;
    if (!DecodeJson(imdoi, dev.otherInfo))
    {
        ULOG(LOG_ERROR, "DecodeJson ImDeviceOtherInfo Failed");
        return false;
    }

    // 获取索引
    for (int iRow = 0; iRow < m_pImTabViewModel->rowCount(); iRow++)
    {
        int index = m_pImTabViewModel->item(iRow, col)->data(indexRole).toInt();
        bool status = m_pImTabViewModel->item(iRow, col)->data(statusRole).toBool();

        // 按索引修改状态
        switch (index)
        {
             case ImFunctionItemId::IM_CLOT_DETECTION:
                imdoi.bClotDetection = status;
				break;
            case ImFunctionItemId::IM_BACK_CLAMSHELL_FAN_DETECTION:
                imdoi.bBackClamshellFanDetection = status;
				break;
            case ImFunctionItemId::IM_BACK_COVER_FAN_DETECTION:
                imdoi.bBackCoverFanDetection = status;
				break;
            case ImFunctionItemId::IM_CONTROL_BOX_FAN_DETECTION:
                imdoi.bControlBoxFanDetection = status;
				break;
            case ImFunctionItemId::IM_WATER_FAN_DETECTION:
                imdoi.bWaterFanDetection = status;
				break;
            case ImFunctionItemId::IM_POWER_BOX_FAN_DETECTION:
                imdoi.bPowerBoxFanDetection = status;
				break;
            case ImFunctionItemId::IM_SUB_BULLER_SENSOR:
                imdoi.bSubBullerSensor = status;
				break;
            case ImFunctionItemId::IM_CONC_BULLER_SENSOR:
                imdoi.bConcBullerSensor = status;
				break;
            case ImFunctionItemId::IM_REAG_WASH_PIPETTOR_BULLER_SENSOR:
                imdoi.bReagWashPipettorBullerSensor = status;
				break;
			case ImFunctionItemId::IM_SUCK_SAMPLE_WASH_POOL_LEAKAGE_DETECTION:
				imdoi.bSuckSampleWashPoolLeakageDetection = status;
				break;
			case ImFunctionItemId::IM_REAG_ARM_2ND_WASH_POOL_LEAKAGE_DETECTION:
				imdoi.bArm2ndWashPoolLeakageDetection = status;
				break;
            case ImFunctionItemId::IM_MIDDLE_BORD_LEAKAGE_DETECTION:
                imdoi.bMiddleBordLeakageDetection = status;
				break;
            case ImFunctionItemId::IM_FLUID_COMPONENT_LEAKAGE_DETECTION:
                imdoi.bFluidComponentLeakageDetection = status;
				break;
            case ImFunctionItemId::IM_CONFECT_LEAKAGE_DETECTION:
                imdoi.bConfectLeakageDetection = status;
				break;
            case ImFunctionItemId::IM_SAMPLE_TUPE_DETECTION:
                imdoi.bSampleTupeDetection = status;
				break;
            case ImFunctionItemId::IM_PRESSURE_DETECTION:
                imdoi.bPressureDetection = status;
				break;
            case ImFunctionItemId::IM_BACK_COVER_SWITCH_DETECTION:
                imdoi.bBackCoverSwitchDetection = status;
				break;
			case ImFunctionItemId::IM_AUTO_LOAD_REAGENT:
				imdoi.bAutoLoadReagent = status;
				break;
			case ImFunctionItemId::IM_BUZZER_VOICE:
				imdoi.bBuzzerVoice = status;
				break;
			case ImFunctionItemId::IM_ULTRASONIC_WASH_PIPETTOR:
				imdoi.bUltrasonicWashPipettor = status;
				break;
            default:
                break;
        }
    }

    std::string xml;
    if (!Encode2Json(xml, imdoi))
    {
        ULOG(LOG_ERROR, "Encode2Json ImDeviceOtherInfo Failed");
        return false;
    }

    ::tf::DeviceInfo newDev;
    newDev.__set_deviceSN(dev.deviceSN);
    newDev.__set_otherInfo(xml);

    if (!CommonInformationManager::GetInstance()->ModifyDeviceInfo(newDev))
    {
        ULOG(LOG_ERROR, "ModifyDeviceInfo Failed");
        return false;
    }

    return true;
}

void functionManageDlg::OnSaveBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    const auto& CIM = CommonInformationManager::GetInstance();

    // 设备运行合维护中禁止修改参数
    if (CIM->IsExistDeviceRuning() || CIM->HasDeviceEqualStatus(::tf::DeviceWorkState::DEVICE_STATUS_MAINTAIN))
    {
        TipDlg(tr("设备运行或维护中禁止修改参数")).exec();
        LoadDataToCtrls();
        return;
    }

    // 生化
    if (!UpdateDeviceOtherInfo(m_pChTabViewModel))
    {
        ULOG(LOG_ERROR, "UpdateDeviceOtherInfo Failed !");
        TipDlg(tr("保存失败"), tr("保存失败")).exec();
        return;
    }

    // 免疫
    if (!UpdateDeviceOtherInfo(m_pImTabViewModel))
    {
        ULOG(LOG_ERROR, "UpdateDeviceOtherInfo Failed !");
        TipDlg(tr("保存失败"), tr("保存失败")).exec();
        return;
    }

    // 通知设置更新
    POST_MESSAGE(MSG_ID_UTILITY_FUNCTION_MNG_UPDATE);

    close();
}

void functionManageDlg::OnFunctionConfigSave()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    auto data = m_pFunctionConfigDlg->GetData();

    // 最少勾选一行
    if (data.size() < 1)
    {
        TipDlg(tr("保存失败"), tr("最少选择一个功能")).exec();
        return;
    }

    std::string flag;
    if (ui->tabWidget->currentWidget()->objectName().contains(CH_FLAG))
    {
        flag = CH_FLAG;
    } 
    else
    {
        flag = IM_FLAG;
    }

    // 更新缓存
    for (auto&item : m_functionManageData[flag])
    {
        auto iter = find_if(data.begin(), data.end(), [&](FunctionManageItem& fmi)
        {
            return fmi.index == item.index;
        });

        if (iter == data.end())
        {
            item.enable = false;
        }
        else
        {
            item = *(iter);
        }
    }

    // 保存显示设置
    if (!DictionaryQueryManager::SaveFunctionManageConfig(m_functionManageData))
    {
        ULOG(LOG_ERROR, "Failed to save function manage config!");
        TipDlg(tr("保存失败"), tr("保存失败")).exec();
        return;
    }

    m_pFunctionConfigDlg->close();

    UpdateRowDataToView();

    if (ui->tabWidget->currentWidget()->objectName().endsWith("ch"))
    {
        ui->ch_tableView->selectRow(0);
    }
}

void functionManageDlg::OnFunctionSetBtnClicked()
{
    const auto& CIM = CommonInformationManager::GetInstance();

    // 设备运行合维护中禁止修改参数
    if (CIM->IsExistDeviceRuning() || CIM->HasDeviceEqualStatus(::tf::DeviceWorkState::DEVICE_STATUS_MAINTAIN))
    {
        TipDlg(tr("设备运行或维护中禁止修改参数")).exec();
        return;
    }

     if (ui->tabWidget->currentWidget()->objectName().contains(CH_FLAG))
     {
         int curRow = ui->ch_tableView->currentIndex().row();
         int index = m_pChTabViewModel->item(curRow,2)->data(indexRole).toInt();
 
         // 显示功能设置弹窗
         m_pFunctionSetDlg->ShowDataToCtrls(m_pFunctionConfigDlg->GetStringByIndex(index), index);
     }
}

void functionManageDlg::OnCurrentRowChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(selected);
    Q_UNUSED(deselected);

    if (ui->tabWidget->currentWidget()->objectName().contains(CH_FLAG))
    {
        auto item = m_pChTabViewModel->item(ui->ch_tableView->currentIndex().row(), 2);
        if (item == nullptr)
        {
            return;
        }

        int index = item->data(indexRole).toInt();
        ui->function_set_btn->setEnabled(index == ChFunctionItemId::CH_CLOT_DETECTION
            || index == ChFunctionItemId::REAGENT_PROBE_RUN || index == ChFunctionItemId::ULTRASONIC_MIXED
            || index == ChFunctionItemId::INCUBATION_TP || index == ChFunctionItemId::PIPELINE_BUBBLES
            || index == ChFunctionItemId::DEVICE_SLEEP);
    }
}

void functionManageDlg::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    if (UserInfoManager::GetInstance()->IsPermisson(PSM_FUNCTION_MANAGE))
    {
        ui->ch_tableView->setRowHidden(START_CUP_NO_CHANGED, false);
        ui->ch_tableView->setRowHidden(INCUBATION_TP, false);
        ui->ch_tableView->setRowHidden(ULTRASONIC_MIXED, false);
        ui->ch_tableView->setRowHidden(REAGENT_PROBE_RUN, false);
        ui->ch_tableView->setRowHidden(SAMPLE_PROBE_RUN, false);
        ui->ch_tableView->setRowHidden(PIPELINE_BUBBLES, false);
        ui->ch_tableView->setRowHidden(ELECTRONIC_COVER, false);
        ui->ch_tableView->setRowHidden(DEGAS_CHECK, false);
        ui->ch_tableView->setRowHidden(DEVICE_SLEEP, false);
        ui->ch_tableView->setRowHidden(CLEAN_STATION, false);
        ui->ch_tableView->setRowHidden(MODULE_ENABLE, false);
    }
    else
    {
        ui->ch_tableView->setRowHidden(START_CUP_NO_CHANGED, true);
        ui->ch_tableView->setRowHidden(INCUBATION_TP, true);
        ui->ch_tableView->setRowHidden(ULTRASONIC_MIXED, true);
        ui->ch_tableView->setRowHidden(REAGENT_PROBE_RUN, true);
        ui->ch_tableView->setRowHidden(SAMPLE_PROBE_RUN, true);
        ui->ch_tableView->setRowHidden(PIPELINE_BUBBLES, true);
        ui->ch_tableView->setRowHidden(ELECTRONIC_COVER, true);
        ui->ch_tableView->setRowHidden(DEGAS_CHECK, true);
        ui->ch_tableView->setRowHidden(DEVICE_SLEEP, true);
        ui->ch_tableView->setRowHidden(CLEAN_STATION, true);
        ui->ch_tableView->setRowHidden(MODULE_ENABLE, true);
    }
}
