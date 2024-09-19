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
/// @file     AiAnalysisWgt.cpp
/// @brief    应用->AI识别设置界面
///
/// @author   4170/TangChuXian
/// @date     2023年11月30日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2023年11月30日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "AiAnalysisWgt.h"
#include "ui_AiAnalysisWgt.h"

#include "shared/tipdlg.h"
#include "shared/uicommon.h"
#include "shared/msgiddef.h"
#include "shared/messagebus.h"
#include "manager/UserInfoManager.h"
#include "manager/DictionaryQueryManager.h"
#include "src/common/Mlog/mlog.h"

AiAnalysisWgt::AiAnalysisWgt(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::AiAnalysisWgt),
      m_bInit(false)
{
    ui->setupUi(this);
    Init();
}

AiAnalysisWgt::~AiAnalysisWgt()
{

}

void AiAnalysisWgt::Init()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 连接信号槽
    connect(ui->flat_save, SIGNAL(clicked()), this, SLOT(OnSaveBtnClicked()));

    // 加载下拉框(≥)
    AddTfEnumItemToComBoBox(ui->XueQingNtcCmpCb, tf::CompareSymbol::COMPARE_SYMBOL_LESS_THAN_OR_EQUAL);
    AddTfEnumItemToComBoBox(ui->XueQingNtcCmpCb, tf::CompareSymbol::COMPARE_SYMBOL_MORE_THAN_OR_EQUAL);
    AddTfEnumItemToComBoBox(ui->ZhiXueNtcCmpCb, tf::CompareSymbol::COMPARE_SYMBOL_LESS_THAN_OR_EQUAL);
    AddTfEnumItemToComBoBox(ui->ZhiXueNtcCmpCb, tf::CompareSymbol::COMPARE_SYMBOL_MORE_THAN_OR_EQUAL);
    AddTfEnumItemToComBoBox(ui->RongXueNtcCmpCb, tf::CompareSymbol::COMPARE_SYMBOL_LESS_THAN_OR_EQUAL);
    AddTfEnumItemToComBoBox(ui->RongXueNtcCmpCb, tf::CompareSymbol::COMPARE_SYMBOL_MORE_THAN_OR_EQUAL);
    AddTfEnumItemToComBoBox(ui->HuangdanNtcCmpCb, tf::CompareSymbol::COMPARE_SYMBOL_LESS_THAN_OR_EQUAL);
    AddTfEnumItemToComBoBox(ui->HuangdanNtcCmpCb, tf::CompareSymbol::COMPARE_SYMBOL_MORE_THAN_OR_EQUAL);
    AddTfEnumItemToComBoBox(ui->XueqingAbnCmpCb, tf::CompareSymbol::COMPARE_SYMBOL_LESS_THAN_OR_EQUAL);
    AddTfEnumItemToComBoBox(ui->XueqingAbnCmpCb, tf::CompareSymbol::COMPARE_SYMBOL_MORE_THAN_OR_EQUAL);
    AddTfEnumItemToComBoBox(ui->ZhixueAbnCmpCb, tf::CompareSymbol::COMPARE_SYMBOL_LESS_THAN_OR_EQUAL);
    AddTfEnumItemToComBoBox(ui->ZhixueAbnCmpCb, tf::CompareSymbol::COMPARE_SYMBOL_MORE_THAN_OR_EQUAL);
    AddTfEnumItemToComBoBox(ui->RongxueAbnCmpCb, tf::CompareSymbol::COMPARE_SYMBOL_LESS_THAN_OR_EQUAL);
    AddTfEnumItemToComBoBox(ui->RongxueAbnCmpCb, tf::CompareSymbol::COMPARE_SYMBOL_MORE_THAN_OR_EQUAL);
    AddTfEnumItemToComBoBox(ui->HuangdanAbnCmpCb, tf::CompareSymbol::COMPARE_SYMBOL_LESS_THAN_OR_EQUAL);
    AddTfEnumItemToComBoBox(ui->HuangdanAbnCmpCb, tf::CompareSymbol::COMPARE_SYMBOL_MORE_THAN_OR_EQUAL);

    // 加载下拉框（-、+、++）
    AddTfEnumItemToComBoBox(ui->ZhiXueNtcRangeCb, tf::RangeSymbol::RANGE_SYMBOL_LESS_THAN_1);
    AddTfEnumItemToComBoBox(ui->ZhiXueNtcRangeCb, tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_1);
    AddTfEnumItemToComBoBox(ui->ZhiXueNtcRangeCb, tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_2);
    AddTfEnumItemToComBoBox(ui->ZhiXueNtcRangeCb, tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_3);
    AddTfEnumItemToComBoBox(ui->ZhiXueNtcRangeCb, tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_4);
    AddTfEnumItemToComBoBox(ui->ZhiXueNtcRangeCb, tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_5);
    AddTfEnumItemToComBoBox(ui->RongXueNtcRangeCb, tf::RangeSymbol::RANGE_SYMBOL_LESS_THAN_1);
    AddTfEnumItemToComBoBox(ui->RongXueNtcRangeCb, tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_1);
    AddTfEnumItemToComBoBox(ui->RongXueNtcRangeCb, tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_2);
    AddTfEnumItemToComBoBox(ui->RongXueNtcRangeCb, tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_3);
    AddTfEnumItemToComBoBox(ui->RongXueNtcRangeCb, tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_4);
    AddTfEnumItemToComBoBox(ui->RongXueNtcRangeCb, tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_5);
    AddTfEnumItemToComBoBox(ui->HuangdanNtcRangeCb, tf::RangeSymbol::RANGE_SYMBOL_LESS_THAN_1);
    AddTfEnumItemToComBoBox(ui->HuangdanNtcRangeCb, tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_1);
    AddTfEnumItemToComBoBox(ui->HuangdanNtcRangeCb, tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_2);
    AddTfEnumItemToComBoBox(ui->HuangdanNtcRangeCb, tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_3);
    AddTfEnumItemToComBoBox(ui->HuangdanNtcRangeCb, tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_4);
    AddTfEnumItemToComBoBox(ui->HuangdanNtcRangeCb, tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_5);
    AddTfEnumItemToComBoBox(ui->ZhixueAbnRangeCb, tf::RangeSymbol::RANGE_SYMBOL_LESS_THAN_1);
    AddTfEnumItemToComBoBox(ui->ZhixueAbnRangeCb, tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_1);
    AddTfEnumItemToComBoBox(ui->ZhixueAbnRangeCb, tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_2);
    AddTfEnumItemToComBoBox(ui->ZhixueAbnRangeCb, tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_3);
    AddTfEnumItemToComBoBox(ui->ZhixueAbnRangeCb, tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_4);
    AddTfEnumItemToComBoBox(ui->ZhixueAbnRangeCb, tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_5);
    AddTfEnumItemToComBoBox(ui->RongxueAbnRangeCb, tf::RangeSymbol::RANGE_SYMBOL_LESS_THAN_1);
    AddTfEnumItemToComBoBox(ui->RongxueAbnRangeCb, tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_1);
    AddTfEnumItemToComBoBox(ui->RongxueAbnRangeCb, tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_2);
    AddTfEnumItemToComBoBox(ui->RongxueAbnRangeCb, tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_3);
    AddTfEnumItemToComBoBox(ui->RongxueAbnRangeCb, tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_4);
    AddTfEnumItemToComBoBox(ui->RongxueAbnRangeCb, tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_5);
    AddTfEnumItemToComBoBox(ui->HuangdanAbnRangeCb, tf::RangeSymbol::RANGE_SYMBOL_LESS_THAN_1);
    AddTfEnumItemToComBoBox(ui->HuangdanAbnRangeCb, tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_1);
    AddTfEnumItemToComBoBox(ui->HuangdanAbnRangeCb, tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_2);
    AddTfEnumItemToComBoBox(ui->HuangdanAbnRangeCb, tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_3);
    AddTfEnumItemToComBoBox(ui->HuangdanAbnRangeCb, tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_4);
    AddTfEnumItemToComBoBox(ui->HuangdanAbnRangeCb, tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_5);

