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
/// @file     AddBackupUnitDlg.h
/// @brief    添加备选单位
///
/// @author   7951/LuoXin
/// @date     2023年11月22日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2023年11月22日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <cfloat>
#include "AddBackupUnitDlg.h"
#include "ui_AddBackupUnitDlg.h"
#include "src/common/Mlog/mlog.h"
#include "thrift/DcsControlProxy.h"
#include "shared/tipdlg.h"
#include "shared/uicommon.h"
#include "shared/msgiddef.h"
#include "shared/messagebus.h"
#include "shared/CommonInformationManager.h"

// 当发生错误时，弹出消息框后退出当前函数
#define OnErr(bErr, msg) \
if ((bErr))\
{\
	TipDlg(msg).exec();\
	return ;\
}

AddBackupUnitDlg::AddBackupUnitDlg(QWidget *parent /*= Q_NULLPTR*/)
    : BaseDlg(parent)
    , ui(new Ui::AddBackupUnitDlg)
{
    ui->setupUi(this);
    connect(ui->save_btn, &QPushButton::clicked, this, &AddBackupUnitDlg::OnSaveBtnClicked);
}

AddBackupUnitDlg::~AddBackupUnitDlg()
{

}

void AddBackupUnitDlg::ShowAddWidget(const QString& assayName)
{
    SetTitleName(tr("新增"));

    ui->unit_edit->clear();
    ui->factor_edit->clear();

    m_unit.clear();
    m_factor.clear();
    m_assayName = assayName;

    show();
}

void AddBackupUnitDlg::ShowModifyWidget(const QString& assayName, const QString& unit, const QString& factor)
{
    SetTitleName(tr("修改"));

    ui->unit_edit->setText(unit);
    ui->factor_edit->setText(factor);

    m_unit = unit;
    m_factor = factor;
    m_assayName = assayName;

    show();
}

void AddBackupUnitDlg::OnSaveBtnClicked()
{
    ULOG(LOG_INFO, __FUNCTION__);

    OnErr(ui->unit_edit->text().isEmpty(), tr("备选单位不能为空"));
    OnErr(ui->factor_edit->text().isEmpty(), tr("单位倍率不能为空"));

    // 检查单位倍率
    bool bOk = false;
    double factor = ui->factor_edit->text().toDouble(&bOk);
    QString Qfactor = QString::number(factor, 'f', 4);

    OnErr(!bOk || factor <= 0, tr("单位倍率填写错误！"));

    // 单位名字
    QString QunitName = ui->unit_edit->text();
    std::string unitName = QunitName.toStdString();
    if (QString::fromStdString(unitName) == m_unit && Qfactor == m_factor)
    {
        return;
    }

    // 查询通用项目信息
    const auto& CIM = CommonInformationManager::GetInstance();
    auto spAssayInfo = CIM->GetAssayInfo(m_assayName.toStdString());
    if (spAssayInfo == nullptr)
    {
        ULOG(LOG_ERROR, "GetAssayInfo By AssayName Failed! ");
        return;
    }

	::tf::AssayUnit unitAdd, unitModBefore, unitModAfter;
    std::vector<tf::AssayUnit> vecUnits = spAssayInfo->units;
    if (m_unit.isEmpty())
    {
        // 检查此单位是否已存在
        auto iterUnit = std::find_if(vecUnits.begin(), vecUnits.end(), [&](auto& unit)->bool { return (unit.name == unitName); });
        OnErr(iterUnit != vecUnits.end(), tr("此单位已存在！"));

        if (TipDlg(tr("是否确认新增该备用单位?"), TipDlgType::TWO_BUTTON).exec() == QDialog::Rejected)
        {
            ULOG(LOG_INFO, "Cancel add backup unit!");
            return;
        }
		unitAdd.name = unitName;	// 备选单位      
		unitAdd.factor = factor;	// 单位倍率   
		unitAdd.isMain = false;		// 是否为主单位       
		unitAdd.isCurrent = false;	// 是否为当前单位    
		unitAdd.isUserDefine = true; // 是否为用户定义
        vecUnits.push_back(unitAdd);
    }
    else
    {
        // 修改了单位名称
        if (m_unit != QunitName)
        {
            // 检查此单位是否已存在
            auto iterUnit = std::find_if(vecUnits.begin(), vecUnits.end(), [&](auto& unit)->bool { return (unit.name == unitName); });
            OnErr(iterUnit != vecUnits.end(), tr("此单位已存在！"));
        }

        // 修改备选单位
        for (int i = 0; i < vecUnits.size(); i++)
        {            
            if (vecUnits[i].name == m_unit.toStdString()) // 使用修改前的单位名（m_unit）去匹配判断是否是当前单位
            {
                OnErr(vecUnits[i].isCurrent, tr("当前使用单位不允许修改！"));
				unitModBefore = vecUnits[i];
                vecUnits[i].name = unitName; // 把新单位名赋值保存
                vecUnits[i].factor = factor;
				unitModAfter = vecUnits[i];
                break;
            }
        }
       
        if (TipDlg(tr("是否确认修改该备用单位?"), TipDlgType::TWO_BUTTON).exec() == QDialog::Rejected)
        {
            ULOG(LOG_INFO, "Cancel modify backup unit!");
            return;
        }
    }

    // 修改项目参数
    auto oldUnit = spAssayInfo->units;
    spAssayInfo->__set_units(std::move(vecUnits));
    if (!DcsControlProxy::GetInstance()->ModifyGeneralAssayInfo(*spAssayInfo))
    {
        ULOG(LOG_ERROR, "%s : ModifyGeneralAssayInfo Failed", __FUNCTION__);
        return;
    }
	// 0025140: [应用] 应用-日志-操作日志界面修改与新增项目备选单位的操作记录描述格式与其他描述格式不统一 Modify by Chenjianlin 20240318
	if (oldUnit.size() == spAssayInfo->units.size())
    {
		QString temStr(tr("修改项目") + m_assayName + tr("的备选单位") + QString::fromStdString(unitModBefore.name));
		// 如果修改了名称
		if (unitModBefore.name != unitModAfter.name)
		{
			temStr += tr("，修改后备选单位:") + QString::fromStdString(unitModAfter.name);
		}
		// 如果修改了倍率
		if (fabs(unitModBefore.factor - unitModAfter.factor) > FLT_EPSILON)
		{
			temStr += tr("，修改后单位倍率:") + QString::number(unitModAfter.factor, 'f', spAssayInfo->decimalPlace);
		}
        if (!AddOptLog(::tf::OperationType::MOD, temStr))
        {
            ULOG(LOG_ERROR, "Add modify backup unit operate log failed! ");
        }
        emit ModifyComplete(QunitName, Qfactor);
    }
    else
    {
		QString temStr(tr("新增项目") + m_assayName + tr("的备选单位:") + QunitName + tr(" 单位倍率:") + QString::number(unitAdd.factor, 'f', spAssayInfo->decimalPlace));
        if (!AddOptLog(::tf::OperationType::ADD, temStr))
        {
            ULOG(LOG_ERROR, "Add addNew backup unit operate log failed! ");
        }
        emit AddComplete(QunitName, Qfactor);
    }

    // 通知项目更新
    CIM->UpdateAssayCfgInfo({ spAssayInfo->assayCode });

    close();
}
