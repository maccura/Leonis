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
/// @file     functionSetDlg.cpp
/// @brief    应用--仪器--功能设置
///
/// @author   7951/LuoXin
/// @date     2023年1月4日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2024年1月4日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "functionSetDlg.h"
#include "ui_functionSetDlg.h"
#include "src/common/common.h"
#include "src/common/Mlog/mlog.h"
#include "src/public/SerializeUtil.hpp"
#include "src/public/ConfigSerialize.h"
#include "src/public/ch/ChConfigSerialize.h"
#include "src/public/ise/IseConfigSerialize.h"
#include "shared/tipdlg.h"
#include "shared/CommonInformationManager.h"

functionSetDlg::functionSetDlg(QWidget *parent /*= Q_NULLPTR*/)
    : BaseDlg(parent)
    , ui(new Ui::functionSetDlg)
{
    ui->setupUi(this);

    // 设置标题
    SetTitleName(tr("功能设置"));

    ui->lineEdit->setValidator(new QIntValidator(1, 999999, ui->lineEdit));
    connect(ui->save_btn, &QPushButton::clicked, this, &functionSetDlg::OnSaveBtnClicked);
}

functionSetDlg::~functionSetDlg()
{

}

void functionSetDlg::ShowDataToCtrls(QString name, int index)
{
    // 设置标题
    SetTitleName(name);

    ui->clot_sample_stop->setVisible(index ==CH_CLOT_DETECTION);
    ui->reagent_r1->setVisible(index == REAGENT_PROBE_RUN);
    ui->reagent_r2->setVisible(index == REAGENT_PROBE_RUN);
    ui->ultrasonic_r1->setVisible(index == ULTRASONIC_MIXED);
    ui->ultrasonic_r2->setVisible(index == ULTRASONIC_MIXED);
    ui->incubation_stop->setVisible(index == INCUBATION_TP);
    ui->bubbles_acidity->setVisible(index == PIPELINE_BUBBLES);
    ui->bubbles_acidity_back->setVisible(index == PIPELINE_BUBBLES);
    ui->bubbles_alkalinity->setVisible(index == PIPELINE_BUBBLES);
    ui->bubbles_alkalinity_back->setVisible(index == PIPELINE_BUBBLES);
    ui->bubbles_sample_probe_s->setVisible(index == PIPELINE_BUBBLES);
    ui->bubbles_reagent_probe_r1->setVisible(index == PIPELINE_BUBBLES);
    ui->bubbles_reagent_probe_r2->setVisible(index == PIPELINE_BUBBLES);
    ui->time_widget->setVisible(index == DEVICE_SLEEP);

    auto devs = CommonInformationManager::GetInstance()->GetDeviceFromType({ ::tf::DeviceType::DEVICE_TYPE_C1000 });
    if (devs.empty())
    {
        ULOG(LOG_ERROR, "Get 1005DeviceInfo Failed!");
        return;
    }

    ChDeviceOtherInfo chDoi;
    if (!DecodeJson(chDoi, devs[0]->otherInfo))
    {
        ULOG(LOG_ERROR, "DecodeJson 1005DeviceInfo OtherInfo Failed!");
        return;
    }

    ui->clot_sample_stop->setChecked(chDoi.clotDetection.enableSampleStop);
    ui->reagent_r1->setChecked(chDoi.reagentProbeRun.enableR1);
    ui->reagent_r2->setChecked(chDoi.reagentProbeRun.enableR2);
    ui->ultrasonic_r1->setChecked(chDoi.ultrasonicMixed.enableR1);
    ui->ultrasonic_r2->setChecked(chDoi.ultrasonicMixed.enableR2);
    ui->incubation_stop->setChecked(chDoi.incubationTp.enableSampleStopAndStop);
    ui->bubbles_acidity->setChecked(chDoi.pipelineBubbles.enableAcidity);
    ui->bubbles_acidity_back->setChecked(chDoi.pipelineBubbles.enableAcidityBack);
    ui->bubbles_alkalinity->setChecked(chDoi.pipelineBubbles.enableAlkalinity);
    ui->bubbles_alkalinity_back->setChecked(chDoi.pipelineBubbles.enableAlkalinityBack);
    ui->bubbles_sample_probe_s->setChecked(chDoi.pipelineBubbles.enableSampleProbeS);
    ui->bubbles_reagent_probe_r1->setChecked(chDoi.pipelineBubbles.enableReagentProbeR1);
    ui->bubbles_reagent_probe_r2->setChecked(chDoi.pipelineBubbles.enableReagentProbeR2);
    ui->lineEdit->setText(QString::number(chDoi.deviceSleepInfo.time));
    ui->lower_time_edit->setTime(QTime::fromString(QString::fromStdString(chDoi.deviceSleepInfo.unSleepTimeStart), "HH:mm:ss"));
    ui->upper_time_edit->setTime(QTime::fromString(QString::fromStdString(chDoi.deviceSleepInfo.unSleepTimeEnd), "HH:mm:ss"));

    int showBoxCount = 0;
    switch (index)
    {
        case CH_CLOT_DETECTION:
        case INCUBATION_TP:
        case DEVICE_SLEEP:
            showBoxCount = 1;
            break;
        case ULTRASONIC_MIXED:
        case REAGENT_PROBE_RUN:
            showBoxCount = 2;
            break;
        case PIPELINE_BUBBLES:
            showBoxCount = 7;
            break;
        default:
            break;
    }

    // 重新设置窗口大小
    int hideHeight = (ui->clot_sample_stop->height() + ui->verticalLayout->spacing())*showBoxCount;

    if (index == DEVICE_SLEEP)
    {
        hideHeight = ui->time_widget->height();
    }

    setFixedHeight(ui->box_widget->y() + hideHeight + ui->btn_widget->height());

    show();
}