//     connect(ui->print_btn, &QPushButton::clicked, this, &AiAnalysisWgt::OnPrintBtnClicked);
//     connect(ui->export_btn, &QPushButton::clicked, this, &AiAnalysisWgt::OnExportBtnClicked);
//     connect(ui->tabWidget, &QTabWidget::currentChanged, this, &AiAnalysisWgt::OnTabWidgetChangePage);

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);
}

///
/// @brief
///     清空数据
///
/// @par History:
/// @li 4170/TangChuXian，2023年12月26日，新建函数
///
void AiAnalysisWgt::ClearData()
{
    // 将数据填到控件中
    const int ciRangeFlagSize = 6;
    ui->AiSwitchCB->setChecked(false);
    ui->TransCB->setChecked(false);
    ui->NoticeCB->setChecked(false);
    ui->AlarmCB->setChecked(false);

    // 黄疸范围
    {
        ui->MarkLowertHgEdit->clear();
        ui->MarkLowertHgEdit_1->clear();
        ui->MarkLowertHgEdit_2->clear();
        ui->MarkLowertHgEdit_3->clear();
        ui->MarkLowertHgEdit_4->clear();
        ui->MarkLowertHgEdit_5->clear();

        ui->MarkUppertHgEdit->clear();
        ui->MarkUppertHgEdit_1->clear();
        ui->MarkUppertHgEdit_2->clear();
        ui->MarkUppertHgEdit_3->clear();
        ui->MarkUppertHgEdit_4->clear();
        ui->MarkUppertHgEdit_5->clear();
    }

    // 脂血范围
    {
        ui->MarkLowertZxEdit->clear();
        ui->MarkLowertZxEdit_1->clear();
        ui->MarkLowertZxEdit_2->clear();
        ui->MarkLowertZxEdit_3->clear();
        ui->MarkLowertZxEdit_4->clear();
        ui->MarkLowertZxEdit_5->clear();

        ui->MarkUppertZxEdit->clear();
        ui->MarkUppertZxEdit_1->clear();
        ui->MarkUppertZxEdit_2->clear();
        ui->MarkUppertZxEdit_3->clear();
        ui->MarkUppertZxEdit_4->clear();
        ui->MarkUppertZxEdit_5->clear();
    }

    // 溶血范围
    {
        ui->MarkLowertRxEdit->clear();
        ui->MarkLowertRxEdit_1->clear();
        ui->MarkLowertRxEdit_2->clear();
        ui->MarkLowertRxEdit_3->clear();
        ui->MarkLowertRxEdit_4->clear();
        ui->MarkLowertRxEdit_5->clear();

        ui->MarkUppertRxEdit->clear();
        ui->MarkUppertRxEdit_1->clear();
        ui->MarkUppertRxEdit_2->clear();
        ui->MarkUppertRxEdit_3->clear();
        ui->MarkUppertRxEdit_4->clear();
        ui->MarkUppertRxEdit_5->clear();
    }

    // 提示设置
    ui->XueQingNtcCmpCb->setCurrentIndex(-1);
    ui->XueQingNtcRangeEdit->clear();
    ui->ZhiXueNtcCmpCb->setCurrentIndex(-1);
    ui->ZhiXueNtcRangeCb->setCurrentIndex(-1);
    ui->RongXueNtcCmpCb->setCurrentIndex(-1);
    ui->RongXueNtcRangeCb->setCurrentIndex(-1);
    ui->HuangdanNtcCmpCb->setCurrentIndex(-1);
    ui->HuangdanNtcRangeCb->setCurrentIndex(-1);

    ui->XueqingAbnCmpCb->setCurrentIndex(-1);
    ui->XueqingAbnRangeEdit->clear();
    ui->ZhixueAbnCmpCb->setCurrentIndex(-1);
    ui->ZhixueAbnRangeCb->setCurrentIndex(-1);
    ui->RongxueAbnCmpCb->setCurrentIndex(-1);
    ui->RongxueAbnRangeCb->setCurrentIndex(-1);
    ui->HuangdanAbnCmpCb->setCurrentIndex(-1);
    ui->HuangdanAbnRangeCb->setCurrentIndex(-1);
}

