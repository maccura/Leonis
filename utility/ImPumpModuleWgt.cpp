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
/// @file     ImPumpModuleWgt.cpp
/// @brief    应用-节点调试-生化泵模块   
///
/// @author   4170/TangChuXian
/// @date     2024年3月25日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月25日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "ImPumpModuleWgt.h"
#include "ui_ImPumpModuleWgt.h" 
#include "src/common/Mlog/mlog.h"
#include "shared/tipdlg.h"
#include "shared/msgiddef.h"
#include "shared/messagebus.h"

ImPumpModuleWgt::ImPumpModuleWgt(QWidget *parent)
    : QWidget(parent)
    , m_ciToothedGearCmdCode(0xf1)
    , ui(new Ui::ImPumpModuleWgt)
{
    ui->setupUi(this);

	InitData();
    InitUi();
	InitConnect();
}

ImPumpModuleWgt::~ImPumpModuleWgt()
{
}

void ImPumpModuleWgt::UpdateUi(const im::tf::DebugModule& stuTfDbgMd)
{
    // 记录调试模块信息
    m_stiTfDbgModule = stuTfDbgMd;

    // 获取所有复选框
    QList<QCheckBox*> allCbList = this->findChildren<QCheckBox*>();

    auto LoadDbgPart = [&allCbList, this](const im::tf::DebugPart& stuPart)
    {
        auto it = std::find_if(allCbList.begin(), allCbList.end(), [&stuPart](QCheckBox* pCB) {return pCB->text() == QString::fromStdString(stuPart.Comment); });
        if (it == allCbList.end())
        {
            return false;
        }

        (*it)->setChecked(stuPart.switchOn > 0);
        m_mapCBdata.insert(*it, stuPart);

        // 齿轮泵加载参数
        if (m_ciToothedGearCmdCode == stuPart.GetCmdCode)
        {
            ui->CurWaterPressureSp->setValue(stuPart.tempParam);
        }

        return true;
    };

    // 遍历所有部件
    m_mapCBdata.clear();
    for (const auto& stuUnit : stuTfDbgMd.DebugUnits)
    {
        for (const auto& stuPart : stuUnit.DebugParts)
        {
            LoadDbgPart(stuPart);
        }
    }
}

void ImPumpModuleWgt::InitUi()
{
	
}

void ImPumpModuleWgt::InitData()
{

}

void ImPumpModuleWgt::InitConnect()
{
    // 获取所有复选框
    QList<QCheckBox*> allCbList = this->findChildren<QCheckBox*>();

    // 逐个连接
    for (QCheckBox* pCheckCB : allCbList)
    {
        connect(pCheckCB, SIGNAL(clicked()), this, SLOT(OnCheckBoxClicked()));
    }
}

///
/// @brief
///     复选框被点击
///
/// @par History:
/// @li 4170/TangChuXian，2024年4月9日，新建函数
///
void ImPumpModuleWgt::OnCheckBoxClicked()
{
    // 获取信号发送者
    QCheckBox* pCheckBox = qobject_cast<QCheckBox*>(sender());
    if (pCheckBox == Q_NULLPTR)
    {
        return;
    }

    // 找到对应的数据
    auto it = m_mapCBdata.find(pCheckBox);
    if (it == m_mapCBdata.end())
    {
        return;
    }

    // 更新数据
    it.value().__set_switchOn(int(pCheckBox->isChecked()));

    // 调用接口
    // 获取模块
    ::tf::ResultLong retl;
    if (!im::LogicControlProxy::Switch(retl, m_strDevSn, pCheckBox->isChecked(), it.value()) || retl.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "%s(), ExcuteDebugUnit failed!", __FUNCTION__);
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("执行失败!")));
        pTipDlg->exec();
        pCheckBox->setChecked(!pCheckBox->isChecked());
        return;
    }
    if (retl.value > 0)
    {
        ui->CurWaterPressureSp->setValue(retl.value);
        it.value().__set_tempParam(retl.value);
    }

    // 更新调试模块信息
    for (auto& stuUnit : m_stiTfDbgModule.DebugUnits)
    {
        for (auto& stuPart : stuUnit.DebugParts)
        {
            // 不匹配则跳过
            if (stuPart.GetCmdCode != it.value().GetCmdCode)
            {
                continue;
            }

            // 更新部件信息
            stuPart = it.value();
            break;
        }
    }

    // 发送通知消息
    POST_MESSAGE(MSG_ID_IM_NODE_DBG_MODULE_UPDATE, m_stiTfDbgModule);
}