void functionSetDlg::OnSaveBtnClicked()
{
    if (ui->lineEdit->text().toInt() <= 0)
    {
        TipDlg(tr("不休眠时间必须大于0 ！")).exec();
        return;
    }

    if (ui->upper_time_edit->time() <= ui->lower_time_edit->time())
    {
        TipDlg(tr("不休眠时间限的上限必须大于下限")).exec();
        return;
    }

    auto CIM = CommonInformationManager::GetInstance();
    auto vecDev = CIM->GetDeviceFromType({ ::tf::DeviceType::DEVICE_TYPE_C1000, ::tf::DeviceType::DEVICE_TYPE_ISE1005 });

    for (auto dev : vecDev)
    {
        std::string xml;
        if (dev->deviceType == ::tf::DeviceType::DEVICE_TYPE_C1000)
        {
            ChDeviceOtherInfo chDoi;
            if (!DecodeJson(chDoi, dev->otherInfo))
            {
                ULOG(LOG_ERROR, "DecodeJson ch1005DeviceInfo OtherInfo Failed!");
                return;
            }

            chDoi.clotDetection.enableSampleStop = ui->clot_sample_stop->isChecked();
            chDoi.reagentProbeRun.enableR1 = ui->reagent_r1->isChecked();
            chDoi.reagentProbeRun.enableR2 = ui->reagent_r1->isChecked();
            chDoi.ultrasonicMixed.enableR1 = ui->ultrasonic_r1->isChecked();
            chDoi.ultrasonicMixed.enableR2 = ui->ultrasonic_r2->isChecked();
            chDoi.incubationTp.enableSampleStopAndStop = ui->incubation_stop->isChecked();
            chDoi.pipelineBubbles.enableAcidity = ui->bubbles_acidity->isChecked();
            chDoi.pipelineBubbles.enableAcidityBack = ui->bubbles_acidity_back->isChecked();
            chDoi.pipelineBubbles.enableAlkalinity = ui->bubbles_alkalinity->isChecked();
            chDoi.pipelineBubbles.enableAlkalinityBack = ui->bubbles_acidity_back->isChecked();
            chDoi.pipelineBubbles.enableSampleProbeS = ui->bubbles_sample_probe_s->isChecked();
            chDoi.pipelineBubbles.enableReagentProbeR1 = ui->bubbles_reagent_probe_r1->isChecked();
            chDoi.pipelineBubbles.enableReagentProbeR2 = ui->bubbles_reagent_probe_r2->isChecked();
            chDoi.deviceSleepInfo.time = ui->lineEdit->text().toInt();
            chDoi.deviceSleepInfo.unSleepTimeStart = ui->lower_time_edit->time().toString().toStdString();
            chDoi.deviceSleepInfo.unSleepTimeEnd = ui->upper_time_edit->time().toString().toStdString();

            if (!Encode2Json(xml,chDoi))
            {
                TipDlg(tr("保存功能配置失败！")).exec();
                ULOG(LOG_ERROR, "Encode2Json ch1005DeviceInfo OtherInfo Failed!");
                return;
            }
        }
        else
        {
            IseDeviceOtherInfo iseDoi;
            if (!DecodeJson(iseDoi, dev->otherInfo))
            {
                TipDlg(tr("保存功能配置失败！")).exec();
                ULOG(LOG_ERROR, "DecodeJson ise1005DeviceInfo OtherInfo Failed!");
                return;
            }

            iseDoi.clotDetection.enableSampleStop = ui->clot_sample_stop->isChecked();
            iseDoi.pipelineBubbles = ui->bubbles_sample_probe_s->isChecked();

            if (!Encode2Json(xml, iseDoi))
            {
                TipDlg(tr("保存功能配置失败！")).exec();
                ULOG(LOG_ERROR, "Encode2Json ise1005DeviceInfo OtherInfo Failed!");
                return;
            }
        }

        ::tf::DeviceInfo di;
        di.__set_deviceSN(dev->deviceSN);
        di.__set_otherInfo(xml);
        if (!CIM->ModifyDeviceInfo(di))
        {
            TipDlg(tr("保存功能配置失败！")).exec();
            ULOG(LOG_ERROR, "ModifyDeviceInfo ise1005DeviceInfo OtherInfo Failed!");
            return;
        }
    }

    close();
}