///
/// @brief
///     加载数据
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月3日，新建函数
///
void AiAnalysisWgt::LoadData()
{
    // 清空数据
    ClearData();

    AiAnalysisCfg data;
    if (!DictionaryQueryManager::GetAiAnalysisConfig(data))
    {
        ULOG(LOG_ERROR, "Failed to get ai analysis config.");
        return;
    }

    // 将数据填到控件中
    const int ciRangeFlagSize = 6;
    ui->AiSwitchCB->setChecked(data.bON);
    ui->TransCB->setChecked(data.bUploadImage);
    ui->NoticeCB->setChecked(data.bNoticeEnable);
    ui->AlarmCB->setChecked(data.bAlarmEnable);

    // 黄疸范围
    if (data.vecHuangdanRange.size() >= ciRangeFlagSize)
    {
        ui->MarkLowertHgEdit->setText(QString::fromStdString(data.vecHuangdanRange[tf::RangeSymbol::RANGE_SYMBOL_LESS_THAN_1].first));
        ui->MarkLowertHgEdit_1->setText(QString::fromStdString(data.vecHuangdanRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_1].first));
        ui->MarkLowertHgEdit_2->setText(QString::fromStdString(data.vecHuangdanRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_2].first));
        ui->MarkLowertHgEdit_3->setText(QString::fromStdString(data.vecHuangdanRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_3].first));
        ui->MarkLowertHgEdit_4->setText(QString::fromStdString(data.vecHuangdanRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_4].first));
        ui->MarkLowertHgEdit_5->setText(QString::fromStdString(data.vecHuangdanRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_5].first));

        ui->MarkUppertHgEdit->setText(QString::fromStdString(data.vecHuangdanRange[tf::RangeSymbol::RANGE_SYMBOL_LESS_THAN_1].second));
        ui->MarkUppertHgEdit_1->setText(QString::fromStdString(data.vecHuangdanRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_1].second));
        ui->MarkUppertHgEdit_2->setText(QString::fromStdString(data.vecHuangdanRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_2].second));
        ui->MarkUppertHgEdit_3->setText(QString::fromStdString(data.vecHuangdanRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_3].second));
        ui->MarkUppertHgEdit_4->setText(QString::fromStdString(data.vecHuangdanRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_4].second));
        ui->MarkUppertHgEdit_5->setText(QString::fromStdString(data.vecHuangdanRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_5].second));
    }

    // 脂血范围
    if (data.vecZhixueRange.size() >= ciRangeFlagSize)
    {
        ui->MarkLowertZxEdit->setText(QString::fromStdString(data.vecZhixueRange[tf::RangeSymbol::RANGE_SYMBOL_LESS_THAN_1].first));
        ui->MarkLowertZxEdit_1->setText(QString::fromStdString(data.vecZhixueRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_1].first));
        ui->MarkLowertZxEdit_2->setText(QString::fromStdString(data.vecZhixueRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_2].first));
        ui->MarkLowertZxEdit_3->setText(QString::fromStdString(data.vecZhixueRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_3].first));
        ui->MarkLowertZxEdit_4->setText(QString::fromStdString(data.vecZhixueRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_4].first));
        ui->MarkLowertZxEdit_5->setText(QString::fromStdString(data.vecZhixueRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_5].first));

        ui->MarkUppertZxEdit->setText(QString::fromStdString(data.vecZhixueRange[tf::RangeSymbol::RANGE_SYMBOL_LESS_THAN_1].second));
        ui->MarkUppertZxEdit_1->setText(QString::fromStdString(data.vecZhixueRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_1].second));
        ui->MarkUppertZxEdit_2->setText(QString::fromStdString(data.vecZhixueRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_2].second));
        ui->MarkUppertZxEdit_3->setText(QString::fromStdString(data.vecZhixueRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_3].second));
        ui->MarkUppertZxEdit_4->setText(QString::fromStdString(data.vecZhixueRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_4].second));
        ui->MarkUppertZxEdit_5->setText(QString::fromStdString(data.vecZhixueRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_5].second));
    }

    // 溶血范围
    if (data.vecRongxueRange.size() >= ciRangeFlagSize)
    {
        ui->MarkLowertRxEdit->setText(QString::fromStdString(data.vecRongxueRange[tf::RangeSymbol::RANGE_SYMBOL_LESS_THAN_1].first));
        ui->MarkLowertRxEdit_1->setText(QString::fromStdString(data.vecRongxueRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_1].first));
        ui->MarkLowertRxEdit_2->setText(QString::fromStdString(data.vecRongxueRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_2].first));
        ui->MarkLowertRxEdit_3->setText(QString::fromStdString(data.vecRongxueRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_3].first));
        ui->MarkLowertRxEdit_4->setText(QString::fromStdString(data.vecRongxueRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_4].first));
        ui->MarkLowertRxEdit_5->setText(QString::fromStdString(data.vecRongxueRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_5].first));

        ui->MarkUppertRxEdit->setText(QString::fromStdString(data.vecRongxueRange[tf::RangeSymbol::RANGE_SYMBOL_LESS_THAN_1].second));
        ui->MarkUppertRxEdit_1->setText(QString::fromStdString(data.vecRongxueRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_1].second));
        ui->MarkUppertRxEdit_2->setText(QString::fromStdString(data.vecRongxueRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_2].second));
        ui->MarkUppertRxEdit_3->setText(QString::fromStdString(data.vecRongxueRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_3].second));
        ui->MarkUppertRxEdit_4->setText(QString::fromStdString(data.vecRongxueRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_4].second));
        ui->MarkUppertRxEdit_5->setText(QString::fromStdString(data.vecRongxueRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_5].second));
    }

    // 提示设置
    ui->XueQingNtcCmpCb->setCurrentIndex(ui->XueQingNtcCmpCb->findData(data.stuNoticeCfg.pairXueqingliang.first));
    ui->XueQingNtcRangeEdit->setText(QString::fromStdString(data.stuNoticeCfg.pairXueqingliang.second));
    ui->ZhiXueNtcCmpCb->setCurrentIndex(ui->ZhiXueNtcCmpCb->findData(data.stuNoticeCfg.pairZhixue.first));
    ui->ZhiXueNtcRangeCb->setCurrentText(QString::fromStdString(data.stuNoticeCfg.pairZhixue.second));
    ui->RongXueNtcCmpCb->setCurrentIndex(ui->RongXueNtcCmpCb->findData(data.stuNoticeCfg.pairRongxue.first));
    ui->RongXueNtcRangeCb->setCurrentText(QString::fromStdString(data.stuNoticeCfg.pairRongxue.second));
    ui->HuangdanNtcCmpCb->setCurrentIndex(ui->HuangdanNtcCmpCb->findData(data.stuNoticeCfg.pairHuangdan.first));
    ui->HuangdanNtcRangeCb->setCurrentText(QString::fromStdString(data.stuNoticeCfg.pairHuangdan.second));

    ui->XueqingAbnCmpCb->setCurrentIndex(ui->XueqingAbnCmpCb->findData(data.stuAlarmCfg.pairXueqingliang.first));
    ui->XueqingAbnRangeEdit->setText(QString::fromStdString(data.stuAlarmCfg.pairXueqingliang.second));
    ui->ZhixueAbnCmpCb->setCurrentIndex(ui->ZhixueAbnCmpCb->findData(data.stuAlarmCfg.pairZhixue.first));
    ui->ZhixueAbnRangeCb->setCurrentText(QString::fromStdString(data.stuAlarmCfg.pairZhixue.second));
    ui->RongxueAbnCmpCb->setCurrentIndex(ui->RongxueAbnCmpCb->findData(data.stuAlarmCfg.pairRongxue.first));
    ui->RongxueAbnRangeCb->setCurrentText(QString::fromStdString(data.stuAlarmCfg.pairRongxue.second));
    ui->HuangdanAbnCmpCb->setCurrentIndex(ui->HuangdanAbnCmpCb->findData(data.stuAlarmCfg.pairHuangdan.first));
    ui->HuangdanAbnRangeCb->setCurrentText(QString::fromStdString(data.stuAlarmCfg.pairHuangdan.second));
}

