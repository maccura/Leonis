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
/// @file     requirecfgdlg.cpp
/// @brief    需求设置对话框
///
/// @author   4170/TangChuXian
/// @date     2023年6月26日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2023年6月27日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "requirecfgdlg.h"
#include "ui_requirecfgdlg.h"
#include "requirecalcwidget.h"

#include "shared/uicommon.h"
#include "shared/uidef.h"
#include "shared/tipdlg.h"
#include "src/common/Mlog/mlog.h"
#include "manager/DictionaryQueryManager.h"

RequireCfgDlg::RequireCfgDlg(QWidget *parent)
    : BaseDlg(parent),
      m_bInit(false)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 初始化ui对象
    ui = new Ui::RequireCfgDlg();
    ui->setupUi(this);

    // 界面显示前初始化
    InitBeforeShow();
}

RequireCfgDlg::~RequireCfgDlg()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    delete ui;
}

///
/// @brief
///     界面显示前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2021年6月22日，新建函数
///
void RequireCfgDlg::InitBeforeShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 设置标题
    SetTitleName(tr("需求设置"));

    // 添加动态计算周期单位（周、月、年）
    AddTfEnumItemToComBoBox(ui->DynamicCircleUnitCombo, tf::TimeUnit::TIME_UNIT_WEEK);
    AddTfEnumItemToComBoBox(ui->DynamicCircleUnitCombo, tf::TimeUnit::TIME_UNIT_MONTH);
    AddTfEnumItemToComBoBox(ui->DynamicCircleUnitCombo, tf::TimeUnit::TIME_UNIT_YEAR);

    // 输入限制
    ui->DynamicCircleEdit->setValidator(new QRegExpValidator(QRegExp(UI_REG_REQ_CALC_CIRCLE)));
    ui->DynamicCalcValEdit->setValidator(new QRegExpValidator(QRegExp(UI_REG_REQ_CALC_ADJUST_PERCENT)));
}

///
/// @brief
///     界面显示后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2021年6月22日，新建函数
///
void RequireCfgDlg::InitAfterShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 初始化字符串资源
    InitStrResource();

    // 初始化信号槽连接
    InitConnect();

    // 初始化子控件
    InitChildCtrl();
}

///
/// @brief
///     初始化字符串资源
///
/// @par History:
/// @li 4170/TangChuXian，2022年1月17日，新建函数
///
void RequireCfgDlg::InitStrResource()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
}

///
/// @brief
///     初始化信号槽连接
///
/// @par History:
/// @li 4170/TangChuXian，2021年6月22日，新建函数
///
void RequireCfgDlg::InitConnect()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 确认按钮被点击
    connect(ui->OkBtn, SIGNAL(clicked()), this, SLOT(OnOkBtnClicked()));

    // 取消按钮被点击
    connect(ui->CancelBtn, SIGNAL(clicked()), this, SLOT(reject()));
}

///
/// @brief
///     初始化子控件
///
/// @par History:
/// @li 4170/TangChuXian，2021年6月22日，新建函数
///
void RequireCfgDlg::InitChildCtrl()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 解码
    RgntReqCalcCfg stuReqCalcCfg;
    if (!DictionaryQueryManager::GetDynCalcConfig(stuReqCalcCfg))
    {
        ULOG(LOG_WARN, "Failed to get dyncalc config.");
        return;
    }

    // 将读取到的配置信息加载到界面
    ui->DynamicCircleEdit->setText(QString::number(stuReqCalcCfg.iCircle));
    ui->DynamicCalcValEdit->setText(QString::number(stuReqCalcCfg.iAdjustPercent));
    ui->UsingDynamicCalcValCB->setChecked(stuReqCalcCfg.bEnable);
    ui->DynamicCircleUnitCombo->setCurrentIndex(ui->DynamicCircleUnitCombo->findData(stuReqCalcCfg.iCircleUnit));
}

///
/// @brief
///     窗口显示事件
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 4170/TangChuXian，2021年6月22日，新建函数
///
void RequireCfgDlg::showEvent(QShowEvent *event)
{
    // 让基类处理事件
    BaseDlg::showEvent(event);

    // 第一次显示时初始化
    if (!m_bInit)
    {
        m_bInit = true;
        InitAfterShow();
    }
}

///
/// @brief
///     确定按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月25日，新建函数
///
void RequireCfgDlg::OnOkBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 输入框检查，不能为空
    if (ui->DynamicCircleEdit->text().isEmpty())
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("动态计算周期不能为空，请重新设置！")));
        pTipDlg->exec();
        return;
    }

    // 百分比
    if (ui->DynamicCalcValEdit->text().isEmpty())
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("批量调整动态计算值不能为空，请重新设置！")));
        pTipDlg->exec();
        return;
    }

    // 获取要保存的信息
    RgntReqCalcCfg stuReqCalcCfg;
    stuReqCalcCfg.bEnable = ui->UsingDynamicCalcValCB->isChecked();
    stuReqCalcCfg.iCircleUnit = ui->DynamicCircleUnitCombo->currentData().toInt();
    stuReqCalcCfg.iCircle = ui->DynamicCircleEdit->text().toInt();
    stuReqCalcCfg.iAdjustPercent = ui->DynamicCalcValEdit->text().toInt();

    // 保存到字典
    if (!DictionaryQueryManager::SaveDynCalcConfig(stuReqCalcCfg))
    {
        ULOG(LOG_ERROR, "Failed to save dyncalc config.");
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("保存失败"), tr("保存失败！")));
        pTipDlg->exec();
        return;
    }

    // 关闭对话框
    accept();
}