///
/// @brief
///     显示之后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2023年12月15日，新建函数
///
void AiAnalysisWgt::InitAfterShow()
{
    // 加载数据
    LoadData();
}

void AiAnalysisWgt::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (!m_bInit)
    {
        InitAfterShow();
        m_bInit = true;
    }
    else
    {
        // 加载数据
        LoadData();
    }
}

///
/// @bref
///        权限变化响应
///
/// @par History:
/// @li 8276/huchunli, 2023年9月25日，新建函数
///
void AiAnalysisWgt::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();
}

///
/// @brief
///     保存按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2023年12月15日，新建函数
///
void AiAnalysisWgt::OnSaveBtnClicked()
{
    ULOG(LOG_INFO, __FUNCTION__);
    // 构造保存的参数
    AiAnalysisCfg data;

    // 是否勾选
    data.bON = ui->AiSwitchCB->isChecked();
    data.bUploadImage = ui->TransCB->isChecked();
    data.bNoticeEnable = ui->NoticeCB->isChecked();
    data.bAlarmEnable = ui->AlarmCB->isChecked();

    // 标志大小
    int ciRangeFlagSize = 6;
    data.vecHuangdanRange.resize(ciRangeFlagSize);
    data.vecRongxueRange.resize(ciRangeFlagSize);
    data.vecZhixueRange.resize(ciRangeFlagSize);

    // 黄疸范围
    if (data.vecHuangdanRange.size() >= ciRangeFlagSize)
    {
        data.vecHuangdanRange[tf::RangeSymbol::RANGE_SYMBOL_LESS_THAN_1].first = ui->MarkLowertHgEdit->text().toStdString();
        data.vecHuangdanRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_1].first = ui->MarkLowertHgEdit_1->text().toStdString();
        data.vecHuangdanRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_2].first = ui->MarkLowertHgEdit_2->text().toStdString();
        data.vecHuangdanRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_3].first = ui->MarkLowertHgEdit_3->text().toStdString();
        data.vecHuangdanRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_4].first = ui->MarkLowertHgEdit_4->text().toStdString();
        data.vecHuangdanRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_5].first = ui->MarkLowertHgEdit_5->text().toStdString();

        data.vecHuangdanRange[tf::RangeSymbol::RANGE_SYMBOL_LESS_THAN_1].second = ui->MarkUppertHgEdit->text().toStdString();
        data.vecHuangdanRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_1].second = ui->MarkUppertHgEdit_1->text().toStdString();
        data.vecHuangdanRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_2].second = ui->MarkUppertHgEdit_2->text().toStdString();
        data.vecHuangdanRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_3].second = ui->MarkUppertHgEdit_3->text().toStdString();
        data.vecHuangdanRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_4].second = ui->MarkUppertHgEdit_4->text().toStdString();
        data.vecHuangdanRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_5].second = ui->MarkUppertHgEdit_5->text().toStdString();
    }

    // 脂血范围
    if (data.vecZhixueRange.size() >= ciRangeFlagSize)
    {
        data.vecZhixueRange[tf::RangeSymbol::RANGE_SYMBOL_LESS_THAN_1].first = ui->MarkLowertZxEdit->text().toStdString();
        data.vecZhixueRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_1].first = ui->MarkLowertZxEdit_1->text().toStdString();
        data.vecZhixueRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_2].first = ui->MarkLowertZxEdit_2->text().toStdString();
        data.vecZhixueRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_3].first = ui->MarkLowertZxEdit_3->text().toStdString();
        data.vecZhixueRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_4].first = ui->MarkLowertZxEdit_4->text().toStdString();
        data.vecZhixueRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_5].first = ui->MarkLowertZxEdit_5->text().toStdString();

        data.vecZhixueRange[tf::RangeSymbol::RANGE_SYMBOL_LESS_THAN_1].second = ui->MarkUppertZxEdit->text().toStdString();
        data.vecZhixueRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_1].second = ui->MarkUppertZxEdit_1->text().toStdString();
        data.vecZhixueRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_2].second = ui->MarkUppertZxEdit_2->text().toStdString();
        data.vecZhixueRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_3].second = ui->MarkUppertZxEdit_3->text().toStdString();
        data.vecZhixueRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_4].second = ui->MarkUppertZxEdit_4->text().toStdString();
        data.vecZhixueRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_5].second = ui->MarkUppertZxEdit_5->text().toStdString();
    }

    // 溶血范围
    if (data.vecRongxueRange.size() >= ciRangeFlagSize)
    {
        data.vecRongxueRange[tf::RangeSymbol::RANGE_SYMBOL_LESS_THAN_1].first = ui->MarkLowertRxEdit->text().toStdString();
        data.vecRongxueRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_1].first = ui->MarkLowertRxEdit_1->text().toStdString();
        data.vecRongxueRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_2].first = ui->MarkLowertRxEdit_2->text().toStdString();
        data.vecRongxueRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_3].first = ui->MarkLowertRxEdit_3->text().toStdString();
        data.vecRongxueRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_4].first = ui->MarkLowertRxEdit_4->text().toStdString();
        data.vecRongxueRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_5].first = ui->MarkLowertRxEdit_5->text().toStdString();

        data.vecRongxueRange[tf::RangeSymbol::RANGE_SYMBOL_LESS_THAN_1].second = ui->MarkUppertRxEdit->text().toStdString();
        data.vecRongxueRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_1].second = ui->MarkUppertRxEdit_1->text().toStdString();
        data.vecRongxueRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_2].second = ui->MarkUppertRxEdit_2->text().toStdString();
        data.vecRongxueRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_3].second = ui->MarkUppertRxEdit_3->text().toStdString();
        data.vecRongxueRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_4].second = ui->MarkUppertRxEdit_4->text().toStdString();
        data.vecRongxueRange[tf::RangeSymbol::RANGE_SYMBOL_MORE_THAN_5].second = ui->MarkUppertRxEdit_5->text().toStdString();
    }

    // 提示设置
    data.stuNoticeCfg.pairXueqingliang.first = ui->XueQingNtcCmpCb->currentData().toInt();
    data.stuNoticeCfg.pairXueqingliang.second = ui->XueQingNtcRangeEdit->text().toStdString();
    data.stuNoticeCfg.pairZhixue.first = ui->ZhiXueNtcCmpCb->currentData().toInt();
    data.stuNoticeCfg.pairZhixue.second = ui->ZhiXueNtcRangeCb->currentText().toStdString();
    data.stuNoticeCfg.pairRongxue.first = ui->RongXueNtcCmpCb->currentData().toInt();
    data.stuNoticeCfg.pairRongxue.second = ui->RongXueNtcRangeCb->currentText().toStdString();
    data.stuNoticeCfg.pairHuangdan.first = ui->HuangdanNtcCmpCb->currentData().toInt();
    data.stuNoticeCfg.pairHuangdan.second = ui->HuangdanNtcRangeCb->currentText().toStdString();

    data.stuAlarmCfg.pairXueqingliang.first = ui->XueqingAbnCmpCb->currentData().toInt();
    data.stuAlarmCfg.pairXueqingliang.second = ui->XueqingAbnRangeEdit->text().toStdString();
    data.stuAlarmCfg.pairZhixue.first = ui->ZhixueAbnCmpCb->currentData().toInt();
    data.stuAlarmCfg.pairZhixue.second = ui->ZhixueAbnRangeCb->currentText().toStdString();
    data.stuAlarmCfg.pairRongxue.first = ui->RongxueAbnCmpCb->currentData().toInt();
    data.stuAlarmCfg.pairRongxue.second = ui->RongxueAbnRangeCb->currentText().toStdString();
    data.stuAlarmCfg.pairHuangdan.first = ui->HuangdanAbnCmpCb->currentData().toInt();
    data.stuAlarmCfg.pairHuangdan.second = ui->HuangdanAbnRangeCb->currentText().toStdString();

    DictionaryQueryManager::SaveAiAnalysisConfig(data);

    // 发送数据更新消息
    //POST_MESSAGE(MSG_ID_EXPORT_SET_UPDATE, savedata);
}
